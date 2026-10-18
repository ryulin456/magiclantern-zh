/*
 *  EOS M6 Mark II consts
 */

#define CARD_LED_ADDRESS            0xD01300D4   /* 0xD01300D8/DC: (no errors, no obvious reaction) */
#define LEDON                       0xD0002
#define LEDOFF                      0xC0003

#define CANON_ORIG_MMU_TABLE_ADDR 0xe0000000

#define BR_DCACHE_CLN_1     0xe0040068   /* first call to dcache_clean, before cstart */
#define BR_ICACHE_INV_1     0xe0040072   /* first call to icache_invalidate, before cstart */
#define BR_DCACHE_CLN_2     0xe00400a0   /* second call to dcache_clean, before cstart */
#define BR_ICACHE_INV_2     0xe00400aa   /* second call to icache_invalidate, before cstart */
#define BR_BR_CSTART        0xe00400c0   // indirect branch to cstart; the first branch is
                                         // absolute to original, we must patch
#define BR_BZERO32          0xe0040152   /* called from cstart */
#define BR_CREATE_ITASK     0xe00401b4   /* called from cstart */

// Used for copying and modifying ROM code before transferring control.
// Approximately: look at BR_ macros for the highest address, subtract MAIN_FIRMWARE_ADDR,
// align up.  This may not be exactly enough.  See boot-d678.c for longer explanation.
#define FIRMWARE_ENTRY_LEN 0x228

#define DRYOS_ASSERT_HANDLER  0x4000    //from debug_asset function, hard to miss
#define DRYOS_SGI_HANDLERS_PTR 0x402c // holds pointer to base of SGI handlers (each is 8 bytes, a pointer and something else)

#define MALLOC_STRUCT_ADDR    0x2d99c   // via meminfo -m

#define CANON_SHUTTER_RATING 100000

/**
IDLEHandler seems to be a goldmine of id -> name mapping... just a part of those
2002 START_PLAY_MODE
2003 START_MENU_MODE
2004 START_RTCSET_MODE

200F START_MENU_STROBO_MODE
2010 GUIMODE_MENU_MOVIE_SIZE

2014 START_MENU_FUTURE_SPECIAL_ADP
2015 missing
2016 GUIMODE_USB_ERR
2017 START_MENU_ISO_MODE
2018 START_SERVICE_AFADJUST
2019 START_SERVICE_MENU
201A START_MENU_DATE_TIME_ZONE
201B START_MENU_STROBO_SETTINGS_MODE
201C START_MENU_STROBO_SETTING_LV_MODE
201D START_MENU_GPS_FUNC_MODE
201E START_MENU_SAFEMODE_LENS
201F START_MENU_SAFEMODE_STROBO
2020 START_MENU_SAFEMODE_BATTERY_GRIP
2021 START_MENU_NETWORK_ERROR
2022 START_MENU_NETWORK_ERROR_LIVESTREAMING
2023 START_MENU_CMST_USB
2024 START_MENU_SMARTPHONE_BLE
2025 START_MENU_WIFI_MODE
2026 START_MENU_SMARTPHONE_BTC
2027 START_PLAYMENU_HINT_GUIDE
2028 START_PLAYMENU_SPECIAL_SCENE

202F START_MENU_FOLDER
2030 START_MENU_MECHALESS_SETTING

2033 START_MENU_LIVESTREAMING_DISCONNECT

3039 START_INFO_MODE

303A START_WARNING_SW1OFF

303C START_WARNING_DISABLE_LV

3046 START_WARNING_PANNING_ASSIST_UNSUPPORT_LENS
3047 START_WARNING_LOCK_BUTTON

5036 START_QR_ERASE_MODE

7038 START_QR_MAGNIFY_MODE

C048 Q menu
9081 Flash settings
9082 ISO menu
8484 shot priority

*/
/**
 More fun, GUIRQMODE (via GUI_ChangeMode_Post):
 0x8 GUIRQMODE_LV
 0xA GUIRQMODE_OLC
 */
#define CURRENT_GUI_MODE   (*(int*)0x8484) // via SetGUIRequestMode
#define GUIMODE_PLAY 0x2002
#define GUIMODE_MENU 0x2003
#define GUIMODE_ML_MENU (lv ? 0xc048 : GUIMODE_MENU) // Use Q mode dialog when in LV.  You want to find a dialog
                                                     // that is available in every different LV sub-mode (e.g. Av, P, A+).
                                                     // If you get this wrong, ML can think cam is not in LV,
                                                     // and will restrict features visible in ML menus.
//#define GUIMODE_ML_MENU (RECORDING ? 0 : lv ? 0x68 : GUIMODE_MENU) // TODO: y

#define GMT_FUNCTABLE               0xe096b554           //from gui_main_task
#define GMT_NFUNCS                  0x7                  //size of table above



#define LV_OVERLAYS_MODE MEM(0x16800)  // via LvInfoToggle_Update()


