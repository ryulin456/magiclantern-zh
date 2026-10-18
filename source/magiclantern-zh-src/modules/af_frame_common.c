/* Shared implementation for AF-frame automatic trigger modules (5D3.123). */
#include <module.h>
#include <dryos.h>
#include <config.h>
#include <menu.h>
#include <lens.h>
#include <focus.h>
#include <property.h>
#include <propvalues.h>
#include <shoot.h>
#include <gui-common.h>
#include <stdio.h>

#ifndef AFMOD_QUICK
#error AFMOD_QUICK must be supplied by the wrapper module
#endif

#if AFMOD_QUICK
#define AFMOD_TITLE "快速框选自动对焦（实验）"
#define AFMOD_LONGNAME "5D3 Quick AF 框选自动对焦（实验）"
#define AFMOD_CONFIG "afquick.enabled"
#define AFMOD_AUTO_CONFIG "afquick.auto_frame"
#define AFMOD_MODE_NAME "Quick AF"
#else
#define AFMOD_TITLE "实时框选自动对焦（实验）"
#define AFMOD_LONGNAME "5D3 Live AF 框选自动对焦（实验）"
#define AFMOD_CONFIG "aflive.enabled"
#define AFMOD_AUTO_CONFIG "aflive.auto_frame"
#define AFMOD_CENTER_CONFIG "aflive.center_first"
#define AFMOD_RECORD_CONFIG "aflive.recording_af"
#define AFMOD_MODE_NAME "Live AF"
#endif

#ifdef VIDAF_LEGACY_R9
#undef MENU_SET_VALUE
#undef MENU_SET_RINFO
#undef MENU_SET_WARNING
#define MENU_SET_VALUE(fmt, ...) snprintf(info->value, MENU_MAX_VALUE_LEN, fmt, ## __VA_ARGS__)
#define MENU_SET_RINFO(fmt, ...) snprintf(info->rinfo, MENU_MAX_RINFO_LEN, fmt, ## __VA_ARGS__)
#define MENU_SET_WARNING(level, fmt, ...) do { if ((level) > info->warning_level) { info->warning_level = (level); if (entry->selected) snprintf(info->warning, MENU_MAX_WARNING_LEN, fmt, ## __VA_ARGS__); } } while (0)
#endif

#define AFMOD_POLL_MS 50
/* Keep Quick AF at r4 timing; Live AF should react 0.35 s after the frame stops. */
#if AFMOD_QUICK
#define AFMOD_DEBOUNCE_MS 500
#define AFMOD_DEBOUNCE_TEXT "0.5"
#else
#define AFMOD_DEBOUNCE_MS 350
#define AFMOD_DEBOUNCE_TEXT "0.35"
#endif
#define AFMOD_COOLDOWN_MS 700
#define AFMOD_START_TIMEOUT_MS 1000
#define AFMOD_DONE_TIMEOUT_MS 5000

static CONFIG_INT(AFMOD_CONFIG, afmod_enabled, 0);
static CONFIG_INT(AFMOD_AUTO_CONFIG, afmod_auto, 1);
#if !AFMOD_QUICK
static CONFIG_INT(AFMOD_CENTER_CONFIG, afmod_center_first, 1);
static CONFIG_INT(AFMOD_RECORD_CONFIG, afmod_recording_af, 0);
#endif
static volatile int afmod_running = 0;
static volatile int afmod_shutdown = 0;
static volatile int afmod_last_x = -1;
static volatile int afmod_last_y = -1;
static volatile uint32_t afmod_cooldown_until = 0;
#if !AFMOD_QUICK
static volatile int afmod_remote_active = 0;
static volatile uint32_t afmod_remote_result_serial = 0;
#endif

static int afmod_idle(void) { return lv_focus_status == 1 || lv_focus_status == 2; }
/* RAW video records from LiveView without always setting Canon's RECORDING
 * flag.  Query its public config value at runtime instead of importing the
 * variable: this also works regardless of module load order. */
