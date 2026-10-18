/*
 * Copyright (C) 2025 Magic Lantern Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

// This is a library for creating MLV 3.0 files.  As used by ML, the intention is
// these should be binary compatible with MLV 2.0 files.  I'm bumping the major version
// because the code has changed a lot, and I'm removing parts of 2.0 that appear unused.
// With a changed version, consumers have a chance of spotting any mistakes I might make.

// It's early days and I'm not sure on what things should be exposed.
// There's some tension between using it for recording MLVs live on cam
// (session based, see start_mlv_session()), and using it as a pure library
// for programmatically working with MLV files.  Possibly these responsibilities
// should be split out, but for now, we only use it for recording MLVs.

// It is the responsibility of the caller to manage sessions as required,
// knowing they are not thread safe.

#include <stdint.h>
#include <string.h>

#include "dryos.h"
#include "timer.h"

// FIXME: mostly, or entirely, included for global variables, that we don't
// want to use in here, since we want to work as a library.
// Global uses should be factored out, passed in somehow.
#include "raw.h"
#include "lens.h"
#include "fps.h"

#include "raw_vid.h" // FIXME only for debug logging, probably move the SEND macro to main log.h
#include "mlv_3.h"

#define MLV_VERSION_STRING "v3.0"
#define MLV_VIDEO_CLASS_RAW          0x01
#define MLV_VIDEO_CLASS_YUV          0x02
#define MLV_VIDEO_CLASS_JPEG         0x03
#define MLV_VIDEO_CLASS_H264         0x04

#define MLV_VIDEO_CLASS_FLAG_LZMA    0x80
#define MLV_VIDEO_CLASS_FLAG_DELTA   0x40
#define MLV_VIDEO_CLASS_FLAG_LJ92    0x20

#define MLV_AUDIO_CLASS_FLAG_LZMA    0x80

#define MLV_FRAME_UNSPECIFIED 0
#define MLV_FRAME_VIDF        1
#define MLV_FRAME_AUDF        2

#pragma pack(push,1)

// An MLV file consists of a series of variable length blocks.
// Each block has a four char type field as the first member,
// and the total block size as the second member.

// MLV v2 used 4 char arrays for type.  v3 uses enums holding equivalent values.
// This gives us proper type checking, simplifies block generation code,
// as well as avoiding potential typos on names, and some usage of string
// functions for assignment and/or comparison.
//
// It should be backwards compatible with v2 for parsing MLV files.
//
// In theory it could lead to endianness problems compared to char array,
// but a) all our targets are little-endian, b) if that does change we would have
// worse problems with all the other fields in v2 too, including block_size.
enum mlv_block_type
{
    mlv_MLVI = 0x49564c4d,
    mlv_VIDF = 0x46444956,
    mlv_AUDF = 0x46445541,
    mlv_RAWI = 0x49574152,
    mlv_RAWC = 0x43574152,
    mlv_WAVI = 0x49564157,
    mlv_EXPO = 0x4f505845,
    mlv_LENS = 0x534e454c,
    mlv_RTCI = 0x49435452,
    mlv_IDNT = 0x544e4449,
    mlv_XREF = 0x46455258,
    mlv_INFO = 0x4f464e49,
    mlv_DISO = 0x4f534944,
    mlv_MARK = 0x4b52414d,
    mlv_STYL = 0x4c595453,
    mlv_ELVL = 0x4c564c45,
    mlv_WBAL = 0x4c414257,
    mlv_DEBG = 0x47424544,
    mlv_VERS = 0x53524556,
    mlv_UNUSED = 0 // never use this, it's for detecting poorly initialised zeroed blocks
};

// Shared header for all MLV blocks, used for passing them to
// functions by casting to this type.
typedef struct mlv_block_header {
    enum mlv_block_type type;
    uint32_t    size;
} mlv_block_header;
SIZE_CHECK_STRUCT(mlv_block_header, 0x8);

typedef struct mlv_file_block {
    enum mlv_block_type type;  /* MLVI: Magic Lantern Video file header */
    uint32_t    size;          /* size of the whole header */
    char        version_string[8];   /* null-terminated C-string of the exact revision of this format */
    uint64_t    file_GUID;           /* UID of the file (group) generated using hw counter, time of day and PRNG */
    uint16_t    file_num;            /* the ID within file_count this file has (0 to file_count - 1) */
    uint16_t    file_count;          /* how many files belong to this group (splitting or parallel) */
    uint32_t    file_flags;          /* 1=out-of-order data, 2=dropped frames, 4=single image mode, 8=stopped due to error */
    uint16_t    video_class;         /* 0=none, 1=RAW, 2=YUV, 3=JPEG, 4=H.264 */
    uint16_t    audio_class;         /* 0=none, 1=WAV */
    uint32_t    video_frame_count;    /* number of video frames in this file. set to 0 on start, updated when finished. */
    uint32_t    audio_frame_count;    /* number of audio frames in this file. set to 0 on start, updated when finished. */
    uint32_t    source_fps_nom;       /* configured fps in 1/s multiplied by source_fps_denom */
    uint32_t    source_fps_denom;     /* denominator for fps. usually set to 1000, but may be 1001 for NTSC */
} mlv_file_block;
SIZE_CHECK_STRUCT(mlv_file_block, 0x34);