#define LVAE_STRUCT 0x5167c // First value written in 0x2251b1e
#define CONTROL_BV      (*(uint16_t*)(LVAE_STRUCT+0x28)) // via "lvae_setcontrolbv"
#define CONTROL_BV_TV   (*(uint16_t*)(LVAE_STRUCT+0x3e)) // via "lvae_setcontrolaeparam"
#define CONTROL_BV_AV   (*(uint16_t*)(LVAE_STRUCT+0x40)) // via "lvae_setcontrolaeparam"
#define CONTROL_BV_ISO  (*(uint16_t*)(LVAE_STRUCT+0x42)) // via "lvae_setcontrolaeparam"
#define CONTROL_BV_ZERO (*(uint16_t*)(LVAE_STRUCT+0x44)) // via "lvae_setcontrolaccumh"
#define LVAE_DISP_GAIN  (*(uint16_t*)(LVAE_STRUCT+0x70)) // via "lvae_setdispgain"
#define LVAE_MOV_M_CTRL (*(uint8_t* )(LVAE_STRUCT+0x24)) // via "lvae_setmoviemanualcontrol"

#define LVAE_ISO_STRUCT 0x69fe4
#define LVAE_ISO_MIN    (*(uint8_t* )LVAE_ISO_STRUCT + 0x0E) // via string: ISOMin:%d

      //#define LVAE_ISO_HIS    (*(uint8_t* )(LVAE_STRUCT+0xXX)) // no idea, not referenced in ./src?!
      //#define LVAE_ISO_SPEED  (*(uint8_t* )(LVAE_STRUCT+0xXX))  //WRONG, not sure how to follow

/*
On R DispVram was 0x374, on M6II it is 0x474... nice.

VramState, accidentally those match R.
IDK what's on 4th and 5th line, wasn't able to trigger it... yet?
EC811[1]>VramState
*************** Panel ***************
,,0x9f420000,0x9f814800,0x9fc09000

*************** Evf ***************
,,0x9f420000,0x9f814800,0x9fc09000

*************** Line ***************
,,0x74c10000,0x75fd6800,0x7739d000

***************  ***************
,,0x00000000,0x00000000,0x00000000

*************** FHD ***************
,,0x00000000,0x00000000,0x00000000
*/
// Note that all three regular buffers are in Uncacheable-only region!
#define YUV422_LV_BUFFER_1   0x9F230000 // For CleanHDMI 0x76087000
#define YUV422_LV_BUFFER_2   0x9F624800 // For CleanHDMI 0x7744d800
#define YUV422_LV_BUFFER_3   0x9FA19000 // For CleanHDMI 0x78814000

#define DISPVRAM_STRUCT_PTR *(unsigned int *)0xb090             // DispVram structure
#define DV_DISP_TYPE  *((unsigned int *)(DISPVRAM_STRUCT_PTR + 0xC))   // Display type mask
#define DV_VRAM_LINE  *((unsigned int *)(DISPVRAM_STRUCT_PTR + 0xA4))  // Pointer to LV buffer for HDMI output
#define DV_VRAM_PANEL *((unsigned int *)(DISPVRAM_STRUCT_PTR + 0xAC))  // Pointer to LV buffer for Panel output
#define DV_VRAM_EVF   *((unsigned int *)(DISPVRAM_STRUCT_PTR + 0xB4))  // Pointer to LV buffer for EVF output

/* Hardcoded to Panel for now. It would be easier if we can replace this with a
 * function call that would be put into functon_overrides.c. Then we could just
 * define full structs there instead of playing with pointers */
#define YUV422_LV_BUFFER_DISPLAY_ADDR DV_VRAM_PANEL
#define YUV422_LV_PITCH               736 // Is it 736 or 720? No scalling on XCM but UI is 720, OutputChunk 736
#define YUV422_HD_BUFFER_DMA_ADDR     0x0 // TODO: Fix it, null pointer!. It expects this to be shamem_read(some_DMA_ADDR)

// At time of writing R uses here "DispOperator_PropertyMasterSetDisplayTurnOffOn (%d)"
// but I already found that our code expects this to be display statobject state
#define _DISPDEV_STRUCT *(unsigned int *)0xb088
#define DISPLAY_STATEOBJ (*(struct state_object **)_DISPDEV_STRUCT + 0x8)    // DispDevSingleScreenState on D6 and up
#define DISPLAY_IS_ON (DISPLAY_STATEOBJ->current_state != 0)

#define INFO_BTN_NAME               "INFO"
#define Q_BTN_NAME                  "Q/SET"


#define NUM_PICSTYLES 11

#define MIN_MSLEEP 11
// TODO: What is GUISTATE?
#define PLAY_MODE (gui_state == GUISTATE_PLAYMENU && CURRENT_GUI_MODE == GUIMODE_PLAY)
#define MENU_MODE (gui_state == GUISTATE_PLAYMENU && CURRENT_GUI_MODE == GUIMODE_MENU)

