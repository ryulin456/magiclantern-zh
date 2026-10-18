// Rather than modify real ML files to support building in this
// unusual test environment (which could be any architecture, etc),
// we provide alternatives here, doing the least amount of work
// required.

#include <stdint.h>
#include <stdio.h>

#include "function_overrides.h"

// globals
int af_mode = 0;
uint32_t camera_model_id = 0;
char camera_model[32] = {0};
char camera_serial[32] = {0};

struct raw_info {
    uint32_t api_version;
    void *buffer;
    
    int32_t height, width, pitch;
    int32_t frame_size;
    int32_t bits_per_pixel;

    int32_t black_level;
    int32_t white_level;
    struct
    {
        int32_t origin[2];
        int32_t size[2];
    } crop;

    struct
    {
        int32_t y1, x1, y2, x2;
    } active_area;

    int32_t exposure_bias[2];
    int32_t cfa_pattern; // stick to 0x02010100 (RGBG) if you can
    int32_t calibration_illuminant1;
    int32_t color_matrix1[18];
    int32_t dynamic_range;
};

struct raw_info raw_info = {
    .api_version = 1,
    .bits_per_pixel = 14,
    .black_level = 1024,
    .white_level = 13000,
    .cfa_pattern = 0x02010100, // Red  Green  Green  Blue
    .calibration_illuminant1 = 1,
    .color_matrix1 = {0},
    .dynamic_range = 1100,
};

struct raw_capture_info {
    /* sensor attributes: resolution, crop factor */
    uint16_t sensor_res_x;  /* sensor resolution */
    uint16_t sensor_res_y;  /* 2-3 GPixel cameras anytime soon? (to overflow this) */
    uint16_t sensor_crop;   /* sensor crop factor x100 */
    uint16_t reserved;      /* reserved for future use */

    /* video mode attributes */
    /* (how the sensor is configured for image capture) */
    /* subsampling factor: (binning_x+skipping_x) x (binning_y+skipping_y) */
    uint8_t  binning_x;     /* 3 (1080p and 720p); 1 (crop, zoom) */
    uint8_t  skipping_x;    /* so far, 0 everywhere */
    uint8_t  binning_y;     /* 1 (most cameras in 1080/720p; also all crop modes); 3 (5D3 1080p); 5 (5D3 720p) */
    uint8_t  skipping_y;    /* 2 (most cameras in 1080p); 4 (most cameras in 720p); 0 (5D3) */
    int16_t  offset_x;      /* crop offset (top-left active pixel) - optional (SHRT_MIN if unknown) */
    int16_t  offset_y;      /* relative to top-left active pixel from a full-res image (FRSP or CR2) */
};

struct raw_capture_info raw_capture_info = {
    .sensor_res_x = 4000,
    .sensor_res_y = 3000,
    .sensor_crop = 100,
    .binning_x = 1,
    .binning_y = 1,
    .skipping_x = 0,
    .skipping_y = 0,
    .offset_x = 0,
    .offset_y = 0
};

struct lens_info
{
    void    *token;
    char     name[ 32 ];
    unsigned focal_len; // in mm
    unsigned focus_dist; // in cm
    unsigned IS; // PROP_LV_LENS_STABILIZE
    unsigned aperture;
    int      ae;        // exposure compensation, in 1/8 EV steps, signed
    unsigned shutter;
    unsigned iso;
    unsigned iso_auto;
    unsigned iso_analog_raw;
    int      iso_digital_ev;
    unsigned iso_equiv_raw;
    unsigned hyperfocal; // in mm
    unsigned dof_near; // in mm
    unsigned dof_far; // in mm
    unsigned job_state; // see PROP_LAST_JOB_STATE

    unsigned wb_mode;  // see property.h for possible values
    unsigned kelvin;   // wb temperature; only used when wb_mode = WB_KELVIN
    unsigned WBGain_R; // only used when wb_mode = WB_CUSTOM
    unsigned WBGain_G; // only used when wb_mode = WB_CUSTOM
    unsigned WBGain_B; // only used when wb_mode = WB_CUSTOM
    int8_t   wbs_gm;
    int8_t   wbs_ba;

    unsigned picstyle; // 1 ... 9: std, portrait, landscape, neutral, faithful, monochrome, user 1, user 2, user 3

    // raw e in 1/8 EV steps
    uint8_t  raw_aperture;
    uint8_t  raw_shutter;
    uint8_t  raw_iso;
    uint8_t  raw_iso_auto;
    uint8_t  raw_picstyle;           /* fixme: move it out */
    uint8_t  raw_aperture_min;
    uint8_t  raw_aperture_max;
    int      flash_ae;
    uint16_t lens_id;
    uint16_t dof_flags;
    int      dof_diffraction_blur;   /* fixme: move those near other DOF fields on next API update */
    //~ floa     lens_rotation;
    //~ floa     lens_step;
    int      focus_pos;              /* fine steps, starts at 0, range is lens-dependent,
                                      * only updates when motor moves (will lose position during MF) */

    /* those from PROP_LENS property */
    uint8_t  lens_exists;
    uint16_t lens_focal_min;
    uint16_t lens_focal_max;
    uint8_t  lens_extender;
    uint8_t  lens_capabilities;
    uint32_t lens_version;
    uint64_t lens_serial;
};

struct lens_info lens_info = {0};

// funcs
uint64_t get_us_clock(void)
{
    static uint64_t time = 0x1122334455667787;
    time++;

    return time;
}

void LoadCalendarFromRTC(struct tm *tm_out)
{
    struct tm tm = {0};
    *tm_out = tm;
}

int get_current_shutter_reciprocal_x1000(void)
{
    return 1;
}

uint32_t FIO_WriteFile(FILE *fp, void *ptr, uint32_t count)
{
    if (fp == NULL)
        return 0;

    return fwrite(ptr, count, 1, fp);
}
