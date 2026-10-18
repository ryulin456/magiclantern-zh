#define FEATURE_VRAM_RGBA

// A blank features.h still has many features defined.
// Undef some broken or dangerous ones.

// prevent ML attempting stack unwinding in some cases.
// This does not yet work (assumes ARM, not Thumb).  Alex recommends
// a good looking fix:
// http://www.mcternan.me.uk/ArmStackUnwinding/
#define CONFIG_CRASH_LOG

#undef CONFIG_ADDITIONAL_VERSION
#undef CONFIG_PROP_REQUEST_CHANGE
#undef CONFIG_AUTOBACKUP_ROM
#undef FEATURE_SHOW_FREE_MEMORY /* submenu probes unverified ExMem pools */

#define FEATURE_SHOW_SHUTTER_COUNT

/* Screenshot allocates large RGB/YUV buffers and writes to the card.  Keep it
 * out of the first 5D4 developer package until memory pools and display
 * addresses are verified on hardware. */
#undef FEATURE_SCREENSHOT

// We can't yet rely on image capture.  Cam crashes due to null pointer,
// I think?  If it fails to AF lock, for example.
#define CONFIG_IMAGE_CAPTURE_NOT_WORKING
