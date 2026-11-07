/* RODATA 0x80011198 0x18 */
#include "types.h"
#include "rr/audio_cd_controller.h"
#include "psyq/libsnd/ssmvol.h"
#include "psyq/libsnd/ssvolset.h"
#include "game/sound/cdmain.h"
#include "rr/overlay668_stream.h"
#include "rr/library.h"
#include "rr/bios_devices.h"

struct S { char pad[0x2864]; s16 a; char p[0x2888 - 0x2866]; s16 b; };


void SeqSetVolume(s16 a0);
void SeqFadeVolume(int a0, int a1);

void CdAudioUpdate(void) {
    s32 ready;
    RrAudioState *phaseState = &SoundState;
    u8 filter[2];
    RrCdLocation location;
    RrAudioState *var_v0;
    s32 temp_v1_2;
    s32 temp_a0;
    s32 temp_a1;
    s32 temp_v0;
    register s32 temp_v0_2 __asm__("a2");
    s32 temp_v0_3;
    s32 rate;
    s32 temp_v0_4;
    register s32 temp_v0_5 __asm__("a2");
    s32 temp_v0_6;
    s32 temp_v1;
    s32 temp_v1_3;

    if (SoundState.sector2664 < 0) {
        switch (SoundState.action2670) {
        case 1:
            CdAudioPlayStep();
            break;
        case 2:
            CdAudioPauseStep();
            break;
        case 3:
            CdAudioResumeStep();
            break;
        case 4:
            CdAudioStopStep();
            break;
        case 5:
            CdAudioLoopStep();
            break;
        case 6:
            CdAudioFilterStep();
            break;
        }
        goto block_34;
    }
    switch (SoundState.phase2674) {
    case 0:
                if (CdSync(1, 0) != 0) {
                    SoundState.state269C = 0;
                    SoundState.timer26A8 = 0;
                    SoundState.phase2674 = (s32) ((u32) SoundState.phase2674 + 1U);
                    goto seek;
                }
                goto timeout;
    case 1:
seek:
        CdIntToPos(SoundState.sector2664, location.c);
        CdControlF(0x15, location.c);
        SoundState.phase2674 = (s32) ((u32) SoundState.phase2674 + 1U);
        goto block_34;
    case 2:
        ready = 2;
            temp_v0_2 = CdSync(1, 0);
            if (temp_v0_2 == ready) {
                filter[0] = 1;
                filter[1] = (u8) SoundState.filter266C;
                if (CdControlF(0xD, filter) != 0) {
                    SoundState.phase2674 = (s32) ((u32) SoundState.phase2674 + 1U);
                }
            } else if (temp_v0_2 == 5) {
                SoundState.phase2674 = 1;
                if (CdDiskReady(1) == 0x10) {
                    ready = -1;
                    SoundState.flag267C = 1;
                    goto block_27;
                }
            } else {
timeout:
                temp_v0_3 = (u32) phaseState->timer26A8 + 1U;
                phaseState->timer26A8 = temp_v0_3;
                if (temp_v0_3 >= 0x169) {
                    ready = -1;
block_27:
                    phaseState->flag2680 = ready;
                    CdFlush();
                    phaseState->sector2664 = ready;
                    phaseState->phase2674 = 0;
                }
            }
            goto block_34;
    case 3:
        ready = 2;
                temp_v0 = CdSync(1, 0);
                if (temp_v0 == ready) {
                    temp_v1 = SoundState.sector2664;
                    SoundState.sector2664 = -1;
                    SoundState.phase2674 = 0;
                    SoundState.state269C = -1;
                    SoundState.flag267C = 0;
                    SoundState.flag2680 = 0;
                    SoundState.sector2668 = temp_v1;
                    SoundState.position2688 = temp_v1;
                    SoundState.index2698 = (s32) SoundState.filter266C;
                    if (SoundState.state26AC == temp_v0) {
                        SoundState.state26AC = 0;
                    }
                } else if (temp_v0 == 5) {
                    SoundState.phase2674 = 1;
                }
                goto block_34;
    }
block_34:
    var_v0 = &SoundState;
    temp_a0 = var_v0->request26B4;
    if (temp_a0 != 0 && var_v0->action2670 == -1) {
        switch (temp_a0) {
        case 1:CdAudioRequestPause();CdAudioStopRequest=0;break;
        case 2:CdAudioMute();CdAudioStopRequest=0;break;
        default:CdAudioStopRequest=0;break;
        }
    }
    if (SoundState.state269C == 1) {
        temp_v0_4 = SoundState.toc28C8[SoundState.bankA8CC][SoundState.index2698].position;
        SoundState.previous26DC = (s32) SoundState.tick26D8;
        SoundState.base26F0 = temp_v0_4;
        rate = SoundState.rate26F4;
        __asm__("" : : "r"(rate));
        if (rate != 0) {
            SoundState.tick26D8 = (s32) ((((u32) temp_v0_4 + 10U) * (u32) rate) / 600000U);
        } else {
            SoundState.tick26D8 = 0;
        }
        SoundState.tick26D0 = 0;
        SoundState.half26D2 = 0;
        temp_v1_2 = (u32) SoundState.tick26D8 - (u32) SoundState.previous26DC;
        SoundState.quarter26D4 = 0;
        if (temp_v1_2 == 1) {
            temp_a1 = (u32) SoundState.tick26D8 + (u32) SoundState.offset26E8;
            SoundState.tick26D0 = temp_v1_2;
            if (!(temp_a1 & 1)) {
                SoundState.half26D2 = temp_v1_2;
            }
            if (!(temp_a1 & 3)) {
                SoundState.quarter26D4 = temp_v1_2;
            }
        }
        if ((u32) SoundState.position2688 > (u32) SoundState.end2690) {
            if (SoundState.flag26A0 == 0) {
                SoundState.action2670 = 4;
                SoundState.step2678 = 0;
            } else if (SoundState.action2670 != 5) {
                SoundState.action2670 = 5;
                SoundState.timer26A4 = 0;
                DebugPrintStub(XaRepeatMsg, &SoundState);
            }
            CdAudioSetVolumeRaw(0);
        }
            if (SoundState.previous268C != SoundState.position2688) {
            SoundState.timer26A8 = 0;
        } else {
            SoundState.timer26A8 = (s32) ((u32) SoundState.timer26A8 + 1U);
        }
        if (SoundState.timer26A8 >= 0x1E) {
            SoundState.action2670 = 4;
            SoundState.step2678 = 0;
            SoundState.timer26A8 = 0;
        }
    }
    CdAudioFadeStep();
    SoundState.previous268C = (s32) SoundState.position2688;
    if ((u32) ((u32) SoundState.state269C - 1U) < 2U) {
        temp_v0_5 = CdSync(1, SoundState.status265C);
        switch (temp_v0_5) {
        case 0:
        case 1:
            break;
        case 2:
            if (CdLastCom() == 0x11) {
                temp_v0_6 = CdPosToInt(&SoundState.status265C[5]);
                if (temp_v0_6 > 0) {
                    SoundState.position2688 = temp_v0_6;
                }
            }
            CdControlF(0x11, 0);
            return;
        case 5:
            if (SoundState.state269C == 2) {
                CdAudioSetVolume(SoundState.volume26CE);
            }
            temp_v1_3 = SoundState.state269C;
            SoundState.state269C = -1;
            SoundState.action2670 = -1;
            SoundState.step2678 = 0;
            SoundState.state26AC = temp_v1_3;
            break;
        }
    }
}

