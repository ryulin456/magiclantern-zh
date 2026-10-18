#include "dryos.h"
#include "fps-engio_per_cam.h"

int get_fps_register_a(void)
{
    // Best I've worked out so far is read from the message
    // array populated with accumh and vsize values before they're
    // sent.  If different code uses different arrays, this
    // will be unreliable.
//    printf("00, 08, 14: %x, %x, %x\n",
//           *(uint32_t *)0x2e4a0,
//           *(uint32_t *)0x2e4b0,
//           *(uint32_t *)0x2e490);
    return *(uint32_t *)0x2e4b0;
}

int get_fps_register_a_default(void)
{
// TODO - given the changes in reading Reg A, does this still work?
// Test if we can read anything from reg or shamem reg.
// No way found so far...
//    return shamem_read(FPS_REGISTER_A + 4);

    // Don't know a way of reading the value from cam yet,
    // may not be possible?
    // For now, we return a valid value logged from 200D.
    // I *think* the shift is because these are u16s really,
    // but this one is not at a word boundary, and old cams
    // couldn't easily read from unaligned values.
    // So we read the wrong address and shift it, every time
    // we need it...  awesome.  Hence, here, we shift
    // in order to correct for that later correction.  Yuck.
    return 0x2ed << 16;
}

int get_fps_register_b(void)
{
    return *(uint32_t *)0x2e490;
}
