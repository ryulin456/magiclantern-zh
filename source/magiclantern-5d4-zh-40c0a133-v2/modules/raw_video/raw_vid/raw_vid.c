#include "module.h"
#include "dryos.h"

#include "raw_vid.h"
#include "raw_vid_event_pusher.h"
#include "raw_vid_worker.h"

static struct msg_queue *event_q = NULL;

static unsigned int init()
{
#ifdef RAW_VID_LOG
    static uint8_t *log_buf = NULL;
    log_buf = malloc(MIN_LOG_BUF_SIZE);
    char log_name[20] = {0};
    if (get_numbered_file_name("raw_v%d.log", 99, log_name, sizeof(log_name)) > -1)
        init_log(log_buf, MIN_LOG_BUF_SIZE, log_name);
#endif

    event_q = msg_queue_create("raw_vid_q", RAW_VID_Q_LEN);

    if (event_q == NULL)
        return -1;

    init_event_pusher(event_q);
    init_worker(event_q);

    return 0;
}

static unsigned int deinit()
{
    return 0;
}

MODULE_INFO_START()
    MODULE_INIT(init)
    MODULE_DEINIT(deinit)
MODULE_INFO_END()

MODULE_CONFIGS_START()
//    MODULE_CONFIG(raw_vid_some_module_global)
MODULE_CONFIGS_END()
