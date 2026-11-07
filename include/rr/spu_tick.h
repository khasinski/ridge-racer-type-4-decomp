#ifndef RR_SPU_TICK_H
#define RR_SPU_TICK_H

#include "rr/layout.h"

typedef void (*RrSpuTickCallback)(void);
typedef struct {
    s8 direct, divided, interrupt, reserved;
} RrSpuTickFlags4;
/* Timer state through the three-word SndSeqTickEnvDividedPhase owner, ending at0x800A4D58. The first two words also have the
 * assembler names _snd_seq_tick_env and SndSeqTickEnvActive; callback pointers retain native width. */
typedef struct {
    s32 selector, active;
    RrSpuTickCallback callback, previous;
    RrSpuTickFlags4 flags;
    s32 state[3];
} RrSpuTickState20;

extern RrSpuTickState20 _snd_seq_tick_env;
extern s32 VBLANK_MINUS;
s32 GetVideoMode(void);
void SsSetTickMode(s32 mode);

RR_ASSERT_SIZE(RrSpuTickFlags4,4);
RR_ASSERT_SIZE(RrSpuTickState20,0x20);
RR_ASSERT_OFFSET(RrSpuTickState20,active,4);
RR_ASSERT_OFFSET(RrSpuTickState20,callback,8);
RR_ASSERT_OFFSET(RrSpuTickState20,previous,0xC);
RR_ASSERT_OFFSET(RrSpuTickState20,flags,0x10);
RR_ASSERT_OFFSET(RrSpuTickState20,state,0x14);

#endif
