#include "types.h"
#include "scratchpad.h"
#include "rr/lookup.h"
#include "rr_asm.h"
#include "game/data/cdfile2.h"
#include "game/gfx/gfxmtx.h"
#include "game/gfx/gtepoly1.h"
#include "psyq/libgte/gtemat3.h"
#include "game/race/raceinit.h"
#include "game/car/carwheel.h"
#include "rr/race_setup.h"

#define G ((Scratch2D784 *)SCRATCHPAD)

typedef struct {
    s32 m[8];
} Mat8;

typedef struct {
    s32 x, y, z, w;
} Vec4;

typedef struct {
    u8 pad00[0x10];
    s32 x10, y14, z18;
    u8 pad1C[0x50 - 0x1C];
    s32 rot50, rot54, rot58, pad5C;
    s32 x60, y64, z68, pad6C;
    s32 x70, y74, z78, pad7C;
    s16 ang80, ang82, ang84;
    u8 pad86[0xC8 - 0x86];
    Vec4 rotC8;
    u8 padD8[0x178 - 0xD8];
    s32 lat178, unk17C;
    u8 pad180[0x1A8 - 0x180];
    s16 unk1A8, unk1AA;
    u8 pad1AC[0x1E6 - 0x1AC];
    s16 slot1E6;
} Vehicle2CC88;

typedef struct {
    s16 w0;
    u16 w2;
    u16 w4;
    u8 pad6[4];
    u8 bA;
} Bounds2CC88;

typedef struct {
    u8 pad00[0x10];
    s32 x10, y14, z18;
    u8 pad1C[0x50 - 0x1C];
    s32 rot50, rot54, rot58, pad5C;
    s32 x60, y64, z68, pad6C;
    s32 x70, y74, z78, pad7C;
    s16 ang80, ang82, ang84;
    u8 pad86[0xC8 - 0x86];
    Vec4 rotC8;
    u8 padD8[0x178 - 0xD8];
    s32 lat178, unk17C;
    u8 pad180[0x1A8 - 0x180];
    s16 unk1A8, unk1AA;
    u8 pad1AC[0x1E4 - 0x1AC];
    u16 flags1E4;
    s16 slot1E6;
    u8 pad1E8[0x23A - 0x1E8];
    s16 drag23A;
    u8 pad23C[0x2A2 - 0x23C];
    s16 unk2A2;
} Vehicle2D1B4;

typedef struct {
    s16 w0;
    u16 w2;
    u16 w4;
    u8 pad6[4];
    u8 bA;
} Bounds2D1B4;

typedef struct {
    u16 pad0;
    u16 flags2;
    u8 pad4[0x2C - 4];
} LightEntry2C;

typedef struct {
    u8 pad00[0x10];
    s32 x10, y14, z18;
    u8 pad1C[0x50 - 0x1C];
    s32 rot50, rot54, rot58, pad5C;
    s32 x60, y64, z68, pad6C;
    s32 x70, y74, z78, pad7C;
    s16 ang80, ang82, ang84;
    u8 pad86[0xC8 - 0x86];
    Vec4 rotC8;
    u8 padD8[0x178 - 0xD8];
    s32 lat178, unk17C;
    u8 pad180[0x1A8 - 0x180];
    s16 unk1A8, unk1AA;
    u8 pad1AC[0x1E4 - 0x1AC];
    u16 flags1E4;
    s16 slot1E6;
    u8 pad1E8[0x23A - 0x1E8];
    s16 drag23A;
    u8 pad23C[0x2A2 - 0x23C];
    s16 unk2A2;
} Vehicle2D784;

typedef struct {
    s16 w0;
    u16 w2;
    u16 w4;
    u8 pad6[4];
    u8 bA;
} Bounds2D784;

typedef struct {
    s16 d0, d2, d4;
} LodDist6;

typedef struct {
    s32 pad00[2];
    s32 x08;
    s32 y0C;
    s32 z10;
    s32 pad14[25];
    s32 col78;
} Scratch2D784;

