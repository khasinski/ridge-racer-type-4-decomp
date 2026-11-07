/* RODATA 0x8001134c 0x14 */
#include "types.h"
#include "scratchpad.h"
#include "game/race/objrec.h"
#include "game/gfx/polytri.h"
#include "game/gfx/poly1.h"
#include "game/race/objclr.h"
#include "game/race/objsim.h"
#include "rr/overlay662_distance.h"
#include "rr/overlay661_init.h"
#include "rr/overlay668_camera.h"
#include "rr/audio_loop.h"

#define B4 ((S_ECB4 *)(i * 0x2C + (int)TrackObjectStates))
#define B0 ((S_ECB0 *)(i * 0x30 + (int)TrackObjectDefs))
#define AC(idx) ((S_ECAC *)((idx) * 0x24 + (int)TrackObjectKeyframes))
#define AB(idx) ((S_ABEA4 *)((idx) * 0x3C + (int)TrackNodes))

typedef struct {
    s32 f00;
    s32 f04;
    s32 f08;
    s32 f0C;
    s32 f10;
    s32 f14;
    s32 f18;
    s16 f1C;
    s16 f1E;
    s16 f20;
    s16 f22;
} Node24;

typedef struct {
    s32 pad[9];
    s32 f24;
    s32 f28;
    s32 f2C;
} Node30;

typedef struct {
    s32 f00;
    s32 f04;
    s32 f08;
    s32 f0C;
    s32 f10;
    s32 f14;
    s32 f18;
    s16 f1C;
    s16 f1E;
    s16 f20;
    s16 f22;
    s32 f24;
    s32 f28;
} Node2C;

extern Node24 *TrackObjectKeyframes;
extern Node30 *TrackObjectDefs;
extern Node2C *TrackObjectStates;
/* size 0x30 */
typedef struct {
    int f0;
    int f4;
    int f8;
    int fC;
    int f10;
    int f14;
    int f18;
    unsigned short f1C;
    unsigned short f1E;
    unsigned short f20;
    unsigned short f22;
} S_ECAC;
/* size 0x24 */
typedef struct {
    int a[4];
} S_E8E8;
/* size 0x10 */
typedef struct {
    int f0;
    int f4;
    int f8;
    int fC;
    short f10;
    short f12;
    int pad[10];
} S_ABEA4;
/* size 0x3C */
extern int D_800A4090;
extern int ModeFrame;
extern short CourseIndex;
extern unsigned short PlayerControllers[];
extern int RaceState;
extern int D_8010E7A8;
extern int D_8010E7AC;
extern int D_8010E7B0;
extern int D_8010E7B4;
extern int D_8010E7B8;
extern int D_8010E7C8[];
extern S_E8E8 D_8010E8E8[];

s32 InterpolateObjectKeyframe(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7);
void UpdateTrackObjects(void);