static int afmod_raw_video_enabled(void)
{
    return get_config_var("raw.video.enabled") || get_config_var("mlv.video.enabled");
}
#if AFMOD_QUICK
static int afmod_recording_allowed(void) { return 0; }
static int afmod_needs_remote_af(void) { return 0; }
#else
/* r18 hardware testing proved that PROP_REMOTE_AFSTART_BUTTON starts and
 * completes AF while H.264 recording continues on 5D3.123.  This is kept
 * opt-in; GUI replay and SW1 remain forbidden while recording.  RAW video
 * may not set RECORDING, so it deliberately uses this path too. */
static int afmod_recording_allowed(void) { return afmod_recording_af; }
static int afmod_needs_remote_af(void) { return RECORDING || afmod_raw_video_enabled(); }
#endif
static int afmod_mode_ok(void)
{
    /* Canon reports 0 for Quick; Live and face/multi Live are nonzero. */
#if AFMOD_QUICK
    return lv_af_system == 0;
#else
    /* Live AF gives one movable on-screen AF frame. Face/multi frames are
     * not a stable one-box target and are deliberately excluded. */
    return lv_af_system == 1;
#endif
}
static int afmod_ready(void)
{
    return !afmod_shutdown && afmod_enabled && lv && is_movie_mode() &&
        (!afmod_needs_remote_af() || afmod_recording_allowed()) &&
        lens_info.lens_exists && !is_manual_focus() && !is_continuous_af() &&
        !lens_info.job_state && afmod_idle() && afmod_mode_ok();
}
static void afmod_notify(void)
{
    if (!lv || !is_movie_mode()) NotifyBox(2500, "框选自动对焦：请先开启视频实时取景");
    else if (afmod_needs_remote_af() && !afmod_recording_allowed()) NotifyBox(3500, "Live AF：RAW/录像自动对焦未开启（请在菜单中开启）");
    else if (!lens_info.lens_exists || is_manual_focus()) NotifyBox(2500, "框选自动对焦：镜头必须处于 AF");
    else if (is_continuous_af()) NotifyBox(2500, "框选自动对焦：请关闭佳能连续 AF");
    else if (!afmod_mode_ok()) NotifyBox(3000, "框选自动对焦：请在佳能菜单选择 " AFMOD_MODE_NAME);
    else NotifyBox(2500, "框选自动对焦：佳能对焦系统正忙");
}

/* Uses SW1 outside recording only.  H.264 uses the verified remote-AF
 * property instead; never replay GUI events or synthetic half-shutter there. */
