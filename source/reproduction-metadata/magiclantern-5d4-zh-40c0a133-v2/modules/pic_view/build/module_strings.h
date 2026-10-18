static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "Picture Viewer";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Author";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "a1ex";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "Image viewer that integrates with the file manager.";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = 
    "Simple image viewer that integrates with ML file manager\n"
    "(file_man.mo).\n"
    "\n"
    "Usage: browse the file system with the file manager, select an\n"
    "image file and choose View.\n"
    "\n"
    "Supported file types:\n"
    "\n"
    " *  PPM (ML screenshots)\n"
    "\n"
    " *  DNG (silent pictures, raw video frames)\n"
    "\n"
    " *  BMP (bitmap images with DryOS palette, uncompressed or RLE)\n"
    "\n"
    " *  BMH (ML help files, same as BMP)\n"
    "\n"
    " *  422 (YUV422 images, e.g. dumps from LiveView buffer)\n"
    "\n"
;
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Build date";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = "2026-09-01 07:36:09 UTC";
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
