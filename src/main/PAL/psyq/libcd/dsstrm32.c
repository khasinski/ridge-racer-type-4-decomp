/* GCC_VERSION: 2.7.2 */
#include "types.h"
#include "rr_asm.h"
#include "psyq/libcd/dsmedia2.h"
#include "psyq/libcd/dsstrm4.h"
#include "psyq/libgte/geo_.h"
#include "psyq/libcd/dsmedia2.h"
#include "psyq/libcd/dsstrm32.h"
#include "psyq/libcd/dsstrm2.h"
#include "psyq/libcd/dsstrm32.h"
#include "game/data/globals.h"

extern s32 CdReady(s32, u8 *);

typedef struct {
    u32 x;
} __attribute__((packed)) Packed32;

typedef struct {
    u16 state;
    u16 type;
    u16 sector_offset;
    u16 sector_size;
    u16 frame_no;
    u16 pad0A;
    u16 width;
    u16 height;
    u8 pad10[12];
    Packed32 headm;
} StHeader;

extern volatile u8 *CD_REG2_STRM;
extern volatile u8 *DS_cd_reg3;
extern volatile s32 *CDROM_DELAY_SIZE;
extern volatile s32 *CdComDelayReg;
extern volatile s32 *MdecOutDmaChcr;
extern volatile s32 *DS_dma_regs;
extern s32 StCdIntrStatus;
extern s32 StCdIntrFlag;
extern s32 StRingHeaderIndex;
extern u32 StEndFrame;
extern void *StRingSectorDest;
extern StHeader *StRingHeaders;
extern s32 StRingSize;
extern void (*StRingFullFunc)(void);
extern StHeader *StCurHeader;

