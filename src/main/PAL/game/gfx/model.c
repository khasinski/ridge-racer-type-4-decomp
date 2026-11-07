#include "types.h"
#include "scratchpad.h"
#include "psyq/libgte/libgte.h"
#include "m2c_macros.h"
#include "rr_asm.h"
#include "game/camera/viewcam.h"
#include "game/course/trkclip.h"
#include "psyq/libmath/muldf1.h"
#include "psyq/other/dpadd.h"
#include "psyq/other/dpcmp.h"
#include "psyq/other/dpmul.h"
#include "rr/overlay662_palette_refresh.h"
#include "psyq/other/dpsub.h"

#define G ((Scratch *)SCRATCHPAD)
#define S16(p, o) (*(s16 *)((char *)(p) + (o)))
#define S32(p, o) (*(s32 *)((char *)(p) + (o)))
#define VEC(p, o) (*(VECTOR *)((char *)(p) + (o)))

typedef struct {
    s16 m[3][3];
    s32 t[3];
} MATRIX;



typedef struct {
    s32 pad00[2];
    s32 x08;
    s32 y0C;
    s32 z10;
    s32 pad14[20];
    s32 flag64;
    s32 pad68[4];
    s32 col78;
} Scratch;

extern int ViewUsesCarTrackNode;
extern short D_800F4C58;
extern int D_800F3820;
extern s32 __fixdfsi(s64);
extern s64 __floatsidf(s32);
extern s32 __ltdf2(s64, s64);
extern s64 __negdf2(s64);
extern void func_8005F0D0(s32, s32, s32, s32);
extern void BuildRotMatrixZ(MATRIX *, s32);
extern void BuildRotMatrixY(MATRIX *, s32);
extern void BuildRotMatrixX(MATRIX *, s32);
extern void SetGteObjectTransform(s32, void *, MATRIX *);
extern void MulMatrix0(void *, MATRIX *, MATRIX *);
extern void MulMatrix(MATRIX *, MATRIX *);
extern void MulMatrix2(MATRIX *, MATRIX *);
extern void SetLightMatrix(MATRIX *);
extern s32 LightMatrix[];
extern s32 ModelBankFormat;
extern s16 *CarBoundsTable[];
extern int D_800F2EC0;
extern int *ModelBankTable[];
extern int D_8010E6D8[];
extern char D_800A3C90[];

int func_8006F1EC();
s32 YawToOctant(void *arg0);
int GetSegmentViewTable(int a0);
void SetFogNearFar(s32 a0, s32 a1, s32 a2);
void InitTimTemplate(int *a0);
s32 func_8006F6F0(char *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);
void RelocateModelPointers(int *a0);
void RegisterModelBank(int *a0, int a1);
void UnrelocateModelBank(int *a0, int a1);
void SelectModelBank(int a0);
void SetVramImageSlot(int a0, int a1);
void LoadVramImageSlot(int a0);

int func_8006F1EC(a0)
int a0;
{
    int t, n;
    if (ViewUsesCarTrackNode == 0) {
        t = GetViewTrackNode(a0);
        n = *(int *)(a0 + 0x14);
        a0 = (int)&D_800F4C58;
        GetTrackSegmentAndYaw(t, n, a0);
        return *(short *)a0;
    }
    return GetViewTrackNode() / 5;
}

s32 YawToOctant(void *arg0) {
    s32 temp_v1;
    s32 var_v0;

    temp_v1 = M2C_FIELD(arg0, s32 *, 0x14) & 0xFFF;
    var_v0 = temp_v1 + 0x100;
    if (var_v0 < 0) {
        var_v0 = temp_v1 + 0x2FF;
    }
    return (var_v0 >> 9) & 7;
}

int GetSegmentViewTable(int a0) {
    int t = func_8006F1EC((SCRATCHPAD + 0x8), a0);
    return *(int *)(((t * 8) + YawToOctant((SCRATCHPAD + 0x8))) * 4 + D_800F3820);
}

