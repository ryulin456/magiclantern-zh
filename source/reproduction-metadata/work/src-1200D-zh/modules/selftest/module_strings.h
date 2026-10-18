static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "Self-testing routines";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Author";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "a1ex";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "Stability tests";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = "Testing routines for ML internals:\n\n- stubs tests\n- memory tests\n- various burn-in tests\n\nPlease run these tests every now and then; when they fail, please report a bug.";

#define MODULE_STRINGS() \
  MODULE_STRINGS_START() \
  MODULE_STRING(__module_string_a_name, __module_string_a_value) \
  MODULE_STRING(__module_string_b_name, __module_string_b_value) \
  MODULE_STRING(__module_string_c_name, __module_string_c_value) \
  MODULE_STRING(__module_string_d_name, __module_string_d_value) \
  MODULE_STRING(__module_string_e_name, __module_string_e_value) \
  MODULE_STRINGS_END()
