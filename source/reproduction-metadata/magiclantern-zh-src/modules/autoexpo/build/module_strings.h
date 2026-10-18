static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "Auto exposure";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Author";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "pravdomil.cz";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Credits";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "ML devs";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = "Auto exposure algorithm";
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Forum";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = "http://www.magiclantern.fm/forum/index.php?topic=7208.0";
static char __module_string_g_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_g_value[] MODULE_STRINGS_SECTION = 
    "Automatic exposure algorithm based on predefined curves. It's\n"
    "like a real 'P' where you actually program what you want the\n"
    "camera to do depending on the light.\n"
    "http://www.magiclantern.fm/forum/?topic=7208\n"
    "\n"
;
static char __module_string_h_name [] MODULE_STRINGS_SECTION = "Build date";
static char __module_string_h_value[] MODULE_STRINGS_SECTION = "2026-08-31 03:12:13 UTC";
static char __module_string_i_name [] MODULE_STRINGS_SECTION = "Build user";
static char __module_string_i_value[] MODULE_STRINGS_SECTION = "ryulin456@gmail.com";

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
    MODULE_STRING(__module_string_i_name, __module_string_i_value) \
  MODULE_STRINGS_END()