typedef struct mlv_vidf_block {
    enum mlv_block_type type;  /* VIDF: this block contains one frame of video data */
    uint32_t    size;          /* size of this struct + padding + image data (padding is part of frame_data) */
    uint64_t    timestamp;          /* hardware counter timestamp for this frame (relative to recording start) */
    uint32_t    frame_number;        /* unique video frame number */
    uint16_t    crop_offset_x;           /* specifies from which sensor row/col the video frame was copied (8x2 blocks) */
    uint16_t    crop_offset_y;           /* (can be used to process dead/hot pixels) */
    uint16_t    pan_offset_x;            /* specifies the panning offset which is crop_pos, but with higher resolution (1x1 blocks) */
    uint16_t    pan_offset_y;            /* (it's the frame area from sensor the user wants to see) */
    uint32_t    frame_padding;         /* size of dummy data before frame_data starts, necessary for EDMAC alignment */
    uint8_t     frame_data[]; // the first frame_padding bytes are not part of the image
} mlv_vidf_block;
SIZE_CHECK_STRUCT(mlv_vidf_block, 0x20);

typedef struct mlv_audf_block {
    enum mlv_block_type type;  /* AUDF: this block contains audio data */
    uint32_t    size;          /* total frame size */
    uint64_t    timestamp;          /* hardware counter timestamp for this frame (relative to recording start) */
    uint32_t    frame_number;        /* unique audio frame number */
    uint32_t    frame_padding;         /* size of dummy data before frame_data starts, necessary for EDMAC alignment */
    uint8_t     frame_data[];
} mlv_audf_block;
SIZE_CHECK_STRUCT(mlv_audf_block, 0x18);

typedef struct mlv_rawi_block {
    enum mlv_block_type type;  /* RAWI: when video_class is RAW, this block will contain detailed format information */
    uint32_t    size;          /* total frame size */
    uint64_t    timestamp;          /* hardware counter timestamp for this frame (relative to recording start) */
    uint16_t    res_x;               /* Configured video resolution, may differ from payload resolution */
    uint16_t    res_y;               /* Configured video resolution, may differ from payload resolution */
    
    // FIXME maybe - do we want this ugly dep on raw.h?
    // Possibly we should have our own mlv_raw_info struct, which raw.c could populate?
    // This code cares about MLV files, not ML internals.
    raw_info_t  raw_info;           /* the raw_info structure delivered by raw.c of ML Core */
} mlv_rawi_block;
SIZE_CHECK_STRUCT(mlv_rawi_block, 0xb4);

typedef struct mlv_rawc_block {
    enum mlv_block_type type;  /* RAWC: raw image capture information */
    uint32_t    size;          /* sizeof(mlv_rawc_block) */
    uint64_t    timestamp;          /* hardware counter timestamp */

    /* see struct raw_capture_info from raw.h */

    /* sensor attributes: resolution, crop factor */
    uint16_t sensor_res_x;          /* sensor resolution */
    uint16_t sensor_res_y;          /* 2-3 GPixel cameras anytime soon? (to overflow this) */
    uint16_t sensor_crop;           /* sensor crop factor x100 */
    uint16_t reserved;              /* reserved for future use */

    /* video mode attributes */
    /* (how the sensor is configured for image capture) */
    /* subsampling factor: (binning_x+skipping_x) x (binning_y+skipping_y) */
    uint8_t  binning_x;             /* 3 (1080p and 720p); 1 (crop, zoom) */
    uint8_t  skipping_x;            /* so far, 0 everywhere */
    uint8_t  binning_y;             /* 1 (most cameras in 1080/720p; also all crop modes); 3 (5D3 1080p); 5 (5D3 720p) */
    uint8_t  skipping_y;            /* 2 (most cameras in 1080p); 4 (most cameras in 720p); 0 (5D3) */
    int16_t  offset_x;              /* crop offset (top-left active pixel) - optional (SHRT_MIN if unknown) */
    int16_t  offset_y;              /* relative to top-left active pixel from a full-res image (FRSP or CR2) */
   
    /* The captured *active* area (raw_info.active_area) will be mapped
     * on a full-res image (which does not use subsampling) as follows:
     *   active_width  = raw_info.active_area.x2 - raw_info.active_area.x1
     *   active_height = raw_info.active_area.y2 - raw_info.active_area.y1
     *   .x1 (left)  : offset_x + full_res.active_area.x1
     *   .y1 (top)   : offset_y + full_res.active_area.y1
     *   .x2 (right) : offset_x + active_width  * (binning_x+skipping_x) + full_res.active_area.x1
     *   .y2 (bottom): offset_y + active_height * (binning_y+skipping_y) + full_res.active_area.y1
     */
} mlv_rawc_block;
SIZE_CHECK_STRUCT(mlv_rawc_block, 0x20);