void SetFogNearFar(s32 a0, s32 a1, s32 a2) {
    s64 d2, d0, d1, d6, t0, d4, d2_out;
    /* ASM_BARRIER: Move d2 into $a0/$a1 so $s0/$s1 is freed for 1.0 constant before __muldf3. */
    register s64 a0_d2 asm("$4");
    /* ASM_BARRIER: Load constant into $a2/$a3 before $s0/$s1 constant setup. */
    register s64 a2_c asm("$6");
    s64 c1, c4096, t_mul;
    s64 c_max, c_min;
    s32 r0, r1;

    d2 = __floatsidf(a2);
    d0 = __floatsidf(a0);
    d1 = __floatsidf(a1);
    a0_d2 = d2;
    a2_c = 0x40F0000000000000LL;
    c1 = 0x3FF0000000000000LL;
    /* ASM_BARRIER: Materialize 1.0 in $s0/$s1 after $a0..$a3 setup and before __muldf3. */
    __asm__("" : "=r"(c1), "=r"(a0_d2), "=r"(a2_c) : "0"(c1), "1"(a0_d2), "2"(a2_c));
    d6 = __muldf3(a0_d2, a2_c);
    d4 = __divdf3(0x4170000000000000LL, __muldf3(d6, __subdf3(__divdf3(c1, d1), __divdf3(c1, d0))));
    t0 = __divdf3(d6, d0);
    d2_out = __muldf3(t0, __negdf2(d4));
    a2_c = 0x40DFFFC000000000LL;
    /* ASM_BARRIER: Load 0x40DFFFC000000000LL into $a2/$a3 before moving d4 to $a0/$a1. */
    __asm__("" : "=r"(a2_c) : "0"(a2_c), "r"(d2_out));
    if (__gtdf2(d4, a2_c) > 0 || __ltdf2(d4, 0xC0E0000000000000LL) < 0) {
        /* ASM_BARRIER: Low word of $v0/$v1 return value for split argument setup. */
        register s32 v0_lo asm("$2");
        /* ASM_BARRIER: High word of $v0/$v1 return value for delay-slot move to $a1. */
        register s32 v1_hi asm("$3");
        /* ASM_BARRIER: Low word of $a0/$a1 argument. */
        register s32 a0_lo asm("$4");
        if (__gtdf2(d4, 0LL) > 0) {
            d4 = 0x40DFFFC000000000LL;
        } else {
            d4 = 0xC0E0000000000000LL;
        }
        d2_out = __muldf3(t0, __negdf2(d4));
        __adddf3(d2_out, __muldf3(d4, __divdf3(d6, d1)));
        /* ASM_BARRIER: Bind $v0/$v1 from __adddf3. */
        __asm__("" : "=r"(v0_lo), "=r"(v1_hi));
        a0_lo = v0_lo;
        a2_c = 0x3F30000000000000LL;
        c4096 = 0x40B0000000000000LL;
        /* ASM_BARRIER: Materialize 4096.0 in $s0/$s1 before __muldf3 while leaving $a1 = $v1 for the call delay slot. */
        __asm__("" : "=r"(c4096), "=r"(a0_lo), "=r"(a2_c), "=r"(v1_hi) : "0"(c4096), "1"(a0_lo), "2"(a2_c), "3"(v1_hi));
        t_mul = ((s64 (*)(s32, s32, s64))__muldf3)(a0_lo, v1_hi, a2_c);
        d2_out = __adddf3(d2_out, __muldf3(__subdf3(c4096, t_mul), c4096));
    }
    c_max = 0x41DFFFFFFFC00000LL;
    /* ASM_BARRIER: Materialize INT_MAX double constant before argument setup. */
    __asm__ volatile("" : "=r"(c_max) : "0"(c_max) : "$4", "$5");
    if (__gtdf2(d2_out, c_max) > 0) {
        d2_out = c_max;
    }
    c_min = 0xC1E0000000000000LL;
    /* ASM_BARRIER: Materialize INT_MIN double constant before argument setup. */
    __asm__ volatile("" : "=r"(c_min) : "0"(c_min) : "$4", "$5");
    if (__ltdf2(d2_out, c_min) < 0) {
        d2_out = c_min;
    }
    r0 = __fixdfsi(d4);
    r1 = __fixdfsi(d2_out);
    __asm__ volatile("ctc2 %0, $27" : : "r"(r0));
    __asm__ volatile("ctc2 %0, $28" : : "r"(r1));
}