void func_8005F0D0(s32, s32, s32, s32);
void BuildRotMatrixZ(Mat8 *, s32);
void BuildRotMatrixY(Mat8 *, s32);
void BuildRotMatrixX(Mat8 *, s32);
void SetGteObjectTransform(s32, Vec4 *, Mat8 *);
void SelectModelBank(s32);
void MulMatrix0(void *, Mat8 *, Mat8 *);
void MulMatrix(Mat8 *, Mat8 *);
void MulMatrix2(Mat8 *, Mat8 *);
extern u8 D_8009F02C[];
extern u8 D_8009F034[];
extern u8 D_8009F03C[];
extern u8 D_8009F044[];
extern s16 LightMatrix[];
extern s32 ModelBankFormat;
extern Bounds2CC88 *CarBoundsTable[];
s32 func_80039494(Vehicle2D1B4 *);
extern LightEntry2C *TrackLightTable;
extern LodDist6 D_8009F04C[];

int TriggerWheelBump(char *a, int b);
void DrawPreviewCar(Vehicle2CC88 *arg0, s32 arg1, s32 arg2);
void DrawPlayerCar(Vehicle2D1B4 *arg0);
void DrawOpponentCar(Vehicle2D784 *arg0, s32 arg1);

int TriggerWheelBump(char *a, int b)
{
    char *p;
    int t;
    if (*(short *)(a + 0x1D8) < 0x320) goto fail;
    p = a + b * 2;
    t = LookupTrackEvent(*(short *)(a + 0x1EA), *(short *)(p + 0x100));
    if (t == -1) {
fail:
        return 0;
    }
    *(short *)(p + 0x1C4) = -(t * *(short *)(a + 0x1D8) / 2800);
    *(short *)(p + 0x1BC) = 1;
    *(unsigned short *)(a + 0x1B8) ^= 1 << b;
    return 1;
}

void DrawPreviewCar(Vehicle2CC88 *arg0, s32 arg1, s32 arg2) {
    Mat8 sp10;
    Mat8 sp30;
    Mat8 sp50;
    Mat8 sp70;
    s16 sp90[4];
    Mat8 sp98[6];
    Vec4 sp158[5];
    s32 slot_half;
    s32 s4_val;
    s32 i;
    u8 a1_val;

    slot_half = arg0->slot1E6 >> 1;
    SelectModelBank(slot_half);
    SetLightColorsScaled(arg2);

    BuildRotMatrixY(&sp70, -arg0->rot54);
    BuildRotMatrixX(&sp30, -arg0->rot50);
    BuildRotMatrixZ(&sp50, -arg0->rot58);
    MulMatrix(&sp30, &sp50);
    MulMatrix(&sp70, &sp30);
    MulMatrix0(LightMatrix, &sp70, &sp50);
    RR_GTE_SETLLM(&sp50);

    BuildRotMatrixY(&sp10, -arg0->rot54);
    MulMatrix2((Mat8 *)SCRATCH_ROT_MATRIX, &sp10);
    BuildRotMatrixX(&sp98[0], arg0->unk1A8 - arg0->rot50);
    BuildRotMatrixZ(&sp50, arg0->unk1AA - arg0->rot58);
    MulMatrix(&sp98[0], &sp50);
    MulMatrix2(&sp10, &sp98[0]);

    sp158[0] = arg0->rotC8;
    sp158[1].x = arg0->x70 + arg0->x60;
    sp158[1].y = arg0->y74 + arg0->y64;
    sp158[1].z = arg0->z78 + arg0->z68;

    BuildRotMatrixX(&sp98[1], -arg0->ang80);
    BuildRotMatrixZ(&sp50, -arg0->ang84);
    MulMatrix(&sp98[1], &sp50);
    MulMatrix2(&sp10, &sp98[1]);

    sp158[2].x = arg0->x10 + arg0->x60;
    sp158[2].y = arg0->y14 + arg0->y64;
    sp158[2].z = arg0->z18 + arg0->z68;

    {
        /* ASM_BARRIER: load unk17C into v0 before copying to s4_val */
        register s32 v0_tmp asm("v0") = arg0->unk17C;
        /* ASM_BARRIER: preserve v0 load before move to s4 */
        __asm__ volatile("" : "+r"(v0_tmp));
        s4_val = v0_tmp;
    }
    if (s4_val & 0x1000) {
        s4_val = -s4_val;
    }
    if (CarBoundsTable[slot_half]->bA == 1) {
        BuildRotMatrixY(&sp98[3], s4_val);
    } else {
        BuildRotMatrixX(&sp98[3], s4_val);
    }

    sp98[2] = sp30;
    MulMatrix2(&sp10, &sp98[2]);
    MulMatrix2(&sp98[2], &sp98[3]);
    BuildRotMatrixY(&sp98[4], arg0->lat178 / 12);
    BuildRotMatrixX(&sp98[5], s4_val);
    MulMatrix2(&sp98[2], &sp98[4]);
    MulMatrix2(&sp98[4], &sp98[5]);

    RR_GTE_SETROT(&sp70);
    for (i = 0; i < 2; i++) {
        /* ASM_BARRIER: pin v0_w to v0 to match branch delay slot and load scheduling */
        register s32 v0_w asm("v0") = CarBoundsTable[slot_half]->w0;
        if (i & 1) {
            v0_w = -v0_w;
        }
        sp90[0] = v0_w * 4;
        sp90[1] = CarBoundsTable[slot_half]->w2 * 4;
        sp90[2] = -CarBoundsTable[slot_half]->w4 * 4;
        RR_GTE_MVMVA_ROT_V0_NOCV(sp90);
        RR_GTE_STORE_IR(&sp158[i + 3]);
        sp158[i + 3].x += sp158[2].x;
        sp158[i + 3].y += sp158[2].y;
        sp158[i + 3].z += sp158[2].z;
    }

    for (i = 0; i < 8; i++) {
        u8 s0_mode = D_8009F034[i];
        if (s0_mode != 1 || DrawRroModel(&sp158[D_8009F03C[i]], &sp98[D_8009F044[i]], D_8009F02C[i], 0) != s0_mode) {
            SetGteObjectTransform((SCRATCHPAD + 0x7C), &sp158[D_8009F03C[i]], &sp98[D_8009F044[i]]);
            *(s32 *)(SCRATCHPAD + 0x78) = 0;
            a1_val = 1;
            if (D_8009F02C[i] < ModelBankFormat) {
                a1_val = D_8009F02C[i];
            }
            func_8005F0D0(SCRATCHPAD, a1_val, i != 0, 0);
        }
    }
}

