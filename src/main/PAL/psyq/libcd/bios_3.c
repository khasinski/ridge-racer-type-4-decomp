/* MASPSX_FLAGS: --assembler-profile=gnu */
/* CC1_FLAGS: -mno-split-addresses */
#include "types.h"
#include "rr_asm.h"
#include "rr/cd.h"
#include "rr/library.h"
#include "psyq/libcd/s_3.h"

extern s32 DebugPrintStub();
extern s32 VSync(s32);
extern void CD_flush(void);
extern s32 PsyqPuts(const char *);
extern const char CdTimeoutPrefixStr[];
extern const char CdTimeoutStatusFmt[];
extern const char CdDatasyncNameStr[];
extern u8 CD_com;
extern char *CD_comstr[];
extern char *CD_intstr[];
extern volatile u8 CdIntrStatusBytes[];
extern s32 CD_timeout_deadline;
extern s32 CD_timeout_count;
extern const char *CdWaitCallerName[];
/* $at as the %hi() base: with $1 allocatable cc1 2.8.1 emits lui $1,%hi / sw %lo($1) and its reorg fills the jr/jal delay slot with the sw (see DECOMP_GUIDE "delay-slot store"). */
extern int CdTestParamNum;

s32 CD_datasync(s32 mode);
s32 CD_getsector(u8 *a0, u32 a1);
s32 CD_getsector2(u8 *a, u32 b);
void CD_set_test_parmnum(int a);
void CdCommandCallback(void);

static inline int check_alarm(s32 limit, char **ac, volatile u8 *st, char **sc) {
    if (VSync(-1) > CD_timeout_deadline || CD_timeout_count++ > limit) {
        PsyqPuts(CdTimeoutPrefixStr);
        {
            u8 s0_ = st[0];
            u8 s1_ = st[1];
            char *x5 = sc[s1_];
            DebugPrintStub(
                CdTimeoutStatusFmt,
                CdWaitCallerName[0],
                ac[CD_com],
                sc[s0_],
                x5
            );
        }
        CD_flush();
        return -1;
    }
    return 0;
}

s32 CD_datasync(s32 mode) {
    s32 limit;
    char **ac;
    volatile u8 *st;
    char **sc;

    s32 t = VSync(-1);
    /* ASM_BARRIER: Set allocno priorities for s2..s4 (mode, ac, limit). */
    __asm__("" : : "r"(mode), "r"(mode));
    CD_timeout_deadline = t + 0x3C0;
    CD_timeout_count = 0;
    CdWaitCallerName[0] = CdDatasyncNameStr;
    limit = 0x3C0000;
    ac = CD_comstr;
    st = CdIntrStatusBytes;
    sc = CD_intstr;
    do {
        if (check_alarm(limit, ac, st, sc) != 0) {
            return -1;
        }
        if ((*DMA3_CHCR & 0x1000000) == 0) {
            return 0;
        }
    } while (mode == 0);
    return 1;
}

s32 CD_getsector(u8 *a0, u32 a1) {
    volatile unsigned char *p;
    *CD_reg0 = 0;
    *CD_reg3 = 0x80;
    *CD_DELAY = 0x20943;
    /* Ordinary write views retain the original setup and teardown scheduling. */
    *(u32 *)COM_DELAY = 0x1323;
    *CD_dpcr |= 0x8000;
    *DMA3_MADR = a0;
    *DMA3_BCR = a1 | 0x10000;
    p = CD_reg0;
    /* ASM_BARRIER: orig has the load-delay nop BEFORE the loop label; maspsx emits it inside the loop */
    RR_NOP_DEP();
    do {
    } while (!(*p & 0x40));
    *DMA3_CHCR = 0x11000000;
    if (*DMA3_CHCR & 0x01000000) {
        do {
        } while (*DMA3_CHCR & 0x01000000);
    }
    *(u32 *)COM_DELAY = 0x1325;
    return 0;
}

s32 CD_getsector2(u8 *a, u32 b) {
    volatile int x;
    *CD_reg0 = 0;
    *CD_reg3 = 0x80;
    *CD_DELAY = 0x21020843;
    *COM_DELAY = 0x1325;
    *CD_dpcr |= 0x8000;
    *DMA3_MADR = a;
    *DMA3_BCR = b | 0x10000;
    while (!(*CD_reg0 & 0x40)) {
    }
    *DMA3_CHCR = 0x11400100;
    x = *DMA3_CHCR;
    return 0;
}

void CD_set_test_parmnum(int a) { CdTestParamNum = a; }

/* legacy typing of shared globals in this part of the unit */
#define CdIntrStatusBytes (*(u8 (*)[2])&CdIntrStatusBytes)

void CdCommandCallback(void) {
    u8 *p=&CdIntrStatusBytes[1];
    u8 *q=p-1;
    s32 saved=*CD_reg0 & 3;
    s32 result;
    while((result=getintr())) {
        if(result & 4) {
            if(CD_cbready) {
                /* ASM_BARRIER: Load the byte argument before preparing its data-page argument. */
                register s32 argument=*p;
                /* ASM_BARRIER: Retain the callback's data-page argument for its call delay slot. */
                register u8 *page __asm__("$5");
                /* ASM_BARRIER: Materialize the callback data page only on its active path. */
                __asm__ volatile("" : : "r"(argument) : "$5");
                page=(u8 *)0x80110000U;
                /* ASM_BARRIER: Keep the data-page offset separate for call scheduling. */
                __asm__("" : "+r"(page));
                CD_cbready(argument,page+0x320);
            }
        }
        if(result & 2) {
            if(CD_cbsync) {
                /* ASM_BARRIER: Load the byte argument before preparing its data-page argument. */
                register s32 argument=*q;
                /* ASM_BARRIER: Retain the callback's data-page argument for its call delay slot. */
                register u8 *page __asm__("$5");
                /* ASM_BARRIER: Materialize the callback data page only on its active path. */
                __asm__ volatile("" : : "r"(argument) : "$5");
                page=(u8 *)0x80110000U;
                /* ASM_BARRIER: Keep the data-page offset separate for call scheduling. */
                __asm__("" : "+r"(page));
                CD_cbsync(argument,page+0x318);
            }
        }
    }
    *CD_reg0=saved;
}

#undef CdIntrStatusBytes
