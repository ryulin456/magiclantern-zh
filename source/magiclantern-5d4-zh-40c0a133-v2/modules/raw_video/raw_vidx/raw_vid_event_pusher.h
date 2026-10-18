#ifndef _raw_vid_pusher_h_
#define _raw_vid_pusher_h_

#include "dryos.h"

void init_event_pusher(struct msg_queue *event_q);
void release_event(struct raw_vid_event *event);
void allow_recording(void);

#endif // _raw_vid_pusher_h_