void StCdInterrupt(void) {
    s32 i;
    struct {
        s16 sp20;
        volatile u16 sp22;
        volatile s16 sp24;
        s16 sp26;
        u8 sp28[8];
        u8 sp30[8];
    } s;
    /* ASM_BARRIER: Keep DMA control word in t0 across branches. */
    register s32 var_t0 __asm__("$8");
    /* ASM_BARRIER: Retain assembler temporary for delay-slot stores. */
    register s32 *page_8011 __asm__("$1");
    /* ASM_BARRIER: Retain assembler temporary for status-code delay-slot stores. */
    register s32 *page_800a __asm__("$1");
    /* ASM_BARRIER: Keep constant 1 in a0 across initial checks. */
    register s32 one __asm__("$4");
    /* ASM_BARRIER: Keep status code in v0 for delay-slot stores. */
    register s32 st __asm__("$2");
    /* ASM_BARRIER: Keep header pointer in v1. */
    register StHeader *hdr_v1 __asm__("$3");

    st = StDataReadyPending;
    one = 1;
    if (st == one) {
        return;
    }
    if (StRgb24 != 0) {
        if (*MdecOutDmaChcr & 0x01000000) {
            s32 f300 = StRingBase;
            /* ASM_BARRIER: Load StRingBase before materializing page_8011 for delay slot. */
            __asm__("" : "=r"(one) : "0"(one), "r"(f300) : "$1");
            page_8011 = (s32 *)0x80110000U;
            /* ASM_BARRIER: Expose StCdIntrFlag store for conditional branch delay slot. */
            __asm__("" : "=r"(page_8011) : "0"(page_8011));
            page_8011[0x2DC / 4] = one;
            if (f300 != 0) {
                StRingHeaderIndex += 1;
            }
            page_800a = (s32 *)0x800A0000U;
            /* ASM_BARRIER: Expose StCdIntrStatus store for epilogue jump delay slot. */
            __asm__("" : "=r"(page_800a) : "0"(page_800a));
            page_800a[0x68B8 / 4] = one;
            return;
        }
    }
    if (CdReady(one, s.sp30) == 5) {
        return;
    }
    s.sp22 = s.sp30[0];
    s.sp24 = s.sp30[1];
    if (s.sp22 & 4) {
        st = 3;
        page_800a = (s32 *)0x800A0000U;
        /* ASM_BARRIER: Expose StCdIntrStatus store for epilogue jump delay slot. */
        __asm__("" : "=r"(page_800a), "=r"(st) : "0"(page_800a), "1"(st));
        page_800a[0x68B8 / 4] = st;
        return;
    }
    {
        /* ASM_BARRIER: Keep index in v0 and base in v1 for addu v1, v1, v0. */
        register s32 f2f4 __asm__("$2") = StRingWriteIndex;
        hdr_v1 = StRingHeaders;
        /* ASM_BARRIER: Materialize both before scaling and adding. */
        __asm__("" : "=r"(f2f4), "=r"(hdr_v1) : "0"(f2f4), "1"(hdr_v1));
        hdr_v1 = (StHeader *)((u8 *)hdr_v1 + (f2f4 << 5));
    }
    StCurHeader = hdr_v1;
    /* ASM_BARRIER: Keep hdr_v1 in v1 after storing to StCurHeader. */
    __asm__ volatile("" : "=r"(hdr_v1) : "0"(hdr_v1) : "memory");
    if (hdr_v1->state != 0) {
        if (StRingBase != 0) {
            StRingHeaderIndex += 1;
        }
        st = 4;
        page_800a = (s32 *)0x800A0000U;
        /* ASM_BARRIER: Expose StCdIntrStatus store for epilogue jump delay slot. */
        __asm__("" : "=r"(page_800a), "=r"(st) : "0"(page_800a), "1"(st));
        page_800a[0x68B8 / 4] = st;
        return;
    }
    *CD_REG0_STRM = 0;
    *DS_cd_reg3 = 0;
    *CD_REG0_STRM = 0;
    *DS_cd_reg3 = 0x80;
    *CDROM_DELAY_SIZE = 0x20943;
    *CdComDelayReg = 0x1323;
    {
        /* ASM_BARRIER: Pin loop counter to a0 and buffer base to a1. */
        register u32 idx __asm__("$4") = 0;
        if (D_80110348 == 0) {
            register u8 *buf_base __asm__("$5") = s.sp28;
            /* ASM_BARRIER: Force array base into a1 before loop. */
            __asm__("" : "=r"(buf_base), "=r"(idx) : "0"(buf_base), "1"(idx));
            do {
                volatile u8 *p78 = CD_REG2_STRM;
                u8 *dst = buf_base + idx;
                u8 val = *p78;
                idx += 1;
                *dst = val;
            } while (idx < 4U);
            idx = 0;
            {
                /* ASM_BARRIER: Pin CD_REG2_STRM pointer to v1 in second loop. */
                register volatile u8 *p78 __asm__("$3") = CD_REG2_STRM;
                /* ASM_BARRIER: Keep load-delay gap before the polling loop label. */
                RR_NOP_DEP_ON(p78);
                do {
                    *p78;
                    idx += 1;
                } while (idx < 8U);
            }
        }
    }
    var_t0 = 0x11000000;
    if (StRingBase != 0) {
        mem2mem(StCurHeader, StRingBase + (StRingHeaderIndex << 11), 8, 0);
    } else {
        dma_execute(3, StCurHeader, 0, 8, var_t0, 0, 0);
    }
    while (*DS_dma_regs & 0x01000000) {
    }
    {
        /* ASM_BARRIER: Order 0x20843 load and unaligned copy through original registers. */
        register s32 c20843 __asm__("$4") = 0x20843;
        register StHeader *h __asm__("$2");
        register volatile s32 *p80 __asm__("$3");
        /* ASM_BARRIER: Materialize 0x20843 in a0 first. */
        __asm__("" : "=r"(c20843) : "0"(c20843));
        h = StCurHeader;
        p80 = CDROM_DELAY_SIZE;
        /* ASM_BARRIER: Keep h in v0 and p80 in v1 across unaligned copy. */
        __asm__("" : "=r"(h), "=r"(p80), "=r"(c20843) : "0"(h), "1"(p80), "2"(c20843));
        h->headm = *(Packed32 *)s.sp28;
        *p80 = c20843;
    }
    *CdComDelayReg = 0x1325;
    if (StMaskEnabled == 1) {
        /* ASM_BARRIER: Keep StStartFrame in a0 and StCurHeader in v1. */
        register s32 e4 __asm__("$4") = StStartFrame;
        if (e4 != 0) {
            hdr_v1 = StCurHeader;
            if (e4 != hdr_v1->frame_no) {
                hdr_v1->state = 0;
                /* ASM_BARRIER: Store state before checking StRingBase. */
                __asm__ volatile("" : : : "memory");
                if (StRingBase != 0) {
                    s32 f2f0 = StRingHeaderIndex + 1;
                    /* ASM_BARRIER: Compute f2f0 before materializing page_8011 for delay slot. */
                    __asm__("" : "=r"(f2f0) : "0"(f2f0) : "$1");
                    page_8011 = (s32 *)0x80110000U;
                    /* ASM_BARRIER: Expose StRingHeaderIndex store for epilogue jump delay slot. */
                    __asm__("" : "=r"(page_8011) : "0"(page_8011));
                    page_8011[0x2F0 / 4] = f2f0;
                }
                return;
            }
            StMaskEnabled = 0;
        }
    }
    if (StCurHeader->state != 0x160 || ((StCurHeader->type >> 10) & 0x1F) != StExpectedChannel) {
        if (StRingBase != 0) {
            /* ASM_BARRIER: Keep branch delay slot empty before materializing page_8011. */
            __asm__ volatile("" : : : "$1");
            page_8011 = (s32 *)0x80110000U;
            /* ASM_BARRIER: Expose StRingHeaderIndex store for jump delay slot. */
            __asm__("" : "=r"(page_8011) : "0"(page_8011));
            page_8011[0x2F0 / 4] = 0;
        } else {
            *(volatile u16 *)&StCurHeader->state;
        }
        StCdIntrStatus = 5;
        StCurHeader->state = 0;
        return;
    }
    if (StNextSectorOffset != StCurHeader->sector_offset || (StCurFrameNo != 0 && StCurFrameNo != StCurHeader->frame_no)) {
        StCurFrameNo = 0;
        StNextSectorOffset = 0;
        init_ring_status(StRingStartIndex, StRingWriteIndex - StRingStartIndex);
        StRingWriteIndex = StRingStartIndex;
        StCurHeader->state = 0;
        /* ASM_BARRIER: Store state before checking StRingBase. */
        __asm__ volatile("" : : : "memory");
        if (StRingBase != 0) {
            StRingHeaderIndex += 1;
        }
        st = 6;
        page_800a = (s32 *)0x800A0000U;
        /* ASM_BARRIER: Expose StCdIntrStatus store for epilogue jump delay slot. */
        __asm__("" : "=r"(page_800a), "=r"(st) : "0"(page_800a), "1"(st));
        page_800a[0x68B8 / 4] = st;
        return;
    }
    if (StCurHeader->sector_offset == 0) {
        s32 fn = StCurHeader->frame_no;
        u32 f304 = StEndFrame;
        /* ASM_BARRIER: Load frame_no and StEndFrame before clearing StNextSectorOffset. */
        __asm__ volatile("" : "=r"(fn) : "0"(fn), "r"(f304) : "memory");
        StNextSectorOffset = 0;
        /* ASM_BARRIER: Clear StNextSectorOffset before masking fn with 0xFFFF. */
        __asm__ volatile("" : "=r"(fn) : "0"(fn) : "memory");
        StCurFrameNo = fn & 0xFFFF;
        if (f304 != 0 && StCurFrameNo >= f304) {
            /* ASM_BARRIER: Pin cb to v1 so it is loaded before st = 1. */
            register void (*cb)(void) __asm__("$3");
            StCurFrameNo = 0;
            StNextSectorOffset = 0;
            init_ring_status(StRingStartIndex, StRingWriteIndex - StRingStartIndex);
            StRingWriteIndex = StRingStartIndex;
            StCurHeader->state = 0;
            /* ASM_BARRIER: Store state before loading callback. */
            __asm__ volatile("" : : : "memory");
            cb = StRingFullFunc;
            /* ASM_BARRIER: Materialize cb in v1 before loading 1 into v0. */
            __asm__("" : "=r"(cb) : "0"(cb));
            st = 1;
            /* ASM_BARRIER: Load cb and 1 before materializing page_8011 for delay slot. */
            __asm__("" : "=r"(st) : "0"(st), "r"(cb) : "$1");
            page_8011 = (s32 *)0x80110000U;
            /* ASM_BARRIER: Expose StMaskEnabled store for branch delay slot. */
            __asm__("" : "=r"(page_8011) : "0"(page_8011));
            page_8011[0x308 / 4] = st;
            if (cb != 0) {
                cb();
            }
            if (StRingBase != 0) {
                StRingHeaderIndex += 1;
            }
            st = 7;
            page_800a = (s32 *)0x800A0000U;
            /* ASM_BARRIER: Expose StCdIntrStatus store for epilogue jump delay slot. */
            __asm__("" : "=r"(page_800a), "=r"(st) : "0"(page_800a), "1"(st));
            page_800a[0x68B8 / 4] = st;
            return;
        }
        if ((u32)(StRingSize - StRingWriteIndex - 1) < StCurHeader->sector_size) {
            if (StEndFrame == 0) {
                /* ASM_BARRIER: Pin cb to v1 so it is loaded before st = 1. */
                register void (*cb)(void) __asm__("$3");
                StCurHeader->state = 1;
                /* ASM_BARRIER: Store state before loading callback. */
                __asm__ volatile("" : : : "memory");
                cb = StRingFullFunc;
                /* ASM_BARRIER: Materialize cb in v1 before loading 1 into v0. */
                __asm__("" : "=r"(cb) : "0"(cb));
                st = 1;
                /* ASM_BARRIER: Load cb and 1 before materializing page_8011 for delay slot. */
                __asm__("" : "=r"(st) : "0"(st), "r"(cb) : "$1");
                page_8011 = (s32 *)0x80110000U;
                /* ASM_BARRIER: Expose StMaskEnabled store for branch delay slot. */
                __asm__("" : "=r"(page_8011) : "0"(page_8011));
                page_8011[0x308 / 4] = st;
                if (cb != 0) {
                    cb();
                }
                if (StRingBase != 0) {
                    StRingHeaderIndex += 1;
                }
                st = 8;
                page_800a = (s32 *)0x800A0000U;
                /* ASM_BARRIER: Expose StCdIntrStatus store for epilogue jump delay slot. */
                __asm__("" : "=r"(page_800a), "=r"(st) : "0"(page_800a), "1"(st));
                page_800a[0x68B8 / 4] = st;
                return;
            }
            if ((s16)StRingHeaders->state != 0) {
                StCurHeader->state = 0;
                /* ASM_BARRIER: Store state before checking StRingBase. */
                __asm__ volatile("" : : : "memory");
                if (StRingBase != 0) {
                    StRingHeaderIndex += 1;
                }
                st = 9;
                page_800a = (s32 *)0x800A0000U;
                /* ASM_BARRIER: Expose StCdIntrStatus store for epilogue jump delay slot. */
                __asm__("" : "=r"(page_800a), "=r"(st) : "0"(page_800a), "1"(st));
                page_800a[0x68B8 / 4] = st;
                return;
            }
            StCurHeader->state = 1;
            /* ASM_BARRIER: Store state before reloading StRingHeaders. */
            __asm__ volatile("" : : : "memory");
            {
                /* ASM_BARRIER: Pin copy pointers and counter to a1, v1, a0. */
                register s32 *dst __asm__("$5") = (s32 *)StRingHeaders;
                register s32 *src __asm__("$3") = (s32 *)StCurHeader;
                register u32 idx __asm__("$4") = 0;
                StRingWriteIndex = 0;
                do {
                    s32 val = *src;
                    src += 1;
                    idx += 1;
                    *dst = val;
                    dst += 1;
                } while (idx < 8U);
            }
            StCurHeader = StRingHeaders;
        }
        StRingStartIndex = StRingWriteIndex;
    }
    StCdIntrStatus = 0xA;
    StNextSectorOffset = (u16)StNextSectorOffset + 1;
    StRingSectorDest = (u8 *)(StRingHeaders + StRingSize) + StRingWriteIndex * 0x7E0;
    var_t0 = 0x11000000;
    if (StRgb24 != 0) {
        {
            /* ASM_BARRIER: Pin 0x20943 to v1 and p80 to v0. */
            register s32 c20943 __asm__("$3");
            register volatile s32 *p80 __asm__("$2");
            /* ASM_BARRIER: Load upper half of 0x20943 into v1 first. */
            __asm__("" : "=r"(c20943) : "0"(0x20000));
            p80 = CDROM_DELAY_SIZE;
            /* ASM_BARRIER: Complete 0x20943 after loading p80. */
            __asm__("" : "=r"(p80), "=r"(c20943) : "0"(p80), "1"(c20943));
            *p80 = c20943 | 0x943;
        }
        {
            /* ASM_BARRIER: Keep p84 in v1 and 0x1323 in v0 so sw fills jump delay slot. */
            register volatile s32 *p84 __asm__("$3") = CdComDelayReg;
            /* ASM_BARRIER: Materialize p84 before loading 0x1323 into v0. */
            __asm__("" : "=r"(p84) : "0"(p84));
            st = 0x1323;
            /* ASM_BARRIER: Expose store for jump delay slot. */
            __asm__("" : "=r"(p84), "=r"(st) : "0"(p84), "1"(st));
            *(s32 *)p84 = st;
        }
    } else {
        /* ASM_BARRIER: Keep 0x21020843 in v1 and p80 in v0. */
        register s32 c2102 __asm__("$3") = 0x21020843;
        register volatile s32 *p80 __asm__("$2");
        /* ASM_BARRIER: Materialize c2102 and upper half of var_t0 before loading p80. */
        __asm__("" : "=r"(c2102), "=r"(var_t0) : "0"(c2102), "1"(0x11400000));
        p80 = CDROM_DELAY_SIZE;
        /* ASM_BARRIER: Complete var_t0 after loading p80. */
        __asm__("" : "=r"(p80), "=r"(var_t0) : "0"(p80), "1"(var_t0), "r"(c2102));
        var_t0 |= 0x100;
        /* ASM_BARRIER: Store c2102 after completing var_t0. */
        __asm__("" : "=r"(var_t0) : "0"(var_t0));
        *(s32 *)p80 = c2102;
    }
    if (StCurHeader->sector_size - 1 == StCurHeader->sector_offset) {
        /* ASM_BARRIER: Keep constant 1 in v1 for StDataReadyPending and dma_execute arg. */
        register s32 one_v1 __asm__("$3") = 1;
        s32 f300 = StRingBase;
        /* ASM_BARRIER: Load StRingBase before storing StDataReadyPending. */
        __asm__("" : "=r"(one_v1) : "0"(one_v1), "r"(f300) : "$1");
        StDataReadyPending = one_v1;
        if (f300 != 0) {
            s32 f2f0;
            mem2mem(StRingSectorDest, f300 + (StRingHeaderIndex << 11) + 0x20, 0x1F8, 1);
            f2f0 = StRingHeaderIndex + 1;
            /* ASM_BARRIER: Compute f2f0 before materializing page_8011 for delay slot. */
            __asm__("" : "=r"(f2f0) : "0"(f2f0) : "$1");
            page_8011 = (s32 *)0x80110000U;
            /* ASM_BARRIER: Expose StRingHeaderIndex store for jump delay slot. */
            __asm__("" : "=r"(page_8011) : "0"(page_8011));
            page_8011[0x2F0 / 4] = f2f0;
        } else {
            dma_execute(3, StRingSectorDest, 0, 0x1F8, var_t0, one_v1, 0);
        }
        {
            s32 e0 = D_801102E0;
            /* ASM_BARRIER: Load D_801102E0 before clearing StNextSectorOffset and StCurFrameNo. */
            __asm__ volatile("" : "=r"(e0) : "0"(e0) : "memory");
            StNextSectorOffset = 0;
            StCurFrameNo = 0;
            /* ASM_BARRIER: Clear StCurFrameNo before materializing page_8011 for delay slot. */
            __asm__ volatile("" : : "r"(e0) : "$1", "memory");
            page_8011 = (s32 *)0x80110000U;
            /* ASM_BARRIER: Expose StExpectedChannel store for jump delay slot. */
            __asm__("" : "=r"(page_8011) : "0"(page_8011));
            page_8011[0x2E8 / 4] = e0;
        }
    } else {
        s32 f300 = StRingBase;
        if (f300 != 0) {
            s32 f2f0;
            mem2mem(StRingSectorDest, f300 + (StRingHeaderIndex << 11) + 0x20, 0x1F8, 0);
            f2f0 = StRingHeaderIndex + 1;
            /* ASM_BARRIER: Compute f2f0 before materializing page_8011 for delay slot. */
            __asm__("" : "=r"(f2f0) : "0"(f2f0) : "$1");
            page_8011 = (s32 *)0x80110000U;
            /* ASM_BARRIER: Expose StRingHeaderIndex store for jump delay slot. */
            __asm__("" : "=r"(page_8011) : "0"(page_8011));
            page_8011[0x2F0 / 4] = f2f0;
        } else {
            dma_execute(3, StRingSectorDest, 0, 0x1F8, var_t0, 0, 0);
        }
    }
    *CdComDelayReg = 0x1325;
    StCurHeader->state = 3;
    /* ASM_BARRIER: Store state before incrementing StRingWriteIndex. */
    __asm__ volatile("" : : : "memory");
    {
        s32 f2f4 = StRingWriteIndex;
        s32 f300 = StRingBase;
        f2f4 += 1;
        /* ASM_BARRIER: Compute f2f4 before materializing page_8011 for delay slot. */
        __asm__("" : "=r"(f2f4) : "0"(f2f4), "r"(f300) : "$1");
        page_8011 = (s32 *)0x80110000U;
        /* ASM_BARRIER: Expose StRingWriteIndex store for conditional branch delay slot. */
        __asm__("" : "=r"(page_8011) : "0"(page_8011));
        page_8011[0x2F4 / 4] = f2f4;
        if (f300 != 0 && StDataReadyPending != 0) {
            data_ready_callback();
        }
    }
}
