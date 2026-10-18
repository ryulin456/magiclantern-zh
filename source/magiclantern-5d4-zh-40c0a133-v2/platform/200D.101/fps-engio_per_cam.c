#include "dryos.h"
#include "fps-engio_per_cam.h"

int get_fps_register_a(void)
{
    extern int _get_fps_register_a(void);
    return _get_fps_register_a(); // This is wrong, probably some shutter related func.
                                  // String usage at e032b1d6 is puzzling though.
//    return shamem_read(FPS_REGISTER_A); // always reads 0
//    return *(int *)(FPS_REGISTER_A); // always reads 0

// candidates in sensor struct:
// 1c790 // prob not, seen 64 and 520
// 1c794 // no, seen it as 0 or 1
// 1c798 // no, only seen 0
// 1c7cc // "PreAccumH" // it's something related, seen 1121 and 373,
                        // but not AccumH (not surprising give the name)
// obvious candidates in this addr range exhausted

// weirder candidates
// 1a392 via e00b2aea // always 0
// 91e76 // changes, sensible ish numbers, but not the right ones

}

int get_fps_register_a_default(void)
{
// TODO - given the changes in reading Reg A, does this still work?
// Test if we can read anything from reg or shamem reg.
// No way found so far...
//    return shamem_read(FPS_REGISTER_A + 4);

    // Don't know a way of reading the value from cam yet,
    // may not be possible?
    // For now, we return a valid value logged from real cam.
    // I *think* the shift is because these are u16s really,
    // but this one is not at a word boundary, and old cams
    // couldn't easily read from unaligned values.
    // So we read the wrong address and shift it, every time
    // we need it...  awesome.  Hence, here, we shift
    // in order to correct for that later correction.  Yuck.
    return 1122 << 16;
}

int get_fps_register_b(void)
{
    extern int _get_fps_register_b(void);
    return _get_fps_register_b();

//    return shamem_read(FPS_REGISTER_B); // this always reads 0
}
