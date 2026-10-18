#ifndef _raw_vid_worker_h_
#define _raw_vid_worker_h_

#include "dryos.h"

void init_worker(struct msg_queue *q);
enum recording_state get_recording_state(void);

// Priority for the main task, which performs writes
// of frame data to disk.  This is a compromise.  Lower values
// are higher priority.  Too high priority and the GUI becomes
// very laggy.  Too low and it limits effective write bandwidth.
// 0x11 maxes out 200D autotuned SD speed, without GUI lag.
// Untested on other cams.
#define WORKER_PRIO 0x11

// Used to get space for our local buffers.  This is the initial
// size attempted, we reduce from their until success.
// Pick a nice round value; this is the aligned size, not underlying size.
#define INITIAL_CROP_BUF_SIZE (16 * 1024 * 1024)
#define CROP_BUF_ALIGN 0x1000
#define MIN_CROP_BUF_SIZE (4 * 1024 * 1024)
#define CROP_BUF_STEP_SIZE (1 * 1024 * 1024)

#endif // _raw_vid_worker_h_
