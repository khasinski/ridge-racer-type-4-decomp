/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "types.h"
#include "rr/library.h"
#include "psyq/libcd/s_4.h"
#include "psyq/libcd/dsstrm4.h"
#include "psyq/libcd/s_1.h"
#include "psyq/libcd/s_4.h"
#include "psyq/libcd/dsmedia2.h"
#include "psyq/libcd/dsstrm32.h"
#include "rr/cd.h"
#include "rr/overlay668_stream.h"

extern s32 CdDataCallback();
extern s32 EnterCriticalSection();
extern s32 ExitCriticalSection();
extern s32 D_800A64A8;
extern s8 *D_800A6800;
extern s8 *D_800A680C;
extern s32 D_80112754;
extern RrLibPackedWord StLastReadyLoc;
extern void (*StDataReadyFunc)(void);
extern int CdPosToInt();

void StClearRing(void);
void StUnSetRing(void);
void data_ready_callback(void);
int StGetBackloc(int a);

void StClearRing(void) {
    int a = StRingSize;
    StRingReadIndex = 0;
    StRingStartIndex = 0;
    StRingWriteIndex = 0;
    StDataReadyPending = 0;
    init_ring_status(0, a);
    StCdIntrFlag = 0;
    StNextSectorOffset = 0;
    StCurFrameNo = 0;
}

void StUnSetRing(void) {
    EnterCriticalSection();
    if (D_800A64A8 == 1) {
        CdAltDataCallback(0);
        CdAltReadyCallback(0);
    } else {
        CdDataCallback(0);
        CdReadyCallback(0);
    }
    *D_800A6800 = 0;
    {
        s8 *port = D_800A680C;
        /* ASM_BARRIER: preserve the pointer's load delay before the final call. */
        __asm__ volatile("nop" : : "r"(port));
        *port = 0;
    }
    ExitCriticalSection();
}

void data_ready_callback(void)
{
    s32 value;
    /* ASM_BARRIER: Keep the next queue index in the original register until publication. */
    register s32 next __asm__("$3");
    /* ASM_BARRIER: Retain the original indirect-call register while publishing queue state. */
    register void (*callback)(void) __asm__("$4");
    RrLibQueueRecord32 *p = StRingHeaders + StRingStartIndex;
    p->state = 2;
    StLastReadyLoc = p->word1C;
    value = p->value08;
    next = StRingWriteIndex;
    callback = StDataReadyFunc;
    D_80112754 = value;
    {
        /* ASM_BARRIER: Preserve the assembler temporary for publishing the next index. */
        register u8 *base __asm__("$1");
        /* ASM_BARRIER: Read callback and next index before publishing state and materializing its address. */
        __asm__ volatile("" : : "r"(next), "r"(callback) : "$1", "memory");
        /* PAL mapping: base + 0x2F8 is &StRingStartIndex. */
        base = (u8 *)0x80110000U;
        /* ASM_BARRIER: Expose the index publication in the callback-test branch delay slot. */
        __asm__("" : "=r"(base) : "0"(base));
        *(s32 *)(base + 0x2F8) = next;
    }
    if (callback != 0) callback();
    StDataReadyPending = 0;
}

/* legacy typing of shared globals in this part of the unit */
#define StLastReadyLoc (*(char (*)[])&StLastReadyLoc)

int StGetBackloc(int a) {
    int r;
    if (D_80110348 == 0) {
        CdIntToPos(CdPosToInt(StLastReadyLoc) + 1, a);
        r = D_80112754;
    } else {
        r = -1;
    }
    return r;
}

#undef StLastReadyLoc
