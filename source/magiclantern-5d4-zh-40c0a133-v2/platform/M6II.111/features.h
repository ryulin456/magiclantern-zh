#define FEATURE_VRAM_RGBA

#define CONFIG_COMPOSITOR_XCM
#define CONFIG_COMPOSITOR_DEDICATED_LAYER
#define CONFIG_COMPOSITOR_XCM_V2
// Don't Click Me menu looks to be intended as a place
// for devs to put custom code in debug.c run_test(),
// and allowing triggering from a menu context.
#define FEATURE_DONT_CLICK_ME

#define FEATURE_SHOW_SHUTTER_COUNT

// working but incomplete, some allocators don't report
// anything yet as they're faked / not yet found
#define FEATURE_SHOW_FREE_MEMORY

#define CONFIG_TSKMON
#define FEATURE_SHOW_TASKS
#define FEATURE_SHOW_CPU_USAGE
#define FEATURE_SHOW_GUI_EVENTS

//#define CONFIG_ADDITIONAL_VERSION
#define FEATURE_SCREENSHOT

#define CONFIG_STATE_OBJECT_HOOKS
#define CONFIG_LIVEVIEW
#define FEATURE_POWERSAVE_LIVEVIEW

// enable global draw
#define FEATURE_GLOBAL_DRAW
#define FEATURE_CROPMARKS

// Enable remapping ROM pages to RAM
#define CONFIG_SGI_HANDLERS
#define CONFIG_MMU_REMAP

// enable for testing gui structure changes
//#define CONFIG_RESTORE_AFTER_FORMAT

// We can't yet rely on image capture.  Cam crashes due to null pointer,
// I think?  If it fails to AF lock, for example.
#define CONFIG_IMAGE_CAPTURE_NOT_WORKING

#define FEATURE_PICSTYLE
#define CONFIG_PROP_REQUEST_CHANGE

// Fast disk logging
#define FEATURE_DISK_LOG

// This should be fine on all D8
#define CONFIG_CRASH_LOG

#undef CONFIG_AUTOBACKUP_ROM
#undef CONFIG_ADDITIONAL_VERSION
