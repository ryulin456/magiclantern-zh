#include "module.h"
#include "dryos.h"
#include "raw.h" // raw_info global, raw_lv_redirect_edmac()
#include "zebra.h" // liveview_display_idle()

// Ugly - we use propvalues.c to hold various globals
// and functions that are nothing to do with properties,
// just global state for various camera things,
// including raw video stuff.
// E.g. is_movie_mode(), set_recording_custom(),
// various RECORDING macros.
#include "propvalues.h"

#include "raw_vid.h"
#include "raw_vid_event_pusher.h"
#include "raw_vid_worker.h"

#if RAW_VID_Q_LEN < 5
    #error "RAW_VID_Q_LEN too short"
#endif

// This is about 56MB/s at 24fps
//#define MLV_3_CROP_WIDTH 1600
//#define MLV_3_CROP_HEIGHT 900

// This is 64.3MB/s at 24fps
#define MLV_3_CROP_WIDTH 1792
#define MLV_3_CROP_HEIGHT 896

// This is 67.8MB/s at 24fps
//#define MLV_3_CROP_WIDTH 1888
//#define MLV_3_CROP_HEIGHT 896

// This is 70.2MB/s at 24fps
//#define MLV_3_CROP_WIDTH 2048
//#define MLV_3_CROP_HEIGHT 856

// This is 73.5MB/s at 24fps
// (and nice round numbers)
//#define MLV_3_CROP_WIDTH 2048
//#define MLV_3_CROP_HEIGHT 896

static struct msg_queue *event_q = NULL;

// We post these events to the msg queue.  Since we push a pointer,
// we need some storage that will live until the msg has been consumed.
//
// Don't access these events directly, only use get_event() / release_event().
// This handles the sem to avoid races.
// Only the worker receives or releases events; single consumer reduces code complexity.
static struct raw_vid_event events[RAW_VID_Q_LEN] = {0};
static struct semaphore *events_sem = NULL;

// Local var to rapidly stop pushing image data to worker.
// Should not be accessed directly.  send_stop_command() uses this.
//
// Should not need a sem, since this side only ever changes it 0 -> 1,
// only worker changes 1 -> 0, via allow_recording().  And worker
// flushes any queued events before making that change.
static int prevent_recording = 1;

static void send_stop_command(void); // forward declaration

// Returns a raw_vid_event in the PREPARING state,
// or returns NULL if all events are in use.
// That should only happen if the worker cannot service our requests fast enough,
// probably because we can't save to card fast enough.
//
// We use a static array of events, and .type == EMPTY to signify
// if the event can be re-used.  Therefore, caller must never set type
// of the obtained event to EMPTY, as this means it can be reassigned
// in a racey manner.
//
// Caller can modify other parts of the event before posting.
// Caller must never modify any part of an event after posting;
// the receiver is responsible for releasing the event
// (which it will do with release_event()).
struct raw_vid_event *get_empty_event(void)
{
    static size_t last_index = 0;
    struct raw_vid_event *event = NULL;

    int err = take_semaphore(events_sem, 2000);
    if (err)
    {
        SEND_LOG_DATA_STR("ERROR: get_next_event() sem timeout\n");
        task_create("stop_com", WORKER_PRIO + 1, 0x800, send_stop_command, NULL);
        return NULL;
    }

    // This is an optimisation; we test the event after the last one
    // we assigned.  Most of the time usage should be sequential.
    if (last_index < RAW_VID_Q_LEN - 1)
        last_index++;
    else
        last_index = 0;

    if (events[last_index].type == EMPTY)
    {
        event = &events[last_index];
    }
    else
    {
        // simple linear search, it's a small contiguous array
        int i = 0;
        while (i < RAW_VID_Q_LEN)
        {
            if (events[i].type == EMPTY)
            {
                event = &events[i];
                break;
            }
            i++;
        }
    }

    if (event != NULL)
    {
        memset(event, 0, sizeof(*event));
        event->type = PREPARING;
    }

    give_semaphore(events_sem);
    return event;
}

// Allows the provided event to be re-used.
// After calling this, the caller must not use
// the event again.
void release_event(struct raw_vid_event *event)
{
    // Here we only zero the type.  This allows get_event() to re-use the event,
    // and that func zeroes the complete event.
    // This removes a source of race conditions due to ARM weak memory model.
    event->type = EMPTY;
}

// This should only be called by worker.c,
// when it wishes to allow this file to start a new recording.
void allow_recording(void)
{
    prevent_recording = 0;
}

void init_event_pusher(struct msg_queue *q)
{
    // Not much init, most of the work is done by the CBR funcs.
    event_q = q;
    events_sem = create_named_semaphore(NULL, SEM_CREATE_UNLOCKED);
}

