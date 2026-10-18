static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "Arkanoid game";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Author";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "pravdomil.cz";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "Arkanoid game";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Forum";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = "http://www.magiclantern.fm/forum/index.php?topic=7847.0";
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = "A stripped-down version of the old 1980s brick breaker-style game \"Arkanoid\". Use LEFT and RIGHT keys (or joystick) to play.\n\nArkanoid is an old arcade game developed in 1986 in the spirit of Atari's Breakout game from the 1970s. The game is pretty straight forward: you have a paddle (it's actually your ship) and you have to keep a ball from falling off the screen by bouncing it up and destroying the blocks in your way (from PetaPixel).\n\nTip: with an actor in the scene on hard difficulty (for example whilst playing during a paid shoot) the game becomes much more challenging but no less enjoyable (from EOSHD).";

#define MODULE_STRINGS() \
  MODULE_STRINGS_START() \
  MODULE_STRING(__module_string_a_name, __module_string_a_value) \
  MODULE_STRING(__module_string_b_name, __module_string_b_value) \
  MODULE_STRING(__module_string_c_name, __module_string_c_value) \
  MODULE_STRING(__module_string_d_name, __module_string_d_value) \
  MODULE_STRING(__module_string_e_name, __module_string_e_value) \
  MODULE_STRING(__module_string_f_name, __module_string_f_value) \
  MODULE_STRINGS_END()