s32 InterpolateObjectKeyframe(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7) {
    s32 *sp = (s32 *)(SCRATCHPAD + 0x7C);
    s32 d1, w, d2;

    sp[14] = arg2;
    sp[10] = arg4;
    sp[11] = arg5;
    sp[12] = arg6;
    sp[13] = arg7;

    if ((TrackObjectKeyframes[arg1].f08 % 10) == 8 &&
        (arg1 + 1) < (TrackObjectDefs[arg0].f28 + TrackObjectDefs[arg0].f24) &&
        (TrackObjectStates[arg0].f00 + 10) > (TrackObjectKeyframes[arg1].f00 * 6)) {
        d1 = (TrackObjectKeyframes[arg1 + 1].f00 - TrackObjectKeyframes[arg1].f00) * 6;
        w = TrackObjectKeyframes[arg1].f00 * 6 - 10;
        w = TrackObjectStates[arg0].f00 - w;
        sp[0] = (TrackObjectKeyframes[arg1].f0C - TrackObjectKeyframes[arg1 - 1].f0C) / sp[11];
        sp[1] = (TrackObjectKeyframes[arg1].f10 - TrackObjectKeyframes[arg1 - 1].f10) / sp[11];
        sp[2] = (TrackObjectKeyframes[arg1].f14 - TrackObjectKeyframes[arg1 - 1].f14) / sp[11];
        sp[3] = (TrackObjectKeyframes[arg1 + 1].f0C - TrackObjectKeyframes[arg1].f0C) / d1;
        sp[4] = (TrackObjectKeyframes[arg1 + 1].f10 - TrackObjectKeyframes[arg1].f10) / d1;
        sp[5] = (TrackObjectKeyframes[arg1 + 1].f14 - TrackObjectKeyframes[arg1].f14) / d1;
        sp[6] = ((sp[0] * (20 - w)) >> 1) + ((sp[3] * w) >> 1);
        sp[7] = ((sp[1] * (20 - w)) >> 1) + ((sp[4] * w) >> 1);
        sp[8] = ((sp[2] * (20 - w)) >> 1) + ((sp[5] * w) >> 1);
        TrackObjectStates[arg0].f0C += sp[6] / 10;
        TrackObjectStates[arg0].f10 += sp[7] / 10;
        TrackObjectStates[arg0].f14 += sp[8] / 10;
    } else if ((arg1 - 1) >= TrackObjectDefs[arg0].f24 &&
               (TrackObjectKeyframes[arg1 - 1].f08 % 10) == 8 &&
               (TrackObjectStates[arg0].f00 - 10) <= (TrackObjectKeyframes[arg1 - 1].f00 * 6)) {
        d2 = (TrackObjectKeyframes[arg1 - 1].f00 - TrackObjectKeyframes[arg1 - 2].f00) * 6;
        sp[0] = (TrackObjectKeyframes[arg1].f0C - TrackObjectKeyframes[arg1 - 1].f0C) / sp[11];
        sp[1] = (TrackObjectKeyframes[arg1].f10 - TrackObjectKeyframes[arg1 - 1].f10) / sp[11];
        sp[2] = (TrackObjectKeyframes[arg1].f14 - TrackObjectKeyframes[arg1 - 1].f14) / sp[11];
        sp[3] = (TrackObjectKeyframes[arg1 - 1].f0C - TrackObjectKeyframes[arg1 - 2].f0C) / d2;
        sp[4] = (TrackObjectKeyframes[arg1 - 1].f10 - TrackObjectKeyframes[arg1 - 2].f10) / d2;
        sp[5] = (TrackObjectKeyframes[arg1 - 1].f14 - TrackObjectKeyframes[arg1 - 2].f14) / d2;
        sp[6] = ((sp[0] * (sp[10] + 9)) >> 1) + ((sp[3] * (11 - sp[10])) >> 1);
        sp[7] = ((sp[1] * (sp[10] + 9)) >> 1) + ((sp[4] * (11 - sp[10])) >> 1);
        sp[8] = ((sp[2] * (sp[10] + 9)) >> 1) + ((sp[5] * (11 - sp[10])) >> 1);
        TrackObjectStates[arg0].f0C += sp[6] / 10;
        TrackObjectStates[arg0].f10 += sp[7] / 10;
        TrackObjectStates[arg0].f14 += sp[8] / 10;
    } else {
        TrackObjectStates[arg0].f0C = ((((((TrackObjectKeyframes[arg1].f0C - TrackObjectKeyframes[arg1 - 1].f0C) / sp[14]) * (100 - TrackObjectKeyframes[arg1].f04)) / 100) * sp[10] * sp[12]) / sp[11]) / sp[13] + TrackObjectKeyframes[arg1 - arg3].f0C;
        TrackObjectStates[arg0].f10 = ((((((TrackObjectKeyframes[arg1].f10 - TrackObjectKeyframes[arg1 - 1].f10) / sp[14]) * (100 - TrackObjectKeyframes[arg1].f04)) / 100) * sp[10] * sp[12]) / sp[11]) / sp[13] + TrackObjectKeyframes[arg1 - arg3].f10;
        TrackObjectStates[arg0].f14 = ((((((TrackObjectKeyframes[arg1].f14 - TrackObjectKeyframes[arg1 - 1].f14) / sp[14]) * (100 - TrackObjectKeyframes[arg1].f04)) / 100) * sp[10] * sp[12]) / sp[11]) / sp[13] + TrackObjectKeyframes[arg1 - arg3].f14;
    }

    TrackObjectStates[arg0].f1C = TrackObjectKeyframes[arg1 - arg3].f1C + ((((((TrackObjectKeyframes[arg1].f1C - TrackObjectKeyframes[arg1 - 1].f1C) / sp[14]) * (100 - TrackObjectKeyframes[arg1].f04)) / 100) * sp[10] * sp[12]) / sp[11]) / sp[13];
    TrackObjectStates[arg0].f1E = TrackObjectKeyframes[arg1 - arg3].f1E + ((((((TrackObjectKeyframes[arg1].f1E - TrackObjectKeyframes[arg1 - 1].f1E) / sp[14]) * (100 - TrackObjectKeyframes[arg1].f04)) / 100) * sp[10] * sp[12]) / sp[11]) / sp[13];
    TrackObjectStates[arg0].f20 = TrackObjectKeyframes[arg1 - arg3].f20 + ((((((TrackObjectKeyframes[arg1].f20 - TrackObjectKeyframes[arg1 - 1].f20) / sp[14]) * (100 - TrackObjectKeyframes[arg1].f04)) / 100) * sp[10] * sp[12]) / sp[11]) / sp[13];

    if (TrackObjectKeyframes[arg1].f04 != 0) {
        if ((TrackObjectKeyframes[arg1].f08 % 10) == 4 || (TrackObjectKeyframes[arg1].f08 % 10) == 5) {
            TrackObjectStates[arg0].f0C += ((((TrackObjectKeyframes[arg1].f0C - TrackObjectKeyframes[arg1 - 1].f0C) / sp[14]) * sp[10]) / sp[11]) * TrackObjectKeyframes[arg1].f04 / 100;
            TrackObjectStates[arg0].f10 += ((((TrackObjectKeyframes[arg1].f10 - TrackObjectKeyframes[arg1 - 1].f10) / sp[14]) * sp[10]) / sp[11]) * TrackObjectKeyframes[arg1].f04 / 100;
            TrackObjectStates[arg0].f14 += ((((TrackObjectKeyframes[arg1].f14 - TrackObjectKeyframes[arg1 - 1].f14) / sp[14]) * sp[10]) / sp[11]) * TrackObjectKeyframes[arg1].f04 / 100;
            TrackObjectStates[arg0].f1C += ((((TrackObjectKeyframes[arg1].f1C - TrackObjectKeyframes[arg1 - 1].f1C) / sp[14]) * sp[10]) / sp[11]) * TrackObjectKeyframes[arg1].f04 / 100;
            TrackObjectStates[arg0].f1E += ((((TrackObjectKeyframes[arg1].f1E - TrackObjectKeyframes[arg1 - 1].f1E) / sp[14]) * sp[10]) / sp[11]) * TrackObjectKeyframes[arg1].f04 / 100;
            TrackObjectStates[arg0].f20 += ((((TrackObjectKeyframes[arg1].f20 - TrackObjectKeyframes[arg1 - 1].f20) / sp[14]) * sp[10]) / sp[11]) * TrackObjectKeyframes[arg1].f04 / 100;
        }
    }
}

