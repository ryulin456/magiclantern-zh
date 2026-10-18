static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "Self-testing routines";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Author";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "a1ex";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "Stability tests";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = 
    "Testing routines for ML internals:\n"
    "\n"
    " *  stubs tests\n"
    "\n"
    " *  memory tests\n"
    "\n"
    " *  various burn-in tests\n"
    "\n"
    "Please run these tests every now and then; when they fail,\n"
    "please report a bug.\n"
    "\n"
;
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Build date";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = "2026-08-31 03:12:52 UTC";
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