typedef struct mlv_wavi_block {
    enum mlv_block_type type;  /* WAVI: when audio_class is WAV, this block contains format details  compatible to RIFF */
    uint32_t    size;          /* total frame size */
    uint64_t    timestamp;          /* hardware counter timestamp for this frame (relative to recording start) */
    uint16_t    format;             /* 1=Integer PCM, 6=alaw, 7=mulaw */
    uint16_t    channels;           /* audio channel count: 1=mono, 2=stereo */
    uint32_t    sampling_rate;       /* audio sampling rate in 1/s */
    uint32_t    bytes_per_second;     /* audio data rate */
    uint16_t    block_align;         /* see RIFF WAV hdr description */
    uint16_t    bits_per_sample;      /* audio ADC resolution */
} mlv_wavi_block;
SIZE_CHECK_STRUCT(mlv_wavi_block, 0x20);

typedef struct mlv_expo_block {
    enum mlv_block_type type;  /* EXPO: exposure information (ISO, shutter) */
    uint32_t    size;          /* total frame size */
    uint64_t    timestamp;          /* hardware counter timestamp for this frame (relative to recording start) */
    uint32_t    iso_mode;            /* 0=manual, 1=auto */
    uint32_t    iso_value;           /* camera delivered ISO value */
    uint32_t    iso_analog;          /* ISO obtained by hardware amplification (most full-stop ISOs, except extreme values) */
    uint32_t    digital_gain;        /* digital ISO gain (1024 = 1 EV) - it's not baked in the raw data, so you may want to scale it or adjust the white level */
    uint64_t    shutter_value;       /* exposure time in microseconds */
} mlv_expo_block;
SIZE_CHECK_STRUCT(mlv_expo_block, 0x28);

typedef struct mlv_lens_block {
    enum mlv_block_type type;  /* LENS: lens information (aperture, focal len, focus distance, lens name...) */
    uint32_t    size;          /* total frame size */
    uint64_t    timestamp;          /* hardware counter timestamp for this frame (relative to recording start) */
    uint16_t    focal_length;        /* in mm */
    uint16_t    focal_dist;          /* in mm (65535 = infinite) */
    uint16_t    aperture;           /* f-number * 100 */
    uint8_t     stabilizer_mode;     /* 0=off, 1=on, (is the new L mode relevant) */
    uint8_t     autofocus_mode;      /* 0=off, 1=on */
    uint32_t    flags;              /* 1=CA avail, 2=Vign avail, ... */
    uint32_t    lens_ID;             /* hexadecimal lens ID (delivered by properties?) */
    char        lens_name[32];       /* full lens string */
    char        lens_serial[32];     /* full lens serial number */
} mlv_lens_block;
SIZE_CHECK_STRUCT(mlv_lens_block, 0x60);

typedef struct mlv_rtci_block {
    enum mlv_block_type type;  /* RTCI: real-time clock metadata */
    uint32_t    size;          /* total frame size */
    uint64_t    timestamp;          /* hardware counter timestamp for this frame (relative to recording start) */
    uint16_t    tm_sec;             /* seconds (0-59) */
    uint16_t    tm_min;             /* minute (0-59) */
    uint16_t    tm_hour;            /* hour (0-23) */
    uint16_t    tm_mday;            /* day of month (1-31) */
    uint16_t    tm_mon;             /* month (0-11) */
    uint16_t    tm_year;            /* year since 1900 */
    uint16_t    tm_wday;            /* day of week */
    uint16_t    tm_yday;            /* day of year */
    uint16_t    tm_isdst;           /* daylight saving */
    uint16_t    tm_gmtoff;          /* GMT offset */
    char        tm_zone[8];         /* time zone string */
} mlv_rtci_block;
SIZE_CHECK_STRUCT(mlv_rtci_block, 0x2c);

