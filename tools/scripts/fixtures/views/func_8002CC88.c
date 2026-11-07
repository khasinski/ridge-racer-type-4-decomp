#include "rr_asm.h"

#define G ((Scratch2D784 *)0x1F800000)

/* legacy typing of shared globals in this part of the unit */
#define CarBoundsTable (*(Bounds2D1B4 *(*)[])&CarBoundsTable)

#undef CarBoundsTable

/* legacy typing of shared globals in this part of the unit */
#define func_80013904 ((s32 (*)(Vehicle2D784 *, s32))func_80013904)

#define DrawWheelTrailEffects ((void (*)(Vehicle2D784 *, Vec4 *, Mat8 *, Mat8 *, s32, s32, s32))DrawWheelTrailEffects)

#define func_80039494 ((s32 (*)(Vehicle2D784 *))func_80039494)

#define CarBoundsTable (*(Bounds2D784 *(*)[])&CarBoundsTable)

#undef func_80013904

#undef DrawWheelTrailEffects

#undef func_80039494

#undef CarBoundsTable

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

s32 DrawRroModel(Vec4 *, Mat8 *, s32, s32);

void SetLightColorsScaled(s32);

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
    MulMatrix2((Mat8 *)0x1F800028, &sp10);
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
            SetGteObjectTransform(0x1F80007C, &sp158[D_8009F03C[i]], &sp98[D_8009F044[i]]);
            *(s32 *)0x1F800078 = 0;
            a1_val = 1;
            if (D_8009F02C[i] < ModelBankFormat) {
                a1_val = D_8009F02C[i];
            }
            func_8005F0D0(0x1F800000, a1_val, i != 0, 0);
        }
    }
}
