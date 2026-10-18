static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "Silent Pictures";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Author";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "a1ex";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "Take pictures in LiveView without shutter actuation";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Forum";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = "http://www.magiclantern.fm/forum/index.php?topic=5240.0";
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = 
    "Take pictures in LiveView without shutter actuations. Format:\n"
    "14-bit DNG, low-resolution, depending on LiveView mode (1080p,\n"
    "720p, zoom and so on).\n"
    "\n"
    "You may want to load pic_view & file_man for reviewing pictures.\n"
    "\n"
    "Modes:\n"
    "\n"
    " *  Simple: press the shutter halfway to take a picture.\n"
    "\n"
    " *  Burst: take pictures until memory gets full, then save to card.\n"
    "\n"
    " *  End Trigger: take pics continuously, save last few pics to card.\n"
    "\n"
    " *  Best Shots: take pics continuously, save the best (focused) pics.\n"
    "\n"
    " *  Slit-Scan: distorted pictures for funky effects.\n"
    "\n"
;
static char __module_string_g_name [] MODULE_STRINGS_SECTION = "Build date";
static char __module_string_g_value[] MODULE_STRINGS_SECTION = "2026-09-01 07:36:18 UTC";
static char __module_string_h_name [] MODULE_STRINGS_SECTION = "Build user";
static char __module_string_h_value[] MODULE_STRINGS_SECTION = 
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
    MODULE_STRING(__module_string_h_name, __module_string_h_value) \
  MODULE_STRINGS_END()
