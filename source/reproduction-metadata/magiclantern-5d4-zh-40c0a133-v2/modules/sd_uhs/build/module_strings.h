static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "SD overclocking";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Author";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "a1ex, Danne, theBilalFkahouri";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Forum";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "http://www.magiclantern.fm/forum/index.php?topic=12862.0";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = 
    "Change SD UHS access speeds, partly via accessing the SD\n"
    "device directly, partly via patching ROM used for normal DryOS\n"
    "configuration of the device.\n"
    "\n"
    "This is not well understood, mostly empirically determined. There\n"
    "is definite risk. Field tests suggest it is not very risky in practice;\n"
    "very few reports of damaged cards, some reports of lost data.\n"
    "Problems are more likely to occur at higher speeds. Some cards\n"
    "tolerate higher speeds much better than others.\n"
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
