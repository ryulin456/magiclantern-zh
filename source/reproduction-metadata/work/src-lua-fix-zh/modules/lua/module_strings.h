static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "Lua scripting";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Authors";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "dmilligan";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "Lua scripting";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Forum";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = "http://www.magiclantern.fm/forum/index.php?topic=14828.0";
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = "Module for running Lua scripts on the camera.\n\nThe scripting engine looks for scripts (\\*.LUA) in ML/SCRIPTS.\nAny scripts found get \"loaded\" at startup by the scripting engine simply running them.\nTherefore the main body of the script should simply *define* the script's menu and behavior,\nnot actually *execute* anything.\n\nThe scripting engine will maintain your script's global state from run to run,\nso any global variables you declare will persist until the camera is turned off.\n\nAPI documentation: http://davidmilligan.github.io/ml-lua/";

#define MODULE_STRINGS() \
  MODULE_STRINGS_START() \
  MODULE_STRING(__module_string_a_name, __module_string_a_value) \
  MODULE_STRING(__module_string_b_name, __module_string_b_value) \
  MODULE_STRING(__module_string_c_name, __module_string_c_value) \
  MODULE_STRING(__module_string_d_name, __module_string_d_value) \
  MODULE_STRING(__module_string_e_name, __module_string_e_value) \
  MODULE_STRING(__module_string_f_name, __module_string_f_value) \
  MODULE_STRINGS_END()
