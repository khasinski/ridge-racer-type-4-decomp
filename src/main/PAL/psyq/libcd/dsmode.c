#include "rr/library.h"
#include "psyq/libcd/s_1.h"
#include "psyq/libcd/dsmedia2.h"
#include "psyq/libcd/dsstop.h"

extern s32 CdControl(s32 command, const u8 *parameters, u8 *result);
extern void CdDataCallback(RrLibCompletionCallback);

void CdRead2(s32 flags) {
    u8 buf[8];
    buf[0] = flags;
    CdControl(0xE, buf, 0);
    if (flags & 0x100) {
        if (flags & 0x20) {
            {
                /* ASM_BARRIER: Retain the assembler temporary for the mode-flag store. */
                register s32 *base __asm__("$1");
                /* ASM_BARRIER: Materialize the mode-store address only after its conditional branch. */
                __asm__ volatile("" : : : "$1", "memory");
                /* PAL mapping: base[0x348 / 4] is StSectorSize2048Mode. */
                base = (s32 *)0x80110000U;
                /* ASM_BARRIER: Expose the PAL mode store without a symbol-address macro. */
                __asm__("" : "=r"(base) : "0"(base));
                base[0x348 / 4] = 0;
            }
        } else {
            {
                s32 *base;
                register s32 one = 1;
                /* PAL mapping: base[0x348 / 4] is StSectorSize2048Mode. */
                base = (s32 *)0x80110000U;
                base[0x348 / 4] = one;
            }
        }
        CdDataCallback(data_ready_callback);
        CdReadyCallback(StCdInterrupt2);
    }
    {
        const u8 *zero = 0;
        /* ASM_BARRIER: Keep the two null arguments independently initialized as in the original call. */
        __asm__("" : "=r"(zero) : "0"(zero));
        CdControl(0x1B, zero, 0);
    }
}
