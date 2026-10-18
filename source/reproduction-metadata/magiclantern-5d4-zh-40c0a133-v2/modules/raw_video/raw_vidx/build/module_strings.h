static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "Raw video test";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Authors";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "names_are_hard";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPLv2";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "Records RAW video";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = 
    "Records RAW video.\n"
    "\n"
    "This is a simplified reimplementation of raw video recording, with\n"
    "very few features.\n"
    "\n"
    "raw_vidX is eXperimental. It's designed to be preserved as a\n"
    "minimal example of raw video, for testing new ports, etc.\n"
    "\n"
    "Symbol names are very likely to conflict with raw_vid module. Do\n"
    "not have both modules on the same cam.\n"
    "\n"
;
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Build date";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = "2026-09-01 07:36:54 UTC";
static char __module_string_g_name [] MODULE_STRINGS_SECTION = "Build user";
static char __module_string_g_value[] MODULE_STRINGS_SECTION = 
    "Administrator@66IHQDCQSXK5FAM\n"
    "\n"
;

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
