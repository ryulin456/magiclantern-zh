/* 5D4 development placeholder; no 5D4 Dual ISO implementation is available.
 * Retain module metadata without linking sensor-write code. */
#include <module.h>

static unsigned int dual_iso_init(void)
{
    return 1; /* Unsupported; do not activate this module. */
}

static unsigned int dual_iso_deinit(void)
{
    return 0;
}

MODULE_INFO_START()
    MODULE_INIT(dual_iso_init)
    MODULE_DEINIT(dual_iso_deinit)
MODULE_INFO_END()
