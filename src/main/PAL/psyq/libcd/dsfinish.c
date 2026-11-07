/* CC1_FLAGS: -mno-split-addresses */
#include "types.h"
#include "psyq/libcd/dsfinish.h"
#include "rr/cd.h"
#include "rr/cd_read_request.h"
#include "rr/library.h"
#include "rr/overlay668_stream.h"

extern int CdReadStartTime[];

s32 CdReadCallback(s32 value);
s32 CdReadMode(s32 value);

int CdReadSync(a0, a1) int a0, a1; {
    int s0;
    int *p = CdReadStartTime;
    int *q = CdReadStartTime + 2;
    for (;;) {
        if (VSync(-1) > p[0] + 0x4B0) {
            s0 = -1;
        } else if (p[-2] < 0 || VSync(-1) > p[-1] + 0x3C) {
            cd_read_retry(1);
            s0 = p[-7];
        } else {
            s0 = p[-2];
        }
        if (a0 == 0) {
            if (q[0] == 0 || s0 != 0) { if (s0 > 0) continue; break; }
            continue;
        }
        break;
    }
    CdReady(1, a1);
    if (q[0] != 0 && s0 == 0) s0 = 1;
    return s0;
}

s32 CdReadCallback(s32 value) {
    s32 previous = CD_cbread;
    /* ASM_BARRIER: use the original assembler temporary for the store address. */
    register s32 *base __asm__("$1");
    /* ASM_BARRIER: load the prior value before materializing the store address. */
    __asm__ volatile("" : "=r"(previous) : "0"(previous) : "$1");
    /* PAL RAM mapping: CD_cbread is base plus the signed offset below. */
    base = (s32 *)0x800A0000U;
    /* ASM_BARRIER: expose a register-relative store so it can occupy the return slot. */
    __asm__("" : "=r"(base) : "0"(base));
    base[(0x63F0) / 4] = value;
    return previous;
}

s32 CdReadMode(s32 value) {
    s32 previous = CD_read_dma_mode;
    /* ASM_BARRIER: use the original assembler temporary for the store address. */
    register s32 *base __asm__("$1");
    /* ASM_BARRIER: load the prior value before materializing the store address. */
    __asm__ volatile("" : "=r"(previous) : "0"(previous) : "$1");
    /* PAL RAM mapping: CD_read_dma_mode is base plus the signed offset below. */
    base = (s32 *)0x800A0000U;
    /* ASM_BARRIER: expose a register-relative store so it can occupy the return slot. */
    __asm__("" : "=r"(base) : "0"(base));
    base[(0x63F4) / 4] = value;
    return previous;
}
