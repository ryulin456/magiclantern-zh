static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "Image name preferences";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Authors";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "pravdomil, a1ex";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "Customize Canon's image file naming (IMG_1234.CR2 -> ABCD1234.CR2)";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Forum";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = "http://www.magiclantern.fm/forum/index.php?topic=21111.0";
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = "This module lets you customize image file names:\n\n- custom file prefix (IMG_1234.CR2 -> ABCD1234.CR2)\n- change file number (IMG_1234.CR2 -> IMG_5678.CR2;\n  requires restart)\n- change folder number (DCIM/100CANON -> 123CANON)\n\nYou will find the new options under the Shoot menu.";

#define MODULE_STRINGS() \
  MODULE_STRINGS_START() \
  MODULE_STRING(__module_string_a_name, __module_string_a_value) \
  MODULE_STRING(__module_string_b_name, __module_string_b_value) \
  MODULE_STRING(__module_string_c_name, __module_string_c_value) \
  MODULE_STRING(__module_string_d_name, __module_string_d_value) \
  MODULE_STRING(__module_string_e_name, __module_string_e_value) \
  MODULE_STRING(__module_string_f_name, __module_string_f_value) \
  MODULE_STRINGS_END()
