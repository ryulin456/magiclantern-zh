#ifndef _platform_lens_h
#define _platform_lens_h

/* Digic 8 brings new properties:
 * PROP_LENS_STATIC_DATA  = PROP_LENS + PROP_LENS_NAME + ???
 * PROP_LENS_DYNAMIC_DATA = PROP_LV_LENS + ???
 *
 * Those are quite huge, they size depend on camera.
 * So far we have data from following models:
 *             M50   SX740    R     RP    SX70   250D   850D   R5/R6
 * STATIC     0x138  0x178  0x184  0x184  0x180  0x180  0x1C8  0x1C8
 * DYNAMIC    0x84   0x8C   0x90   0x90   0x90   0x8C   0x90   0x94
 * DryOS ICU   P2     P3     P4     P4     P4     P5     P8     P9
 *
 * A lot of PROP_LENS_STATIC_DATA can be decoded via `readid` evshell function.
 * Some can be decoded via ShootingInfoEx (models like SX70 don't have readid...)
 *
 * Structs seems to have `packed` attribute set, thus fields moving left and
 * right between models. For easier debugging I left those paddings filled in.
 */
struct prop_lens_static_data
{
        uint8_t                 attached;
        uint8_t                 type;                          // 90 EF, 91 RF
        uint8_t                 LENSID[38];                    // LENSID
        uint16_t                lens_id;                       // and yes, this one is lens_id...
        uint16_t                lens_id_ext;
        uint16_t                fl_wide;
        uint16_t                fl_tele;
        uint8_t                 lens_serial[5];
        uint8_t                _unk_01[24];                    // Not referenced in readid
        uint8_t                 extender_id[6];
        uint8_t                 lens_firm_ver[3];
        uint8_t                 field_vision;
        uint8_t                 lens_type;
        uint8_t                 lens_name_len;
        char                    lens_name[73];
        uint8_t                _unk_02;                        // Not referenced in readid
        uint8_t                 mount_size;
        uint8_t                 lens_switch_exists;
        uint8_t                _unk_03[2];
        uint8_t                 lens_is_switch_exists;
        uint8_t                 lens_is_funct_exists;
        uint8_t                 af_speed_setting_available;
        uint8_t                 dafLimitFno;
        uint8_t                 distortionCorrectionInfo;
        uint8_t                 bcfInfo;
        uint8_t                 lens_id_l292;
        uint8_t                 emd_hot_limit;
        uint8_t                _unk_04;                        // Not referenced in readid
        uint16_t                zoom_pos_size;
        uint16_t                focus_pos_size;
        uint16_t                fine_focus_size;
        uint8_t                _unk_05;
        uint8_t                 av_stop_div;
        uint8_t                 av_slow_enable;
        uint8_t                 av_dlp_lens;
        uint16_t                av_dlp_max_spd;
        uint16_t                av_dlp_silent_spd;
        uint16_t                av_dlp_min_spd;
        uint8_t                _unk_06[149];                   // Not referenced in readid
        uint8_t                 colorBalance;
        uint16_t                os_shift_max;
        uint8_t                 pza_exists;
        uint8_t                 pza_id[5];
        uint8_t                 pza_firm_ver[3];
        uint8_t                 pza_firmup;
        uint8_t                 dlAdp_count;
        uint8_t                _unk_07;                        // Not referenced in readid
        uint32_t                dlAdp1_id;
        uint8_t                 dlAdp1_func1;
        uint8_t                 dlAdp1_firm_ver[3];
        uint32_t                dlAdp2_id;
        uint8_t                 dlAdp2_func1;
        uint8_t                 dlAdp2_firm_ver[3];
        uint8_t                 safemode_info;                 // named only on M50
        uint8_t                 demandWarnDispFromLens;
        uint8_t                 demandWarnDispFromAdp;
        uint8_t                _unk_08[13];                    // Not referenced in readid
};

SIZE_CHECK_STRUCT( prop_lens_static_data, 0x184);

struct prop_lens_dynamic_data {
        uint16_t                AVEF;             // ShootingInfoEx: avef
        uint16_t                AVO;              // ShootingInfoEx: avo
        uint16_t                AVMAX;            // ShootingInfoEx: avmax
        uint16_t                AVD;              // Not referenced in M50
        uint16_t                NowAvEF;          // Not referenced in M50. Before 850D named just NowAv
        uint8_t                _pad_01[22];       // M50, R, RP, 250D, 850D, R6
        uint16_t                jsstep;
        uint8_t                _pad_02[4];        // M50, R, RP, 250D, 850D, R6
        uint16_t                IDC;
        uint16_t                po;
        uint16_t                po05;
        uint16_t                po10;
        uint16_t                po00;
        uint16_t                po15;
        uint16_t                Npo;
        uint16_t                po0;
        uint16_t                po25;
        uint16_t                po50;
        uint16_t                po75;
        uint16_t                po100;            // po100+ not referenced on 850D
        uint16_t                po125;
        uint16_t                po150;
        uint16_t                po175;
        uint16_t                po200;
        uint8_t                 v0;               // vignetting
        uint8_t                 v1;
        uint8_t                 v2;
        uint8_t                 v3;
        uint16_t                FL;
        uint16_t                focal_len_image;  // ShootingInfoEx R6, R
        uint16_t                focus_far;        // ShootingInfoEx: abs_inf; FarAbs
        uint16_t                focus_near;       // ShootingInfoEx: abs_near; NearAbs
        uint16_t                zoomPos;          // ShootingInfoEx: zoom_pos
        uint16_t                focusPos;         //
        uint16_t                fineFocusPos;     // ShootingInfoEx: fine_focus_pos
        uint16_t                HighResoZoomPos;  // ShootingInfoEx: high_res_zoom_pos
        uint16_t                HighResoFocusPos; // ShootingInfoEx: high_res_focus_pos
        uint8_t                 st1;
        uint8_t                 st2;              // & 0x80 true -> MF, false -> AF
        uint8_t                 st3;              // & 0x0F looks equv to PROP_LV_LENS_STABILIZE
        uint8_t                 st4;
        uint8_t                 st5;
        uint8_t                _pad_04[3];        // M50, R, RP, 250D, 850D, R6
        uint8_t                 FcsSt1;           // FocusStep?
        uint8_t                 FcsSt2;           // FcsSt* are unnamed on M50
        uint8_t                 FcsSt3;
        uint8_t                 FcsSt4;
        uint8_t                 ZmSt1;            // ZoomStep?
        uint8_t                 ZmSt2;            // ZmSt* are unnamed on M50
        uint8_t                 ZmSt3;
        uint8_t                 ZmSt4;
        uint8_t                _pad_05[4];        // M50, R, RP, 250D, 850D, R6
        uint16_t                ts_shift;         // via ShootingInfoEx
        uint8_t                 ts_tilt;          // via ShootingInfoEx
        uint8_t                 ts_all_revo;      // via ShootingInfoEx
        uint8_t                 ts_ts_revo;       // via ShootingInfoEx
        uint8_t                _pad_06[7];        // M50, R, RP, 250D, 850D, R6
        uint8_t                 LENSEr;           // not mentioned on 850D
        uint8_t                _pad_07[15];       // 850D, 250D, R, RP
};

SIZE_CHECK_STRUCT( prop_lens_dynamic_data, 0x8C);

#endif