void InitTimTemplate(int *a0) {
    int n;
    *a0++ = 0x10;
    *a0++ = 8;
    *a0++ = 0x2C;
    *a0++ = 0x400280;
    *a0++ = 0x10010;
    *a0++ = 0x7FFF0000;
    *a0++ = 0x7C1F03FF;
    *a0++ = 0x7FE0001F;
    *a0++ = 0x7C0003E0;
    *a0++ = 0x42100000;
    *a0++ = 0x40100210;
    *a0++ = 0x42000010;
    *a0++ = 0x40000200;
    *a0++ = 0x80C;
    *a0++ = 0x280;
    *a0++ = 0x400010;
    n = 0x1FF;
    do {
        *a0 = 0;
        n--;
        a0++;
    } while (n >= 0);
}

s32 func_8006F6F0(char *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6) {
    MATRIX m10;
    MATRIX m30;
    MATRIX m50;
    MATRIX m70;
    MATRIX m90;
    MATRIX mB0;
    SVECTOR svD0;
    MATRIX mD8[6];
    VECTOR v198[5];
    s32 dist;
    s32 i;
    s32 s5;
    s32 a1;

    SelectModelBank(arg1);
    v198[0].vx = S16(arg0, 0x20) - (s16)G->x08;
    v198[0].vy = 0;
    v198[0].vz = S16(arg0, 0x28) - (s16)G->z10;
    if (v198[0].vx < 0) {
        v198[0].vx = -v198[0].vx;
    }
    if (v198[0].vz < 0) {
        v198[0].vz = -v198[0].vz;
    }
    dist = v198[0].vx + v198[0].vz;
    switch (arg2) {
    case 0:
        i = dist;
        break;
    case 1:
        i = arg3 - 1;
        break;
    case 2:
        i = arg4 - 1;
        break;
    case 3:
        i = arg5 - 1;
        break;
    }
    if (i < arg3) {
        s5 = 0;
        if (G->flag64 != 0) {
            s5 = -0x60;
        }
        BuildRotMatrixY(&m90, -S32(arg0, 0x54));
        BuildRotMatrixX(&m50, -S32(arg0, 0x50));
        MulMatrix2(&m90, &m50);
        BuildRotMatrixZ(&m70, -S32(arg0, 0x58));
        MulMatrix2(&m50, &m70);
        MulMatrix0(LightMatrix, &m70, &mB0);
        SetLightMatrix(&mB0);
        BuildRotMatrixY(&m10, -S32(arg0, 0x54));
        MulMatrix2((MATRIX *)SCRATCH_ROT_MATRIX, &m10);
        BuildRotMatrixX(&mD8[0], S16(arg0, 0x1A8) - S32(arg0, 0x50));
        BuildRotMatrixZ(&m50, S16(arg0, 0x1AA) - S32(arg0, 0x58));
        MulMatrix(&mD8[0], &m50);
        MulMatrix2(&m10, &mD8[0]);
        v198[0] = VEC(arg0, 0xC8);
        v198[1].vx = S32(arg0, 0x70) + S32(arg0, 0x60);
        v198[1].vy = S32(arg0, 0x74) + S32(arg0, 0x64);
        v198[1].vz = S32(arg0, 0x78) + S32(arg0, 0x68);
        BuildRotMatrixX(&mD8[1], -S16(arg0, 0x80));
        BuildRotMatrixZ(&m50, -S16(arg0, 0x84));
        MulMatrix(&mD8[1], &m50);
        MulMatrix2(&m10, &mD8[1]);
        v198[2].vx = S32(arg0, 0x10) + S32(arg0, 0x60);
        v198[2].vy = S32(arg0, 0x14) + S32(arg0, 0x64);
        v198[2].vz = S32(arg0, 0x18) + S32(arg0, 0x68);
        BuildRotMatrixX(&m30, -S32(arg0, 0x50));
        BuildRotMatrixX(&mD8[3], S32(arg0, 0x17C));
        BuildRotMatrixZ(&mD8[2], -S32(arg0, 0x58));
        MulMatrix2(&m30, &mD8[2]);
        m30 = mD8[2];
        MulMatrix2(&m10, &mD8[2]);
        MulMatrix2(&mD8[2], &mD8[3]);
        BuildRotMatrixY(&mD8[4], S32(arg0, 0x178) / 12);
        BuildRotMatrixX(&mD8[5], S32(arg0, 0x17C));
        MulMatrix2(&m30, &mD8[4]);
        MulMatrix2(&m10, &mD8[4]);
        MulMatrix2(&mD8[4], &mD8[5]);
        MulMatrix(&m90, &m30);
        RR_GTE_SETROT(&m90);
        for (i = 0; i < 2; i++) {
            /* ASM_BARRIER: pin x to v0 to match branch delay slot and load scheduling */
            register s32 x asm("v0") = CarBoundsTable[0][0];
            if (i & 1) {
                x = -x;
            }
            svD0.vx = x << 2;
            /* ASM_BARRIER: force reload of CarBoundsTable[0] after storing svD0.vx */
            __asm__ volatile("" : : : "memory");
            svD0.vy = (u16)CarBoundsTable[0][1] << 2;
            svD0.vz = -((u16)CarBoundsTable[0][2]) << 2;
            RR_GTE_MVMVA_ROT_V0_NOCV(&svD0);
            RR_GTE_STORE_IR(&v198[i + 3]);
            v198[i + 3].vx += v198[2].vx;
            v198[i + 3].vy += v198[2].vy;
            v198[i + 3].vz += v198[2].vz;
        }
        SetGteObjectTransform((SCRATCHPAD + 0x7C), &v198[0], &mD8[0]);
        a1 = 1;
        G->col78 = 0x05000000;
        if (ModelBankFormat >= 4) {
            a1 = 3;
        }
        func_8005F0D0(SCRATCHPAD, a1, 0, s5);
        SetGteObjectTransform((SCRATCHPAD + 0x7C), &v198[1], &mD8[1]);
        G->col78 = 0x05000000;
        func_8005F0D0(SCRATCHPAD, ModelBankFormat < 1, arg6, s5);
        SetGteObjectTransform((SCRATCHPAD + 0x7C), &v198[2], &mD8[2]);
        a1 = 1;
        G->col78 = 0x05000000;
        if (ModelBankFormat >= 10) {
            a1 = 9;
        }
        func_8005F0D0(SCRATCHPAD, a1, 0, s5);
        SetGteObjectTransform((SCRATCHPAD + 0x7C), &v198[2], &mD8[3]);
        a1 = 1;
        G->col78 = 0x05000000;
        if (ModelBankFormat >= 7) {
            a1 = 6;
        }
        func_8005F0D0(SCRATCHPAD, a1, arg6, s5);
        SetGteObjectTransform((SCRATCHPAD + 0x7C), &v198[3], &mD8[4]);
        a1 = 1;
        G->col78 = 0x05000000;
        if (ModelBankFormat >= 9) {
            a1 = 8;
        }
        func_8005F0D0(SCRATCHPAD, a1, 0, s5);
        SetGteObjectTransform((SCRATCHPAD + 0x7C), &v198[4], &mD8[4]);
        a1 = 1;
        G->col78 = 0x05000000;
        if (ModelBankFormat >= 8) {
            a1 = 7;
        }
        func_8005F0D0(SCRATCHPAD, a1, 0, s5);
        SetGteObjectTransform((SCRATCHPAD + 0x7C), &v198[3], &mD8[5]);
        a1 = 1;
        G->col78 = 0x05000000;
        if (ModelBankFormat >= 6) {
            a1 = 5;
        }
        func_8005F0D0(SCRATCHPAD, a1, arg6, s5);
        SetGteObjectTransform((SCRATCHPAD + 0x7C), &v198[4], &mD8[5]);
        a1 = 1;
        G->col78 = 0x05000000;
        if (ModelBankFormat >= 5) {
            a1 = 4;
        }
        func_8005F0D0(SCRATCHPAD, a1, arg6, s5);
    } else if (i < arg4) {
        s5 = 0;
        if (G->flag64 != 0) {
            s5 = -0x60;
        }
        BuildRotMatrixY(&m90, -S32(arg0, 0x54));
        BuildRotMatrixX(&m50, -S32(arg0, 0x50));
        MulMatrix2(&m90, &m50);
        BuildRotMatrixZ(&m70, -S32(arg0, 0x58));
        MulMatrix2(&m50, &m70);
        MulMatrix0(LightMatrix, &m70, &mB0);
        SetLightMatrix(&mB0);
        BuildRotMatrixY(&m10, -S32(arg0, 0x54));
        MulMatrix2((MATRIX *)SCRATCH_ROT_MATRIX, &m10);
        BuildRotMatrixX(&mD8[0], S16(arg0, 0x1A8) - S32(arg0, 0x50));
        BuildRotMatrixZ(&m50, S16(arg0, 0x1AA) - S32(arg0, 0x58));
        MulMatrix(&mD8[0], &m50);
        MulMatrix2(&m10, &mD8[0]);
        v198[0] = VEC(arg0, 0xC8);
        v198[1].vx = S32(arg0, 0x70) + S32(arg0, 0x60);
        v198[1].vy = S32(arg0, 0x74) + S32(arg0, 0x64);
        v198[1].vz = S32(arg0, 0x78) + S32(arg0, 0x68);
        BuildRotMatrixX(&mD8[1], -S16(arg0, 0x80));
        BuildRotMatrixZ(&m50, -S16(arg0, 0x84));
        MulMatrix(&mD8[1], &m50);
        MulMatrix2(&m10, &mD8[1]);
        SetGteObjectTransform((SCRATCHPAD + 0x7C), &v198[0], &mD8[0]);
        a1 = 1;
        G->col78 = 0;
        if (ModelBankFormat >= 4) {
            a1 = 3;
        }
        func_8005F0D0(SCRATCHPAD, a1, 0, s5);
        SetGteObjectTransform((SCRATCHPAD + 0x7C), &v198[1], &mD8[1]);
        G->col78 = 0;
        func_8005F0D0(SCRATCHPAD, 1, arg6, s5);
    } else if (i < arg5) {
        s5 = 0;
        if (G->flag64 != 0) {
            s5 = -0x7F;
        }
        BuildRotMatrixY(&m50, -S32(arg0, 0x54));
        BuildRotMatrixX(&m90, -S32(arg0, 0x50));
        MulMatrix2(&m50, &m90);
        MulMatrix0(LightMatrix, &m90, &mB0);
        SetLightMatrix(&mB0);
        BuildRotMatrixZ(&m50, -S32(arg0, 0x58));
        MulMatrix2(&m90, &m50);
        MulMatrix2((MATRIX *)SCRATCH_ROT_MATRIX, &m50);
        SetGteObjectTransform((SCRATCHPAD + 0x7C), arg0 + 0x20, &m50);
        a1 = 1;
        G->col78 = 0;
        if (ModelBankFormat >= 3) {
            a1 = 2;
        }
        func_8005F0D0(SCRATCHPAD, a1, 0, s5);
    }
    return dist;
}

