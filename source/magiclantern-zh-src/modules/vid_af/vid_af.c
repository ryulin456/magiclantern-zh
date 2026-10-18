/* 5D3.123 native LiveView AF trigger experiment; no raw lens-drive commands. */
#include <module.h>
#include <dryos.h>
#include <config.h>
#include <menu.h>
#include <lens.h>
#include <focus.h>
#include <propvalues.h>
#include <shoot.h>
#include <stdio.h>

/* The r9 core does not export menu_snprintf(). */
#ifdef VIDAF_LEGACY_R9
#undef MENU_SET_VALUE
#undef MENU_SET_RINFO
#undef MENU_SET_HELP
#undef MENU_SET_WARNING
#define MENU_SET_VALUE(fmt, ...) snprintf(info->value, MENU_MAX_VALUE_LEN, fmt, ## __VA_ARGS__)
#define MENU_SET_RINFO(fmt, ...) snprintf(info->rinfo, MENU_MAX_RINFO_LEN, fmt, ## __VA_ARGS__)
#define MENU_SET_HELP(fmt, ...) do { if (entry->selected) snprintf(info->help, MENU_MAX_HELP_LEN, fmt, ## __VA_ARGS__); } while (0)
#define MENU_SET_WARNING(level, fmt, ...) do { if ((level) > info->warning_level) { info->warning_level = (level); if (entry->selected) snprintf(info->warning, MENU_MAX_WARNING_LEN, fmt, ## __VA_ARGS__); } } while (0)
#endif

#define VIDAF_FRAME_POLL_MS       50
#define VIDAF_MOVE_DEBOUNCE_MS   350
#define VIDAF_START_TIMEOUT_MS  1000
#define VIDAF_DONE_TIMEOUT_MS   5000

static CONFIG_INT("vidaf.enabled", vidaf_enabled, 0);
static CONFIG_INT("vidaf.auto_frame", vidaf_auto_frame, 1);
static volatile int vidaf_running = 0;
static volatile int vidaf_shutdown = 0;
static volatile int vidaf_last_result = 0;
static volatile int vidaf_last_x = -1;
static volatile int vidaf_last_y = -1;

enum { VIDAF_RESULT_NONE = 0, VIDAF_RESULT_OK, VIDAF_RESULT_NO_START, VIDAF_RESULT_TIMEOUT, VIDAF_RESULT_NOT_READY };

/* Canon uses 3 while focusing; 1 and (on some paths) 2 are idle. */
static int vidaf_focus_is_idle(void) { return lv_focus_status == 1 || lv_focus_status == 2; }

static int vidaf_ready(void)
{
    if (vidaf_shutdown || !vidaf_enabled) return 0;
    if (!lv || !is_movie_mode() || RECORDING) return 0;
    if (!lens_info.lens_exists || is_manual_focus()) return 0;
    if (is_continuous_af() || lens_info.job_state != 0) return 0;
    return vidaf_focus_is_idle();
}

static void vidaf_notify_not_ready(void)
{
    if (!lv) NotifyBox(2500, "视频自动对焦：请先开启实时取景");
    else if (!is_movie_mode()) NotifyBox(2500, "视频自动对焦：请切换到视频模式");
    else if (RECORDING) NotifyBox(3000, "视频自动对焦：录制中不自动触发");
    else if (!lens_info.lens_exists) NotifyBox(2500, "视频自动对焦：未检测到电子镜头");
    else if (is_manual_focus()) NotifyBox(2500, "视频自动对焦：请将镜头切换到 AF");
    else if (is_continuous_af()) NotifyBox(3000, "视频自动对焦：请关闭佳能连续 AF");
    else NotifyBox(2500, "视频自动对焦：佳能对焦系统正忙");
}

/* Same SW1 route used by ML Lua lens.autofocus(). Canon chooses the AF point. */
static void vidaf_native_task(void *unused)
{
    (void) unused;
    int result = VIDAF_RESULT_NOT_READY;
    int af_setup = 0;
    int pressed = 0;
    int t0;

    if (!vidaf_ready()) goto cleanup;
    if (!lens_setup_af(AF_ENABLE)) goto cleanup;
    af_setup = 1;
    module_send_keypress(MODULE_KEY_PRESS_HALFSHUTTER);
    pressed = 1;

    t0 = get_ms_clock();
    while (!vidaf_shutdown && get_ms_clock() - t0 < VIDAF_START_TIMEOUT_MS)
    {
        if (RECORDING || !lv || !is_movie_mode()) goto cleanup;
        if (lv_focus_status == 3) break;
        msleep(10);
    }
    if (lv_focus_status != 3) { result = VIDAF_RESULT_NO_START; goto cleanup; }

    t0 = get_ms_clock();
    while (!vidaf_shutdown && get_ms_clock() - t0 < VIDAF_DONE_TIMEOUT_MS)
    {
        if (RECORDING || !lv || !is_movie_mode()) goto cleanup;
        if (vidaf_focus_is_idle()) { result = VIDAF_RESULT_OK; goto cleanup; }
        msleep(10);
    }
    result = VIDAF_RESULT_TIMEOUT;

cleanup:
    if (pressed) module_send_keypress(MODULE_KEY_UNPRESS_HALFSHUTTER);
    if (af_setup) lens_cleanup_af();
    vidaf_last_result = result;
    vidaf_running = 0;
    if (!vidaf_shutdown)
    {
        if (result == VIDAF_RESULT_OK) NotifyBox(1200, "视频自动对焦：佳能对焦完成");
        else if (result == VIDAF_RESULT_NO_START) NotifyBox(2500, "视频自动对焦：佳能未开始对焦");
        else if (result == VIDAF_RESULT_TIMEOUT) NotifyBox(3000, "视频自动对焦：佳能对焦超时，已释放按键");
        else vidaf_notify_not_ready();
    }
}

