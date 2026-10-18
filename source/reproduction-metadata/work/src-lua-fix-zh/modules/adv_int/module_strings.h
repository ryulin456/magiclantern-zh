static char __module_string_a_name [] MODULE_STRINGS_SECTION = "Name";
static char __module_string_a_value[] MODULE_STRINGS_SECTION = "Advanced Intervalometer";
static char __module_string_b_name [] MODULE_STRINGS_SECTION = "Author";
static char __module_string_b_value[] MODULE_STRINGS_SECTION = "David Milligan";
static char __module_string_c_name [] MODULE_STRINGS_SECTION = "License";
static char __module_string_c_value[] MODULE_STRINGS_SECTION = "GPL 3.0";
static char __module_string_d_name [] MODULE_STRINGS_SECTION = "Summary";
static char __module_string_d_value[] MODULE_STRINGS_SECTION = "Advanced ramping and exposure control for the intervalometer";
static char __module_string_e_name [] MODULE_STRINGS_SECTION = "Forum";
static char __module_string_e_value[] MODULE_STRINGS_SECTION = "http://www.magiclantern.fm/forum/index.php?topic=8431.0";
static char __module_string_f_name [] MODULE_STRINGS_SECTION = "Description";
static char __module_string_f_value[] MODULE_STRINGS_SECTION = "Advanced ramping and exposure control for the intervalometer\n\nCreate keyframes from current camera settings and specify the\nframe at which the keyframe is applied. You will select which\nparameters you would like to set in the keyframe and which you\nwould like to ignore. The module ramps (linearly) the vaules\nof selected parameters from one keyframe to the next while the\nintervalometer is running.\n\nUsage\n\nYou should create an initial keyframe at 1, that has the\nsettings you are going to start with, that way the module\nknows what to ramp from. If you don't do this the module won't\nramp to your first keyframe, it will simply set the values\nwhen it gets there (it doesn't know how to calculate the ramp,\nb/c it doesn't know what the values started as).\n\nExposure ramping\n\nTo ramp basic expo controls (Av, Tv, ISO) set them like normal\nin the canon GUI as if you were taking a picture, then go into\nthe ML menu under 'Shoot' -> 'Intervalometer' ->\n'Advanced Intervalometer' -> 'New Keyframe' and create a new\nkeyframe. Turn on or off to specify which parameters are to be\nincluded in the keyframe. The current values are displayed on\nthe right for you, but you cannot change them from here.\n\nFocus ramping\n\nFor Focus you do specify the offset to focus to from the 'New\nKeyframe' menu. The module will calculate the number of focus\nsteps it need to take each frame to arrive at your offset that\nyou specify. It steps from wherever the lens is focused at the\nstart. Negative values mean focus closer, positve is towards\ninfinity. You need to be in LiveView for the focusing to work\nand make sure there is enough time after the exposure is taken,\nbefore the next one starts for the focusing to take place (I\nrecommend at least 2s, maybe more depending on camera and lens)\n\nMisc notes\n\nYou can ramp the intervalometer period as well, this you also\nspecify from within the new keyframe menu\n\nOnce you have created keyframes you can view them with the 'List\nKeyframes' menu. You can also save your keyframe sequence to\nfile, and then reload them later. If not saved, any keyframes\nyou created will be lost when the camera is turned off.\nKeyframes are saved and loaded from a file called \"SEQ.TXT\" in\nthe ML/SETTINGS directory.\n\nWhen you have created keyframes and are ready to begin, make\nsure the advanced intervalometer is turned on, then turn the\nintervalometer on as usual from the ML Shoot menu.\n\nAutoETTR compatibility\n\nThis module is compatible with AutoETTR so long as you don't try\nto ramp the parameters that AutoETTR is trying to change. You\ncan ramp Av and let AutoETTR take care of Tv and ISO, or you can\nramp Av and Tv and let AuttoETTR only set ISO, or put AutoETTR to\nlink to Canon shutter (then your Tv will basically be setting\nthe slowest Tv parameter of AutoETTR)";

#define MODULE_STRINGS() \
  MODULE_STRINGS_START() \
  MODULE_STRING(__module_string_a_name, __module_string_a_value) \
  MODULE_STRING(__module_string_b_name, __module_string_b_value) \
  MODULE_STRING(__module_string_c_name, __module_string_c_value) \
  MODULE_STRING(__module_string_d_name, __module_string_d_value) \
  MODULE_STRING(__module_string_e_name, __module_string_e_value) \
  MODULE_STRING(__module_string_f_name, __module_string_f_value) \
  MODULE_STRINGS_END()
