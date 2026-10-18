static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Author";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "a1ex";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "Dynamic range improvement by using two ISOs in one picture";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Forum";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = "http://www.magiclantern.fm/forum/index.php?topic=7139.0";
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = "Increases dynamic range by sampling the sensor at two different ISOs, switching ISO for every other line pair.\nThis trick cleans up shadow noise, resulting in a dynamic range improvement of around 3 stops,\nat the cost of reduced vertical resolution, aliasing and moire.\n\nWorks for both raw photos (CR2) and raw videos (DNG). You need to postprocess these files with a tool called **cr2hdr**.\n\nAfter postprocessing, you will get a DNG that looks like a dark ISO 100 shot,\nbut you can bring the exposure back up and be delighted by how little noise is present in those recovered shadows.\n\nQuick start\n\n* Start at ISO 100 in Canon menu\n* Expose to the right by changing shutter and aperture\n* If the image is still dark, enable dual ISO\n* Adjust recovery ISO: higher values = cleaner shadows, but more artifacts\n* Try not to go past ISO 1600; you will not see any major improvements,\n  but you will get more interpolation artifacts and hot pixels.\n\nTips and tricks\n\n* Do not use dual ISO for regular scenes that don't require a very high dynamic range.\n* Raw zebras are aware of dual ISO: weak zebras are displayed where only the high ISO is overexposed,\n  strong (solid) zebras are displayed where both ISOs are overexposed.\n* Raw histogram will display only the low-ISO half of the image (since the high-ISO data is used\n  for cleaning up shadow noise).\n* For optimal exposure (minimal noise without clipped highlights), try both dual ISO and ETTR.\n* Do not be afraid of less aggressive settings like 100/400. They are almost as good as 100/1600\n  regarding shadow noise, but with much less aliasing artifacts.\n* Be careful with long exposures, you may get lots of hot pixels.";

#define MODULE_STRINGS() \
  MODULE_STRINGS_START() \
  MODULE_STRING(__module_string_a_name, __module_string_a_value) \
  MODULE_STRING(__module_string_b_name, __module_string_b_value) \
  MODULE_STRING(__module_string_c_name, __module_string_c_value) \
  MODULE_STRING(__module_string_d_name, __module_string_d_value) \
  MODULE_STRING(__module_string_e_name, __module_string_e_value) \
  MODULE_STRING(__module_string_f_name, __module_string_f_value) \
  MODULE_STRINGS_END()