static void afmod_focus_task(void *unused)
{
    (void)unused;
    int setup = 0, pressed = 0, result = 0, t0;
    if (!afmod_ready()) goto cleanup;
    if (afmod_needs_remote_af())
    {
        int request = 1;
        uint32_t result_before = afmod_remote_result_serial;
        afmod_remote_active = 1;
        prop_request_change(PROP_REMOTE_AFSTART_BUTTON, &request, 4);
        t0 = get_ms_clock();
        while (!afmod_shutdown && afmod_needs_remote_af() && get_ms_clock() - t0 < 5000)
        {
            if (afmod_remote_result_serial != result_before) { result = 1; break; }
            msleep(10);
        }
        request = 0;
        prop_request_change(PROP_REMOTE_AFSTART_BUTTON, &request, 4);
        afmod_remote_active = 0;
        if (!result && !afmod_shutdown && afmod_needs_remote_af()) result = 3;
        goto cleanup;
    }
    if (!lens_setup_af(AF_ENABLE)) goto cleanup;
    setup = 1;
    module_send_keypress(MODULE_KEY_PRESS_HALFSHUTTER);
    pressed = 1;
    t0 = get_ms_clock();
    while (!afmod_shutdown && get_ms_clock() - t0 < AFMOD_START_TIMEOUT_MS)
    {
        if (!lv || !is_movie_mode() || (afmod_needs_remote_af() && !afmod_recording_allowed())) goto cleanup;
        if (lv_focus_status == 3) break;
        msleep(10);
    }
    if (lv_focus_status != 3) { result = 2; goto cleanup; }
    t0 = get_ms_clock();
    while (!afmod_shutdown && get_ms_clock() - t0 < AFMOD_DONE_TIMEOUT_MS)
    {
        if (!lv || !is_movie_mode() || (afmod_needs_remote_af() && !afmod_recording_allowed())) goto cleanup;
        if (afmod_idle()) { result = 1; goto cleanup; }
        msleep(10);
    }
    result = 3;
cleanup:
    if (pressed == 1) module_send_keypress(MODULE_KEY_UNPRESS_HALFSHUTTER);
    if (setup) lens_cleanup_af();
    afmod_running = 0;
    afmod_cooldown_until = get_ms_clock() + AFMOD_COOLDOWN_MS;
    if (!afmod_shutdown)
    {
        if (result == 1) NotifyBox(1200, AFMOD_MODE_NAME "：对焦完成");
        else if (result == 2) NotifyBox(2500, AFMOD_MODE_NAME "：佳能未开始对焦");
        else if (result == 3) NotifyBox(3000, AFMOD_MODE_NAME "：对焦超时，已释放按键");
    }
}
static void afmod_start(void)
{
    if (afmod_running) return;
    if (!afmod_ready()) { afmod_notify(); return; }
    afmod_running = 1;
    if (task_create("af_frame", 0x1d, 0x2000, afmod_focus_task, 0) & 1) afmod_running = 0;
}
static void afmod_watch_task(void *unused)
{
    (void)unused;
    int seen = 0, ox = -1, oy = -1, px = -1, py = -1;
    uint32_t changed = 0;
#if !AFMOD_QUICK
    int in_live_session = 0, center_pending = 0;
    uint32_t center_last_attempt = 0;
#endif
    while (!afmod_shutdown)
    {
        msleep(AFMOD_POLL_MS);
        if (!afmod_enabled || !afmod_auto || !lv || !is_movie_mode())
        {
            seen = 0;
#if !AFMOD_QUICK
            in_live_session = center_pending = 0;
#endif
            continue;
        }
#if !AFMOD_QUICK
        if (!afmod_mode_ok()) { seen = 0; in_live_session = center_pending = 0; continue; }
        /* Only a new video Live View session may request the initial center AF.
         * Do not reset this flag while Canon is busy focusing: doing so would
         * cause a completed initial focus to center and focus again forever. */
        if (!in_live_session)
        {
            in_live_session = 1;
            seen = 0;
            if (afmod_center_first && !RECORDING)
            {
                center_pending = 1;
                center_last_attempt = 0;
                continue;
            }
        }
        if (!afmod_ready()) { seen = 0; continue; }
#endif
        int x = 0, y = 0;
        get_afframe_pos(720, 480, &x, &y);
#if !AFMOD_QUICK
        if (center_pending)
        {
            int sensor_w = 0, sensor_h = 0, sensor_x = 0, sensor_y = 0;
            get_afframe_sensor_res(&sensor_w, &sensor_h);
            if (sensor_w <= 0 || sensor_h <= 0) continue;
            get_afframe_pos(sensor_w, sensor_h, &sensor_x, &sensor_y);
            if (ABS(sensor_x - sensor_w / 2) <= MAX(16, sensor_w / 80) &&
                ABS(sensor_y - sensor_h / 2) <= MAX(16, sensor_h / 80))
            {
                center_pending = 0;
                ox = x; oy = y; afmod_last_x = x; afmod_last_y = y; seen = 1;
                afmod_start();
                continue;
            }
            if (get_ms_clock() - center_last_attempt >= 350)
            {
                /* Move to the actual sensor center directly.  The core helper
                 * cycles through user-configured jump points and may not land
                 * on center, so it is intentionally not used here. */
                move_lv_afframe(sensor_w / 2 - sensor_x, sensor_h / 2 - sensor_y);
                center_last_attempt = get_ms_clock();
            }
            continue;
        }
#endif
        if (!seen) { ox=x; oy=y; afmod_last_x=x; afmod_last_y=y; seen=1; continue; }
        if (x != ox || y != oy) { ox=x; oy=y; px=x; py=y; changed=get_ms_clock(); continue; }
        if (px < 0 || afmod_running || (int32_t)((uint32_t)get_ms_clock() - afmod_cooldown_until) < 0 || get_ms_clock()-changed < AFMOD_DEBOUNCE_MS) continue;
        if (px != afmod_last_x || py != afmod_last_y) { afmod_last_x=px; afmod_last_y=py; afmod_start(); }
        px = py = -1;
    }
}
static MENU_SELECT_FUNC(afmod_manual) { (void)priv; (void)delta; gui_stop_menu(); afmod_start(); }
/* Historical AF-ON capture tools are retained only in source history.  They
 * are deliberately excluded from the released module and its menu. */