/* legacy typing of shared globals in this part of the unit */
#define CarBoundsTable (*(Bounds2D1B4 *(*)[])&CarBoundsTable)

void DrawPlayerCar(Vehicle2D1B4 *arg0) {
    Mat8 sp20;
    Mat8 sp40;
    Mat8 sp60;
    Mat8 sp80;
    s16 spA0[4];
    Mat8 spA8[6];
    Vec4 sp168[5];
    s32 slot_half;
    s32 slot_idx;
    s32 s5_val;
    s32 v0_drag;
    s32 s4_val;
    s32 i;
    u8 a1_val;

    slot_half = arg0->slot1E6 >> 1;
    SelectModelBank(slot_half);
    s5_val = func_80013904(arg0, func_80039494(arg0));
    BuildRotMatrixY(&sp80, -arg0->rot54);
    BuildRotMatrixX(&sp40, -arg0->rot50);
    BuildRotMatrixZ(&sp60, -arg0->rot58);
    MulMatrix(&sp40, &sp60);
    MulMatrix(&sp80, &sp40);
    MulMatrix0(LightMatrix, &sp80, &sp60);
    RR_GTE_SETLLM(&sp60);
    BuildRotMatrixY(&sp20, -arg0->rot54);
    MulMatrix2((Mat8 *)SCRATCH_ROT_MATRIX, &sp20);
    BuildRotMatrixX(&spA8[0], arg0->unk1A8 - arg0->rot50);
    BuildRotMatrixZ(&sp60, arg0->unk1AA - arg0->rot58);
    MulMatrix(&spA8[0], &sp60);
    MulMatrix2(&sp20, &spA8[0]);
    sp168[0] = arg0->rotC8;
    sp168[1].x = arg0->x70 + arg0->x60;
    sp168[1].y = arg0->y74 + arg0->y64;
    sp168[1].z = arg0->z78 + arg0->z68;
    BuildRotMatrixX(&spA8[1], -arg0->ang80);
    BuildRotMatrixZ(&sp60, -arg0->ang84);
    MulMatrix(&spA8[1], &sp60);
    MulMatrix2(&sp20, &spA8[1]);
    sp168[2].x = arg0->x10 + arg0->x60;
    sp168[2].y = arg0->y14 + arg0->y64;
    sp168[2].z = arg0->z18 + arg0->z68;
    {
        /* ASM_BARRIER: load unk17C into v0 before copying to s4_val */
        register s32 v0_tmp asm("v0") = arg0->unk17C;
        /* ASM_BARRIER: preserve v0 load before move to s4 */
        __asm__ volatile("" : "+r"(v0_tmp));
        s4_val = v0_tmp;
    }
    if (s4_val & 0x1000) {
        s4_val = -s4_val;
    }
    if (CarBoundsTable[slot_half]->bA == 1) {
        BuildRotMatrixY(&spA8[3], s4_val);
    } else {
        BuildRotMatrixX(&spA8[3], s4_val);
    }
    spA8[2] = sp40;
    MulMatrix2(&sp20, &spA8[2]);
    MulMatrix2(&spA8[2], &spA8[3]);
    BuildRotMatrixY(&spA8[4], arg0->lat178 / 12);
    BuildRotMatrixX(&spA8[5], s4_val);
    MulMatrix2(&spA8[2], &spA8[4]);
    MulMatrix2(&spA8[4], &spA8[5]);
    RR_GTE_SETROT(&sp80);
    for (i = 0; i < 2; i++) {
        /* ASM_BARRIER: pin v0_w to v0 to match branch delay slot and load scheduling */
        register s32 v0_w asm("v0") = CarBoundsTable[slot_half]->w0;
        if (i & 1) {
            v0_w = -v0_w;
        }
        spA0[0] = v0_w * 4;
        spA0[1] = CarBoundsTable[slot_half]->w2 * 4;
        spA0[2] = -CarBoundsTable[slot_half]->w4 * 4;
        RR_GTE_MVMVA_ROT_V0_NOCV(spA0);
        RR_GTE_STORE_IR(&sp168[i + 3]);
        sp168[i + 3].x += sp168[2].x;
        sp168[i + 3].y += sp168[2].y;
        sp168[i + 3].z += sp168[2].z;
    }
    s5_val = (s16)(TrackLightTable[s5_val].flags2 | D_800ABEF0);
    slot_idx = arg0->slot1E6 + D_800ABEC4;
    if (arg0->flags1E4 & 0x10) {
        v0_drag = arg0->unk2A2;
    } else {
        v0_drag = arg0->drag23A;
    }
    if (v0_drag > 0) {
        s5_val |= 2;
    }
    DrawWheelTrailEffects(arg0, &sp168[1], &spA8[1], &sp80, slot_idx, s5_val, 0);
    for (i = 0; i < 8; i++) {
        u8 s0_mode = D_8009F034[i];
        if (s0_mode != 1 || DrawRroModel(&sp168[D_8009F03C[i]], &spA8[D_8009F044[i]], D_8009F02C[i], 0) != s0_mode) {
            SetGteObjectTransform((SCRATCHPAD + 0x7C), &sp168[D_8009F03C[i]], &spA8[D_8009F044[i]]);
            *(s32 *)(SCRATCHPAD + 0x78) = 0;
            a1_val = 1;
            if (D_8009F02C[i] < ModelBankFormat) {
                a1_val = D_8009F02C[i];
            }
            func_8005F0D0(SCRATCHPAD, a1_val, i != 0, 0);
        }
    }
}