/* legacy typing of shared globals in this part of the unit */
#define CdGetSector ((int (*)())CdGetSector)
#define SoundState (*(char (*)[])&SoundState)

void CdAudioReadyCallback(u8 a0) {
    char *b;
    char *q;
    s32 *s;
    if (a0 == 1) {
        b = SoundState;
        s = (s32 *)(b + 0x8000);
        q = b + 0x28C8;
        CdGetSector((s[0x28C8 / 4] << 11) + (int)q, 0x20);
        s[0x28CC / 4] = s[0x28C8 / 4];
        s[0x28C8 / 4] = (s[0x28C8 / 4] + 1) & 0xF;
    }
}

#undef CdGetSector
#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char pad[0x288A]; s16 a, b, c, d; } (*))&SoundState)

void SeqResetFade(void) {
    SoundState.a = 0;
    SoundState.b = 0;
    SoundState.c = 0;
    SoundState.d = 0;
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct S (*))&SoundState)

void SeqSetVolume(s16 a0) {
    struct S *p = &SoundState;
    s16 v;
    p->b = a0;
    v = (a0 * D_800AD558) / 16;
    SsSeqSetVol(p->a, v, v);
}

#undef SoundState

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char p0[0x2864]; s32 x2864; s32 x2868; s32 x286C; char p1[0x18]; u16 x2888; s16 x288A; s16 x288C; s16 x288E; s16 x2890; } (*))&SoundState)

void SeqFadeVolume(int a0, int a1) {
    if ((s16)a1 >= 2) {
        u16 v = SoundState.x2888;
        SoundState.x288A = a1;
        SoundState.x288C = 0;
        SoundState.x288E = a0;
        SoundState.x2890 = v;
    } else {
        int s = (s16)a0;
        SeqSetVolume(s);
        SoundState.x288A = 0;
        SoundState.x288C = 0;
        SoundState.x288E = 0;
        SoundState.x2890 = 0;
        if (s == 0) {
            if (SoundState.x2864 >= 0) {
                SsSeqStop((s16)SoundState.x2864);
                SsSeqClose((s16)SoundState.x2864);
                PsyqBzero(&SoundState.x286C, 0x1C);
            }
            SoundState.x2864 = -1;
            SoundState.x2868 = -1;
        }
    }
}

#undef SoundState
