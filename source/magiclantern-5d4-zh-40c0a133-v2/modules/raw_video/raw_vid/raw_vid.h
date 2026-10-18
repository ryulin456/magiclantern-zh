#ifndef _raw_vid_h_
#define _raw_vid_h_

#undef RAW_VID_LOG
//#define RAW_VID_LOG 1
#ifdef RAW_VID_LOG
#include "log.h"
#endif

// While we do send an event for every frame pushed,
// the queue receiving them doesn't need to be very long,
// because we must keep up, or we will drop frames.
#define RAW_VID_Q_LEN 20

#define BPP 14

enum event_type
{
    // Events with these types should never be posted to the queue.
    EMPTY = 0, // We zero the event to init or re-use it, so this value must be explicit.
    PREPARING, // Reserved, but otherwise empty; used to limit time spent in critical section.

    // Events with these types only need to set creation_time and type,
    // the other fields are ignored.
    COMMAND_START, // Producer / event_handler requests start.  Consumer / worker may reject the request.
    COMMAND_STOP,  // As above, but stop.

    // This event must set valid values for all fields.
    IMAGE_DATA, // Some image data to save to file.
};

struct raw_vid_event
{
    uint32_t creation_time;
    enum event_type type;
    char *src; // points to image data if capturing

    uint32_t orig_frame_width;
    uint32_t x; // x offset within the frame, for cropping
    uint32_t y; // y as above
    uint32_t w; // our crop width
    uint32_t h; // our crop height

    int32_t size; // size calculated by producer, in bytes
};

enum recording_state
{
    ERROR,
    // RECORDING // can't use this name because propvalues.h #defines it
    INACTIVE,
    ACTIVE
};

#endif // _raw_vid_h_