#if 0
static MENU_SELECT_FUNC(afmod_learn_afon)
{
    (void)priv; (void)delta;
    /* The ML menu cannot be operated normally while 5D3 H.264 is recording.
     * Arm first, then ignore Start/Stop and capture AF-ON after recording. */
    afmod_learn_state = AFMOD_LEARN_WAIT_RECORDING;
    afmod_learn_press = 0;
    afmod_learn_press_type = afmod_learn_press_arg = 0;
    afmod_learn_release_type = afmod_learn_release_arg = 0;
    afmod_learn_press_obj = afmod_learn_release_obj = 0;
    afmod_learn_e3 = afmod_learn_e3_type = afmod_learn_e3_arg = 0;
    afmod_learn_e4 = afmod_learn_e4_type = afmod_learn_e4_arg = 0;
    afmod_learn_e3_obj = afmod_learn_e4_obj = 0;
    afmod_learn_start = 0;
    afmod_learn_deadline = get_ms_clock() + 20000;
    gui_stop_menu();
    NotifyBox(5000, "Live AF：请在 20 秒内开始录像；随后按 AF-ON");
}

/* The 5D3 port has no published AF-ON event constant. Record the actual
 * press/release pair instead of using an unknown code. Do not consume it. */
static unsigned int afmod_raw_keypress(unsigned int raw_event)
{
    struct event *event = (struct event *)raw_event;
    uint32_t now = get_ms_clock();
    if (!event || afmod_learn_state < 1 || afmod_learn_state > 4 || now < afmod_learn_start || now >= afmod_learn_deadline) return 1;
    if (afmod_learn_state == 1)
    {
        afmod_learn_press = event->param;
        afmod_learn_press_type = event->type;
        afmod_learn_press_arg = event->arg;
        afmod_learn_press_obj = (uint32_t)event->obj;
        afmod_learn_state = 2;
        NotifyBox(2500, "Live AF：已记录 AF-ON 按下，请松开按键");
    }
    else if (afmod_learn_state == 2)
    {
        afmod_afon_press = afmod_learn_press;
        afmod_afon_release = event->param;
        afmod_learn_release_type = event->type;
        afmod_learn_release_arg = event->arg;
        afmod_learn_release_obj = (uint32_t)event->obj;
        afmod_learn_state = 3;
        NotifyBox(2500, "Live AF：已记录第 2 项，请松开 AF-ON");
    }
    else if (afmod_learn_state == 3)
    {
        afmod_learn_e3 = event->param;
        afmod_learn_e3_type = event->type;
        afmod_learn_e3_arg = event->arg;
        afmod_learn_e3_obj = (uint32_t)event->obj;
        afmod_learn_state = 4;
        NotifyBox(2500, "Live AF：已记录第 3 项");
    }
    else
    {
        afmod_learn_e4 = event->param;
        afmod_learn_e4_type = event->type;
        afmod_learn_e4_arg = event->arg;
        afmod_learn_e4_obj = (uint32_t)event->obj;
        afmod_learn_state = 0;
        NotifyBox(3000, "Live AF：AF-ON 4 项数据已记录");
    }
    return 1;
}

/* The following handlers are passive observers.  They do not call any Canon
 * function and do not alter the buffers they receive. */
