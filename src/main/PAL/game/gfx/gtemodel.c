#include "scratchpad.h"
#include "rr_asm.h"
#include "game/menu/mnsel.h"
#include "game/gfx/gtemodel.h"
#include "psyq/libgpu/gpuenv.h"
#include "rr/memory.h"
#include "rr/course_choices.h"
#include "game/data/screenframe.h"

typedef struct { int a, b, c, d; } V16;

typedef struct { int a, b; } __attribute__((packed)) V8;

extern int func_8001AC24();
extern int RotMatrixZ();
extern int CameraViewMatrix[];
extern int FlashingLightColorTable[];
extern int SetupRenderer();
extern int InitMenuGrid();
extern int ApplyMatrixSV();
extern int RotMatrix();
extern V8 D_800101BC;
extern V16 D_800101C4;
extern V16 D_800101D4;
extern V16 D_8009E540;
extern int D_800FFC48;
extern char *MenuGridRecords;
extern char *D_8010BF78;
extern void BuildCameraMatrices();

void func_8001ADA0();
void DrawTrackObjectModel(int a0, int a1);
void InitIntroSceneLights(void);
void func_8001B640(void);

void func_8001ADA0(a0, a1) int a0; int a1;
{
    int *m = (int *)(SCRATCHPAD + 0x7C);
    int *g;
    /* ASM_BARRIER: keep v pointer in $v0 */
    register int *v __asm__("$2");
    /* ASM_BARRIER: keep a1*12 offset and v[3]/v[1] temp in $v1 */
    register int v1 __asm__("$3");
    /* ASM_BARRIER: keep SCRATCHPAD / (SCRATCHPAD + 0x7C) arg in $a1 */
    register int r_a1 __asm__("$5");
    char *a3;
    int s2, s4, s5, a2, cnt;
    char *s0;
    /* ASM_BARRIER: keep GTE table pointer in $t0 */
    register int *p__r8 __asm__("$8");
    int g20, g1, pt1, pt2;

    v1 = a1 * 3;
    r_a1 = SCRATCHPAD;
    /* ASM_BARRIER: load SCRATCHPAD into $a1 after a1*3 before shifting by 2 */
    __asm__("" : "+r"(r_a1), "+r"(v1));
    g = (int *)r_a1;
    /* ASM_BARRIER: copy $a1 to $s3 before shifting v1 by 2 */
    __asm__("" : "+r"(g), "+r"(v1));
    v1 <<= 2;
    v = (int *)g[0x48 / 4];
    g20 = g[0x20 / 4];
    /* ASM_BARRIER: load g[0x20/4] in delay slot of g[0x48/4] before subtracting 4 */
    __asm__("" : "+r"(v), "+r"(g20));
    v = (int *)((char *)v - 4);
    /* ASM_BARRIER: subtract 4 before adding v1 */
    __asm__("" : "+r"(v));
    v = (int *)((char *)v + v1);
    /* ASM_BARRIER: keep r_a1 |= 0x7C after v addition so it fills v[3] load delay slot */
    __asm__ volatile("" : "+r"(r_a1), "+r"(v));
    v1 = v[3];
    r_a1 |= 0x7C;
    m[0x30 / 4] = v1;
    v1 = v[1];
    m[0] = 0x1000;
    m[0x10 / 4] = 0x1000;
    m[0x8 / 4] = 0x1000;
    m[0xC / 4] = 0;
    m[1] = 0;
    m[0x2C / 4] = v1;
    RotMatrixZ(g20, r_a1);
    RR_GTE_SETROT(m);
    p__r8 = CameraViewMatrix;
    RR_GTE_SETLLM(p__r8);
    p__r8 = D_800FF6A4;
    RR_GTE_SETBK_P(p__r8);
    {
        /* ASM_BARRIER: keep a0*12 in $v0 */
        register int off0 __asm__("$2") = a0 * 12;
        s4 = 0x12A;
        a3 = *(char **)(g[0x48 / 4] + off0 + 8);
        /* ASM_BARRIER: keep off0 ($v0) live so g[0x48/4] + off0 writes into $v1 */
        __asm__("" : : "r"(off0));
    }
    s2 = *(unsigned short *)(a3 + 2);
    a3 += 4;
    if (g[0x64 / 4]) s4 = 0x95;
    cnt = s2;
    s2--;
    /* ASM_BARRIER: keep cnt = s2 and s2-- at top of .L8001AEC8 before preheader loads */
    __asm__ volatile("");
    a2 = g[0];
    g1 = g[1];
    /* ASM_BARRIER: order g[0]/g[1] loads before a3 loads */
    __asm__ volatile("" : : "r"(a2), "r"(g1));
    pt1 = *(int *)a3;
    pt2 = *(unsigned short *)(a3 + 4);
    m[0x24 / 4] = pt1;
    *(short *)((char *)m + 0x28) = pt2;
    /* ASM_BARRIER: order m+0x24/0x28 stores before g[0x60/4] load and s5 addiu in branch delay slot */
    __asm__ volatile("");
    g = (int *)g[0x60 / 4];
    s5 = g1 + 0x200;
    if (cnt > 0) {
        s0 = a3 + 7;
        do {
            int t1, t2, t, sx;
            /* ASM_BARRIER: prevent loop strength reduction from creating a second pointer GIV for s0+1/s0+5 */
            __asm__("" : "+r"(s0));
            /* ASM_BARRIER: load V0 from m+0x24 and run GTE MVMVA (0x4A4A2012) */
            __asm__ volatile ("lwc2 $0, 0(%0); lwc2 $1, 4(%0)" : : "r"((char *)m + 0x24));
            /* ASM_BARRIER: two delay slot nops before GTE MVMVA */
            __asm__ volatile ("nop; nop");
            /* ASM_BARRIER: GTE MVMVA 1, 1, 0, 1, 0 (0x4A4A2012) */
            __asm__ volatile (".word 0x4A4A2012");
            t1 = *(int *)(s0 + 1);
            t2 = *(unsigned short *)(s0 + 5);
            m[0x24 / 4] = t1;
            *(short *)((char *)m + 0x28) = t2;
            RR_GTE_SWC2_IR3((char *)m + 0x54);
            t = m[0x54 / 4];
            sx = t >> (int)g;
            if (t > 0) {
                if (sx < s4) {
                    RR_GTE_TR_FROM_IR();
                    a2 = func_8001AC24(m, s5 + sx * 4 + *(signed char *)s0 * 4, a2);
                }
            }
            s0 += 8;
        } while (s2-- > 0);
    }
    *(int *)SCRATCHPAD = a2;
}

