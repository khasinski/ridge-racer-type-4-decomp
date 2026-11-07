#ifndef RR_SPU_TICK_CONTROL_H
#define RR_SPU_TICK_CONTROL_H
#include "rr/spu_tick.h"
#include "rr/library.h"
#include "rr/counters.h"
/* The first member is the existing four-slot prefix. A prefix pointer can
 * be converted back to its complete eight-slot owner without narrowing. */
typedef void (*RrSpuTickInstall)(s32 slot,RrSpuTickCallback callback);
typedef struct {
    RrLibInterruptDriverPrefix prefix;
    void (*reserved10)(void);
    RrSpuTickInstall install14;
    void (*reserved18)(void);
    RrLibSavedState *state1C;
} RrSpuTickInterruptDriver20;
extern RrSpuTickCallback VSyncCallbackTable[8];
void setIntrVSync(s32 slot,RrSpuTickCallback callback);
void VSyncCallback(RrSpuTickCallback callback);
void EnterCriticalSection(void);
void ExitCriticalSection(void);
s32 ResetRCnt(u32 selector);
s32 SetRCnt(u32 selector,u16 target,s32 mode);
void _SsTickCallback(void);
void _SsTickCallbackDivided(void);
void _SsStart(s32 direct);
RR_ASSERT_SIZE(RrSpuTickInterruptDriver20,0x20);
RR_ASSERT_OFFSET(RrSpuTickInterruptDriver20,install14,0x14);
RR_ASSERT_OFFSET(RrSpuTickInterruptDriver20,state1C,0x1C);
#endif
