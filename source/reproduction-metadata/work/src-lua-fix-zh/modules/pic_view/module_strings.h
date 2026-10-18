static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "Picture Viewer";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Author";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "a1ex";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "Image viewer that integrates with the file manager.";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = "Simple image viewer that integrates with ML file manager (file_man.mo).\n\nUsage: browse the file system with the file manager, select an image file and choose View.\n\nSupported file types:\n\n* PPM (ML screenshots)\n* DNG (silent pictures, raw video frames)\n* BMP (bitmap images with DryOS palette, uncompressed or RLE)\n* BMH (ML help files, same as BMP)\n* 422 (YUV422 images, e.g. dumps from LiveView buffer)";

#define MODULE_STRINGS() \
  MODULE_STRINGS_START() \
  MODULE_STRING(__module_string_a_name, __module_string_a_value) \
  MODULE_STRING(__module_string_b_name, __module_string_b_value) \
  MODULE_STRING(__module_string_c_name, __module_string_c_value) \
  MODULE_STRING(__module_string_d_name, __module_string_d_value) \
  MODULE_STRING(__module_string_e_name, __module_string_e_value) \
  MODULE_STRINGS_END()