void DrawTrackObjectModel(int a0, int a1) {
    /* ASM_BARRIER: keep scratch pointer (SCRATCHPAD + 0x7C) in $a2 */
    register int *m __asm__("$6") = (int *)(SCRATCHPAD + 0x7C);
    /* ASM_BARRIER: keep SCRATCHPAD base / index temp in $a1 */
    register int r_a1 __asm__("$5");
    /* ASM_BARRIER: keep temp/count/vertex ptr in $a0 */
    register int r_a0 __asm__("$4");
    /* ASM_BARRIER: keep temp in $v0 */
    register int r_v0 __asm__("$2");
    /* ASM_BARRIER: keep temp in $v1 */
    register int r_v1 __asm__("$3");
    /* ASM_BARRIER: keep fourth vertex index/sxy2 in $a3 */
    register int r_a3 __asm__("$7");
    /* ASM_BARRIER: keep GTE IR3 / otz index in $t3 */
    register int t3 __asm__("$11");
    /* ASM_BARRIER: keep loop counter in $t8 */
    register int t8 __asm__("$24");
    /* ASM_BARRIER: keep vertex base pointer in $t9 */
    register char *t9 __asm__("$25");
    /* ASM_BARRIER: keep ordering table base in $s0 */
    register int s0 __asm__("$16");
    /* ASM_BARRIER: keep OT shift + 2 in $s2 */
    register int s2 __asm__("$18");
    char *t2, *t7;
    int x0, x1, x2, x3, y0, y1, y2, y3, cval;

    m[0x54 / 4] = a1;
    r_a1 = SCRATCHPAD;
    /* ASM_BARRIER: load SCRATCHPAD into $a1 after storing a1 */
    __asm__ volatile("" : "+r"(r_a1));
    r_v0 = a0 * 12;
    r_v1 = ((int *)r_a1)[0x48 / 4];
    t7 = (char *)((int *)r_a1)[0];
    /* ASM_BARRIER: load 0x48($a1) and 0($a1) before subtracting 4 */
    __asm__ volatile("" : "+r"(r_v1), "+r"(t7), "+r"(r_v0));
    r_v1 -= 4;
    /* ASM_BARRIER: subtract 4 before adding a0*12 */
    __asm__ volatile("" : "+r"(r_v1));
    r_v1 += r_v0;
    t2 = *(char **)(r_v1 + 0xC);
    t9 = *(char **)(r_v1 + 0x4);
    t8 = *(unsigned short *)(t2 + 2);
    t2 += 4;
    /* ASM_BARRIER: advance t2 by 4 before reading +0x1C and +0x1A */
    __asm__ volatile("" : "+r"(t2), "+r"(t8));
    r_v0 = *(int *)(t2 + 0x1C);
    r_v1 = *(unsigned short *)(t2 + 0x1A);
    /* ASM_BARRIER: load +0x1C and +0x1A before copying t8 to $a0 */
    __asm__ volatile("" : "+r"(r_v0), "+r"(r_v1));
    r_a0 = t8;
    t8--;
    /* ASM_BARRIER: copy and decrement t8 before storing to m */
    __asm__ volatile("" : "+r"(r_a0), "+r"(t8));
    m[0] = r_v0;
    *(short *)((char *)m + 4) = r_v1;
    r_v0 = ((int *)r_a1)[1];
    r_v1 = ((int *)r_a1)[0x60 / 4];
    /* ASM_BARRIER: load 4($a1) and 0x60($a1) before computing s0 and s2 */
    __asm__ volatile("" : "+r"(r_v0), "+r"(r_v1));
    s0 = r_v0 + 0x200;
    s2 = r_v1 + 2;
    if (r_a0 > 0) {
        do {
            /* ASM_BARRIER: load V0 from $a2 and run GTE MVMVA (0x4A4A2012) */
            __asm__ volatile ("lwc2 $0, 0(%0); lwc2 $1, 4(%0)" : : "r"(m));
            /* ASM_BARRIER: two delay slot nops before GTE MVMVA */
            __asm__ volatile ("nop; nop");
            /* ASM_BARRIER: GTE MVMVA 1, 1, 0, 1, 0 (0x4A4A2012) */
            __asm__ volatile (".word 0x4A4A2012");

            r_v0 = *(int *)(t2 + 0x3C);
            r_v1 = *(unsigned short *)(t2 + 0x3A);
            /* ASM_BARRIER: load next normal before storing to m */
            __asm__ volatile("" : "+r"(r_v0), "+r"(r_v1));
            m[0] = r_v0;
            *(short *)((char *)m + 4) = r_v1;

            /* ASM_BARRIER: read GTE IR3 into $t3 */
            __asm__ volatile ("mfc2 %0, $11" : "=r"(t3));
            /* ASM_BARRIER: mfc2 load delay slot */
            RR_NOP_DEP_ON(t3);
            if (t3 > 0) {
                /* ASM_BARRIER: keep blez delay slot as nop before srav */
                __asm__ volatile("");
                t3 >>= s2;
                if (t3 < 0x1C0) {
                    m[0x50 / 4] = t3;
                    RR_GTE_TR_FROM_IR();

                    r_v0 = *(int *)t2;
                    r_a1 = *(int *)(t2 + 0x04);
                    /* ASM_BARRIER: load packed vertex indices before unpacking */
                    __asm__ volatile("" : "+r"(r_v0), "+r"(r_a1));
                    r_v1 = r_v0 >> 16;
                    r_a3 = r_a1 >> 16;
                    r_v0 &= 0xFFFF;
                    r_a1 &= 0xFFFF;
                    /* ASM_BARRIER: order index shifts and masks before address math */
                    __asm__ volatile("" : "+r"(r_v1), "+r"(r_a3), "+r"(r_v0), "+r"(r_a1));
                    r_a0 = r_v0 << 3;
                    /* ASM_BARRIER: shift first vertex index into $a0 before adding t9 */
                    __asm__ volatile("" : "+r"(r_a0));
                    r_a0 = (int)(t9 + r_a0);
                    /* ASM_BARRIER: compute first vertex address in $a0 */
                    __asm__ volatile("" : "+r"(r_a0));
                    r_v1 = (int)(t9 + (r_v1 << 3));
                    /* ASM_BARRIER: compute second vertex address in $v1 */
                    __asm__ volatile("" : "+r"(r_v1));
                    r_v0 = (int)(t9 + (r_a1 << 3));
                    /* ASM_BARRIER: compute third vertex address in $v0 */
                    __asm__ volatile("" : "+r"(r_v0));
                    RR_GTE_RTPT_LDV(r_a0, r_v1, r_v0);

                    r_v0 = *(int *)(t2 + 0x0C);
                    r_v1 = *(int *)(t2 + 0x10);
                    /* ASM_BARRIER: load first two UV words before storing */
                    __asm__ volatile("" : "+r"(r_v0), "+r"(r_v1));
                    *(int *)(t7 + 0x0C) = r_v0;
                    *(int *)(t7 + 0x14) = r_v1;
                    r_v0 = *(int *)(t2 + 0x14);
                    /* ASM_BARRIER: load third UV word before shifting */
                    __asm__ volatile("" : "+r"(r_v0));
                    r_v1 = r_v0 >> 16;
                    /* ASM_BARRIER: shift before storing +0x19 */
                    __asm__ volatile("" : "+r"(r_v1));
                    *(int *)(t7 + 0x1C) = r_v0;
                    r_v0 = (int)((char *)m + 0x58);
                    /* ASM_BARRIER: stage m+0x58 in $v0 before storing +0x21 */
                    __asm__ volatile("" : "+r"(r_v0));
                    *(int *)(t7 + 0x24) = r_v1;
                    RR_GTE_SWC2_SXYZ(r_v0);

                    r_v0 = (int)(t9 + (r_a3 << 3));
                    RR_GTE_RTPS_LDV(r_v0);

                    if (m[0x54 / 4] != 0) {
                        cval = FlashingLightColorTable[(ScreenFrame + ({
                            /* ASM_BARRIER: extend %hi(ScreenFrame) lifetime for loop invariant hoisting */
                            __asm__ volatile("");
                            /* ASM_BARRIER: extend %hi(ScreenFrame) lifetime for loop invariant hoisting */
                            __asm__ volatile("");
                            /* ASM_BARRIER: extend %hi(ScreenFrame) lifetime for loop invariant hoisting */
                            __asm__ volatile("");
                            /* ASM_BARRIER: extend %hi(ScreenFrame) lifetime for loop invariant hoisting */
                            __asm__ volatile("");
                            /* ASM_BARRIER: extend %hi(ScreenFrame) lifetime for loop invariant hoisting */
                            __asm__ volatile("");
                            *(signed char *)(t2 + 0x18);
                        })) & 0x1F];
                    } else {
                        cval = FlashingLightColorTable[0];
                    }
                    r_v0 = cval;

                    r_v1 = m[0x58 / 4];
                    r_a1 = m[0x5C / 4];
                    r_a3 = m[0x60 / 4];
                    /* ASM_BARRIER: load SXY0..SXY2 before storing to packet */
                    __asm__ volatile("" : "+r"(r_v0), "+r"(r_v1), "+r"(r_a1), "+r"(r_a3));
                    *(int *)(t7 + 0x04) = r_v0;
                    r_v0 = (int)((char *)m + 0x64);
                    /* ASM_BARRIER: stage m+0x64 in $v0 before storing SXY0..SXY2 */
                    __asm__ volatile("" : "+r"(r_v0));
                    *(int *)(t7 + 0x08) = r_v1;
                    *(int *)(t7 + 0x10) = r_a1;
                    *(int *)(t7 + 0x18) = r_a3;
                    RR_GTE_SWC2_SXY2(r_v0);
                    r_v0 = m[0x64 / 4];
                    *(int *)(t7 + 0x20) = r_v0;

                    x0 = *(short *)((char *)m + 0x58);
                    x1 = *(short *)((char *)m + 0x5C);
                    x2 = *(short *)((char *)m + 0x60);
                    x3 = *(short *)((char *)m + 0x64);
                    if ((x0 >= 0 || x1 >= 0 || x2 >= 0 || x3 >= 0) &&
                        (x0 < 0x140 || x1 < 0x140 || x2 < 0x140 || x3 < 0x140)) {
                        y0 = *(short *)((char *)m + 0x5A);
                        y1 = *(short *)((char *)m + 0x5E);
                        y2 = *(short *)((char *)m + 0x62);
                        y3 = *(short *)((char *)m + 0x66);
                        if ((y0 >= 0 || y1 >= 0 || y2 >= 0 || y3 >= 0) &&
                            (y0 < 0x100 || y1 < 0x100 || y2 < 0x100 || y3 < 0x100)) {
                            *(int *)t7 = *(int *)((*(signed char *)(t2 + 0x19) << 2) + ((*(volatile int *)((char *)m + 0x50) << 2) + s0));
                            *(char *)(t7 + 3) = 9;

                            *(int *)((*(signed char *)(t2 + 0x19) << 2) + ((*(volatile int *)((char *)m + 0x50) << 2) + s0)) = (int)t7;
                            t7 += 0x28;

                            *(char *)((*(signed char *)(t2 + 0x19) << 2) + ((*(volatile int *)((char *)m + 0x50) << 2) + s0) + 3) = 0;
                        }
                    }
                }
            }
            t2 += 0x20;
        } while (t8-- > 0);
    }
    *(int *)SCRATCHPAD = (int)t7;
}

