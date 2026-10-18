// Some ROMs have code to automatically determine
// a higher speed for SD cards.
//
// First noticed on 200D.  It's not called!
// This cam defaults to 40MB/s, increasing to 60MB/s
// simply by calling the function.
//
// Given that D5 cams can do 90MB/s, I would guess 200D can go faster,
// so presumably this autotune function is limited, or perhaps
// deliberately cautious.

#include "dryos.h"
#include "menu.h"
#include "config.h"

#ifdef FEATURE_SD_AUTOTUNE

#if defined(CONFIG_MMU_REMAP) && defined(CONFIG_200D)
#include "patch.h"
static int is_patched = 0;

#if !defined(CONFIG_FW_VERSION) || CONFIG_FW_VERSION != 101
    #error "The mode_192 addresses for patching have only been found for fw 1.0.1.  You will need to validate them."
#endif

struct patch mode_192[] =
{
// 200D has code to autotune SD speed, but there seems to be a bug.
// There's an array which holds an index for selecting speed,
// out of a set starting 192Mhz, then 156, 130, 111.
// They loop over the possible speeds, but they increment the index
// at the top of the loop - the first possible speed is skipped.
// Thus it will never select higher than 156Mhz.
//
// We patch this table so it selects from 192, 156, 111.
// (even pretty old cards can do 156, but we keep the slowest "fast"
// speed as a fallback).

// e0ea7eb8 - this stores 192MHz mode index; 9, but it's never used (looks like a bug)
// e0ea7ebc - this stores 156Mhz mode index; 8, which is checked first
// e0ea7ec0 - this stores 130Mhz mode index; 7, checked second

    {
        .addr = (uint8_t *)0xe0ea7ebc,
        .old_value = 0x8,
        .new_value = 0x9,
        .size = 4,
        .description = "Allow 192MHz SD speed (156)"
    },
    {
        .addr = (uint8_t *)0xe0ea7ec0,
        .old_value = 0x7,
        .new_value = 0x8,
        .size = 4,
        .description = "Allow 156MHz SD speed (130)"
    },
};
#endif // CONFIG_MMU_REMAP && CONFIG_200D

static CONFIG_INT("SD.enable_autotune_on_boot", is_autotune_enabled, 0);

// Wraps DryOS function for ML.
// Note this isn't "enable" - I'm not aware of a "disable"
// function.  Presumably the card remains autotuned until next
// restart, or should the card errors, when it will probably
// fallback to a lesser speed (this is speculation).
//
// Toggling the menu off -> on, will redo the autotune,
// possibly useful if the card ever resets and you don't
// want to restart cam?
static void run_SD_autotune(void *priv_unused, int unused)
{
    extern void autotune_SD(void);
    is_autotune_enabled = !is_autotune_enabled;

    if (is_autotune_enabled)
    {
        #if defined(CONFIG_MMU_REMAP) && defined(CONFIG_200D)
        if (!is_patched)
        {
            apply_patches(mode_192, COUNT(mode_192));
            is_patched = 1;
        }
        #endif
        autotune_SD();
    }
}

static struct menu_entry autotune_SD_speed_menu[] = {
    {
        .name = "SD speed autotune",
        .priv = &is_autotune_enabled,
        .select = run_SD_autotune,
        .max = 1,
        .help = "Run Canon's SD autotune. You can benchmark to check improvement",
    }
};

static void autotune_SD_init()
{
    menu_add("Prefs", autotune_SD_speed_menu, COUNT(autotune_SD_speed_menu));
}

static void autotune_SD_task()
{
    if (is_autotune_enabled)
    {
        msleep(1500); // Don't run during early boot while OS is still configuring SD.
                      // That seems to cause hangs.
                      // This is running as a task so sleeping is fine.
        #if defined(CONFIG_MMU_REMAP) && defined(CONFIG_200D)
        if (!is_patched)
        {
            apply_patches(mode_192, COUNT(mode_192));
            is_patched = 1;
        }
        #endif
        extern void autotune_SD(void);
        autotune_SD();
    }
}

INIT_FUNC("SD_auto", autotune_SD_init);
TASK_CREATE("sd_tune", autotune_SD_task, NULL, 0x1e, 0x400);

#endif // FEATURE_SD_AUTOTUNE
