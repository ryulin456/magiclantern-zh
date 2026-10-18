static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "Benchmarks";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Author";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "a1ex";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "5D4 1.3.3 基准测试安全锁";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Forum";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = "http://www.magiclantern.fm/forum/index.php?topic=12630.0";
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = 
    "上游 5D4\n"
    "构建清单保留了基准测试模块，但平台内存、缓存、存储卡、\n"
    "图像缓冲区和 EDMAC 地址尚未完成实机验证。\n"
    "\n"
    "本开发测试包在构建时排除所有测试实现，仅编入安全锁提示。\n"
    "没有内存、存储卡、缓存、EDMAC\n"
    "或显示测试回调，不能通过菜单取消锁定。\n"
    "\n"
;
static char __module_string_g_name [] MODULE_STRINGS_SECTION = "Build date";
static char __module_string_g_value[] MODULE_STRINGS_SECTION = "2026-09-22 08:30:27 UTC";
static char __module_string_h_name [] MODULE_STRINGS_SECTION = "Build user";
static char __module_string_h_value[] MODULE_STRINGS_SECTION = "ryulin456@gmail.com";

#define MODULE_STRINGS() \
  MODULE_STRINGS_START() \
    MODULE_STRING(__module_string_a_name, __module_string_a_value) \
    MODULE_STRING(__module_string_b_name, __module_string_b_value) \
    MODULE_STRING(__module_string_c_name, __module_string_c_value) \
    MODULE_STRING(__module_string_d_name, __module_string_d_value) \
    MODULE_STRING(__module_string_e_name, __module_string_e_value) \
    MODULE_STRING(__module_string_f_name, __module_string_f_value) \
    MODULE_STRING(__module_string_g_name, __module_string_g_value) \
    MODULE_STRING(__module_string_h_name, __module_string_h_value) \
  MODULE_STRINGS_END()