void InitIntroSceneLights(void) {
    V8 sp18;
    short *a = (short *)(SCRATCHPAD + 0x7C);
    short *b = (short *)(SCRATCHPAD + 0x84);
    short *p, *q;
    int i;

    D_8010BF78 = LoadCursor;
    LoadCursor += 0xBC;
    SetupRenderer(1);
    sp18 = D_800101BC;
    ClearImage2(&sp18, 0, 0, 0);
    *(V16 *)(SCRATCHPAD + 0x18) = D_800101C4;
    *(V16 *)(SCRATCHPAD + 0x8) = D_8009E540;
    *(int *)(D_8010BF78 + 0x24) = 0xFAAB;
    *(int *)(D_8010BF78 + 0x28) = 0xFAAB0000;
    *(int *)(D_8010BF78 + 0x20) = 0;
    *(int *)(D_8010BF78 + 0x2C) = 0;
    *(int *)(D_8010BF78 + 0x30) = -0x555;
    b[2] = -0x555;
    b[1] = 0;
    b[0] = 0;
    a[0] = -0x200;
    a[1] = 0x200;
    a[2] = 0;
    RotMatrix((SCRATCHPAD + 0x7C), (SCRATCHPAD + 0x8C));
    ApplyMatrixSV((SCRATCHPAD + 0x8C), (SCRATCHPAD + 0x84), D_8010BF78);
    a[0] = 0x400;
    a[1] = 0x400;
    a[2] = 0;
    RotMatrix((SCRATCHPAD + 0x7C), (SCRATCHPAD + 0x8C));
    ApplyMatrixSV((SCRATCHPAD + 0x8C), (SCRATCHPAD + 0x84), D_8010BF78 + 6);
    a[0] = 0x400;
    a[1] = 0x400;
    a[2] = 0;
    RotMatrix((SCRATCHPAD + 0x7C), (SCRATCHPAD + 0x8C));
    ApplyMatrixSV((SCRATCHPAD + 0x8C), (SCRATCHPAD + 0x84), D_8010BF78 + 0xC);
    p = (short *)D_8010BF78;
    i = 7;
    q = p + 7;
    p[0x24] = 0;
    p[0x25] = 0;
    p[0x26] = 0;
    p[0x27] = 0;
    p[0x28] = 0;
    p[0x29] = 0;
    p[0x2A] = 0;
    p[0x2B] = 0;
    p[0x2C] = 0;
    p[0x2D] = 0;
    p[0x2E] = 0;
    p[0x2F] = 0;
    p[0x30] = 0;
    p[0x31] = 0;
    p[0x32] = 0;
    p[0x33] = 0;
    p[0x34] = 0;
    p[0x35] = 0;
    p[0x36] = 0;
    p[0x37] = 0;
    p[0x38] = 0;
    p[0x39] = 0;
    p[0x3A] = 0;
    p[0x3B] = 0;
    p[0x3C] = 0;
    p[0x3D] = 0;
    p[0x3E] = 0;
    p[0x3F] = 0;
    p[0x40] = 0;
    p[0x41] = 0;
    p[0x42] = 0;
    p[0x43] = 0;
    p[0x44] = 0;
    p[0x45] = 0;
    p[0x46] = 0;
    p[0x47] = 0;
    p[0x48] = 0;
    p[0x49] = 0;
    p[0x4A] = 0;
    p[0x4B] = 0;
    p[0x4C] = 0;
    p[0x4D] = 0;
    p[0x4E] = 0;
    p[0x4F] = 0;
    p[0x50] = 0;
    p[0x51] = 0;
    p[0x52] = 0;
    p[0x53] = 0;
    p[0x54] = 0;
    do {
        q[0x55] = 0;
        i--;
        q--;
    } while (i >= 0);
    AllocMenuGridRecords(&D_800FFC48, 1);
    InitMenuGrid(0, 1, 3, 0xB4, 0x60);
    PsyqMemset(MenuGridRecords + 0x1C, 0, 0x10);
    *(V16 *)(MenuGridRecords + 0xC) = D_800101D4;
}

void func_8001B640(void)
{
    unsigned short *g;
    BuildCameraMatrices();
    g = (unsigned short *)SCRATCHPAD;
    g[0x17] <<= 1;
    g[0x18] <<= 1;
    g[0x19] <<= 1;
}