/* 101AC marks start of WINSYS data structure.
 * Struct itself is easy to confirm as a few fields later there's a harcoded
 * pointer (exists in romcpy itself) to string "Window Instance".
 * As for "dirty bit flag", D6 to early D8 cameras have two flags - one prohibits
 * redraw from dialog_redraw only, 2nd prohibits redraw at all - this is
 * the one that we expected.
 * But then... they finally removed 2nd flag somewhere around M6II...
 * thus routing it to the existing one.
 */
#define WINSYS_BMP_DIRTY_BIT_NEG MEM(0x11ae4+0x14)
//address of XimrContext structure to redraw in FEATURE_VRAM_RGBA
#define XCM_PTR *(unsigned int *)0x78ce8
#define XIMR_CONTEXT ((unsigned int *)(XCM_PTR + 0x10))

// Now it goes to GRAPHICS_ImageZoom*... eg GRAPHICS_ImageZoomDown e053dd0a
// It calls e041744c that returns from 0x1000C... confirmed on camera
// and behaves like 5D3 description.
#define IMGPLAY_ZOOM_LEVEL_ADDR (0x1000C)
#define IMGPLAY_ZOOM_LEVEL_MAX 14
// commented out due to some dependency in af_patterns.c I don't understand
// wants IMGPLAY_ZOOM_POS_DELTA_X/Y and yet 5D3 doesn't define those
//#define IMGPLAY_ZOOM_POS_X MEM(0x10100) // Saved just before ZoomResult CenterX print
//#define IMGPLAY_ZOOM_POS_Y MEM(0x10104)
#define IMGPLAY_ZOOM_POS_X_CENTER 360
#define IMGPLAY_ZOOM_POS_Y_CENTER 240

/* Looking at older models it is "whatever fits" on given model */
#define HALFSHUTTER_PRESSED 0
// Possibly look around e0479db2
// Focusinfo is fn f81bc08c at 5D3 this seems to be the successor
#define FOCUS_CONFIRMATION 0                    // fake bool
#define LV_BOTTOM_BAR_DISPLAYED 0x0             // fake bool
#define AUDIO_MONITORING_HEADPHONES_CONNECTED 0 // fake bool


// For feature "keep ML on format". Requires winsys structs tho.
#define DIALOG_MnCardFormatBegin   (0x20698) // just before StartMnCardFormatBeginApp it is checked to be DialogClass
#define DIALOG_MnCardFormatExecute (0x24e9c) // similar, MnCardFormatexcuteApp; yes - typos in name.
#define FORMAT_BTN       BGMT_INFO
#define FORMAT_BTN_NAME  "[INFO]"
    #define FORMAT_STR_LOC   13     // Resource ID, to be checked

// Things definitely wrong.
// this block all copied from 50D, and probably wrong, though likely safe
#define FASTEST_SHUTTER_SPEED_RAW 160
#define MAX_AE_EV 2
#define FLASH_MAX_EV 3
#define FLASH_MIN_EV -10
#define COLOR_FG_NONLV 80
#define AF_BTN_HALFSHUTTER 0
#define AF_BTN_STAR 2
// another block copied from 50D
#define GUIMODE_WB 5
#define GUIMODE_FOCUS_MODE 9
#define GUIMODE_DRIVE_MODE 8
#define GUIMODE_PICTURE_STYLE 4
#define GUIMODE_Q_UNAVI 0x18
#define GUIMODE_FLASH_AE 0x22
#define GUIMODE_PICQ 6

// all these MVR ones are junk, don't try and record video and they probably don't get used?
#define MVR_190_STRUCT (*(void**)0x1ed8) // look in MVR_Initialize for AllocateMemory call;
                                         // decompile it and see where ret_AllocateMemory is stored.
#define div_maybe(a,b) ((a)/(b))
// see mvrGetBufferUsage, which is not really safe to call => err70
// macros copied from arm-console
#define MVR_BUFFER_USAGE 70 /* obviously wrong, don't try and record video
       // div_maybe(-100*MEM(236 + MVR_190_STRUCT) - \
       // 100*MEM(244 + MVR_190_STRUCT) - 100*MEM(384 + MVR_190_STRUCT) - \
       // 100*MEM(392 + MVR_190_STRUCT) + 100*MEM(240 + MVR_190_STRUCT) + \
       // 100*MEM(248 + MVR_190_STRUCT), \
       // - MEM(236 + MVR_190_STRUCT) - MEM(244 + MVR_190_STRUCT) + \
       // MEM(240 + MVR_190_STRUCT) +  MEM(248 + MVR_190_STRUCT)) */
#define MVR_FRAME_NUMBER (*(int*)(220 + MVR_190_STRUCT))
//#define MVR_LAST_FRAME_SIZE (*(int*)(512 + MVR_752_STRUCT))
#define MVR_BYTES_WRITTEN MEM((212 + MVR_190_STRUCT))
    
