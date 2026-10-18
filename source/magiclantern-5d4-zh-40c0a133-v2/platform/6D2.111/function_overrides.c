// functions 6D2/105

// SJE overrides for functions I couldn't find stubs for,
// as well as ones that I think function sig has changed,
// so we can define wrappers.

#include <dryos.h>
#include <property.h>
#include <bmp.h>
#include <config.h>
#include <consts.h>
#include <lens.h>
#include <tasks.h>
#include <edmac.h>

struct chs_entry
{
    uint8_t head;
    uint8_t sector; //sector + cyl_msb
    uint8_t cyl_lsb;
}__attribute__((packed));

struct partition
{
    uint8_t  state;
    struct   chs_entry start;
    uint8_t  type;
    struct   chs_entry end;
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

void fsuDecodePartitionTable(void *partIn, struct partition_table *pTable){
    struct partition *part = (struct partition *)partIn;
    pTable->state = part->state;
    pTable->type = part->type;
    pTable->start_head = part->start.head;
    pTable->end_head = part->end.head;
    pTable->sectors_before_partition = part->start_sector;
    pTable->sectors_in_partition = part->size;

    //tricky bits - TBD
    pTable->start_cylinder_sector = 0;
    pTable->end_cylinder_sector = 0;
}

void LoadCalendarFromRTC(struct tm *tm)
{
    _LoadCalendarFromRTC(tm, 0, 0, 16);
}

#ifndef CONFIG_INSTALLER
extern struct task* first_task;
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

void RegisterEDmacCompleteCBR(int channel, void (*cbr)(void*), void* cbr_ctx)
{
    return;
}

void UnregisterEDmacCompleteCBR(int channel)
{
    return;
}

void RegisterEDmacAbortCBR(int channel, void (*cbr)(void*), void* cbr_ctx)
{
    return;
}

void UnregisterEDmacAbortCBR(int channel)
{
    return;
}

void RegisterEDmacPopCBR(int channel, void (*cbr)(void*), void* cbr_ctx)
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

void _engio_write(uint32_t* reg_list)
{
    return;
}

// SJE FIXME large hackish crap, based on 200D:
// Fake funcs for the edmac funcs required by mlv_lite.c
// This is much simpler than fixing edmac-memcpy.c to be
// safe / working for D78.
//
// This all needs cleaning up after 2025 release,
// want to do a D6 cam too so we can be more confident
// about the shape of an appropriate fix.
void edmac_memcpy_res_lock()
{
}

void edmac_memcpy_res_unlock()
{
}

// Mild hack: this avoids defining CONFIG_EDMAC_MEMCPY in order
// to get edmac_memcpy_sem global.  mlv_lite.c doesn't give a shit
// about CONFIG_EDMAC_MEMCPY, just goes ahead anyway.
static struct semaphore *edmac_copy_sem = NULL;
static struct semaphore *mem2mem_sem = NULL;

// This is not obvious at all, but mlv_lite module calls this
// func when DMA write finishes.  We can't do the required cleanup
// in this function, because it fires in an interrupt context,
// and some of the things we need to do assert if called within an interrupt.
// So all we do is release a sem, and edmac_copy_rectangle_cbr_start()
// does the cleanup.
//
// edmac-memcpy.c looks like a prime target for simplification.
// Over-engineered and under-utilised.  Maybe we can make everything blocking,
// you have to serialise usage of EDMAC anyway.
void edmac_copy_rectangle_adv_cleanup()
{
    //DryosDebugMsg(0, 15, "adv_cl giving mem2mem_sem");
    give_semaphore(mem2mem_sem);
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
        return NULL;
    }

    // Old code doesn't explain why it checks this, my guess is
    // because DMA transfers don't invalidate CPU cache, since
    // they're outside of the CPU.
    ASSERT(dst == UNCACHEABLE(dst));

    /* clean the cache before reading from regular (cacheable) memory */
    /* see FIO_WriteFile for more info */
    if (src == CACHEABLE(src))
    {
        sync_caches();
    }

// Somebody wanted to do this.  Nobody knows why.
// Perhaps trying to guard against overflow into the cache bits?
// But the effect is to remove the uncache bit that we did
// an assert check for just above!
//
//    void *src_adjusted = (void *)(((uint32_t)src & 0x1FFFFFFF) + src_x + src_y * src_width);
//    void *dst_adjusted = (void *)(((uint32_t)dst & 0x1FFFFFFF) + dst_x + dst_y * dst_width);
    void *src_adjusted = (void *)(((uint32_t)src) + src_x + src_y * src_width);
    void *dst_adjusted = (void *)(((uint32_t)dst) + dst_x + dst_y * dst_width);

    // TODO if / when we unify EDMAC memcpy API across cam gens, this will need work.
    // edmac-memcpy.c expects to use edmac_memcpy_sem global.
    // For now, only mlv_lite does m2m_cpy stuff on D7, so we can get away with this less-global
    // var.
    if (mem2mem_sem == NULL)
        mem2mem_sem = create_named_semaphore("mem2mem", SEM_CREATE_LOCKED);
    if (edmac_copy_sem == NULL)
        edmac_copy_sem = create_named_semaphore("edmac_copy", SEM_CREATE_UNLOCKED);
    if (take_semaphore(edmac_copy_sem, 1000))
    {
        DryosDebugMsg(0, 15, "cbr_start failed to take edmac_copy_sem");
        return NULL;
    }
//    DryosDebugMsg(0, 15, "cbr_start took edmac_copy_sem");