void UpdateTrackObjects(void) {
    int i;
    int s0;
    int *sp = (int *)(SCRATCHPAD + 0x7C);
    int v1, a0, a2;
    int t0, t1;
    int r1, r2, r3, r5, r6;
    register int r4 asm("v0"); /* ASM_BARRIER: force modulo result into v0 before copying to a3 */
    int *hw;

    UpdateTrackObjectActivation();
    for (i = 0; i < TrackObjectCount; i++) {
            if (B4->f24 == 1) {
                if (B0->f8 == 0xA) {
                    if (B4->f8 == 0) {
                        if (RaceType != 1) {
                            B4->f8 = 0xB;
                        } else if (CourseIndex == 7) {
                            if (D_800AC1A2 != 0) {
                                B4->f8 = D_800AC1A2 + 0xA;
                            } else {
                                B4->f8 = 0xB;
                            }
                        } else if (D_800AC1A2 <= 0 || (CourseIndex == 6 && D_800AC1A2 < 2)) {
                            B4->f8 = 0xB;
                        } else {
                            switch (D_800AC0E6 + 1) {
                            case 1:
                                B4->f8 = 0xC;
                                break;
                            case 2:
                            case 3:
                                B4->f8 = 0xB;
                                break;
                            default:
                                B4->f8 = 0xD;
                                break;
                            }
                        }
                    }
                }
                if (B0->f4 == 5) {
                    if (B4->f8 == 0 || B4->f0 >= 0xD1) {
                        if (B0->f0 > 0 && (PsyqRand() % (B0->f0 * 8 + 0x14)) != 0) {
                            B4->f8 = 0;
                            goto next_iter;
                        }
                        r1 = PsyqRand();
                        B4->fC = AC(B0->f24)->fC + (r1 % AC(B0->f24)[1].fC);
                        r2 = PsyqRand();
                        B4->f10 = AC(B0->f24)->f10 + (r2 % AC(B0->f24)[1].f10);
                        r3 = PsyqRand();
                        B4->f14 = AC(B0->f24)->f14 + (r3 % AC(B0->f24)[1].f14);
                        r4 = PsyqRand() % 6;
                        B4->f8 = r4 + 1;
                        func_80070934(B0->f0, 7, &B4->fC, r4);
                        B4->f0 = 0;
                    } else if (B4->f0 == 0x3C) {
                        hw = (int *)SCRATCHPAD;
                        sp[0] = (B4->fC - hw[2]) >> 10;
                        sp[1] = (B4->f14 - hw[4]) >> 10;
                        sp[2] = CordicSqrt((sp[0] * sp[0] + sp[1] * sp[1]) >> 8) >> 4;
                        B4->f4 = sp[2] - 0x46;
                    }
                    B4->f0++;
                } else {
                    s0 = B0->f24 + B4->f28;
                    if (B4->f0 == AC(s0)->f0 * 6) {
                        if (s0 < B0->f28 + B0->f24) {
                            B4->f28++;
                            s0++;
                            if (B4->f0 == B0->f10 * 6) {
                                B4->f0 = 0;
                                B4->f28 = 1;
                                s0 = B0->f24 + 1;
                            }
                            v1 = AC(s0)->f8 / 10;
                            if (v1 == 1) {
                                B4->f4 = (B4->f4 % 100) + 300;
                            } else if (v1 == 0) {
                                B4->f4 = B4->f4 % 100;
                            } else if (v1 == 4) {
                                B4->f4 = (B4->f4 % 100) + 100;
                            } else if (v1 == 5) {
                                B4->f4 = (B4->f4 % 100) + 200;
                            }
                        }
                    }
                    if (B0->f10 == 1) {
                        B4->fC = AC(s0)->fC;
                        B4->f10 = AC(s0)->f10;
                        B4->f14 = AC(s0)->f14;
                        B4->f1C = AC(s0)->f1C;
                        B4->f1E = AC(s0)->f1E;
                        B4->f20 = AC(s0)->f20;
                    } else {
                        v1 = B4->f4 / 100;
                        if (v1 == 1) {
                            B4->f4 = (B4->f4 == 0x86) ? 0x73 : B4->f4 + 1;
                        } else if (v1 == 2) {
                            if ((r5 = B4->f4 - 0xC8) != 0) {
                                B4->f4 = (B4->f4 == 0xF4) ? 0xC8 : B4->f4 + 1;
                            }
                        }
                        if ((unsigned short)RaceType - 2 < 3U) {
                            a2 = B4->f0;
                            B4->f0 = a2 + 1;
                            if (D_8010E7C8[i] == ModeFrame) {
                                B4->f0 = a2;
                            }
                            D_8010E7C8[i] = ModeFrame;
                        } else {
                            B4->f0++;
                        }
                        v1 = AC(s0)[-1].f0;
                        t0 = (AC(s0)->f0 - v1) * 6;
                        t1 = B4->f0 - v1 * 6;
                        if (t0 == 0) {
                            B4->f24 = 0;
                        } else {
                            switch (AC(s0)->f8 % 10) {
                            case 1:
                            case 4:
                                InterpolateObjectKeyframe(i, s0, 1, 1, t1, t0, t1, t0);
                                break;
                            case 2:
                            case 5:
                                InterpolateObjectKeyframe(i, s0, -1, 0, t0 - t1, t0, t0 - t1, t0);
                                break;
                            case 3:
                                t0 /= 2;
                                if (t1 < t0) {
                                    InterpolateObjectKeyframe(i, s0, 2, 1, t1, t0, t1, t0);
                                } else {
                                    InterpolateObjectKeyframe(i, s0, -2, 0, t0 * 2 - t1, t0, t0 * 2 - t1, t0);
                                }
                                break;
                            default:
                                InterpolateObjectKeyframe(i, s0, 1, 1, t1, t0, 1, 1);
                                break;
                            }
                            if (AC(s0)->f8 / 10 == 1) {
                                switch (B0->fC % 10) {
                                case 2:
                                case 5:
                                    sp[0] = B0->fC / 10;
                                    sp[1] = (sp[0] << 4) + (sp[0] << 2);
                                    if (B4->f0 == 1 || B4->f4 == 300) {
                                        for (a0 = 0; a0 < 20; a0++) {
                                            D_8010E8E8[sp[1] + a0].a[0] = B4->fC >> 3;
                                            D_8010E8E8[sp[1] + a0].a[1] = B4->f10 >> 3;
                                            D_8010E8E8[sp[1] + a0].a[2] = B4->f14 >> 3;
                                        }
                                    } else if (B4->f0 % 2 == 1) {
                                        for (a0 = 19; a0 > 0; a0--) {
                                            D_8010E8E8[sp[1] + a0] = D_8010E8E8[sp[1] + a0 - 1];
                                        }
                                        D_8010E8E8[sp[1]].a[0] = B4->fC >> 3;
                                        D_8010E8E8[sp[1]].a[1] = B4->f10 >> 3;
                                        D_8010E8E8[sp[1]].a[2] = B4->f14 >> 3;
                                    }
                                    break;
                                case 3:
                                    if (B4->f0 == 1) {
                                        for (a0 = 0; a0 < 40; a0++) {
                                            D_8010E8E8[a0].a[0] = B4->fC >> 3;
                                            D_8010E8E8[a0].a[1] = B4->f10 >> 3;
                                            D_8010E8E8[a0].a[2] = B4->f14 >> 3;
                                        }
                                    } else if (B4->f0 % 12 == 1) {
                                        for (a0 = 39; a0 > 0; a0--) {
                                            D_8010E8E8[a0] = D_8010E8E8[a0 - 1];
                                        }
                                        D_8010E8E8[0].a[0] = B4->fC >> 3;
                                        D_8010E8E8[0].a[1] = B4->f10 >> 3;
                                        D_8010E8E8[0].a[2] = B4->f14 >> 3;
                                    }
                                    break;
                                }
                            }
                            if (B0->f8 == 0xD) {
                                if (RaceState == 1) {
                                    switch (B4->f8) {
                                    case 0:
                                    case 4:
                                        if (PlayerControllers[4] & 0x8000) {
                                            B4->f8++;
                                        } else if (*(int *)&PlayerControllers[4] & 0x7000) {
                                            B4->f8 = 5;
                                        }
                                        break;
                                    case 1:
                                    case 2:
                                    case 3:
                                        if (PlayerControllers[4] & 0x2000) {
                                            B4->f8++;
                                        } else if (*(int *)&PlayerControllers[4] & 0xD000) {
                                            B4->f8 = 5;
                                        }
                                        break;
                                    }
                                    D_8010E7A8 = 0x37;
                                    D_8010E7AC = 0x36;
                                    D_8010E7B0 = 0x35;
                                    D_8010E7B4 = 0x20;
                                    if (ModeFrame == 0xAF) {
                                        if (B4->f8 != 4) {
                                            B4->f24 = -1;
                                        }
                                    }
                                } else {
                                    D_800A4090 += D_8010E7B4;
                                    if (D_800A4090 >= 0x80) {
                                        D_800A4090 -= 0x80;
                                        a0 = D_8010E7B0 - 1;
                                        if (D_8010E7B0 <= 0) {
                                            a0 += TrackNodeCount;
                                        }
                                        D_8010E7B0 = a0;
                                        v1 = D_8010E7AC - 1;
                                        if (D_8010E7AC <= 0) {
                                            v1 += TrackNodeCount;
                                        }
                                        D_8010E7AC = v1;
                                        v1 = D_8010E7A8 - 1;
                                        if (D_8010E7A8 <= 0) {
                                            v1 += TrackNodeCount;
                                        }
                                        D_8010E7A8 = v1;
                                    }
                                    sp[2] = (((AB(D_8010E7AC)->f8 - AB(D_8010E7A8)->f8) * D_800A4090) >> 5) + AB(D_8010E7A8)->f8 * 4;
                                    sp[3] = (((AB(D_8010E7AC)->f10 - AB(D_8010E7A8)->f10) * D_800A4090) >> 7) + AB(D_8010E7A8)->f10;
                                    sp[4] = (((AB(D_8010E7AC)->fC - AB(D_8010E7A8)->fC) * D_800A4090) >> 5) + AB(D_8010E7A8)->fC * 4;
                                    sp[5] = ((((AB(D_8010E7B0)->f8 - AB(D_8010E7AC)->f8) * D_800A4090) >> 5) + AB(D_8010E7AC)->f8 * 4) - sp[2];
                                    sp[6] = ((((AB(D_8010E7B0)->f10 - AB(D_8010E7AC)->f10) * D_800A4090) >> 7) + AB(D_8010E7AC)->f10) - sp[3];
                                    sp[7] = ((((AB(D_8010E7B0)->fC - AB(D_8010E7AC)->fC) * D_800A4090) >> 5) + AB(D_8010E7AC)->fC * 4) - sp[4];
                                    B4->fC += sp[2];
                                    B4->f10 += sp[3] - 0xFA;
                                    B4->f14 += sp[4];
                                    sp[8] = 0x400 - Atan2Table(sp[5], sp[7]);
                                    if (sp[8] == D_8010E7B8) {
                                        D_8010E7B4 += (D_8010E7B4 < 0x2C);
                                    } else if ((sp[8] >> 4) == (D_8010E7B8 >> 4)) {
                                        if (D_8010E7B4 < 0x26) {
                                            D_8010E7B4++;
                                        } else if (D_8010E7B4 >= 0x27) {
                                            D_8010E7B4--;
                                        }
                                    } else {
                                        D_8010E7B4 -= (D_8010E7B4 >= 0x22);
                                    }
                                    r6 = sp[8];
                                    B4->f1E = r6;
                                    D_8010E7B8 = r6;
                                    B4->f1C = 0x400 - Atan2Table(sp[6], CordicSqrt(sp[5] * sp[5] + sp[7] * sp[7]) >> 6);
                                    B4->f20 = 0;
                                }
                            }
                            if (B0->f14 > 0) {
                                if (B4->f0 == B0->f14 * 6) {
                                    B4->f24 = -1;
                                }
                            }
                        }
                    }
                }
            }
next_iter:
        ;
    }
}
