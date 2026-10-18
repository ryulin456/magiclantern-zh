static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "Raw video v2.0 (MLV)";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Authors";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "g3gg0, a1ex";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "Records 14-bit RAW video";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Forum";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = "http://www.magiclantern.fm/forum/index.php?topic=7122.0";
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = "Records 14-bit RAW video in MLV format (with metadata).\n\nYou may want to load mlv_snd for recording sound and mlv_play for reviewing clips.\n\nFor details, please see the \"Raw Video\" section on the Magic Lantern forum.";

#define MODULE_STRINGS() \
  MODULE_STRINGS_START() \
  MODULE_STRING(__module_string_a_name, __module_string_a_value) \
  MODULE_STRING(__module_string_b_name, __module_string_b_value) \
  MODULE_STRING(__module_string_c_name, __module_string_c_value) \
  MODULE_STRING(__module_string_d_name, __module_string_d_value) \
  MODULE_STRING(__module_string_e_name, __module_string_e_value) \
  MODULE_STRING(__module_string_f_name, __module_string_f_value) \
  MODULE_STRINGS_END()