typedef struct mlv_idnt_block {
    enum mlv_block_type type;  /* IDNT: camera identification (model, serial number) */
    uint32_t    size;          /* total frame size */
    uint64_t    timestamp;          /* hardware counter timestamp for this frame (relative to recording start) */
    char        camera_name[32];     /* PROP (0x00000002), offset 0, length 32 */
    uint32_t    camera_model;        /* PROP (0x00000002), offset 32, length 4 */
    char        camera_serial[32];   /* Camera serial number (if available) */
} mlv_idnt_block;
SIZE_CHECK_STRUCT(mlv_idnt_block, 0x54);

typedef struct mlv_xref_block {
    enum mlv_block_type type;  /* XREF: can be added in post processing when out of order data is present */
    uint32_t    size;          /* this can also be placed in a separate file with only file header plus this block */
    uint64_t    timestamp;
    uint32_t    frame_type;          /* bitmask: 1=video, 2=audio */
    uint32_t    entry_count;         /* number of xrefs that follow here */
 /* mlv_xref_t  xrefEntries; */     /* this structure refers to the n'th video/audio frame offset in the files */
} mlv_xref_block;
SIZE_CHECK_STRUCT(mlv_xref_block, 0x18);

typedef struct mlv_info_block {
    enum mlv_block_type type;  /* INFO: user definable info string. take number, location, etc. */
    uint32_t    size;
    uint64_t    timestamp;
 /* uint8_t     stringData[variable]; */
} mlv_info_block;
SIZE_CHECK_STRUCT(mlv_info_block, 0x10);

typedef struct mlv_diso_block {
    enum mlv_block_type type;  /* DISO: Dual-ISO information */
    uint32_t    size;
    uint64_t    timestamp;
    uint32_t    dual_mode;           /* bitmask: 0=off, 1=odd lines, 2=even lines, upper bits may be defined later */
    uint32_t    iso_value;
} mlv_diso_block;
SIZE_CHECK_STRUCT(mlv_diso_block, 0x18);

typedef struct mlv_mark_block {
    enum mlv_block_type type;  /* MARK: markers set by user while recording */
    uint32_t    size;
    uint64_t    timestamp;
    uint32_t    mark_type;    /* value may depend on the button being pressed or counts up (t.b.d) */
} mlv_mark_block;
SIZE_CHECK_STRUCT(mlv_mark_block, 0x14);

typedef struct mlv_styl_block {
    enum mlv_block_type type;  /* STYL: picture style information */
    uint32_t    size;
    uint64_t    timestamp;
    uint32_t    pic_style_ID;
    int32_t     contrast;
    int32_t     sharpness;
    int32_t     saturation;
    int32_t     colortone;
    uint8_t     pic_style_name[16];
} mlv_styl_block;
SIZE_CHECK_STRUCT(mlv_styl_block, 0x34);

typedef struct mlv_elvl_block {
    enum mlv_block_type type;  /* ELVL: picture style information */
    uint32_t    size;
    uint64_t    timestamp;
    uint32_t    roll;               /* degrees x100 (here, 45.00 degrees) */
    uint32_t    pitch;              /* 10.00 degrees */
} mlv_elvl_block;
SIZE_CHECK_STRUCT(mlv_elvl_block, 0x18);

typedef struct mlv_wbal_block {
    enum mlv_block_type type;  /* WBAL: White balance info */
    uint32_t    size;
    uint64_t    timestamp;
    uint32_t    wb_mode;            /* WB_AUTO 0, WB_SUNNY 1, WB_SHADE 8, WB_CLOUDY 2, WB_TUNGSTEN 3, WB_FLUORESCENT 4, WB_FLASH 5, WB_CUSTOM 6, WB_KELVIN 9 */
    uint32_t    kelvin;             /* only when wb_mode is WB_KELVIN */
    uint32_t    wbgain_r;           /* only when wb_mode is WB_CUSTOM */
    uint32_t    wbgain_g;           /* 1024 = 1.0 */
    uint32_t    wbgain_b;           /* note: it's 1/canon_gain (uses dcraw convention) */
    uint32_t    wbs_gm;             /* WBShift (no idea how to use these in post) */
    uint32_t    wbs_ba;             /* range: -9...9 */
} mlv_wbal_block;
SIZE_CHECK_STRUCT(mlv_wbal_block, 0x2c);