void RelocateModelPointers(int *a0) {
    int *a1 = a0 + 1;
    int i = 0;
    int n = a0[0];
    int lim;
    int *p;
    *(int *)(SCRATCHPAD + 0x48) = (int)a1;
    D_800F2EC0 = n;
    if (n > 0) {
        lim = n;
        p = a0 + 3;
        do {
            i++;
            *a1 += (int)a0;
            a1 += 3;
            *p += (int)a0;
            p += 3;
        } while (i < lim);
    }
}

void RegisterModelBank(int *a0, int a1) {
    int *p = a0 + 3;
    unsigned i;
    ModelBankTable[a1] = a0;
    i = 0;
    a0[1] += (int)a0;
    a0[2] += (int)a0;
    if (a0[0] != 0) {
        do {
            i++;
            *p += (int)a0;
            p++;
        } while (i < (unsigned)a0[0]);
    }
}

void UnrelocateModelBank(int *a0, int a1) {
    int *p = a0 + 3;
    unsigned i = 0;
    a0[1] -= a1;
    a0[2] -= a1;
    if (a0[0] != 0) {
        do {
            i++;
            *p -= a1;
            p++;
        } while (i < (unsigned)a0[0]);
    }
}

void SelectModelBank(int a0) {
    int *v = ModelBankTable[a0];
    int *w;
    int a2;
    int *g = (int *)SCRATCHPAD;
    /* ASM_BARRIER: legacy, reason not recorded (forces reg copy/CSE/scheduling); retire & retest with fd.py */
    __asm__("" : "=r"(w) : "0"(v));
    a2 = v[0];
    g[0x15] = w[1];
    g[0x16] = ModelBankTable[a0][2];
    ModelBankFormat = a2;
    g[0x14] = (int)ModelBankTable[a0] + 0xC;
}

void SetVramImageSlot(int a0, int a1) {
    D_8010E6D8[a1] = a0;
}

void LoadVramImageSlot(int a0) {
    LoadImage(D_800A3C90, D_8010E6D8[a0]);
}
