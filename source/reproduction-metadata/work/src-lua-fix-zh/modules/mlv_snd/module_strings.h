static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "MLV Sound Support";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Author";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "g3gg0";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "Adds sound recording functionality to mlv_rec";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Website";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = "http://www.magiclantern.fm/";
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = "With this module, mlv_rec is extended by sound recording support.";

#define MODULE_STRINGS() \
  MODULE_STRINGS_START() \
  MODULE_STRING(__module_string_a_name, __module_string_a_value) \
  MODULE_STRING(__module_string_b_name, __module_string_b_value) \
  MODULE_STRING(__module_string_c_name, __module_string_c_value) \
  MODULE_STRING(__module_string_d_name, __module_string_d_value) \
  MODULE_STRING(__module_string_e_name, __module_string_e_value) \
  MODULE_STRING(__module_string_f_name, __module_string_f_value) \
  MODULE_STRINGS_END()