typedef struct mlv_debg_block {
    enum mlv_block_type type;  /* DEBG: debug messages for development use, contains no production data */
    uint32_t    size;
    uint64_t    timestamp;
    uint32_t    debug_type;               /* debug data type, for now 0 - text log */
    uint32_t    length;             /* to allow that data can be of arbitrary length and blocks are padded to 32 bits, so store real length */
 /* uint8_t     stringData[variable]; */
} mlv_debg_block;
SIZE_CHECK_STRUCT(mlv_debg_block, 0x18);

typedef struct mlv_vers_block {
    enum mlv_block_type type;  /* VERS - Version information block, appears once per module */
    uint32_t    size;
    uint64_t    timestamp;
    uint32_t    length;             /* to allow that data can be of arbitrary length and blocks are padded to 32 bits, so store real length */
 /* uint8_t     stringData[variable];  // Version string, e.g. "ml-core 20130912", "mlv_rec v2.1" or "mlv_lite 0d3fbdaf crop_rec_8k"
                                       // general format "<module_name> <version_information>"
                                       // where <module_name> must not contain spaces whereas <version_information> may be of any characters in UTF-8 format
*/
} mlv_vers_block;
SIZE_CHECK_STRUCT(mlv_vers_block, 0x14);

#pragma pack(pop)

// space for MLV blocks of all used types
//
// We set the type here and nowhere else.  We re-use this storage
// when we need a block of the appropriate type.
// We also rely on the fact this 0 initialises the rest of the struct.
static mlv_file_block file_block = {.type = mlv_MLVI};
static mlv_rawi_block rawi_block = {.type = mlv_RAWI};
static mlv_rawc_block rawc_block = {.type = mlv_RAWC};
static mlv_rtci_block rtci_block = {.type = mlv_RTCI};
static mlv_expo_block expo_block = {.type = mlv_EXPO};
static mlv_lens_block lens_block = {.type = mlv_LENS};
static mlv_idnt_block idnt_block = {.type = mlv_IDNT};
static mlv_wbal_block wbal_block = {.type = mlv_WBAL};
static mlv_vidf_block vidf_block = {.type = mlv_VIDF};

// file-global state for an MLV recording session
static uint64_t session_start_timestamp = 0;
static int session_in_progress = 0;
static int session_fps = 0;
static int session_res_x = 0;
static int session_res_y = 0;
static int session_crop_offset_x = 0;
static int session_crop_offset_y = 0;
static int session_pan_offset_x = 0;
static int session_pan_offset_y = 0;
static uint64_t session_GUID = 0;
static int session_frame_number = 0;
static int session_image_data_alignment = 0;

static uint64_t prng_lfsr(uint64_t value)
{
    uint64_t lfsr = value;
    int max_clocks = 512;

    for(int clocks = 0; clocks < max_clocks; clocks++)
    {
        /* maximum length LFSR according to http://www.xilinx.com/support/documentation/application_notes/xapp052.pdf */
        int bit = ((lfsr >> 63) ^ (lfsr >> 62) ^ (lfsr >> 60) ^ (lfsr >> 59)) & 1;
        lfsr = (lfsr << 1) | bit;
    }

    return lfsr;
}

static uint64_t get_GUID()
{
    struct tm now;
    uint64_t guid = get_us_clock();
    LoadCalendarFromRTC(&now);

    // run through prng to shuffle bits
    guid = prng_lfsr(guid);

    // mix with rtc time
    guid ^= now.tm_sec;
    guid ^= now.tm_min << 7;
    guid ^= now.tm_hour << 12;
    guid ^= now.tm_yday << 17;
    guid ^= now.tm_year << 26;
    guid ^= get_us_clock() << 37;

    return prng_lfsr(guid);
}

