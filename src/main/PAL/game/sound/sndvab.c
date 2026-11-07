#include "types.h"
#include "m2c_macros.h"
#include "rr/audio_pan.h"
#include "psyq/libsnd/utrev3.h"
#include "game/sound/sndvab.h"
#include "rr/library.h"
#include "rr/menu_choice.h"

extern int SsVabClose();
extern short SsVabOpenHeadSticky();
extern int SsVabTransCompleted();
extern int func_800963C4();
extern int DebugPrintStub();
extern char VabOpenHeadErrorStr[];
extern char D_800110F0[];
extern s32 D_800F7650;

void LoadVab(int *arg0, int arg1, int arg2);
void SetReverb(int a0, int a1);
void TriggerSfx(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
void PlaySfxVolPan(int a0, s16 a1, s16 a2, int a3);

/* legacy typing of shared globals in this part of the unit */
#define SsVabTransBody ((short (*)())SsVabTransBody)
#define SoundState (*(short (*)[])&SoundState)

void LoadVab(int *arg0, int arg1, int arg2) {
    int k = (int)SoundState; short *p = (short *)(k + (arg2 << 1));
    short v;
    /* ASM_BARRIER: original keeps the first result sign-extended in $a1 */
    register int t asm("$5");
    short v2;
    if (p[0x12] >= 0) {
        SsVabClose(p[0x12]);
        p[0x12] = -1;
    }
    v = SsVabOpenHeadSticky(arg0[0], -1, arg1);
    p[0x12] = v;
    /* ASM_BARRIER: keep the result store ahead of the sign-extension in the schedule */
    __asm__ volatile("");
    t = v;
    if (t == -1) {
        DebugPrintStub(VabOpenHeadErrorStr);
        func_800963C4(1);
    }
    v2 = SsVabTransBody(arg0[1]);
    p[0x12] = v2;
    if (v2 == -1) {
        DebugPrintStub(D_800110F0);
        func_800963C4(1);
    }
    SsVabTransCompleted(1);
}

#undef SsVabTransBody
#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x2894]; s32 a, b, c, d; } (*))&SoundState)

void ReverbReset(void) {
    SoundState.a = 0;
    SoundState.b = 0;
    SoundState.c = 0;
    SoundState.d = 0xFFFFFF;
    SsUtReverbOff();
    SsUtSetReverbType(2);
    SsUtReverbOn();
    SsUtSetReverbDepth(0, 0);
}

#undef SoundState

void SetReverbDepthBoth(s32 arg0) {
    D_800F7650 = arg0;
    SsUtSetReverbDepth((s16) arg0, (s16) arg0);
}

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x2894]; s32 a, b, c; } (*))&SoundState)

void SetReverb(int a0, int a1) {
    SsUtReverbOff();
    if (a0 > 0) {
        SoundState.a = a0;
        SoundState.b = a1;
        SoundState.c = a1;
        SsUtSetReverbType((s16)a0);
        SsUtReverbOn();
    } else {
        SoundState.a = 0;
        SoundState.b = 0;
        SoundState.c = 0;
    }
}

#undef SoundState

