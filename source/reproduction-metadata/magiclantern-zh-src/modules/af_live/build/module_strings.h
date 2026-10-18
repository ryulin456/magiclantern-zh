static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "5D3 Live AF 框选自动对焦（实验）";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Author";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "ryulin456@gmail.com";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "移动佳能 Live AF 框后自动对焦";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = "在未录制的视频实时取景中，佳能 Live 或面部 Live AF 框停止移动后自动触发一次佳能原生对焦。";
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Build date";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = "2026-08-31 10:07:56 UTC";
static char __module_string_g_name [] MODULE_STRINGS_SECTION = "Build user";
static char __module_string_g_value[] MODULE_STRINGS_SECTION = "ryulin456@gmail.com";

#define MODULE_STRINGS() \
  MODULE_STRINGS_START() \
    MODULE_STRING(__module_string_a_name, __module_string_a_value) \
    MODULE_STRING(__module_string_b_name, __module_string_b_value) \
    MODULE_STRING(__module_string_c_name, __module_string_c_value) \
    MODULE_STRING(__module_string_d_name, __module_string_d_value) \
    MODULE_STRING(__module_string_e_name, __module_string_e_value) \
    MODULE_STRING(__module_string_f_name, __module_string_f_value) \
    MODULE_STRING(__module_string_g_name, __module_string_g_value) \
  MODULE_STRINGS_END()