#undef CarBoundsTable

/* legacy typing of shared globals in this part of the unit */
#define func_80013904 ((s32 (*)(Vehicle2D784 *, s32))func_80013904)
#define DrawWheelTrailEffects ((void (*)(Vehicle2D784 *, Vec4 *, Mat8 *, Mat8 *, s32, s32, s32))DrawWheelTrailEffects)
#define func_80039494 ((s32 (*)(Vehicle2D784 *))func_80039494)
#define CarBoundsTable (*(Bounds2D784 *(*)[])&CarBoundsTable)

void DrawOpponentCar(Vehicle2D784 *arg0, s32 arg1) {
    Mat8 sp20;
    Mat8 sp40;
    Mat8 sp60;
    Mat8 sp80;
    s16 spA0[4];
    Mat8 spA8[6];
    Vec4 sp168[5];
    s32 slot_half;
    s32 slot_idx;
    s32 fp_val;
    s32 s7_val;
    s32 s5_val;
    s32 v0_drag;
    s32 s4_val;
    s32 i;
    u8 a1_val;

    slot_half = arg0->slot1E6 >> 1;
    fp_val = (arg0->slot1E6 & 1) * 0x05000000;
    slot_idx = arg0->slot1E6 + D_800ABEC4;
    SelectModelBank(slot_half);

    sp168[0].x = (arg0->x10 - G->x08) >> 2;
    sp168[0].y = 0;
    sp168[0].z = (arg0->z18 - G->z10) >> 2;
    ApplyMatrixLV(SCRATCH_ROT_MATRIX, &sp168[0], &sp168[1]);
    if (sp168[0].x < 0) {
        sp168[0].x = -sp168[0].x;
    }
    if (sp168[0].z < 0) {
        sp168[0].z = -sp168[0].z;
    }
    i = sp168[0].x + sp168[0].z;
    if (sp168[1].z >= 0) {
        s7_val = 0;
        if (arg1 == 1) {
            s7_val = -0x5A;
        }
        s5_val = func_80013904(arg0, func_80039494(arg0));
        s5_val = (s16)(TrackLightTable[s5_val].flags2 | D_800ABEF0);
        if (arg0->flags1E4 & 0x10) {
            v0_drag = arg0->unk2A2;
        } else {
            v0_drag = arg0->drag23A;
        }
        if (v0_drag > 0) {
            s5_val |= 2;
        }
        s5_val &= 3;
        /* ASM_BARRIER: boost s5_val reference count so global-alloc assigns s5 before &sp80 */
        __asm__ volatile("" : "+r"(s5_val));
        /* ASM_BARRIER: second reference boost so floor_log2(n_refs) reaches 4 */
        __asm__ volatile("" : "+r"(s5_val));
        BuildRotMatrixY(&sp80, -arg0->rot54);
        BuildRotMatrixX(&sp40, -arg0->rot50);
        BuildRotMatrixZ(&sp60, -arg0->rot58);
        MulMatrix(&sp40, &sp60);
        MulMatrix(&sp80, &sp40);
        MulMatrix0(LightMatrix, &sp80, &sp60);
        RR_GTE_SETLLM(&sp60);
        if (i < D_8009F04C[arg1].d0) {
            BuildRotMatrixY(&sp20, -arg0->rot54);
            MulMatrix2((Mat8 *)SCRATCH_ROT_MATRIX, &sp20);
            BuildRotMatrixX(&spA8[0], arg0->unk1A8 - arg0->rot50);
            BuildRotMatrixZ(&sp60, arg0->unk1AA - arg0->rot58);
            MulMatrix(&spA8[0], &sp60);
            MulMatrix2(&sp20, &spA8[0]);
            sp168[0] = arg0->rotC8;
            sp168[1].x = arg0->x70 + arg0->x60;
            sp168[1].y = arg0->y74 + arg0->y64;
            sp168[1].z = arg0->z78 + arg0->z68;
            BuildRotMatrixX(&spA8[1], -arg0->ang80);
            BuildRotMatrixZ(&sp60, -arg0->ang84);
            MulMatrix(&spA8[1], &sp60);
            MulMatrix2(&sp20, &spA8[1]);
            sp168[2].x = arg0->x10 + arg0->x60;
            sp168[2].y = arg0->y14 + arg0->y64;
            sp168[2].z = arg0->z18 + arg0->z68;
            {
                /* ASM_BARRIER: load unk17C into v0 before copying to s4_val */
                register s32 v0_tmp asm("v0") = arg0->unk17C;
                /* ASM_BARRIER: preserve v0 load before move to s4 */
                __asm__ volatile("" : "+r"(v0_tmp));
                s4_val = v0_tmp;
            }
            if (s4_val & 0x1000) {
                s4_val = -s4_val;
            }
            if (CarBoundsTable[slot_half]->bA == 1) {
                BuildRotMatrixY(&spA8[3], s4_val);
            } else {
                BuildRotMatrixX(&spA8[3], s4_val);
            }
            spA8[2] = sp40;
            MulMatrix2(&sp20, &spA8[2]);
            MulMatrix2(&spA8[2], &spA8[3]);
            BuildRotMatrixY(&spA8[4], arg0->lat178 / 12);
            BuildRotMatrixX(&spA8[5], s4_val);
            MulMatrix2(&spA8[2], &spA8[4]);
            MulMatrix2(&spA8[4], &spA8[5]);
            RR_GTE_SETROT(&sp80);
            for (i = 0; i < 2; i++) {
                /* ASM_BARRIER: pin v0_w to v0 to match branch delay slot and load scheduling */
                register s32 v0_w asm("v0") = CarBoundsTable[slot_half]->w0;
                if (i & 1) {
                    v0_w = -v0_w;
                }
                spA0[0] = v0_w * 4;
                spA0[1] = CarBoundsTable[slot_half]->w2 * 4;
                spA0[2] = -CarBoundsTable[slot_half]->w4 * 4;
                RR_GTE_MVMVA_ROT_V0_NOCV(spA0);
                RR_GTE_STORE_IR(&sp168[i + 3]);
                sp168[i + 3].x += sp168[2].x;
                sp168[i + 3].y += sp168[2].y;
                sp168[i + 3].z += sp168[2].z;
            }
            DrawWheelTrailEffects(arg0, &sp168[1], &spA8[1], &sp80, slot_idx, s5_val, 0);
            for (i = 0; i < 8; i++) {
                u8 s0_mode = D_8009F034[i];
                if (s0_mode != 1 || DrawRroModel(&sp168[D_8009F03C[i]], &spA8[D_8009F044[i]], D_8009F02C[i], fp_val) != s0_mode) {
                    SetGteObjectTransform((SCRATCHPAD + 0x7C), &sp168[D_8009F03C[i]], &spA8[D_8009F044[i]]);
                    *(s32 *)(SCRATCHPAD + 0x78) = fp_val;
                    a1_val = 1;
                    if (D_8009F02C[i] < ModelBankFormat) {
                        a1_val = D_8009F02C[i];
                    }
                    func_8005F0D0(SCRATCHPAD, a1_val, i != 0, 0);
                }
            }
            return;
        } else if (i < D_8009F04C[arg1].d2) {
            BuildRotMatrixY(&sp20, -arg0->rot54);
            MulMatrix2((Mat8 *)SCRATCH_ROT_MATRIX, &sp20);
            BuildRotMatrixX(&spA8[0], arg0->unk1A8 - arg0->rot50);
            BuildRotMatrixZ(&sp60, arg0->unk1AA - arg0->rot58);
            MulMatrix(&spA8[0], &sp60);
            MulMatrix2(&sp20, &spA8[0]);
            sp168[0] = arg0->rotC8;
            sp168[1].x = arg0->x70 + arg0->x60;
            sp168[1].y = arg0->y74 + arg0->y64;
            sp168[1].z = arg0->z78 + arg0->z68;
            BuildRotMatrixX(&spA8[1], -arg0->ang80);
            BuildRotMatrixZ(&sp60, -arg0->ang84);
            MulMatrix(&spA8[1], &sp60);
            MulMatrix2(&sp20, &spA8[1]);
            DrawWheelTrailEffects(arg0, &sp168[1], &spA8[1], &sp80, slot_idx, s5_val, s7_val);
            SetGteObjectTransform((SCRATCHPAD + 0x7C), &sp168[0], &spA8[0]);
            G->col78 = 0;
            a1_val = 1;
            if (ModelBankFormat >= 4) {
                a1_val = 3;
            }
            func_8005F0D0(SCRATCHPAD, a1_val, 0, s7_val);
            if (DrawRroModel(&sp168[1], &spA8[1], 1, fp_val) == 0) {
                SetGteObjectTransform((SCRATCHPAD + 0x7C), &sp168[1], &spA8[1]);
                G->col78 = fp_val;
                func_8005F0D0(SCRATCHPAD, 1, 1, s7_val);
            }
            return;
        } else if (i < D_8009F04C[arg1].d4) {
            MulMatrix0((void *)SCRATCH_ROT_MATRIX, &sp80, &spA8[0]);
            sp168[0].x = arg0->x10;
            sp168[0].y = arg0->y14;
            sp168[0].z = arg0->z18;
            DrawWheelTrailEffects(arg0, &sp168[0], &spA8[0], &sp80, slot_idx, s5_val, s7_val);
            SetGteObjectTransform((SCRATCHPAD + 0x7C), &sp168[0], &spA8[0]);
            *(s32 *)(SCRATCHPAD + 0x78) = fp_val;
            a1_val = 1;
            if (ModelBankFormat >= 3) {
                a1_val = 2;
            }
            func_8005F0D0(SCRATCHPAD, a1_val, 1, s7_val);
            return;
        }
    }
    DrawWheelTrailEffects(arg0, 0, 0, 0, slot_idx, 0, 0);
}

#undef func_80013904
#undef DrawWheelTrailEffects
#undef func_80039494
#undef CarBoundsTable