// Send stop command, handling the possiblity that the queue
// may be full.
//
// Also sets the flag to immediately disallow sending image data
// from this file to the queue.  This allows the queue to empty,
// should it be full.
//
// Probably works if called directly, but it's expected you'd
// use task_create().  That allows this function to be slow as it
// attempts retries, etc, without blocking whatever context you
// trigger it from.  We need to handle problems encountered in
// timing critical CBR contexts.
static void send_stop_command(void)
{
    prevent_recording = 1;
    SEND_LOG_DATA_STR("Sending stop command to worker\n");

    const int max_tries = 10;
    int tries = 0;
    struct raw_vid_event *event = get_empty_event();
    while (event == NULL && tries < max_tries)
    { // should happen only if the queue is completely full,
      // we must wait and hope the worker can process some events.
        msleep(50); // more time than one frame at 24 fps,
                    // probably enough we only wait once.
        event = get_empty_event();
        tries++;
    }
    if (tries == max_tries)
    {
        SEND_LOG_DATA_STR("ERROR: could not get event for stop command\n");
        return;
    }

    event->creation_time = get_ms_clock();
    event->type = COMMAND_STOP;

    tries = 0;
    int err = msg_queue_post(event_q, (uint32_t)event);
    while (err && tries < max_tries)
    {
        msleep(50);
        // FIXME: use msg_queue_post_with_timeout()?
        err = msg_queue_post(event_q, (uint32_t)event);
        tries++;
    }
    if (tries == max_tries)
    {
        SEND_LOG_DATA_STR("ERROR: could not post stop command event\n");
    }
}

// Here we judge if the keypress should start or stop recording.
// We also suppress some events during recording.
//
// This is a slightly simplified copy of the logic in mlv_lite.c
//
// We must return 1 if we are not suppressing the event;
// that is, if we want to give other code a chance to handle the event.
static unsigned int raw_vid_keypress_cbr(unsigned int key)
{
    if (!is_movie_mode()) // in propvalues.c
        return 1;

    // keys are only hooked in LiveView
    if (!liveview_display_idle())
        return 1;

    // if you somehow managed to start recording H.264, let it stop
    if (RECORDING_H264)
        return 1;

    // Fetch recording state when rec key pressed.
    // Theoretically this can change before we use it,
    // but the window is small and the worst case outcome is
    // not a real problem; we either start or stop recording inappropriately.
    enum recording_state recording_state = get_recording_state();

    // block the zoom key while recording
    if (recording_state == INACTIVE
        && key == MODULE_KEY_PRESS_ZOOMIN)
        return 0;

    // start/stop recording with the LiveView key
    int rec_key_pressed = (key == MODULE_KEY_LV || key == MODULE_KEY_REC);

    if (rec_key_pressed)
    {
        SEND_LOG_DATA_STR("REC key pressed.\n");

        if (event_q == NULL)
        {
            // not initialized; block the event
            return 0;
        }

        if (recording_state == ACTIVE)
        {
            // We wish to stop recording.
            SEND_LOG_DATA_STR("Sending stop com due to rec keypress.\n");
            // task_create to avoid blocking in key handling cbr:
            task_create("stop_com", WORKER_PRIO + 1, 0x800, send_stop_command, NULL);
        }
        else if (recording_state == INACTIVE)
        {
            // Trying to start recording.
            struct raw_vid_event *event = get_empty_event();
            if (event != NULL)
            {
                event->creation_time = get_ms_clock();
                event->type = COMMAND_START;
                // FIXME we should take these from menu config when we have that
                event->w = MLV_3_CROP_WIDTH;
                event->h = MLV_3_CROP_HEIGHT;
                msg_queue_post(event_q, (uint32_t)event);
            }
            // If we were trying to start recording and there was an earlier
            // error, we won't try to send the start signal.  Which is okay.
        }

        // In other states, we ignore the button
        return 0;
    }

    return 1;
}