// Any block passed in must have a valid initialised type member.
// Everything else will get overwritten.
static int init_block(mlv_block_header *orig_block)
{
    if (orig_block == NULL)
    {
        SEND_LOG_DATA_STR("ERROR: orig_block NULL\n");
        return -1;
    }

    switch (orig_block->type)
    {
    case mlv_MLVI:
        mlv_file_block *block_1 = (mlv_file_block *)orig_block;
        block_1->size = sizeof(mlv_file_block);
        strncpy(block_1->version_string, MLV_VERSION_STRING, sizeof(block_1->version_string));
        block_1->file_GUID = session_GUID;
        block_1->file_num = 0;
        block_1->file_count = 0; //autodetect // FIXME wtf does this mean?  Probably change this
        block_1->file_flags = 4; // "single image mode" FIXME Should perhaps be an enum or define
        block_1->video_class = MLV_VIDEO_CLASS_RAW; // FIXME handle compression (probably worker.c needs to pass in
                                                  // these kind of things?)
        block_1->audio_class = 0;
        block_1->video_frame_count = 0; //autodetect
        block_1->audio_frame_count = 0;
        if (session_fps == 0)
            block_1->source_fps_nom = 1;
        else
            block_1->source_fps_nom = session_fps;
        block_1->source_fps_denom = 1000;
        return 0;

    case mlv_RAWI:
        mlv_rawi_block *block_2 = (mlv_rawi_block *)orig_block;
        block_2->size = sizeof(mlv_rawi_block);
        block_2->timestamp = get_us_clock() - session_start_timestamp;
        block_2->res_x = session_res_x;
        block_2->res_y = session_res_y;
        // FIXME I don't like that this accesses a global.  It could change
        // between triggering getting here and starting this copy.
        // Caller should probably copy and pass the copy somehow:
        block_2->raw_info = raw_info;
        // After copying from the global, MLV 2 updates (or sets??) some fields.
        // Why?  Are these not already populated, in which case maybe they shouldn't
        // be in the global?
        block_2->raw_info.bits_per_pixel = BPP; // FIXME another global to remove
        block_2->raw_info.pitch = block_2->raw_info.width * BPP / 8;
        int black14 = block_2->raw_info.black_level;
        int white14 = block_2->raw_info.white_level;
        int bpp_scaling = (1 << (14 - BPP));
        block_2->raw_info.black_level = (black14 + bpp_scaling / 2) / bpp_scaling;
        block_2->raw_info.white_level = (white14 + bpp_scaling / 2) / bpp_scaling;
        return 0;

    case mlv_RAWC:
        mlv_rawc_block *block_3 = (mlv_rawc_block *)orig_block;
        block_3->size = sizeof(mlv_rawc_block);
        block_3->timestamp = get_us_clock() - session_start_timestamp;
        // FIXME similar to RAWI above, I don't like using this global.
        // We should pass in the values that were valid when we wanted
        // to save this block, not whatever exists now.
        // And maybe we can remove the raw.h dep.
        block_3->sensor_res_x = raw_capture_info.sensor_res_x;
        block_3->sensor_res_y = raw_capture_info.sensor_res_y;
        // block_3->reserved
        block_3->sensor_crop = raw_capture_info.sensor_crop;
        block_3->binning_x = raw_capture_info.binning_x;
        block_3->binning_y = raw_capture_info.binning_y;
        block_3->skipping_x = raw_capture_info.skipping_x;
        block_3->skipping_y = raw_capture_info.skipping_y;
        block_3->offset_x = raw_capture_info.offset_x;
        block_3->offset_y = raw_capture_info.offset_y;
        return 0;

    case mlv_RTCI:
        mlv_rtci_block *block_4 = (mlv_rtci_block *)orig_block;
        block_4->size = sizeof(mlv_rtci_block);
        block_4->timestamp = get_us_clock() - session_start_timestamp;
        struct tm now = {0};
        LoadCalendarFromRTC(&now);
        block_4->tm_sec = now.tm_sec;
        block_4->tm_min = now.tm_min;
        block_4->tm_hour = now.tm_hour;
        block_4->tm_mday = now.tm_mday;
        block_4->tm_mon = now.tm_mon;
        block_4->tm_year = now.tm_year;
        block_4->tm_wday = now.tm_wday;
        block_4->tm_yday = now.tm_yday;
        block_4->tm_isdst = now.tm_isdst;
        block_4->tm_gmtoff = now.tm_gmtoff;
        // block_4->tm_zone // 0 via static initialisation, doesn't really exist
        return 0;

    case mlv_EXPO:
        mlv_expo_block *block_5 = (mlv_expo_block *)orig_block;
        block_5->size = sizeof(mlv_expo_block);
        block_5->timestamp = get_us_clock() - session_start_timestamp;
        // FIXME another global to consider removing from here,
        // lens_info.
        //
        // iso is zero when auto-iso is enabled
        if(lens_info.iso == 0)
        {
            block_5->iso_mode = 1;
            block_5->iso_value = lens_info.iso_auto;
        }
        else
        {
            block_5->iso_mode = 0;
            block_5->iso_value = lens_info.iso;
        }
        block_5->iso_analog = lens_info.iso_analog_raw;
        block_5->digital_gain = lens_info.iso_digital_ev;
        block_5->shutter_value = (uint32_t)(1000.0f * (1000000.0f / (float)get_current_shutter_reciprocal_x1000()));
        return 0;

    case mlv_LENS:
        mlv_lens_block *block_6 = (mlv_lens_block *)orig_block;
        block_6->size = sizeof(mlv_lens_block);
        block_6->timestamp = get_us_clock() - session_start_timestamp;
        block_6->focal_length = lens_info.focal_len;
        block_6->focal_dist = lens_info.focus_dist;
        block_6->aperture = lens_info.aperture * 10;
        block_6->stabilizer_mode = lens_info.IS;
        block_6->autofocus_mode = af_mode;
        block_6->flags = 0;
        block_6->lens_ID = lens_info.lens_id;

        char buf[33];
        snprintf(buf, sizeof(buf), "%X%08X",
                 (uint32_t)(lens_info.lens_serial >> 32),
                 (uint32_t)(lens_info.lens_serial & 0xFFFFFFFF));
        strncpy(block_6->lens_name, lens_info.name, 32);
        strncpy(block_6->lens_serial, buf, 32);
        return 0;

    case mlv_IDNT:
        mlv_idnt_block *block_7 = (mlv_idnt_block *)orig_block;
        block_7->size = sizeof(mlv_idnt_block);
        block_7->timestamp = get_us_clock() - session_start_timestamp;
        // FIXME, another global to remove from this library,
        // camera_model_id and friends.  From propvalues.c.
        block_7->camera_model = camera_model_id;
        memcpy(block_7->camera_name, camera_model, 32);
        memcpy(block_7->camera_serial, camera_serial, 32);
        return 0;

    case mlv_WBAL:
        mlv_wbal_block *block_8 = (mlv_wbal_block *)orig_block;
        block_8->size = sizeof(mlv_wbal_block);
        block_8->timestamp = get_us_clock() - session_start_timestamp;
        block_8->wb_mode = lens_info.wb_mode;
        block_8->kelvin = lens_info.kelvin;
        block_8->wbgain_r = lens_info.WBGain_R;
        block_8->wbgain_g = lens_info.WBGain_G;
        block_8->wbgain_b = lens_info.WBGain_B;
        block_8->wbs_gm = lens_info.wbs_gm;
        block_8->wbs_ba = lens_info.wbs_ba;
        return 0;

    case mlv_VIDF:
    case mlv_AUDF:
    case mlv_WAVI:
    case mlv_XREF:
    case mlv_INFO:
    case mlv_DISO:
    case mlv_MARK:
    case mlv_STYL:
    case mlv_ELVL:
    case mlv_DEBG:
    case mlv_VERS:
        SEND_LOG_DATA_STR("ERROR: unhandled block type\n");
        return -1;
    case mlv_UNUSED:
        // this probably means someone sent a zeroed block
        SEND_LOG_DATA_STR("ERROR: bad type in init_block()\n");
        return -1;
    default:
        // shouldn't happen, unknown type
        SEND_LOG_DATA_STR("ERROR: unknown type in init_block()\n");
        return -1;
    }

}