void TriggerSfx(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    s16 temp_a2_2;
    register s16 temp_s0_2;
    register s16 temp_s0_4;
    s16 temp_v1;
    s16 var_a0;
    s16 var_a1;
    register s32 var_t0;
    s32 temp_a2;
    register s32 temp_s0;
    register s32 temp_s0_3;
    register s32 temp_s1;
    register s32 temp_s2;
    register s32 temp_t1;
    s32 temp_v0;
    s32 var_a0_2;
    register s32 var_s0;
    s32 var_s1;
    s32 var_v0;
    s32 first_div,second_div;
    register s32 volume;
    s16 *table;
    /* ASM_BARRIER: retain this phase value in its original MIPS register. */
    register s32 note asm("$19");
    s32 var_v1;
    register u16 var_a3;
    /* ASM_BARRIER: retain this phase value in its original MIPS register. */
    register s32 selector asm("$11")=arg4;

    var_t0 = arg0;
    temp_t1 = (s32)((u32)arg3 + 0x3CU);
    if (var_t0 < 0x1E) {
        var_a3 = D_800F4DDC;
    } else if (var_t0 < 0x3C) {
        var_a3 = D_800F4DDE;
        var_t0 -= 0x1E;
    } else {
        var_a3 = D_800F4DEA;
        var_t0 -= 0x3C;
    }
    table=PanGainTable;

    temp_a2 = arg2 & 0x7F;
    temp_a2_2 = (table[temp_a2] * (s16)arg1) / 16129;
    var_a0 = temp_a2_2;
    temp_v1 = (table[0x7F-temp_a2] * (s16)arg1) / 16129;
    var_a1 = temp_v1;
    if (Options.audio.mono32 != 0) {
        temp_v0 = (s32) (temp_v1 + temp_a2_2) / 2;
        var_a0 = (s16) temp_v0;
        var_a1 = (s16) temp_v0;
    }
    var_v1 = var_a0 * SoundGain;
    if (var_v1 < 0) {
        var_v1 += 0x7F;
    }

    var_a0_2 = var_a1 * SoundGain;
    first_div=var_v1>>7;
    if (var_a0_2 < 0) {
        var_a0_2 += 0x7F;
    }

    first_div=(s32)((u32)first_div<<16);

    volume=Options.audio.volume31;
    var_s1 = (first_div>>16) * volume;
    second_div=var_a0_2>>7;
    if (var_s1 < 0) {
        var_s1 += 0xF;
    }

    var_v0 = (s16)second_div * volume;
    temp_s1 = var_s1 >> 4;
    if (var_v0 < 0) {
        var_v0 += 0xF;
    }
    temp_s2 = var_v0 >> 4;
    var_s0 = temp_t1;
    if (selector == -1) {
        /* ASM_BARRIER: retain this phase value in its original MIPS register. */
        register s32 outbank asm("$4"),outprogram asm("$5"),mode asm("$6");
        register s32 outnote;
        register s32 hi;
        register s32 bank;
        register s32 program;
        hi=(s32)((u32)var_a3<<16); bank=hi>>16;
        hi=(s32)((u32)var_t0<<16); program=hi>>16;
        if (temp_t1 < 0) {
            var_s0 = temp_t1 + 0x7F;
        }
        outbank=bank;outprogram=program;mode=0;
        /* ASM_BARRIER: preserve argument preparation before the next phase. */
        __asm__("" : : "r"(outbank));
        temp_s0 = var_s0 >> 7;
        note=(s16)temp_s0;
        outnote=note;

        temp_s0_2 = temp_t1 - (u32)temp_s0 * 128U;
        SsUtKeyOn(outbank, outprogram, mode, outnote, (s32) temp_s0_2, (s32) (s16) temp_s2, (s32) (s16) temp_s1);
        SsUtKeyOn(bank, program, 1, note, (s32) temp_s0_2, (s32) (s16) temp_s2, (s32) (s16) temp_s1);
        return;
    }
    {
    register u32 voice=(u32)selector*2U;
    /* ASM_BARRIER: retain this phase value in its original MIPS register. */
    register s32 hi asm("$2");
    register s32 bank;
    register s32 program;
    register s32 outvoice;
    /* ASM_BARRIER: retain this phase value in its original MIPS register. */
    register s32 outbank asm("$5"),outprogram asm("$6"),mode asm("$7");
    /* ASM_BARRIER: preserve argument preparation before the next phase. */
    __asm__("" : : "r"(voice));
    hi=(s32)((u32)voice<<16);
    outvoice=hi>>16;

    hi=(s32)((u32)var_a3<<16); bank=hi>>16;
    hi=(s32)((u32)var_t0<<16); program=hi>>16;
    if (temp_t1 < 0) {
        var_s0 = temp_t1 + 0x7F;
    }
    outbank=bank;outprogram=program;mode=0;
    /* ASM_BARRIER: preserve argument preparation before the next phase. */
    __asm__("" : : "r"(outbank));
    temp_s0_3 = var_s0 >> 7;
    note=(s16)temp_s0_3;
    temp_s0_4 = temp_t1 - (u32)temp_s0_3 * 128U;
    SsUtKeyOnV(outvoice, outbank, outprogram, mode, note, (s32) temp_s0_4, (s32) (s16) temp_s2, (s32) (s16) temp_s1);
    SsUtKeyOnV((s16)(voice+1U), bank, program, 1, note, (s32) temp_s0_4, (s32) (s16) temp_s2, (s32) (s16) temp_s1);
}
}

void PlaySfxVolPan(int a0, s16 a1, s16 a2, int a3) {
    TriggerSfx(a0, a1, a2, 0x1E00, a3);
}

void PlaySfx(int a0) {
    PlaySfxVolPan(a0, 0x7F, 0x40, -1);
}

void PlaySfxBank2(s32 arg0) {
    PlaySfxVolPan(arg0 + 0x3C, 0x7F, 0x40, -1);
}

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x19D0]; s32 a; char p1[4]; s32 b, c; char p2[8]; s16 arr[8]; } (*))&SoundState)

void SoundQueueClear(void) {
    s16 m = -1;
    int i = 7;
    SoundState.a = 0;
    SoundState.b = 0;
    SoundState.c = 0;
    do { SoundState.arr[i] = m; i--; } while (i >= 0);
}

#undef SoundState