// src/raw.c makes this fire every LV vsync (which I guess is tied to every sensor capture?).
// In any case, it works to get 60fps captures on 200D.
static unsigned int FAST raw_vid_vsync_cbr(unsigned int unused)
{
    static int count = 0; // frame count, reset every recording session
    struct raw_vid_event *event = NULL;

    // The cam will capture sensor data for display on LV.
    // Without needing to modify *what* the cam captures (which is hard),
    // we can change *where* the data goes.  This happens to be easy
    // because of how EDMAC works (and which parts we understand well).
    //
    // We don't know precisely when the EDMAC capture will finish, only
    // that it is in the future.  But we do know it must finish before
    // the next vsync, or LV would hang.
    //
    // Therefore we double buffer capture, changing where cam saves the
    // data every vsync. That means we know the older captured frame has
    // completed capture.
    //
    // We message worker.c with the details of this captured frame.
    // It optionally crops out a region of interest, and saves to disk.

    static char *capture_frames[2] = {NULL};
    static char *frame = NULL; // current capture_frame in use

    if (get_recording_state() != ACTIVE)
        return 0;

    if (prevent_recording)
    {
        // Consumer thinks recording is active, but something on this,
        // the producer side, has requested stop.  This should mean the recording
        // has very recently stopped.  Reset session.
        //
        // Consumer will re-enable recording when it is ready.
        count = 0;
        if (capture_frames[0] != NULL)
        {
            fio_free(capture_frames[0]);
            capture_frames[0] = NULL;
        }
        if (capture_frames[1] != NULL)
        {
            fio_free(capture_frames[1]);
            capture_frames[1] = NULL;
        }
        return 0;
    }

    // In worker.c we request LV changes to raw mode, which takes some time.
    // When it occurs, raw_info global struct gets updated with the raw dimensions.
    if (raw_info.width == 0
        || raw_info.height == 0)
    {
        return 0;
    }

    count++;
    if (count == 1)
    { // per recording-session setup
        SEND_LOG_DATA_STR("Video starting.\n");

        // We lie a little to increase height, we want to slightly over-allocate.
        // Old code does this, not quite sure why, perhaps DMA block size related?
        const uint32_t lv_raw_size = raw_info.width * (raw_info.height + 2) * BPP/8;
        capture_frames[0] = fio_malloc(lv_raw_size);

        if (capture_frames[0] == NULL)
        {
            SEND_LOG_DATA_STR("Could not alloc capture_frames[0]\n");

            // This is likely unrecoverable, stop recording
            task_create("stop_com", WORKER_PRIO + 1, 0x800, send_stop_command, NULL);
            return 0;
        }
        capture_frames[1] = fio_malloc(lv_raw_size);
        if (capture_frames[1] == NULL)
        {
            SEND_LOG_DATA_STR("Could not alloc capture_frames[1]\n");
            fio_free(capture_frames[0]);
            capture_frames[0] = NULL;

            // This is likely unrecoverable, stop recording
            task_create("stop_com", WORKER_PRIO + 1, 0x800, send_stop_command, NULL);
            return 0;
        }

        frame = capture_frames[0];
    }

    event = get_empty_event();
    if (event == NULL)
    {
        // We have filled the queue.  This should mean the consumer is not keeping up.
        // We have to stop.

        // TODO classic MLV supports user choosing stop or skip frames,
        // logic for skipping frames would go in here, perhaps some loop with a sleep,
        // waiting for queue space to be available.
        SEND_LOG_DATA_STR("All events are non-empty, stopping\n");

        task_create("stop_com", WORKER_PRIO + 1, 0x800, send_stop_command, NULL);
        return 0;
    }

    event->creation_time = get_ms_clock();
    event->type = IMAGE_DATA;

    event->orig_frame_width = raw_info.pitch;

    // TODO allow choosing offset and crop?
    event->x = 200; // x offset within the frame, for cropping
    event->y = 100; // y as above
    event->w = MLV_3_CROP_WIDTH;
    event->h = MLV_3_CROP_HEIGHT;

    int32_t frame_size = event->w * event->h * BPP/8;
    event->size = frame_size;

    // Send prior captured data to worker for cropping
    // and writing
    if (frame != NULL)
        event->src = frame;

    // Swap buffer in use
    if (frame == capture_frames[0])
    {
        frame = capture_frames[1];
    }
    else
    {
        frame = capture_frames[0];
    }

    // Make the next LV EDMAC capture go to updated buffer
    if (frame != NULL)
        raw_lv_redirect_edmac(frame);

    // Don't send request on first vsync, we've set up for EDMAC capture
    // but it hasn't had time to run.
    if (count > 1)
    {
        int err = msg_queue_post(event_q, (uint32_t)event);
        if (err)
        {
            // queue was full, should mean card writes aren't keeping up,
            // we should stop.
            task_create("stop_com", WORKER_PRIO + 1, 0x800, send_stop_command, NULL);
        }
    }

//ASSERT(frame_size_uncompressed % 4 == 0);

    return 0;
}

MODULE_CBRS_START()
    MODULE_CBR(CBR_VSYNC, raw_vid_vsync_cbr, 0)
    MODULE_CBR(CBR_KEYPRESS, raw_vid_keypress_cbr, 0)
MODULE_CBRS_END()