// Writes the standard set of MLV file headers to the file.
// Returns size of written headers.  0 signifies error.
int write_file_headers(char *start, char *end)
{
    if (start == NULL || end == NULL)
    {
        SEND_LOG_DATA_STR("ERROR: incoming buf ptr was NULL in write_file_headers()\n");
        return 0;
    }

    // Always write an MLVI block.
    if (init_block((mlv_block_header *)&file_block))
        return 0;
    int32_t written = file_block.size;

    // In theory, sometimes write metadata blocks.
    //
    // TODO write_mlv_chunk_headers() takes a chunk param (basically, a file number),
    // and only stores these headers in the first file in a session.
    // Unless "something changes", which is not explained.
    //
    // This will need revisiting when we handle splitting MLV into files,
    // e.g. 4GB limit, card spanning.
    //
    // For now, we always write these, and we don't support multi-file MLVs.

    if (init_block((mlv_block_header *)&rawi_block))
        return 0;
    written += rawi_block.size;

    if (init_block((mlv_block_header *)&rawc_block))
        return 0;
    written += rawc_block.size;

    if (init_block((mlv_block_header *)&idnt_block))
        return 0;
    written += idnt_block.size;

    if (init_block((mlv_block_header *)&expo_block))
        return 0;
    written += expo_block.size;

    if (init_block((mlv_block_header *)&lens_block))
        return 0;
    written += lens_block.size;

    if (init_block((mlv_block_header *)&rtci_block))
        return 0;
    written += rtci_block.size;

    if (init_block((mlv_block_header *)&wbal_block))
        return 0;
    written += wbal_block.size;

    // FIXME do we need this block?  It's complex for something that I've never
    // seen used.  Let's try without and see if mlvapp, mlvdump and ffplay can handle them.
    //    fail |= mlv_write_vers_blocks(f, mlv_start_timestamp);

    if (end - start < written)
    {
        SEND_LOG_DATA_STR("ERROR: not enough space for headers");
        return 0;
    }

    // write the set of headers
    char *cur = start;
    memcpy(cur, &file_block, file_block.size); cur += file_block.size;
    memcpy(cur, &rawi_block, rawi_block.size); cur += rawi_block.size;
    memcpy(cur, &rawc_block, rawc_block.size); cur += rawc_block.size;
    memcpy(cur, &idnt_block, idnt_block.size); cur += idnt_block.size;
    memcpy(cur, &expo_block, expo_block.size); cur += expo_block.size;
    memcpy(cur, &lens_block, lens_block.size); cur += lens_block.size;
    memcpy(cur, &rtci_block, rtci_block.size); cur += rtci_block.size;
    memcpy(cur, &wbal_block, wbal_block.size); cur += wbal_block.size;

    return written;
}

