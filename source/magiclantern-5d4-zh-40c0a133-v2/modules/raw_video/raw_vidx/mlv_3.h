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

#ifndef _mlv_3_h_
#define _mlv_3_h_

// This header contains all that is required for external code to
// create MLV files.  Things that use the MLV 3 library don't need to
// know anything about the structure of MLV files.  Conceptually, they
// are just storing image data.

// TODO should MLV version be in here?  I guess yes, so code that wants
// to parse MLV files can check what version(s) are supported.

// TODO should structs live in another header, not mlv_3.c?  Inside the .c
// means people have to try quite hard to expose the internals.
// 3rd party code that wants to handle MLV as video files also doesn't
// *have* to have access to the structs.  We could provide functions
// to export to some external struct format.  I don't think we gain much
// and it would be more work.
// So, probably another header?

struct mlv_session
{
    int fps;
    int res_x;
    int res_y;
    int crop_offset_x;
    int crop_offset_y;
    int pan_offset_x;
    int pan_offset_y;
    uint32_t image_data_alignment;
};

int start_mlv_session(struct mlv_session *session);
int stop_mlv_session(void);

int write_file_headers(char *start, char *end);
char *write_video_frame_header(uint32_t image_size, char *buf_start, char *buf_end);
int update_video_frame_count(uint32_t count, char *start, char *end);

#endif