PROP_HANDLER(PROP_LV_FOCUS_DATA)
{
    (void)property; (void)token; (void)len;
    if (afmod_prop_state != 2 || !RECORDING || !buf) return;
    int value = (int)buf[0];
    if (!afmod_prop_data_count) afmod_prop_data_first = value;
    afmod_prop_data_last = value;
    afmod_prop_data_count++;
}
PROP_HANDLER(PROP_LV_FOCUS_DONE)
{
    (void)property; (void)token; (void)len;
    if (afmod_prop_state != 2 || !RECORDING || !buf) return;
    afmod_prop_done = (int)buf[0];
    afmod_prop_done_count++;
}
PROP_HANDLER(PROP_LV_AF_RESULT)
{
    (void)property; (void)token; (void)len;
    if (!RECORDING || !buf) return;
    if (afmod_remote_active) afmod_remote_result_serial++;
    if (afmod_prop_state != 2) return;
    afmod_prop_result = (int)buf[0];
    afmod_prop_result_count++;
}
#endif
#if !AFMOD_QUICK
/* The only recording-side observer used by the released module: it marks the
 * end of an AF request made through PROP_REMOTE_AFSTART_BUTTON. */
PROP_HANDLER(PROP_LV_AF_RESULT)
{
    (void)property; (void)token; (void)buf; (void)len;
    if (RECORDING && afmod_remote_active) afmod_remote_result_serial++;
}
#endif
static MENU_UPDATE_FUNC(afmod_main_update)
{
    if (!afmod_enabled) MENU_SET_VALUE("已关闭");
    else if (!lv || !is_movie_mode()) MENU_SET_VALUE("等待视频实时取景");
    else if (afmod_running) MENU_SET_VALUE("佳能正在对焦");
    else if (afmod_raw_video_enabled()) MENU_SET_VALUE("RAW 视频兼容模式");
    else MENU_SET_VALUE("已启用");

    if (!lv || !is_movie_mode()) MENU_SET_WARNING(MENU_WARN_NOT_WORKING, "请先手动进入视频实时取景，再开启本模块。\n");
    else if (!afmod_mode_ok()) MENU_SET_WARNING(MENU_WARN_NOT_WORKING, "佳能 AF 模式不匹配。\n");
    else if (afmod_needs_remote_af() && !afmod_recording_allowed()) MENU_SET_WARNING(MENU_WARN_ADVICE, "RAW/录像自动对焦已关闭。\n");
    else if (afmod_raw_video_enabled()) MENU_SET_WARNING(MENU_WARN_ADVICE, "RAW 视频使用佳能远程 AF；不会模拟半按快门。\n");
}
static MENU_UPDATE_FUNC(afmod_auto_update) { MENU_SET_VALUE(afmod_auto ? "移动框后自动" : "仅手动"); }
#if !AFMOD_QUICK
static MENU_UPDATE_FUNC(afmod_center_update) { MENU_SET_VALUE(afmod_center_first ? "开启实时取景时执行" : "关闭"); }
static MENU_UPDATE_FUNC(afmod_record_update)
{
    MENU_SET_VALUE(afmod_recording_af ? "开启：停框后自动" : "关闭");
}
#if 0
static MENU_UPDATE_FUNC(afmod_learn_update)
{
    if (afmod_learn_state == AFMOD_LEARN_WAIT_RECORDING) MENU_SET_VALUE("请先开始录像");
    else if (afmod_learn_state == 1) MENU_SET_VALUE("等待 AF-ON 按下");
    else if (afmod_learn_state == 2) MENU_SET_VALUE("等待第 2 项");
    else if (afmod_learn_state == 3) MENU_SET_VALUE("等待第 3 项");
    else if (afmod_learn_state == 4) MENU_SET_VALUE("等待第 4 项");
    else if (afmod_afon_learned()) MENU_SET_VALUE("已记录；查看下两项");
    else MENU_SET_VALUE("未记录");
}
static MENU_UPDATE_FUNC(afmod_learn_press_update)
{
    MENU_SET_VALUE("P%02X T%d A%d O%08X", afmod_afon_press, afmod_learn_press_type, afmod_learn_press_arg, afmod_learn_press_obj);
}
static MENU_UPDATE_FUNC(afmod_learn_release_update)
{
    MENU_SET_VALUE("R%02X T%d A%d O%08X", afmod_afon_release, afmod_learn_release_type, afmod_learn_release_arg, afmod_learn_release_obj);
}
static MENU_UPDATE_FUNC(afmod_learn_e3_update)
{
    MENU_SET_VALUE("E3%02X T%d A%d O%08X", afmod_learn_e3, afmod_learn_e3_type, afmod_learn_e3_arg, afmod_learn_e3_obj);
}
static MENU_UPDATE_FUNC(afmod_learn_e4_update)
{
    MENU_SET_VALUE("E4%02X T%d A%d O%08X", afmod_learn_e4, afmod_learn_e4_type, afmod_learn_e4_arg, afmod_learn_e4_obj);
}
static MENU_UPDATE_FUNC(afmod_capture_update)
{
    if (afmod_prop_state == 1) MENU_SET_VALUE("请先开始录像");
    else if (afmod_prop_state == 2) MENU_SET_VALUE("等待真实 AF-ON");
    else MENU_SET_VALUE("数据已保留");
}
static MENU_UPDATE_FUNC(afmod_capture_data_update)
{
    MENU_SET_VALUE("%d 次 %d→%d", afmod_prop_data_count, afmod_prop_data_first, afmod_prop_data_last);
}
static MENU_UPDATE_FUNC(afmod_capture_done_update)
{
    MENU_SET_VALUE("%d 次 值%d", afmod_prop_done_count, afmod_prop_done);
}
static MENU_UPDATE_FUNC(afmod_capture_result_update)
{
    MENU_SET_VALUE("%d 次 值%d", afmod_prop_result_count, afmod_prop_result);
}
#endif
#endif
static struct menu_entry afmod_menu[] = {
    { .name=AFMOD_TITLE, .priv=&afmod_enabled, .min=0, .max=1, .update=afmod_main_update,
      .depends_on=DEP_MOVIE_MODE|DEP_LIVEVIEW, .submenu_width=710,
      .help="移动佳能 AF 框后自动调用一次佳能 " AFMOD_MODE_NAME "。",
      .help2="框停止约 " AFMOD_DEBOUNCE_TEXT " 秒才触发；不拦截方向键。",
      .children=(struct menu_entry[]){
        { .name="移动对焦框后自动对焦", .priv=&afmod_auto, .min=0, .max=1, .update=afmod_auto_update,
          .help="对焦框稳定约 " AFMOD_DEBOUNCE_TEXT " 秒后自动对焦。", .help2="一次对焦后冷却约 0.7 秒，避免反复拉风箱。" },
#if !AFMOD_QUICK
        { .name="开启实时取景时先居中对焦", .priv=&afmod_center_first, .min=0, .max=1, .update=afmod_center_update,
          .help="进入视频实时取景后，先将佳能 Live AF 框移到画面中央并对焦一次。",
          .help2="直接按真实坐标居中；确认前会继续重试，不会跳过。" },
        { .name="RAW/录像中移动框自动对焦", .priv=&afmod_recording_af, .min=0, .max=1, .update=afmod_record_update,
          .help="RAW 视频或佳能录像时，停框约 0.35 秒请求一次佳能远程 AF；默认关闭。",
          .help2="每次等佳能结果或最多 5 秒再释放；不模拟按键、不停止录像。" },
#if 0
        { .name="录像后记录 AF-ON", .select=afmod_learn_afon, .update=afmod_learn_update, .icon_type=IT_ACTION,
          .help="先选择本项，再按录像键；录像稳定后按一下 AF-ON。",
          .help2="忽略录像键事件，仅记录录像中的 AF-ON 数据。" },
        { .name="录像后被动记录 AF 状态", .select=afmod_capture_recording_af, .update=afmod_capture_update, .icon_type=IT_ACTION,
          .help="只读取你真实按下 AF-ON 时佳能发出的对焦状态；不模拟任何按键。",
          .help2="先选择本项，再开始录像并按 AF-ON；停止录像后查看下三项。" },
        { .name="录像后最长 5 秒远程 AF（r18）", .select=afmod_remote_af_test, .icon_type=IT_ACTION,
          .help="录像稳定后请求一次 PROP_REMOTE_AFSTART_BUTTON；收到结果或 5 秒即释放。",
          .help2="不模拟按键、不改地址；仅验证 5D3 录像 AF 能否完整合焦。" },
        { .name="AF 数据变化", .update=afmod_capture_data_update,
          .help="录像 AF-ON 期间的 PROP_LV_FOCUS_DATA 次数与首末值。", .help2="仅诊断，不会驱动镜头或改变佳能状态。" },
        { .name="AF 完成状态", .update=afmod_capture_done_update,
          .help="录像 AF-ON 期间的 PROP_LV_FOCUS_DONE。", .help2="仅诊断，不会驱动镜头或改变佳能状态。" },
        { .name="AF 对焦结果", .update=afmod_capture_result_update,
          .help="录像 AF-ON 期间的 PROP_LV_AF_RESULT（0 通常代表成功）。", .help2="仅诊断，不会驱动镜头或改变佳能状态。" },
        { .name="AF-ON 按下数据", .update=afmod_learn_press_update,
          .help="r11 仅显示原始数据；不在录像中重放按键。", .help2="请拍下右边 P、T、A、O 4 组值。" },
        { .name="AF-ON 松开数据", .update=afmod_learn_release_update,
          .help="r11 仅显示原始数据；不在录像中重放按键。", .help2="请拍下右边 R、T、A、O 4 组值。" },
        { .name="AF-ON E3 数据", .update=afmod_learn_e3_update,
          .help="记录 AF-ON 完整事件序列的第 3 项。", .help2="请拍下右边 E3、T、A、O 4 组值。" },
        { .name="AF-ON E4 数据", .update=afmod_learn_e4_update,
          .help="记录 AF-ON 完整事件序列的第 4 项。", .help2="请拍下右边 E4、T、A、O 4 组值。" },
#endif
#endif
        { .name="立即执行一次", .select=afmod_manual, .icon_type=IT_ACTION,
          .help="按 SET 立即执行一次佳能原生对焦。", .help2="不会拍照，也不会开始录像。" }, MENU_EOL } }, };
