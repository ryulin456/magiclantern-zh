#ifndef _cameraspecific_gui_h_
#define _cameraspecific_gui_h_

/* As in M6II 1.1.1 */


#define BGMT_WHEEL_UP                0x00
#define BGMT_WHEEL_DOWN              0x01
#define BGMT_WHEEL_LEFT              0x02
#define BGMT_WHEEL_RIGHT             0x03

/* Two thumb wheels are defined. Shutter wheel is codes 0x8 / 0x9 */
#define BGMT_PRESS_SET               0x0A
#define BGMT_UNPRESS_SET             0x0B

#define BGMT_MENU                    0x0C // unpress 0xD
#define BGMT_INFO                    0x0E // unpress 0xF

#define BGMT_PLAY                    0x12 // I think?
#define BGMT_TRASH                   0x21 // map to M-fn for now

#define BGMT_PRESS_RIGHT             0x3B
#define BGMT_UNPRESS_RIGHT           0x3C
#define BGMT_PRESS_LEFT              0x3D
#define BGMT_UNPRESS_LEFT            0x3E
#define BGMT_PRESS_UP                0x3F
#define BGMT_UNPRESS_UP              0x40
#define BGMT_PRESS_DOWN              0x41
#define BGMT_UNPRESS_DOWN            0x42

#define BGMT_PRESS_HALFSHUTTER       0x9C // unpress 0x9D

// needed for correct shutdown from powersave modes
#define GMT_GUICMD_LOCK_OFF          0xAC // GUICMD_LOCK_OFF
#define GMT_GUICMD_START_AS_CHECK    0xB3 // GUICMD_START_AS_CHECK
#define GMT_OLC_INFO_CHANGED         0xB4 // copyOlcDataToStorage uiCommand(%d)

/* Codes below are WRONG: DNE in R */
#define BGMT_LV                      -0xF3 // MILC, always LV.

#define BGMT_PRESS_ZOOM_IN           -0xF4
#define GMT_GUICMD_OPEN_SLOT_COVER   -0xF5 // N/A on this model
//#define BGMT_UNPRESS_ZOOM_IN         -0xF5
//#define BGMT_PRESS_ZOOM_OUT          -0xF6
//#define BGMT_UNPRESS_ZOOM_OUT        -0xF7
#endif
