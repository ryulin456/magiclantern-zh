/** \file
 * Function overrides needed for 7D2 1.1.2
 */
/*
 * Copyright (C) 2021 Magic Lantern Team
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

#include <dryos.h>
#include <property.h>
#include <bmp.h>
#include <config.h>
#include <consts.h>
#include <lens.h>
#include <edmac.h>

void LoadCalendarFromRTC(struct tm *tm)
{
    // differs from D78, one arg is missing
    _LoadCalendarFromRTC(tm, 0, 16);
}

/*
 * Partition tables stuff. Copied from R as I was too lazy to search for stub.
 * And clean implementation in ML code is worth it anyway I guess.
 */

struct chs_entry
{
    uint8_t head;
    uint8_t sector; //sector + cyl_msb
    uint8_t cyl_lsb;
}__attribute__((packed));

struct partition
{
    uint8_t state;
    struct chs_entry start;
    uint8_t type;
    struct chs_entry end;
    uint32_t start_sector;
    uint32_t size;
}__attribute__((aligned,packed));

struct partition_table
{
    uint8_t  state; // 0x80 = bootable
    uint8_t  start_head;
    uint16_t start_cylinder_sector;
    uint8_t  type;
    uint8_t  end_head;
    uint16_t end_cylinder_sector;
    uint32_t sectors_before_partition;
    uint32_t sectors_in_partition;
}__attribute__((packed));

void fsuDecodePartitionTable(void *partIn, struct partition_table *pTable)
{
    struct partition *part = (struct partition *)partIn;
    pTable->state = part->state;
    pTable->type = part->type;
    pTable->start_head = part->start.head;
    pTable->end_head = part->end.head;
    pTable->sectors_before_partition = part->start_sector;
    pTable->sectors_in_partition = part->size;

    //tricky bits - TBD
    pTable->start_cylinder_sector = 0;
    pTable->end_cylinder_sector   = 0;
}

#ifndef CONFIG_INSTALLER
extern struct task *first_task;
int get_task_info_by_id(int unknown_flag, int task_id, void *task_attr)
{
    // task_id is something like two u16s concatenated.  The flag argument,
    // present on D45 but not on D678 allows controlling if the task info request
    // uses the whole thing, or only the low half.
    //
    // ML calls with this set to 1, meaning task_id is used as is,
    // if 0, the high half is masked out first.
    //
    // D678 doesn't have the 1 option, we use the low half as index
    // to find the full value.
    struct task *task = first_task + (task_id & 0xffff);
    return _get_task_info_by_id(task->taskId, task_attr);
}
#endif

/** WRONG: temporary overrides to get CONFIG_HELLO_WORLD working **/

void SetEDmac(unsigned int channel, void *address, struct edmac_info *ptr, int flags)
{
    return;
}

void ConnectWriteEDmac(unsigned int channel, unsigned int where)
{
    return;
}

void ConnectReadEDmac(unsigned int channel, unsigned int where)
{
    return;
}

void StartEDmac(unsigned int channel, int flags)
{
    return;
}

void AbortEDmac(unsigned int channel)
{
    return;
}

void RegisterEDmacCompleteCBR(int channel, void (*cbr)(void*), void *cbr_ctx)
{
    return;
}

void UnregisterEDmacCompleteCBR(int channel)
{
    return;
}

void RegisterEDmacAbortCBR(int channel, void (*cbr)(void*), void *cbr_ctx)
{
    return;
}

void UnregisterEDmacAbortCBR(int channel)
{
    return;
}

void RegisterEDmacPopCBR(int channel, void (*cbr)(void*), void *cbr_ctx)
{
    return;
}

void UnregisterEDmacPopCBR(int channel)
{
    return;
}

void _EngDrvOut(uint32_t reg, uint32_t value)
{
    return;
}

void _engio_write(uint32_t *reg_list)
{
    return;
}

struct LockEntry *CreateResLockEntry(uint32_t *resIds, uint32_t resIdCount)
{
    return NULL;
}

unsigned int UnLockEngineResources(struct LockEntry *lockEntry)
{
    return 0;
}

void *dma_memcpy(void *dest, void *srce, size_t n)
{
    extern void *_memcpy_fast(int channel, void *dst, void *src, size_t count);
    uint32_t dma_channel = 0; // untested if this is safe in all contexts on 7D2
    return _memcpy_fast(dma_channel, dest, srce, n);
}

// SJE FIXME large hackish crap:
// Fake funcs for the edmac funcs required by mlv_lite.c
// This is much simpler than fixing edmac-memcpy.c to be
// safe / working for 7D2, given that 7D2 seems to do all EDMAC
// stuff from Omar, a separate core with no direct RPC found so far.
// There's a command message system, we can probably use that,
// but for now we can get things working with dma_memcpy,
// and just not allow cropped regions (needs edmac for perf).
void edmac_memcpy_res_lock()
{
}

void edmac_memcpy_res_unlock()
{
}

void* edmac_copy_rectangle_cbr_start(void *dst, void *src,
                                     int src_width, int src_x, int src_y,
                                     int dst_width, int dst_x, int dst_y,
                                     int w, int h,
                                     void (*cbr_r)(void *), void (*cbr_w)(void *), void *cbr_ctx)
{
    // "src_width" is width of the frame including dark areas, borders etc.
    // "w" is the width of the region to copy out, e.g. if stripping the borders.

    if ((src == NULL) || (dst == NULL))
    {
        ASSERT(0);
        // This returns without clearing edmac_active via cbr_w(),
        // so recording will stall.  Which is fine because it's noticeable
        // and you should never pass null ptrs in.
        return NULL;
    }

    // For 7D2, we copy with a DMA func with sig close to memcpy().
    // Digic 6 cams seem to use this instead of dma_memcpy(), which they do have,
    // but it's not initialised correctly so you can't use it.
    //
    // This can't do a clever rect crop, so we need more checks on call params.
    //
    // We do this because we don't have full control of EDMAC from ICU,
    // Digic 6 cams do EDMAC ops from Omar.  Omar presumably can do it,
    // (and maybe we can trigger it from ICU), but so far we don't know how.

    // These imply mlv_lite should have raw_info.pitch == (res_x * BPP/8)
    if (src_width != dst_width || dst_width != w)
        return NULL;

    // These imply mlv_lite should have skip_x == 0
    if (src_x != 0 || dst_x != 0)
        return NULL;

    dma_memcpy(dst + (dst_y * dst_width),
               src + (src_y * src_width),
               w * h);

    cbr_w(NULL); // clears edmac_active in mlv_lite
    return dst;
}

void edmac_copy_rectangle_adv_cleanup()
{
}

// SJE FIXME these are only here because mlv_lite module has them as deps,
// even though the code doesn't use them, since we use m2m_copy func.
// Needs a cleaner solution.
uint32_t edmac_read_chan = 0x6; // dummy, don't use
uint32_t edmac_write_chan = 0x6; // dummy, don't use
