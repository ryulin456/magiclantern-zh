#ifndef _log_h_
#define _log_h_

#include "dryos.h"

#if defined(FEATURE_DISK_LOG) || defined(MODULE)

// A fairly generic implementation of a circular buffer for logging,
// which is periodically flushed to disk.

#define MIN_LOG_BUF_SIZE 0x10000
#define MIN_LOG_WRITE_SIZE 0x1000

// user is responsible for providing buffer
// (allows use on different stages of ports, and different run time contexts)
int init_log(uint8_t *buf, uint32_t size, char *filename);

// Stops all logging until cam is restarted.
// Flushes log to disk.
void stop_log(void);

// Send some data to be written to disk.
// This is thread safe and blocking re copying
// the data into the central logging buffer.
// Writes to disk happen periodically.
int send_log_data(uint8_t *data, uint32_t size);

// Convenience function for null terminated strings
int send_log_data_str(char *s);

// Optional funcs to control logging.  It starts enabled.
// If you disable, send_log_data() will silently discard all data
// until you enable.
void enable_logging(void);
void disable_logging(void);

#endif // FEATURE_DISK_LOG

#endif // _log_h_