    // Not sure this is essential, but these are the devices locked
    // by code in 6D2 rom.
    // This seems to trigger "edmac timeout" from mlv_lite
//    uint32_t resIDs[3] = {0x00320021,
//                          0x00320007,
//                          0x00320035}; // via static analysis, e.g. e0147f64

// fake "magic" values, see notes in 200D.
    uint32_t resIDs[1] = {0x00000080};

    // channels found via edmaclog module, exercising cam, finding ones that
    // don't seem to be used.
//    DryosDebugMsg(0, 15, "pre m2m_lock");
//    uint32_t *probe = (uint32_t *)0x2a588; // global mem2mem_lock
//    if (probe != NULL)
//    {
//        DryosDebugMsg(0, 15, "probe: %x", *probe);
//        probe = (uint32_t *)(*probe);
//        if (probe != NULL)
//        {
//            DryosDebugMsg(0, 15, "*probe: %x", *probe);
//            DryosDebugMsg(0, 15, "m2m_resLock status: %d", *(probe + 1));
//            DryosDebugMsg(0, 15, "m2m_resLock semaphore: %d", *(probe + 1));
//        }
//    }

// This one can, sometimes, work in video mode and record sensor data to MLV.
// It seems unreliable though, mostly the cam hard reboots.
    create_mem_to_mem_lock_and_pwrmng_globals(0x21, 0x13, resIDs, COUNT(resIDs));

// These work okay with "magic" res lock in LV, but not video mode:
//    create_mem_to_mem_lock_and_pwrmng_globals(0x31, 0x10, resIDs, 1);
// Channel too high for edmac, as expected:
//    create_mem_to_mem_lock_and_pwrmng_globals(0x35, 0x1a, resIDs, 1);

// defaults:
// Works okay outside of LV, with 3 resIDs.
// Hangs waiting for lock in LV, with 3 resIDs?
// Fast reboot in LV, with 1 resID.
//    create_mem_to_mem_lock_and_pwrmng_globals(0x2f, 0x15, resIDs, 3);

    // channels that make it angry:
    // 2nd param:

    // incorrect mode bits:
    // 2nd param: 1c, 1e

    // "failed to take mem2mem_sem" (edmac doesn't complete)
    // 2nd param: c, 10

    // D7 does this before lock_and_wake_mem_to_mem(), even
    // though that seems risky to me
    set_mem_to_mem_cbr(cbr_w, cbr_ctx);

    // Seems to me you want to lock before changing the cbr,
    // or setting the channels for the copy,
    // to avoid clobbering an active transfer.
    // R does it in this order, so this may be a bug on D7.
//    DryosDebugMsg(0, 15, "pre lock_m2m");
//    probe = (uint32_t *)0x2a588;
//    if (probe != NULL)
//    {
//       DryosDebugMsg(0, 15, "probe: %x", *probe);
//        probe = (uint32_t *)(*probe);
//        if (probe != NULL)
//        {
//            DryosDebugMsg(0, 15, "*probe: %x", *probe);
//            DryosDebugMsg(0, 15, "m2m_resLock status: %d", *(probe + 1));
//            DryosDebugMsg(0, 15, "m2m_resLock semaphore: %d", *(probe + 1));
//        }
//    }
    lock_and_wake_mem_to_mem(); // blocking

    struct edmac_info src_region = {
        .off1b = src_width - w,
        .xb = w,
        .yb = h - 1,
    };

    struct edmac_info dst_region = {
        .off1b = 0,
        .xb = w,
        .yb = h - 1,
    };

    mem_to_mem_setup_copy(&src_region, &dst_region);

    // start the copy, which on completion should trigger
    // our cbr, releasing our sem
    mem_to_mem_set_src_dst_and_start(src_adjusted, dst_adjusted);

    // sem should be freed by edmac_copy_rectangle_adv_cleanup()
    // when copy completes (i.e. cbr_w() should call that function,
    // as well as whatever else it wants to do).
    if (take_semaphore(mem2mem_sem, 1000))
    {
        // EDMAC copy timeout?
        DryosDebugMsg(0, 15, "cbr_start failed to take mem2mem_sem");
        return NULL;
        // This will break all future EDMAC copy usage,
        // since we aren't releasing edmac_copy_sem
    }

    // This does several things, including releasing device 6, the Channel Switch.
    // Asserts if called from an interrupt context, since it calls
    // edmac_release_channel() and that checks "interrupt_level > 0".
    cleanup_mem_to_mem();

    unlock_and_sleep_mem_to_mem();
    set_default_mem_to_mem_cbr();
    delete_mem_to_mem_lock();

    give_semaphore(edmac_copy_sem);
    return dst;
}

// SJE FIXME these are only here because mlv_lite module has them as deps,
// even though the code doesn't use them.
// Needs a cleaner solution.
uint32_t edmac_read_chan = 0x6; // dummy, don't use
uint32_t edmac_write_chan = 0x6; // dummy, don't use
