#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../mlv_3.h"

// This creates a simple, valid, MLV file.

int main(void)
{
    char *header_buf = NULL; // space for MLV file header
    const uint32_t header_buf_size = 1024; // wants to be big enough for combined headers,
                                           // this file shouldn't know their size, it's okay to just guess.
    uint32_t header_size = 0; // MLV lib returns real size here
    header_buf = malloc(header_buf_size);
    if (header_buf == NULL)
        return -1;

    FILE *fp = NULL;
    fp = fopen("test.mlv", "w");
    if (fp == NULL)
        return -2;

    struct mlv_session session = {
        .fps = 24,
        .res_x = 2,
        .res_y = 2,
        .image_data_alignment = 0x10
    };
    uint32_t fake_image_size = session.res_x * session.res_y;

    // start a session and write the file headers
    start_mlv_session(&session);
    header_size = write_file_headers(header_buf, header_buf + header_buf_size);
    if (header_size == 0)
        return -3;
    fwrite(header_buf, header_size, 1, fp);

    // write a single 2x2 video frame
    const uint32_t buf_size = 0x2000;
    char buf[buf_size];
    char *dst = write_video_frame_header(fake_image_size, buf, buf + buf_size);
    if (dst == NULL)
        return -4;
    memset(dst, 0x69, fake_image_size);
    fwrite(buf, (dst - buf) + fake_image_size, 1, fp);

    // update file header with frame count
    int err = update_video_frame_count(1, header_buf, header_buf + header_size);
    if (err)
        return -5;
    fseek(fp, 0, SEEK_SET);
    fwrite(header_buf, header_size, 1, fp);

    free(header_buf);
    stop_mlv_session();
    fclose(fp);
    return 0;
}