// If there's enough space between start and end to hold the VIDF header,
// padding, and image data of size image_size, write the header and padding
// to start.  In this case, the returned pointer will point to where
// the image data should be written - caller must not write more than
// the size provided.
//
// If an error occurs, perform no writes and return NULL.
//
// This slightly strange looking split is so MLV structures are not exposed
// to the caller, and EDMAC copy functionality is not needed in this library.
// The caller is free to use whatever fast copy is available for the large
// image data.
char *write_video_frame_header(uint32_t image_size, char *start, char *end)
{
    if (start == NULL || end == NULL)
        return NULL;

    char *image_start = (char *)(ALIGN_UP(((uint32_t)start + sizeof(mlv_vidf_block)),
                                          session_image_data_alignment));
    if ((image_start < start)
        || (image_start + image_size < start))
    {   // overflow
        return NULL;
    }
    if (image_start + image_size > end) // doesn't fit
        return NULL;

    // write vidf header to buf_start
    vidf_block.size = (image_start + image_size) - start;
    vidf_block.timestamp = get_us_clock();
    vidf_block.frame_number = session_frame_number;
    vidf_block.crop_offset_x = session_crop_offset_x;
    vidf_block.crop_offset_y = session_crop_offset_y;
    vidf_block.pan_offset_x = session_pan_offset_x;
    vidf_block.pan_offset_y = session_pan_offset_y;
    vidf_block.frame_padding = (image_start - start) - sizeof(mlv_vidf_block);
    memcpy(start, &vidf_block, sizeof(mlv_vidf_block));
    memset(((mlv_vidf_block *)start)->frame_data, 0, vidf_block.frame_padding);

    session_frame_number++;
    return image_start;
}

// When finalising a file, we want to update the file header with the final
// count of frames.
int update_video_frame_count(uint32_t count, char *start, char *end)
{
    if (start == NULL || end == NULL)
        return -1;

    if (end < start)
        return -1;

    uint32_t space_available = end - start;

    if (space_available < sizeof(mlv_file_block))
        return -1;

    mlv_file_block *mlvi = (mlv_file_block *)start;
    mlvi->video_frame_count = count;

    return 0;
}

// MLV 2.0 needed to get info from lots of external funcs, e.g.
// to obtain FPS of recording.  To make this a cleaner lib, for MLV 3.0
// we require these are passed in, making it the caller's responsibility.
//
// Note that session state is unrelated to worker.c recording_state.
int start_mlv_session(struct mlv_session *session)
{
    if (session_in_progress)
    {
        SEND_LOG_DATA_STR("ERROR: attempt to start MLV session while session already in progress\n");
        return -1;
    }
    session_fps = session->fps;
    session_res_x = session->res_x;
    session_res_y = session->res_y;
    session_start_timestamp = get_us_clock();
    session_GUID = get_GUID();
    session_frame_number = 0;
    session_image_data_alignment = session->image_data_alignment;

    session_in_progress = 1;
    return 0;
}

int stop_mlv_session()
{
    if (!session_in_progress)
    {
        SEND_LOG_DATA_STR("ERROR: attempt to stop MLV session while no session in progress\n");
        return -1;
    }

    session_in_progress = 0;
    return 0;
}