static int vidaf_start_native(void)
{
    if (vidaf_running) return 0;
    if (!vidaf_ready()) { vidaf_notify_not_ready(); return 0; }
    vidaf_running = 1;
    if (task_create("vid_af", 0x1d, 0x2000, vidaf_native_task, 0) & 1)
    {
        vidaf_running = 0;
        NotifyBox(2500, "视频自动对焦：无法创建任务");
        return 0;
    }
    return 1;
}

/* Polling avoids intercepting direction keys; it only triggers after the Canon AF box settles. */
static void vidaf_frame_watch_task(void *unused)
{
    (void) unused;
    int observed_x = -1, observed_y = -1, pending_x = -1, pending_y = -1;
    int frame_seen = 0;
    uint32_t moved_at = 0;

    while (!vidaf_shutdown)
    {
        msleep(VIDAF_FRAME_POLL_MS);
        if (!vidaf_enabled || !vidaf_auto_frame || !lv || !is_movie_mode() || RECORDING)
        {
            frame_seen = 0;
            continue;
        }
        int x = 0, y = 0;
        get_afframe_pos(720, 480, &x, &y);
        if (!frame_seen)
        {
            observed_x = x; observed_y = y;
            vidaf_last_x = x; vidaf_last_y = y;
            frame_seen = 1;
            continue;
        }
        if (x != observed_x || y != observed_y)
        {
            observed_x = x; observed_y = y;
            pending_x = x; pending_y = y; moved_at = get_ms_clock();
            continue;
        }
        if (pending_x < 0 || vidaf_running || get_ms_clock() - moved_at < VIDAF_MOVE_DEBOUNCE_MS) continue;
        if (pending_x != vidaf_last_x || pending_y != vidaf_last_y)
        {
            vidaf_last_x = pending_x;
            vidaf_last_y = pending_y;
            vidaf_start_native();
        }
        pending_x = -1; pending_y = -1;
    }
}

static MENU_SELECT_FUNC(vidaf_run_select)
{
    (void) priv; (void) delta;
    gui_stop_menu();
    vidaf_start_native();
}

static MENU_UPDATE_FUNC(vidaf_main_update)
{
    MENU_SET_VALUE(vidaf_running ? "佳能正在对焦" : (vidaf_enabled ? "已启用" : "已关闭"));
    if (vidaf_last_result == VIDAF_RESULT_OK) MENU_SET_RINFO("上次成功");
    if (!lv || !is_movie_mode()) MENU_SET_WARNING(MENU_WARN_NOT_WORKING, "仅支持未录制的视频实时取景。\n");
    else if (RECORDING) MENU_SET_WARNING(MENU_WARN_NOT_WORKING, "录制中停用；请先停止录制。\n");
    else if (!lens_info.lens_exists || is_manual_focus()) MENU_SET_WARNING(MENU_WARN_NOT_WORKING, "需要带对焦马达且切到 AF 的电子镜头。\n");
    else if (is_continuous_af()) MENU_SET_WARNING(MENU_WARN_NOT_WORKING, "请先关闭佳能连续 AF。\n");
    else MENU_SET_WARNING(MENU_WARN_INFO, "使用佳能原生对焦，不执行盲扫。\n");
}

static MENU_UPDATE_FUNC(vidaf_run_update) { MENU_SET_VALUE(vidaf_running ? "佳能对焦中" : "按 SET 执行"); }
static MENU_UPDATE_FUNC(vidaf_auto_update) { MENU_SET_VALUE(vidaf_auto_frame ? "移动框后自动" : "仅手动"); }

static struct menu_entry vidaf_menu[] =
{
    {
        .name = "视频自动对焦（实验）", .priv = &vidaf_enabled, .min = 0, .max = 1,
        .update = vidaf_main_update, .works_best_in = DEP_MOVIE_MODE | DEP_LIVEVIEW, .submenu_width = 710,
        .help = "使用佳能原生实时取景自动对焦，不再逐步盲扫镜头。",
        .help2 = "对准佳能 AF 框；停止移动约 0.35 秒后自动触发。",
        .children = (struct menu_entry[])
        {
            { .name = "移动对焦框后自动对焦", .priv = &vidaf_auto_frame, .min = 0, .max = 1, .update = vidaf_auto_update,
              .help = "佳能 AF 框位置稳定后，自动调用佳能原生对焦。", .help2 = "仅在未录制的视频实时取景中工作；默认开启。" },
            { .name = "立即使用佳能自动对焦", .select = vidaf_run_select, .update = vidaf_run_update, .icon_type = IT_ACTION,
              .help = "不移动对焦框时，可按 SET 手动调用一次佳能对焦。", .help2 = "与半按快门使用相同的佳能对焦流程。" },
            MENU_EOL,
        },
    },
};

static unsigned int vidaf_init(void)
{
    if (!is_camera("5D3", "1.2.3")) return -1;
    vidaf_shutdown = 0; vidaf_running = 0;
    menu_add("Focus", vidaf_menu, COUNT(vidaf_menu));
    task_create("vid_af_watch", 0x1d, 0x2000, vidaf_frame_watch_task, 0);
    return 0;
}

static unsigned int vidaf_deinit(void)
{
    vidaf_shutdown = 1;
    for (int i = 0; i < 300 && vidaf_running; i++) msleep(20);
    menu_remove("Focus", vidaf_menu, COUNT(vidaf_menu));
    return 0;
}

MODULE_INFO_START()
    MODULE_INIT(vidaf_init)
    MODULE_DEINIT(vidaf_deinit)
    MODULE_LONGNAME("5D3 原生视频自动对焦（实验）")
MODULE_INFO_END()
