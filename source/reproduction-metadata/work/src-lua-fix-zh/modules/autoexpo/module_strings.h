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
static char __module_string_g_value[] MODULE_STRINGS_SECTION = "Automatic exposure algorithm based on predefined curves.\nIt's like a real 'P' where you actually program what you want the camera to do depending on the light.\nhttp://www.magiclantern.fm/forum/?topic=7208";

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