static unsigned int afmod_init(void)
{
    if (!is_camera("5D3", "1.2.3")) return -1;
    afmod_shutdown=0; afmod_running=0;
    /* Never restore an armed module at camera startup.  The user must first
     * enter video LiveView and then enable it deliberately. */
    if (!lv || !is_movie_mode()) afmod_enabled=0;
    menu_add("Focus", afmod_menu, COUNT(afmod_menu));
    task_create("af_watch", 0x1d, 0x2000, afmod_watch_task, 0); return 0;
}
static unsigned int afmod_deinit(void)
{
    afmod_shutdown=1; for (int i=0;i<300 && afmod_running;i++) msleep(20);
    menu_remove("Focus", afmod_menu, COUNT(afmod_menu)); return 0;
}
MODULE_INFO_START() MODULE_INIT(afmod_init) MODULE_DEINIT(afmod_deinit) MODULE_LONGNAME(AFMOD_LONGNAME) MODULE_INFO_END()
#if !AFMOD_QUICK
MODULE_PROPHANDLERS_START()
    MODULE_PROPHANDLER(PROP_LV_AF_RESULT)
MODULE_PROPHANDLERS_END()
MODULE_CONFIGS_START()
    MODULE_CONFIG(afmod_enabled)
    MODULE_CONFIG(afmod_auto)
    MODULE_CONFIG(afmod_center_first)
    MODULE_CONFIG(afmod_recording_af)
MODULE_CONFIGS_END()
#endif
