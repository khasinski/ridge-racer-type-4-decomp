#include "types.h"
#include "scratchpad.h"
#include "rr_asm.h"
#include "m2c_macros.h"
#include "game/gfx/gtepoly1.h"
#include "rr/fade_sprite.h"
#include "rr/audio_cd_controller.h"
#include "game/data/globals.h"
#include "game/data/screenframe.h"

#define SP ((Scr *)s2)
#define S4 ((Ent *)s4)
#define GTE_STORE_SXY_SZ(p) \
    __asm__ volatile( \
        "swc2 $12, 0(%0); swc2 $17, 4(%0); swc2 $13, 8(%0); swc2 $18, 12(%0); swc2 $14, 16(%0); swc2 $19, 20(%0)" \
        : : "r"(p) : "memory")
#define GTE_SETROT_AND_SAVE(src, dst) \
    __asm__ volatile( \
        "lw $12, 0(%0); lw $13, 4(%0); ctc2 $12, $0; ctc2 $13, $1; sw $12, 0(%1); sw $13, 4(%1); " \
        "lw $12, 8(%0); lw $13, 12(%0); ctc2 $12, $2; ctc2 $13, $3; sw $12, 8(%1); " \
        "lw $12, 16(%0); sw $13, 12(%1); ctc2 $12, $4; sw $12, 16(%1)" \
        : : "r"(src), "r"(dst) : "$12", "$13", "memory")
#define GTE_CTC2_27(val) \
    __asm__ volatile("ctc2 %0, $27" : : "r"(val))
#define GTE_CTC2_28(val) \
    __asm__ volatile("ctc2 %0, $28" : : "r"(val))
/* ASM_BARRIER: empty asm that only reads its operands; keeps both values live to the end of the block
 * so gcc expands the signed divisions with a copy (bgez; move; addiu) as in the original. */
#define KEEP_LIVE2(a, b) __asm__ volatile("" : : "r"(a), "r"(b))
#define GTE_LWC2_IR0(p) __asm__ volatile("lwc2 $8, 0(%0)" : : "r"(p))
#define GTE_LWC2_IR0_MEM(p) __asm__ volatile("lwc2 $8, 0(%0)" : : "r"(p) : "memory")
#define GTE_DPCS_COL(col, dc) __asm__ volatile("mtc2 %1, $8\n\tmtc2 %0, $6\n\tnop\n\tnop\n\t.word 0x4A780010\n\tmfc2 %0, $22" : "+r"(col) : "r"(dc))
#define GTE_MTC2_RGB_DPCS(c) __asm__ volatile("mtc2 %0, $6\n\tnop\n\tnop\n\t.word 0x4A780010" : : "r"(c))

typedef struct {
    int v01;
    int v23;
    int pad08;
    int c0;
    int c1;
    int c2;
    struct {
        short pad0;
        short c3;
        char pad4;
        signed char unk5;
        short pad6;
    } tail;
} Quad1C;

typedef struct {
    unsigned short i0;
    unsigned short i1;
    unsigned short i2;
    unsigned short i3;
    char pad08;
    signed char unk09;
    short pad0A;
} Tri0C;

typedef struct {
    s32 sxy;
    s32 sz;
} VertScreen;

typedef struct {
    s16 vx;
    s16 vy;
    s16 vz;
    s16 pad;
} SVector;

typedef struct {
    s16 unk0;
    u16 unk2;
    VertScreen *verts;
} TrailEntry4;

typedef union {
    s32 w[0x70];
    s16 h[0xE0];
    u16 uh[0xE0];
    void *p[0x70];
    char *c[0x70];
    s32 *pw[0x70];
    SVector *sv[0x70];
} Scr;

typedef union {
    s32 w[0x32];
    s16 h[0x64];
    u16 uh[0x64];
} Ent;

typedef struct {
    s16 unk0;
    s16 unk2;
    SVector *verts;
} TrailEntry8;

extern int MaskBitSetting;
extern s32 ModelTexturePage;
extern int SetGteObjectTransform();
extern int UnknownRroTypeFmt;
extern s32 PsyqRand(void);
extern signed char RacePaused;
extern int D_8009E0B0[];
extern int D_8009E0E0[];
extern char D_8009E124[];
extern void *CarBoundsTable[];
extern s16 RaceType;
extern u8 CarSlotBuffers[];

void *EmitRroQuadsFT4(char *a0_in, int *a1_in, int a2_in);
void *EmitRroLitQuadsG4(char *a0_in, int *a1_in, int a2_in);
void *EmitRroEnvMapQuads(char *a0_in, int *a1_in, int a2_in);
void SetModelTexturePage(s32 arg0);
int DrawRroModel(int a0, int a1, int a2, int a3);
void DrawSparkLines(char *a0_in, char *a1_in, s32 a2_in);
void func_80014F34(s32 *a0_in, char *a1_in);
void func_80015110(s32 *a0_in, char *a1_in);
void func_800153FC();
void DrawGlowFan(char *a0, char *a1);
int *EmitModelQuadsFT4(int *a0, int a1, char *a2);
void func_80015918(Tri0C *a0, int a1, char *a2);
char *func_80015D00(char *a0_in, s32 a1_in, char *a2_in);
void DrawWheelTrailEffects(void *arg0, s32 *arg1, void *arg2, void *arg3, s32 arg4, s32 arg5, s32 arg6);

void *EmitRroQuadsFT4(char *a0_in, int *a1_in, int a2_in) {
    /* ASM_BARRIER: keep temp in $v0 */
    register int r_v0 __asm__("$2");
    /* ASM_BARRIER: keep temp in $v1 */
    register int r_v1 __asm__("$3");
    /* ASM_BARRIER: keep a0_in / temp in $a0 */
    register int r_a0 __asm__("$4") = (int)a0_in;
    /* ASM_BARRIER: keep context pointer in $a1 */
    register int *r_a1 __asm__("$5") = a1_in;
    /* ASM_BARRIER: keep loop counter in $a2 */
    register int r_a2 __asm__("$6") = a2_in;
    /* ASM_BARRIER: keep temp in $a3 */
    register int r_a3 __asm__("$7");
    /* ASM_BARRIER: keep temp in $t0 */
    register int t0 __asm__("$8");
    /* ASM_BARRIER: keep temp in $t1 */
    register int t1 __asm__("$9");
    /* ASM_BARRIER: keep packet+3 cursor in $t2 */
    register char *t2 __asm__("$10");
    /* ASM_BARRIER: keep poly+0x15 cursor in $t3 */
    register char *t3 __asm__("$11");
    /* ASM_BARRIER: keep packet cursor in $t4 */
    register char *t4 __asm__("$12");
    /* ASM_BARRIER: keep vertex table base in $t5 */
    register int t5 __asm__("$13");
    /* ASM_BARRIER: keep fourth vertex index in $t6 */
    register int t6 __asm__("$14");
    /* ASM_BARRIER: keep poly cursor in $t7 */
    register char *t7 __asm__("$15");
    /* ASM_BARRIER: keep OT shift in $t8 */
    register int t8 __asm__("$24");
    /* ASM_BARRIER: keep 0x2F000000 constant in $t9 */
    register int t9 __asm__("$25");
    int x0, x1, x2, x3, y0, y1, y2, y3;

    t7 = (char *)r_a0;
    t9 = 0x2F000000;
    /* ASM_BARRIER: order t7 and t9 initialization before loads */
    __asm__ volatile("" : "+r"(t7), "+r"(t9));
    r_v0 = *(int *)(t7 + 0);
    r_v1 = *(int *)(t7 + 4);
    t5 = r_a1[0];
    /* ASM_BARRIER: order initial loads before index shifts */
    __asm__ volatile("" : "+r"(r_v0), "+r"(r_v1), "+r"(t5));
    t1 = r_v0 >> 16;
    t6 = r_v1 >> 16;
    r_v0 &= 0xFFFF;
    /* ASM_BARRIER: order shifts and mask before first vertex address */
    __asm__ volatile("" : "+r"(t1), "+r"(t6), "+r"(r_v0));
    r_v0 = (r_v0 << 3) + t5;
    /* ASM_BARRIER: compute first vertex pointer in $v0 */
    __asm__ volatile("" : "+r"(r_v0));
    r_a3 = *(int *)(r_v0 + 0);
    t0 = *(int *)(r_v0 + 4);
    /* ASM_BARRIER: load first vertex words before second vertex address */
    __asm__ volatile("" : "+r"(r_a3), "+r"(t0));
    r_v0 = (t1 << 3) + t5;
    /* ASM_BARRIER: compute second vertex pointer in $v0 */
    __asm__ volatile("" : "+r"(r_v0));
    t1 = *(int *)(r_v0 + 0);
    r_v0 = *(int *)(r_v0 + 4);
    /* ASM_BARRIER: load second vertex words before masking v1 */
    __asm__ volatile("" : "+r"(t1), "+r"(r_v0));
    r_v1 &= 0xFFFF;
    /* ASM_BARRIER: mask v1 in load delay slot before storing 0x34 */
    __asm__ volatile("" : "+r"(r_v1));
    r_a1[0x34 / 4] = r_v0;
    r_v0 = (r_v1 << 3) + t5;
    /* ASM_BARRIER: compute third vertex pointer before storing 0x28..0x30 */
    __asm__ volatile("" : "+r"(r_v0));
    r_a1[0x28 / 4] = r_a3;
    r_a1[0x2C / 4] = t0;
    r_a1[0x30 / 4] = t1;
    r_a3 = *(int *)(r_v0 + 0);
    t0 = *(int *)(r_v0 + 4);
    /* ASM_BARRIER: load third vertex words before fourth vertex address */
    __asm__ volatile("" : "+r"(r_a3), "+r"(t0));
    r_v0 = (t6 << 3) + t5;
    r_v1 = r_a2;
    /* ASM_BARRIER: compute fourth vertex pointer and copy a2 to v1 */
    __asm__ volatile("" : "+r"(r_v0), "+r"(r_v1));
    t1 = *(int *)(r_v0 + 0);
    r_v0 = *(int *)(r_v0 + 4);
    /* ASM_BARRIER: load fourth vertex words before storing 0x44 */
    __asm__ volatile("" : "+r"(t1), "+r"(r_v0));
    r_a1[0x44 / 4] = r_v0;
    r_v0 = SCRATCHPAD;
    /* ASM_BARRIER: load SCRATCHPAD into $v0 before storing 0x38..0x40 */
    __asm__ volatile("" : "+r"(r_v0));
    r_a1[0x38 / 4] = r_a3;
    r_a1[0x3C / 4] = t0;
    r_a1[0x40 / 4] = t1;
    t4 = *(char **)(r_v0 + 0x0);
    t8 = *(int *)(r_v0 + 0x60);
    /* ASM_BARRIER: load scratchpad fields before decrementing a2 and branching */
    __asm__ volatile("" : "+r"(t4), "+r"(t8), "+r"(r_v1));
    r_a2--;
    if (r_v1 > 0) {
        t3 = (char *)r_a0 + 0x15;
        t2 = t4 + 3;
        do {
            /* ASM_BARRIER: keep loop pointers live at top of loop */
            __asm__ volatile("" : "+r"(t3), "+r"(t2), "+r"(t4), "+r"(t7));
            r_v0 = (int)((char *)r_a1 + 0x28);
            RR_GTE14_LWC2_V012(r_v0);
            RR_GTE14_NOP2();
            RR_GTE14_RTPT();

            r_v0 = *(int *)(t3 + 3);
            r_v1 = *(int *)(t3 + 7);
            /* ASM_BARRIER: load next packed indices before shifts */
            __asm__ volatile("" : "+r"(r_v0), "+r"(r_v1));
            t1 = r_v0 >> 16;
            t6 = r_v1 >> 16;
            r_v0 &= 0xFFFF;
            /* ASM_BARRIER: order shifts and mask before first vertex address */
            __asm__ volatile("" : "+r"(t1), "+r"(t6), "+r"(r_v0));
            r_v0 = (r_v0 << 3) + t5;
            /* ASM_BARRIER: compute first vertex pointer in $v0 */
            __asm__ volatile("" : "+r"(r_v0));
            r_a3 = *(int *)(r_v0 + 0);
            t0 = *(int *)(r_v0 + 4);
            /* ASM_BARRIER: load first vertex words before masking v1 */
            __asm__ volatile("" : "+r"(r_a3), "+r"(t0));
            r_v1 &= 0xFFFF;
            /* ASM_BARRIER: mask v1 before storing 0x28..0x2C */
            __asm__ volatile("" : "+r"(r_v1));
            r_a1[0x28 / 4] = r_a3;
            r_a1[0x2C / 4] = t0;
            RR_GTE14_NCLIP();

            r_v0 = (t1 << 3) + t5;
            /* ASM_BARRIER: compute second vertex pointer in $v0 */
            __asm__ volatile("" : "+r"(r_v0));
            r_a3 = *(int *)(r_v0 + 0);
            t0 = *(int *)(r_v0 + 4);
            /* ASM_BARRIER: load second vertex words before storing MAC0/SXYZ and loading V0 */
            __asm__ volatile("" : "+r"(r_a3), "+r"(t0));
            r_v0 = (int)((char *)r_a1 + 0x1C);
            RR_GTE14_SWC2_MAC0(r_v0);
            r_v0 = (int)((char *)r_a1 + 0x50);
            RR_GTE14_SWC2_SXYZ(r_v0);
            r_v0 = (int)((char *)r_a1 + 0x40);
            RR_GTE14_LWC2_V0(r_v0);
            r_a1[0x30 / 4] = r_a3;
            r_a1[0x34 / 4] = t0;
            RR_GTE14_RTPS();

            r_v0 = (r_v1 << 3) + t5;
            /* ASM_BARRIER: compute third vertex pointer in $v0 */
            __asm__ volatile("" : "+r"(r_v0));
            r_a3 = *(int *)(r_v0 + 0);
            t0 = *(int *)(r_v0 + 4);
            /* ASM_BARRIER: load third vertex words before storing 0x38..0x3C */
            __asm__ volatile("" : "+r"(r_a3), "+r"(t0));
            r_a1[0x38 / 4] = r_a3;
            r_a1[0x3C / 4] = t0;
            RR_GTE14_AVSZ4();

            r_v0 = (t6 << 3) + t5;
            /* ASM_BARRIER: compute fourth vertex pointer in $v0 */
            __asm__ volatile("" : "+r"(r_v0));
            r_a3 = *(int *)(r_v0 + 0);
            t0 = *(int *)(r_v0 + 4);
            /* ASM_BARRIER: load fourth vertex words before storing OTZ */
            __asm__ volatile("" : "+r"(r_a3), "+r"(t0));
            r_v0 = (int)((char *)r_a1 + 0x18);
            RR_GTE14_SWC2_OTZ(r_v0);
            r_v0 = r_a1[0x18 / 4];
            /* ASM_BARRIER: load OTZ before storing 0x40..0x44 */
            __asm__ volatile("" : "+r"(r_v0));
            r_a1[0x40 / 4] = r_a3;
            r_a1[0x44 / 4] = t0;
            if (r_v0 > 0) {
                r_v0 >>= t8;
                r_a1[0x18 / 4] = r_v0;
                if (r_v0 < 0x1C0) {
                    RR_GTE14_NCLIP();
                    r_a3 = r_a1[0x50 / 4];
                    t0 = r_a1[0x54 / 4];
                    t1 = r_a1[0x58 / 4];
                    /* ASM_BARRIER: load SXY0..SXY2 before staging a1+0x20 */
                    __asm__ volatile("" : "+r"(r_a3), "+r"(t0), "+r"(t1));
                    r_v0 = (int)((char *)r_a1 + 0x20);
                    /* ASM_BARRIER: stage a1+0x20 before stores */
                    __asm__ volatile("" : "+r"(r_v0));
                    *(int *)(t2 + 0x05) = r_a3;
                    *(int *)(t2 + 0x0D) = t0;
                    *(int *)(t2 + 0x15) = t1;
                    RR_GTE14_SWC2_MAC0(r_v0);
                    if (r_a1[0x1C / 4] > 0 || r_a1[0x20 / 4] < 0) {
                        RR_GTE14_SWC2_SXY2((char *)r_a1 + 0x5C);
                        r_v0 = r_a1[0x5C / 4];
                        *(int *)(t2 + 0x1D) = r_v0;

                        x0 = *(short *)((char *)r_a1 + 0x50);
                        x1 = *(short *)((char *)r_a1 + 0x54);
                        x2 = *(short *)((char *)r_a1 + 0x58);
                        x3 = *(short *)((char *)r_a1 + 0x5C);
                        if ((x0 >= 0 || x1 >= 0 || x2 >= 0 || x3 >= 0) &&
                            (x0 < 0x140 || x1 < 0x140 || x2 < 0x140 || x3 < 0x140)) {
                            y0 = *(short *)((char *)r_a1 + 0x52);
                            y1 = *(short *)((char *)r_a1 + 0x56);
                            y2 = *(short *)((char *)r_a1 + 0x5A);
                            y3 = *(short *)((char *)r_a1 + 0x5E);
                            if ((y0 >= 0 || y1 >= 0 || y2 >= 0 || y3 >= 0) &&
                                (y0 < 0x200 || y1 < 0x200 || y2 < 0x200 || y3 < 0x200)) {
                                *(int *)(t2 + 0x01) = t9;
                                r_a3 = *(int *)(t3 - 0x0D);
                                t1 = *(int *)(t3 - 0x05);
                                t0 = *(int *)(t3 - 0x09);
                                /* ASM_BARRIER: load UV words before shift and stores */
                                __asm__ volatile("" : "+r"(r_a3), "+r"(t1), "+r"(t0));
                                r_v0 = t1 >> 16;
                                /* ASM_BARRIER: shift before UV stores */
                                __asm__ volatile("" : "+r"(r_v0));
                                *(int *)(t2 + 0x09) = r_a3;
                                *(int *)(t2 + 0x11) = t0;
                                *(int *)(t2 + 0x19) = t1;
                                *(int *)(t2 + 0x21) = r_v0;

                                r_v1 = r_a1[0x18 / 4];
                                r_a0 = r_a1[0x14 / 4];
                                r_v0 = *(signed char *)t3;
                                /* ASM_BARRIER: load otz, ot base, and offset byte before shifts */
                                __asm__ volatile("" : "+r"(r_v1), "+r"(r_a0), "+r"(r_v0));
                                r_v1 <<= 2;
                                r_a0 += r_v1;
                                r_v0 <<= 2;
                                r_a0 += r_v0;
                                /* ASM_BARRIER: keep OT entry address in $a0 */
                                __asm__ volatile("" : "+r"(r_a0));
                                r_v0 = *(int *)r_a0;
                                *(int *)t4 = r_v0;
                                *t2 = 9;
                                t2 += 0x28;
                                /* ASM_BARRIER: advance t2 before storing t4 to OT */
                                __asm__ volatile("" : "+r"(t2));
                                *(int *)r_a0 = (int)t4;
                                t4 += 0x28;
                                /* ASM_BARRIER: advance t4 before clearing top byte of OT entry */
                                __asm__ volatile("" : "+r"(t4));
                                *(char *)(r_a0 + 3) = 0;
                            }
                        }
                    }
                }
            }
            t3 += 0x18;
            t7 += 0x18;
        } while (r_a2-- > 0);
    }
    *(char **)SCRATCHPAD = t4;
    return t7;
}

void *EmitRroLitQuadsG4(char *a0_in, int *a1_in, int a2_in) {
    /* ASM_BARRIER: keep temp in $v0 */
    register int r_v0 __asm__("$2");
    /* ASM_BARRIER: keep temp in $v1 */
    register int r_v1 __asm__("$3");
    /* ASM_BARRIER: keep a0_in / temp in $a0 */
    register int r_a0 __asm__("$4") = (int)a0_in;
    /* ASM_BARRIER: keep a1_in / temp in $a1 */
    register int r_a1 __asm__("$5") = (int)a1_in;
    /* ASM_BARRIER: keep loop counter in $a2 */
    register int r_a2 __asm__("$6") = a2_in;
    /* ASM_BARRIER: keep temp in $a3 */
    register int r_a3 __asm__("$7");
    /* ASM_BARRIER: keep temp in $t0 */
    register int t0 __asm__("$8");
    /* ASM_BARRIER: keep temp in $t1 */
    register int t1 __asm__("$9");
    /* ASM_BARRIER: keep context pointer in $t2 */
    register int *t2 __asm__("$10");
    /* ASM_BARRIER: keep packet+0x20 cursor in $t3 */
    register char *t3 __asm__("$11");
    /* ASM_BARRIER: keep fourth vertex/normal index in $t4 */
    register int t4 __asm__("$12");
    /* ASM_BARRIER: keep poly+0x15 cursor in $t5 */
    register char *t5 __asm__("$13");
    /* ASM_BARRIER: keep packet cursor in $t6 */
    register char *t6 __asm__("$14");
    /* ASM_BARRIER: keep vertex table base in $t7 */
    register int t7 __asm__("$15");
    /* ASM_BARRIER: keep poly cursor in $t8 */
    register char *t8 __asm__("$24");
    /* ASM_BARRIER: keep 0xE6000000 constant in $t9 */
    register int t9 __asm__("$25");
    /* ASM_BARRIER: keep OT shift in $s0 */
    register int s0 __asm__("$16");
    int x0, x1, x2, x3, y0, y1, y2, y3;

    t8 = (char *)r_a0;
    t2 = (int *)r_a1;
    t9 = 0xE6000000;
    /* ASM_BARRIER: order t8, t2, t9 setup before initial loads */
    __asm__ volatile("" : "+r"(t8), "+r"(t2), "+r"(t9));
    r_v0 = *(int *)(t8 + 0);
    t0 = *(int *)(t8 + 4);
    t7 = t2[0];
    /* ASM_BARRIER: order initial loads before index shifts */
    __asm__ volatile("" : "+r"(r_v0), "+r"(t0), "+r"(t7));
    r_v1 = r_v0 >> 16;
    t4 = t0 >> 16;
    r_v0 &= 0xFFFF;
    /* ASM_BARRIER: order shifts and mask before first vertex address */
    __asm__ volatile("" : "+r"(r_v1), "+r"(t4), "+r"(r_v0));
    r_v0 = (r_v0 << 3) + t7;
    /* ASM_BARRIER: compute first vertex pointer in $v0 */
    __asm__ volatile("" : "+r"(r_v0));
    r_a1 = *(int *)(r_v0 + 0);
    r_a3 = *(int *)(r_v0 + 4);
    /* ASM_BARRIER: load first vertex words before second vertex address */
    __asm__ volatile("" : "+r"(r_a1), "+r"(r_a3));
    r_v0 = (r_v1 << 3) + t7;
    t0 &= 0xFFFF;
    /* ASM_BARRIER: compute second vertex pointer and mask t0 */
    __asm__ volatile("" : "+r"(r_v0), "+r"(t0));
    t1 = *(int *)(r_v0 + 0);
    r_v0 = *(int *)(r_v0 + 4);
    /* ASM_BARRIER: load second vertex words before copying a2 to v1 */
    __asm__ volatile("" : "+r"(t1), "+r"(r_v0));
    r_v1 = r_a2;
    /* ASM_BARRIER: copy a2 in load delay slot before storing 0x34 */
    __asm__ volatile("" : "+r"(r_v1));
    t2[0x34 / 4] = r_v0;
    r_v0 = (t0 << 3) + t7;
    /* ASM_BARRIER: compute third vertex pointer before storing 0x28..0x30 */
    __asm__ volatile("" : "+r"(r_v0));
    t2[0x28 / 4] = r_a1;
    t2[0x2C / 4] = r_a3;
    t2[0x30 / 4] = t1;
    r_a1 = *(int *)(r_v0 + 0);
    r_a3 = *(int *)(r_v0 + 4);
    /* ASM_BARRIER: load third vertex words before fourth vertex address */
    __asm__ volatile("" : "+r"(r_a1), "+r"(r_a3));
    r_v0 = (t4 << 3) + t7;
    /* ASM_BARRIER: compute fourth vertex pointer in $v0 */
    __asm__ volatile("" : "+r"(r_v0));
    t1 = *(int *)(r_v0 + 0);
    r_v0 = *(int *)(r_v0 + 4);
    /* ASM_BARRIER: load fourth vertex words before storing 0x44 */
    __asm__ volatile("" : "+r"(t1), "+r"(r_v0));
    t2[0x44 / 4] = r_v0;
    r_v0 = SCRATCHPAD;
    /* ASM_BARRIER: load SCRATCHPAD into $v0 before storing 0x38..0x40 */
    __asm__ volatile("" : "+r"(r_v0));
    t2[0x38 / 4] = r_a1;
    t2[0x3C / 4] = r_a3;
    t2[0x40 / 4] = t1;
    t6 = *(char **)(r_v0 + 0x0);
    s0 = *(int *)(r_v0 + 0x60);
    /* ASM_BARRIER: load scratchpad fields before decrementing a2 and branching */
    __asm__ volatile("" : "+r"(t6), "+r"(s0), "+r"(r_v1));
    r_a2--;
    if (r_v1 > 0) {
        t5 = (char *)r_a0 + 0x15;
        t3 = t6 + 0x20;
        do {
            /* ASM_BARRIER: keep loop pointers live at top of loop */
            __asm__ volatile("" : "+r"(t5), "+r"(t3), "+r"(t6), "+r"(t8));
            r_v0 = (int)((char *)t2 + 0x28);
            RR_GTE14_LWC2_V012(r_v0);
            RR_GTE14_NOP2();
            RR_GTE14_RTPT();

            r_v0 = *(int *)(t5 + 3);
            t0 = *(int *)(t5 + 7);
            /* ASM_BARRIER: load next packed indices before shifts */
            __asm__ volatile("" : "+r"(r_v0), "+r"(t0));
            r_v1 = r_v0 >> 16;
            t4 = t0 >> 16;
            r_v0 &= 0xFFFF;
            /* ASM_BARRIER: order shifts and mask before first vertex address */
            __asm__ volatile("" : "+r"(r_v1), "+r"(t4), "+r"(r_v0));
            r_v0 = (r_v0 << 3) + t7;
            /* ASM_BARRIER: compute first vertex pointer in $v0 */
            __asm__ volatile("" : "+r"(r_v0));
            r_a1 = *(int *)(r_v0 + 0);
            r_a3 = *(int *)(r_v0 + 4);
            /* ASM_BARRIER: load first vertex words before masking t0 */
            __asm__ volatile("" : "+r"(r_a1), "+r"(r_a3));
            t0 &= 0xFFFF;
            /* ASM_BARRIER: mask t0 before storing 0x28..0x2C */
            __asm__ volatile("" : "+r"(t0));
            t2[0x28 / 4] = r_a1;
            t2[0x2C / 4] = r_a3;
            RR_GTE14_NCLIP();

            r_v0 = (r_v1 << 3) + t7;
            /* ASM_BARRIER: compute second vertex pointer in $v0 */
            __asm__ volatile("" : "+r"(r_v0));
            r_a1 = *(int *)(r_v0 + 0);
            r_a3 = *(int *)(r_v0 + 4);
            /* ASM_BARRIER: load second vertex words before storing MAC0/SXYZ and loading V0 */
            __asm__ volatile("" : "+r"(r_a1), "+r"(r_a3));
            r_v0 = (int)((char *)t2 + 0x1C);
            RR_GTE14_SWC2_MAC0(r_v0);
            r_v0 = (int)((char *)t2 + 0x50);
            RR_GTE14_SWC2_SXYZ(r_v0);
            r_v0 = (int)((char *)t2 + 0x40);
            RR_GTE14_LWC2_V0(r_v0);
            t2[0x30 / 4] = r_a1;
            t2[0x34 / 4] = r_a3;
            RR_GTE14_RTPS();

            r_v0 = (t0 << 3) + t7;
            /* ASM_BARRIER: compute third vertex pointer in $v0 */
            __asm__ volatile("" : "+r"(r_v0));
            r_a1 = *(int *)(r_v0 + 0);
            r_a3 = *(int *)(r_v0 + 4);
            /* ASM_BARRIER: load third vertex words before storing 0x38..0x3C */
            __asm__ volatile("" : "+r"(r_a1), "+r"(r_a3));
            t2[0x38 / 4] = r_a1;
            t2[0x3C / 4] = r_a3;
            RR_GTE14_AVSZ4();

            r_v0 = (t4 << 3) + t7;
            /* ASM_BARRIER: compute fourth vertex pointer in $v0 */
            __asm__ volatile("" : "+r"(r_v0));
            r_a1 = *(int *)(r_v0 + 0);
            r_a3 = *(int *)(r_v0 + 4);
            /* ASM_BARRIER: load fourth vertex words before storing OTZ */
            __asm__ volatile("" : "+r"(r_a1), "+r"(r_a3));
            r_v0 = (int)((char *)t2 + 0x18);
            RR_GTE14_SWC2_OTZ(r_v0);
            r_v0 = t2[0x18 / 4];
            /* ASM_BARRIER: load OTZ before storing 0x40..0x44 */
            __asm__ volatile("" : "+r"(r_v0));
            t2[0x40 / 4] = r_a1;
            t2[0x44 / 4] = r_a3;
            if (r_v0 > 0) {
                r_v0 >>= s0;
                t2[0x18 / 4] = r_v0;
                if (r_v0 < 0x1C0) {
                    RR_GTE14_NCLIP();
                    r_a1 = t2[0x50 / 4];
                    r_a3 = t2[0x54 / 4];
                    t1 = t2[0x58 / 4];
                    /* ASM_BARRIER: load SXY0..SXY2 before staging t2+0x20 */
                    __asm__ volatile("" : "+r"(r_a1), "+r"(r_a3), "+r"(t1));
                    r_v0 = (int)((char *)t2 + 0x20);
                    /* ASM_BARRIER: stage t2+0x20 before stores */
                    __asm__ volatile("" : "+r"(r_v0));
                    *(int *)(t3 - 0x14) = r_a1;
                    *(int *)(t3 - 0x0C) = r_a3;
                    *(int *)(t3 - 0x04) = t1;
                    RR_GTE14_SWC2_MAC0(r_v0);
                    if (t2[0x1C / 4] > 0 || t2[0x20 / 4] < 0) {
                        RR_GTE14_SWC2_SXY2((char *)t2 + 0x5C);
                        r_v0 = t2[0x5C / 4];
                        *(int *)(t3 + 0x04) = r_v0;

                        x0 = *(short *)((char *)t2 + 0x50);
                        x1 = *(short *)((char *)t2 + 0x54);
                        x2 = *(short *)((char *)t2 + 0x58);
                        x3 = *(short *)((char *)t2 + 0x5C);
                        if ((x0 >= 0 || x1 >= 0 || x2 >= 0 || x3 >= 0) &&
                            (x0 < 0x140 || x1 < 0x140 || x2 < 0x140 || x3 < 0x140)) {
                            y0 = *(short *)((char *)t2 + 0x52);
                            y1 = *(short *)((char *)t2 + 0x56);
                            y2 = *(short *)((char *)t2 + 0x5A);
                            y3 = *(short *)((char *)t2 + 0x5E);
                            if ((y0 >= 0 || y1 >= 0 || y2 >= 0 || y3 >= 0) &&
                                (y0 < 0x200 || y1 < 0x200 || y2 < 0x200 || y3 < 0x200)) {
                                r_v0 = *(int *)(t5 - 0x0D);
                                t0 = *(int *)(t5 - 0x09);
                                r_a1 = t2[1];
                                /* ASM_BARRIER: load packed normal indices and normal table base before shifts */
                                __asm__ volatile("" : "+r"(r_v0), "+r"(t0), "+r"(r_a1));
                                r_v1 = r_v0 >> 16;
                                t4 = t0 >> 16;
                                r_v0 &= 0xFFFF;
                                t0 &= 0xFFFF;
                                /* ASM_BARRIER: order index shifts and masks before address math */
                                __asm__ volatile("" : "+r"(r_v1), "+r"(t4), "+r"(r_v0), "+r"(t0));
                                r_a0 = r_v0 << 3;
                                /* ASM_BARRIER: shift first normal index before adding base */
                                __asm__ volatile("" : "+r"(r_a0));
                                r_a0 = r_a1 + r_a0;
                                /* ASM_BARRIER: compute first normal address in $a0 */
                                __asm__ volatile("" : "+r"(r_a0));
                                r_v1 = r_a1 + (r_v1 << 3);
                                /* ASM_BARRIER: compute second normal address in $v1 */
                                __asm__ volatile("" : "+r"(r_v1));
                                r_v0 = r_a1 + (t0 << 3);
                                /* ASM_BARRIER: compute third normal address in $v0 */
                                __asm__ volatile("" : "+r"(r_v0));
                                RR_GTE14_LWC2_V0_V1_V2(r_a0, r_v1, r_v0);
                                r_v0 = (int)(t8 + 0x10);
                                RR_GTE14_LWC2_RGB(r_v0);
                                RR_GTE14_NOP2();
                                RR_GTE14_LIGHT0();

                                r_v0 = (t4 << 3) + r_a1;
                                /* ASM_BARRIER: compute fourth normal address in $v0 */
                                __asm__ volatile("" : "+r"(r_v0));
                                r_a1 = *(int *)(r_v0 + 0);
                                r_a3 = *(int *)(r_v0 + 4);
                                /* ASM_BARRIER: load fourth normal words before storing 0x48..0x4C */
                                __asm__ volatile("" : "+r"(r_a1), "+r"(r_a3));
                                t2[0x48 / 4] = r_a1;
                                t2[0x4C / 4] = r_a3;
                                RR_GTE14_LIGHT1();

                                r_a0 = t2[0x18 / 4];
                                r_v1 = t2[0x14 / 4];
                                r_v0 = *(signed char *)t5;
                                /* ASM_BARRIER: load otz, ot base, and offset byte before shifts */
                                __asm__ volatile("" : "+r"(r_a0), "+r"(r_v1), "+r"(r_v0));
                                r_a0 <<= 2;
                                r_v1 += r_a0;
                                r_v0 <<= 2;
                                r_v1 += r_v0;
                                /* ASM_BARRIER: keep OT entry address in $v1 */
                                __asm__ volatile("" : "+r"(r_v1));
                                r_v0 = *(int *)r_v1;
                                /* ASM_BARRIER: load-use delay on *(int *)r_v1 */
                                __asm__ volatile("" : "+r"(r_v0));
                                *(int *)t6 = r_v0;
                                r_v0 = 0xA;
                                /* ASM_BARRIER: keep 0xA in $v0 before storing -0x1D(t3) */
                                __asm__ volatile("" : "+r"(r_v0));
                                *(char *)(t3 - 0x1D) = r_v0;
                                /* ASM_BARRIER: store -0x1D(t3) before OT stores and MaskBitSetting load */
                                __asm__ volatile("" : "+r"(r_v1));
                                *(int *)r_v1 = (int)t6;
                                *(char *)(r_v1 + 3) = 0;
                                r_v1 = MaskBitSetting;
                                /* ASM_BARRIER: load MaskBitSetting into $v1 before computing t9|1 in load delay slot */
                                __asm__ volatile("" : "+r"(r_v1));
                                r_v0 = t9 | 1;
                                *(int *)(t3 - 0x1C) = r_v0;
                                /* ASM_BARRIER: store -0x1C(t3) before computing t9|r_v1 */
                                __asm__ volatile("" : "+r"(r_v1));
                                r_v1 = t9 | r_v1;
                                *(int *)(t3 + 0x08) = r_v1;
                                RR_GTE14_LIGHT2();
                                r_v0 = (int)(t6 + 4);
                                RR_GTE14_SWC2_RGB012(r_v0);
                                r_v0 = (int)((char *)t2 + 0x48);
                                RR_GTE14_LWC2_V0(r_v0);
                                RR_GTE14_NOP2();
                                RR_GTE14_LIGHT0();
                                RR_GTE14_SWC2_RGB2(t3);
                                t3 += 0x2C;
                                t6 += 0x2C;
                            }
                        }
                    }
                }
            }
            t5 += 0x18;
            t8 += 0x18;
        } while (r_a2-- > 0);
    }
    *(char **)SCRATCHPAD = t6;
    return t8;
}

void *EmitRroEnvMapQuads(char *a0_in, int *a1_in, int a2_in) {
    /* ASM_BARRIER: keep temp in $v0 */
    register int r_v0 __asm__("$2");
    /* ASM_BARRIER: keep temp in $v1 */
    register int r_v1 __asm__("$3");
    /* ASM_BARRIER: keep a0_in / temp in $a0 */
    register int r_a0 __asm__("$4") = (int)a0_in;
    /* ASM_BARRIER: keep a1_in / temp in $a1 */
    register int r_a1 __asm__("$5") = (int)a1_in;
    /* ASM_BARRIER: keep a2_in / temp in $a2 */
    register int r_a2 __asm__("$6") = a2_in;
    /* ASM_BARRIER: keep packet+0x6C cursor in $a3 */
    register char *r_a3 __asm__("$7");
    /* ASM_BARRIER: keep context pointer in $t0 */
    register int *t0 __asm__("$8");
    /* ASM_BARRIER: keep envmap U in $t1 */
    register int t1 __asm__("$9");
    /* ASM_BARRIER: keep envmap V in $t2 */
    register int t2 __asm__("$10");
    /* ASM_BARRIER: keep packet cursor in $t3 */
    register char *t3 __asm__("$11");
    /* ASM_BARRIER: keep poly+0x1E cursor in $t4 */
    register char *t4 __asm__("$12");
    /* ASM_BARRIER: keep fourth vertex index in $t5 */
    register int t5 __asm__("$13");
    /* ASM_BARRIER: keep vertex table base in $t6 */
    register int t6 __asm__("$14");
    /* ASM_BARRIER: keep temp in $t7 */
    register int t7 __asm__("$15");
    /* ASM_BARRIER: keep envmap V shift in $t8 */
    register int t8 __asm__("$24");
    /* ASM_BARRIER: keep loop counter in $t9 */
    register int t9 __asm__("$25");
    /* ASM_BARRIER: keep poly cursor in $s0 */
    register char *s0 __asm__("$16");
    /* ASM_BARRIER: keep 0xE6000000 constant in $s1 */
    register int s1 __asm__("$17");
    /* ASM_BARRIER: keep 0xD tag size in $s2 */
    register int s2 __asm__("$18");
    /* ASM_BARRIER: keep OTZ shift in $s3 */
    register int s3 __asm__("$19");
    /* ASM_BARRIER: keep MaskBitSetting | 0xE6000000 in $s4 */
    register int s4 __asm__("$20");
    int x0, x1, x2, x3, y0, y1, y2, y3;

    s0 = (char *)r_a0;
    t0 = (int *)r_a1;
    t9 = r_a2;
    /* ASM_BARRIER: keep s0, t0, t9 in their registers before initial loads */
    __asm__("" : "+r"(s0), "+r"(t0), "+r"(t9));
    s1 = 0xE6000000;
    r_v0 = *(int *)(s0 + 0);
    r_a2 = *(int *)(s0 + 4);
    t6 = t0[0];
    /* ASM_BARRIER: order initial loads before shifts */
    __asm__ volatile("" : "+r"(r_v0), "+r"(r_a2), "+r"(t6), "+r"(s1));
    r_a1 = r_v0 >> 16;
    t5 = r_a2 >> 16;
    r_v0 &= 0xFFFF;
    /* ASM_BARRIER: order shifts and mask before first vertex address */
    __asm__ volatile("" : "+r"(r_a1), "+r"(t5), "+r"(r_v0));
    r_v0 = (r_v0 << 3) + t6;
    /* ASM_BARRIER: compute first vertex pointer in $v0 */
    __asm__ volatile("" : "+r"(r_v0));
    r_v1 = *(int *)(r_v0 + 0);
    r_a0 = *(int *)(r_v0 + 4);
    /* ASM_BARRIER: load first vertex words before second vertex address */
    __asm__ volatile("" : "+r"(r_v1), "+r"(r_a0));
    r_v0 = (r_a1 << 3) + t6;
    r_a2 &= 0xFFFF;
    /* ASM_BARRIER: compute second vertex pointer and mask a2 */
    __asm__ volatile("" : "+r"(r_v0), "+r"(r_a2));
    r_a1 = *(int *)(r_v0 + 0);
    t7 = *(int *)(r_v0 + 4);
    /* ASM_BARRIER: load second vertex words before third vertex address */
    __asm__ volatile("" : "+r"(r_a1), "+r"(t7));
    r_v0 = (r_a2 << 3) + t6;
    /* ASM_BARRIER: compute third vertex pointer before storing 0x28..0x34 */
    __asm__ volatile("" : "+r"(r_v0));
    t0[0x28 / 4] = r_v1;
    t0[0x2C / 4] = r_a0;
    t0[0x30 / 4] = r_a1;
    t0[0x34 / 4] = t7;
    r_v1 = *(int *)(r_v0 + 0);
    r_a0 = *(int *)(r_v0 + 4);
    /* ASM_BARRIER: load third vertex words before fourth vertex address */
    __asm__ volatile("" : "+r"(r_v1), "+r"(r_a0));
    r_v0 = (t5 << 3) + t6;
    /* ASM_BARRIER: compute fourth vertex pointer in $v0 */
    __asm__ volatile("" : "+r"(r_v0));
    r_a1 = *(int *)(r_v0 + 0);
    t7 = *(int *)(r_v0 + 4);
    /* ASM_BARRIER: load fourth vertex words before MaskBitSetting and stores */
    __asm__ volatile("" : "+r"(r_a1), "+r"(t7));
    /* ASM_BARRIER: materialize %hi(MaskBitSetting) before storing r_a0 to 0x3C(t0) */
    __asm__("" : "+r"(r_a0) : "m"(MaskBitSetting));
    *(int *)((char *)t0 + 0x3C) = r_a0;
    r_a0 = SCRATCHPAD;
    *(int *)((char *)t0 + 0x38) = r_v1;
    *(int *)((char *)t0 + 0x40) = r_a1;
    r_v0 = MaskBitSetting;
    /* ASM_BARRIER: load MaskBitSetting before setting v1=0x100 and storing 0x44 */
    __asm__ volatile("" : "+r"(r_v0), "+r"(r_a0));
    r_v1 = 0x100;
    /* ASM_BARRIER: set v1=0x100 before storing 0x44 */
    __asm__ volatile("" : "+r"(r_v1));
    *(int *)((char *)t0 + 0x44) = t7;
    /* ASM_BARRIER: store 0x44 before computing s4 */
    __asm__ volatile("" : "+r"(r_v0));
    s4 = r_v0 | s1;
    /* ASM_BARRIER: compute s4 before loading CurrentDrawBuf */
    __asm__ volatile("" : "+r"(s4));
    r_v0 = (int)CurrentDrawBuf;
    t3 = *(char **)(r_a0 + 0x0);
    /* ASM_BARRIER: load CurrentDrawBuf and t3 before loading CurrentDrawBuf[3] and s3 */
    __asm__ volatile("" : "+r"(r_v0), "+r"(t3));
    r_v0 = *(short *)(r_v0 + 6);
    s3 = *(int *)(r_a0 + 0x60);
    /* ASM_BARRIER: load CurrentDrawBuf[3] and s3 before branch */
    __asm__ volatile("" : "+r"(r_v0), "+r"(s3));
    t8 = 7;
    if (r_v0 == r_v1) {
        t8 = 6;
    }
    /* ASM_BARRIER: keep t8 after branch */
    __asm__ volatile("" : "+r"(t8), "+r"(t9));
    if (t9-- > 0) {
        s2 = 0xD;
        t4 = s0 + 0x1E;
        r_a3 = t3 + 0x6C;
        do {
            /* ASM_BARRIER: keep loop pointers live at top of loop */
            __asm__ volatile("" : "+r"(s2), "+r"(t4), "+r"(r_a3) : "r"(t3), "r"(s0));
            r_v0 = (int)((char *)t0 + 0x28);
            RR_GTE14_LWC2_V012(r_v0);
            RR_GTE14_NOP2();
            RR_GTE14_RTPT();

            r_v0 = *(int *)(t4 + 2);
            r_a2 = *(int *)(t4 + 6);
            /* ASM_BARRIER: load next packed indices before shifts */
            __asm__ volatile("" : "+r"(r_v0), "+r"(r_a2));
            r_a1 = r_v0 >> 16;
            t5 = r_a2 >> 16;
            r_v0 &= 0xFFFF;
            /* ASM_BARRIER: order shifts and mask before first vertex address */
            __asm__ volatile("" : "+r"(r_a1), "+r"(t5), "+r"(r_v0));
            r_v0 = (r_v0 << 3) + t6;
            /* ASM_BARRIER: compute first vertex pointer in $v0 */
            __asm__ volatile("" : "+r"(r_v0));
            r_v1 = *(int *)(r_v0 + 0);
            r_a0 = *(int *)(r_v0 + 4);
            /* ASM_BARRIER: load first vertex words before masking a2 */
            __asm__ volatile("" : "+r"(r_v1), "+r"(r_a0));
            r_a2 &= 0xFFFF;
            /* ASM_BARRIER: mask a2 before storing 0x28..0x2C */
            __asm__ volatile("" : "+r"(r_a2));
            t0[0x28 / 4] = r_v1;
            t0[0x2C / 4] = r_a0;
            RR_GTE14_NCLIP();

            r_v0 = (r_a1 << 3) + t6;
            /* ASM_BARRIER: compute second vertex pointer in $v0 */
            __asm__ volatile("" : "+r"(r_v0));
            r_v1 = *(int *)(r_v0 + 0);
            r_a0 = *(int *)(r_v0 + 4);
            /* ASM_BARRIER: load second vertex words before storing MAC0/SXYZ and loading V0 */
            __asm__ volatile("" : "+r"(r_v1), "+r"(r_a0));
            r_v0 = (int)((char *)t0 + 0x1C);
            RR_GTE14_SWC2_MAC0(r_v0);
            r_v0 = (int)((char *)t0 + 0x50);
            RR_GTE14_SWC2_SXYZ(r_v0);
            r_v0 = (int)((char *)t0 + 0x40);
            RR_GTE14_LWC2_V0(r_v0);
            t0[0x30 / 4] = r_v1;
            t0[0x34 / 4] = r_a0;
            RR_GTE14_RTPS();

            r_v0 = (r_a2 << 3) + t6;
            /* ASM_BARRIER: compute third vertex pointer in $v0 */
            __asm__ volatile("" : "+r"(r_v0));
            r_v1 = *(int *)(r_v0 + 0);
            r_a0 = *(int *)(r_v0 + 4);
            /* ASM_BARRIER: load third vertex words before storing 0x38..0x3C */
            __asm__ volatile("" : "+r"(r_v1), "+r"(r_a0));
            t0[0x38 / 4] = r_v1;
            t0[0x3C / 4] = r_a0;
            RR_GTE14_AVSZ4();

            r_v0 = (t5 << 3) + t6;
            /* ASM_BARRIER: compute fourth vertex pointer in $v0 */
            __asm__ volatile("" : "+r"(r_v0));
            r_v1 = *(int *)(r_v0 + 0);
            r_a0 = *(int *)(r_v0 + 4);
            /* ASM_BARRIER: load fourth vertex words before storing OTZ */
            __asm__ volatile("" : "+r"(r_v1), "+r"(r_a0));
            r_v0 = (int)((char *)t0 + 0x18);
            RR_GTE14_SWC2_OTZ(r_v0);
            r_v0 = t0[0x18 / 4];
            /* ASM_BARRIER: load OTZ before storing 0x40..0x44 */
            __asm__ volatile("" : "+r"(r_v0));
            t0[0x40 / 4] = r_v1;
            t0[0x44 / 4] = r_a0;
            if (r_v0 > 0) {
                r_v0 >>= s3;
                t0[0x18 / 4] = r_v0;
                if (r_v0 < 0x1C0) {
                    RR_GTE14_NCLIP();
                    r_v1 = t0[0x50 / 4];
                    r_a0 = t0[0x54 / 4];
                    r_a1 = t0[0x58 / 4];
                    /* ASM_BARRIER: load SXY0..SXY2 before staging t0+0x20 */
                    __asm__ volatile("" : "+r"(r_v1), "+r"(r_a0), "+r"(r_a1));
                    r_v0 = (int)((char *)t0 + 0x20);
                    /* ASM_BARRIER: stage t0+0x20 before stores */
                    __asm__ volatile("" : "+r"(r_v0));
                    *(int *)(r_a3 - 0x60) = r_v1;
                    *(int *)(r_a3 - 0x28) = r_v1;
                    *(int *)(r_a3 - 0x54) = r_a0;
                    *(int *)(r_a3 - 0x1C) = r_a0;
                    *(int *)(r_a3 - 0x48) = r_a1;
                    *(int *)(r_a3 - 0x10) = r_a1;
                    RR_GTE14_SWC2_MAC0(r_v0);
                    if (t0[0x1C / 4] > 0 || t0[0x20 / 4] < 0) {
                        /* ASM_BARRIER: read SXY2 from GTE $14 into $t7 */
                        __asm__ volatile("mfc2 %0, $14" : "=r"(t7));
                        RR_NOP_DEP();
                        t0[0x5C / 4] = t7;
                        *(int *)(r_a3 - 0x3C) = t7;
                        *(int *)(r_a3 - 0x04) = t7;

                        x0 = *(short *)((char *)t0 + 0x50);
                        x1 = *(short *)((char *)t0 + 0x54);
                        x2 = *(short *)((char *)t0 + 0x58);
                        x3 = *(short *)((char *)t0 + 0x5C);
                        if ((x0 >= 0 || x1 >= 0 || x2 >= 0 || x3 >= 0) &&
                            (x0 < 0x140 || x1 < 0x140 || x2 < 0x140 || x3 < 0x140)) {
                            y0 = *(short *)((char *)t0 + 0x52);
                            y1 = *(short *)((char *)t0 + 0x56);
                            y2 = *(short *)((char *)t0 + 0x5A);
                            y3 = *(short *)((char *)t0 + 0x5E);
                            if ((y0 >= 0 || y1 >= 0 || y2 >= 0 || y3 >= 0) &&
                                (y0 < 0x200 || y1 < 0x200 || y2 < 0x200 || y3 < 0x200)) {
                                r_v1 = *(int *)(t4 - 0x16);
                                r_v0 = *(int *)(t4 - 0x12);
                                r_a2 = t0[1];
                                /* ASM_BARRIER: load packed normal indices and normal table base before shifts */
                                __asm__ volatile("" : "+r"(r_v1), "+r"(r_v0), "+r"(r_a2));
                                r_a0 = r_v1 >> 16;
                                r_v1 &= 0xFFFF;
                                r_a1 = r_v0 >> 16;
                                r_v0 &= 0xFFFF;
                                /* ASM_BARRIER: order shifts and masks before normal address math */
                                __asm__ volatile("" : "+r"(r_a0), "+r"(r_v1), "+r"(r_a1), "+r"(r_v0));
                                r_v1 <<= 3;
                                /* ASM_BARRIER: shift first normal index before adding base */
                                __asm__ volatile("" : "+r"(r_v1));
                                r_v1 = r_a2 + r_v1;
                                /* ASM_BARRIER: compute first normal address in $v1 */
                                __asm__ volatile("" : "+r"(r_v1));
                                r_a0 = r_a2 + (r_a0 << 3);
                                /* ASM_BARRIER: compute second normal address in $a0 */
                                __asm__ volatile("" : "+r"(r_a0));
                                r_v0 = r_a2 + (r_v0 << 3);
                                /* ASM_BARRIER: compute third normal address in $v0 */
                                __asm__ volatile("" : "+r"(r_v0));
                                RR_GTE14_LWC2_V0_V1_V2(r_v1, r_a0, r_v0);
                                RR_GTE14_NOP2();
                                RR_GTE_OP(0x4A4A6412);

                                r_a1 = (r_a1 << 3) + r_a2;
                                /* ASM_BARRIER: compute fourth normal address in $a1 */
                                __asm__ volatile("" : "+r"(r_a1));
                                r_v1 = *(int *)(r_a1 + 0);
                                r_a0 = *(int *)(r_a1 + 4);
                                /* ASM_BARRIER: load fourth normal words before storing 0x48..0x4C */
                                __asm__ volatile("" : "+r"(r_v1), "+r"(r_a0));
                                t0[0x48 / 4] = r_v1;
                                t0[0x4C / 4] = r_a0;
                                RR_GTE_SQR12();
                                RR_GTE_SQR12();
                                RR_GTE_OP(0x4B38041C);

                                r_v1 = *(int *)(t4 - 0x0E);
                                r_a0 = *(int *)(t4 - 0x0A);
                                /* ASM_BARRIER: load UV0/UV1 before MVMVA V1 */
                                __asm__ volatile("" : "+r"(r_v1), "+r"(r_a0));
                                RR_GTE_OP(0x4A4AE412);
                                r_v0 = t0[0x08 / 4];
                                /* ASM_BARRIER: load-use delay on t0[2] */
                                __asm__ volatile("" : "+r"(r_v0));
                                r_v0 = r_v1 + r_v0;
                                *(int *)(r_a3 - 0x5C) = r_v0;
                                *(int *)(r_a3 - 0x50) = r_a0;
                                RR_GTE_SQR12();
                                RR_GTE_SQR12();
                                RR_GTE_OP(0x4B38041C);

                                r_v1 = *(int *)(t4 - 0x06);
                                /* ASM_BARRIER: load-use delay on *(int *)(t4 - 6) */
                                __asm__ volatile("" : "+r"(r_v1));
                                r_a0 = r_v1 >> 16;
                                /* ASM_BARRIER: shift r_a0 before MVMVA V2 */
                                __asm__ volatile("" : "+r"(r_a0));
                                RR_GTE_OP(0x4A4B6412);
                                *(int *)(r_a3 - 0x44) = r_v1;
                                *(int *)(r_a3 - 0x38) = r_a0;
                                RR_GTE_SQR12();
                                RR_GTE_SQR12();
                                RR_GTE_OP(0x4B38041C);

                                r_a2 = (int)(t3 + 4);
                                /* ASM_BARRIER: store RGB0..RGB2 into POLY_GT4 packet */
                                __asm__ volatile("swc2 $20, 4(%0); swc2 $21, 16(%0); swc2 $22, 28(%0)" : : "r"(r_a2) : "memory");

                                r_v0 = t0[0x18 / 4];
                                r_a0 = t0[0x14 / 4];
                                r_v1 = *(signed char *)(t4 - 1);
                                r_a1 = *(unsigned short *)(t4 + 0);
                                /* ASM_BARRIER: load otz, ot base, offset, and flags before shifts */
                                __asm__ volatile("" : "+r"(r_v0), "+r"(r_a0), "+r"(r_v1), "+r"(r_a1));
                                r_v0 <<= 2;
                                r_a0 += r_v0;
                                r_v1 <<= 2;
                                r_v0 = r_a1 & 3;
                                /* ASM_BARRIER: compute r_a1 & 3 before adding r_v1 in bnez delay slot */
                                __asm__ volatile("" : "+r"(r_v0), "+r"(r_a0), "+r"(r_v1));
                                r_a0 += r_v1;
                                if (r_v0 == 0) {
                                    r_v0 = (int)((char *)t0 + 0x48);
                                    RR_GTE14_LWC2_V0(r_v0);
                                    RR_GTE14_NOP2();
                                    RR_GTE14_LIGHT0();
                                    r_v0 = (unsigned int)r_a1 >> 1;
                                    /* ASM_BARRIER: shift r_a1 >> 1 into $v0 before storing -0x61(a3) */
                                    __asm__ volatile("" : "+r"(r_v0), "+r"(r_a0));
                                    *(char *)(r_a3 - 0x61) = r_v0;
                                    r_v0 = *(int *)r_a0;
                                    /* ASM_BARRIER: load-use delay on *(int *)r_a0 */
                                    __asm__ volatile("" : "+r"(r_v0));
                                    *(int *)(r_a3 - 0x68) = r_v0;
                                    r_v0 = 0xC;
                                    /* ASM_BARRIER: keep 0xC in $v0 before storing -0x65(a3) */
                                    __asm__ volatile("" : "+r"(r_v0));
                                    *(char *)(r_a3 - 0x65) = r_v0;
                                    r_v0 = (int)(t3 + 0x2C);
                                    /* ASM_BARRIER: compute t3+0x2C before OT stores */
                                    __asm__ volatile("" : "+r"(r_v0));
                                    *(int *)r_a0 = r_a2;
                                    *(char *)(r_a0 + 3) = 0;
                                    RR_GTE14_SWC2_RGB2(r_v0);
                                    r_a3 += 0x38;
                                    t3 += 0x38;
                                } else {
                                    /* ASM_BARRIER: prevent delay slot filling into bnez */
                                    __asm__ volatile("" : "+r"(r_a0));
                                    *(int *)(r_a3 - 0x68) = s1;
                                    r_v0 = *(unsigned short *)(t4 + 0);
                                    /* ASM_BARRIER: load-use delay on *(unsigned short *)t4 */
                                    __asm__ volatile("" : "+r"(r_v0));
                                    r_v0 &= 3;
                                    r_v0 = s1 | r_v0;
                                    *(int *)(r_a3 - 0x30) = r_v0;
                                    r_v0 = (int)(t3 + 0x3C);
                                    /* ASM_BARRIER: store RGB0..RGB2 into second POLY_GT4 packet */
                                    __asm__ volatile("swc2 $20, 4(%0); swc2 $21, 16(%0); swc2 $22, 28(%0)" : : "r"(r_v0) : "memory");
                                    r_v0 = 0x3E;
                                    /* ASM_BARRIER: keep 0x3E in $v0 before storing -0x29(a3) */
                                    __asm__ volatile("" : "+r"(r_v0));
                                    *(char *)(r_a3 - 0x29) = r_v0;
                                    RR_GTE_OP(0x4A486012);
                                    r_v0 = *(int *)r_a0;
                                    /* ASM_BARRIER: load-use delay on *(int *)r_a0 */
                                    __asm__ volatile("" : "+r"(r_v0));
                                    r_v1 = *(unsigned char *)(r_v0 + 4);
                                    r_v0 = 0xE6;
                                    /* ASM_BARRIER: load 0xE6 in load delay slot before beq */
                                    __asm__ volatile("" : "+r"(r_v1), "+r"(r_v0));
                                    if (r_v1 != r_v0) {
                                        /* ASM_BARRIER: prevent delay slot filling into beq */
                                        __asm__ volatile("" : "+r"(r_a0));
                                        *(int *)(r_a3 + 0x08) = s4;
                                        r_v0 = *(int *)r_a0;
                                        /* ASM_BARRIER: load-use delay on *(int *)r_a0 */
                                        __asm__ volatile("" : "+r"(r_v0));
                                        *(int *)(r_a3 + 0x04) = r_v0;
                                        r_v0 = 1;
                                        /* ASM_BARRIER: keep 1 in $v0 before storing 0x7(a3) */
                                        __asm__ volatile("" : "+r"(r_v0));
                                        *(char *)(r_a3 + 0x07) = r_v0;
                                        r_v0 = (int)(t3 + 0x70);
                                        /* ASM_BARRIER: compute t3+0x70 before OT stores */
                                        __asm__ volatile("" : "+r"(r_v0));
                                        *(int *)r_a0 = r_v0;
                                        *(char *)(r_a0 + 3) = 0;
                                    }
                                    /* ASM_BARRIER: join at .L8001492C */
                                    __asm__ volatile("" : "+r"(r_a0));
                                    r_v0 = *(int *)r_a0;
                                    /* ASM_BARRIER: load-use delay on *(int *)r_a0 */
                                    __asm__ volatile("" : "+r"(r_v0));
                                    *(int *)(r_a3 - 0x34) = r_v0;
                                    r_v0 = (int)(t3 + 0x38);
                                    /* ASM_BARRIER: compute t3+0x38 before sb and OT stores */
                                    __asm__ volatile("" : "+r"(r_v0));
                                    *(char *)(r_a3 - 0x31) = s2;
                                    *(int *)r_a0 = r_v0;
                                    *(char *)(r_a0 + 3) = 0;
                                    /* ASM_BARRIER: order first OT link before second OT link */
                                    __asm__ volatile("" : "+r"(r_a0) : : "memory");
                                    r_v0 = *(int *)r_a0;
                                    /* ASM_BARRIER: load-use delay on *(int *)r_a0 */
                                    __asm__ volatile("" : "+r"(r_v0));
                                    *(int *)t3 = r_v0;
                                    *(char *)(r_a3 - 0x69) = s2;
                                    *(int *)r_a0 = (int)t3;
                                    *(char *)(r_a0 + 3) = 0;

                                    /* ASM_BARRIER: read IR1 and IR2 from GTE $9..$10 */
                                    __asm__ volatile("mfc2 %0, $9; mfc2 %1, $10" : "=r"(t1), "=r"(t2));
                                    r_v0 = t1 >> 6;
                                    t1 = r_v0 + 0xC0;
                                    r_v0 = t2 >> t8;
                                    t2 = r_v0 + 0xC0;
                                    r_v0 = t2 << 8;
                                    r_v1 = t0[0x0C / 4];
                                    /* ASM_BARRIER: load t0[3] before ORing t1 and v1 */
                                    __asm__ volatile("" : "+r"(r_v0), "+r"(r_v1), "+r"(t1));
                                    r_v0 |= t1;
                                    r_v0 |= r_v1;
                                    *(int *)(r_a3 - 0x24) = r_v0;

                                    r_v0 = (int)((char *)t0 + 0x48);
                                    RR_GTE14_LWC2_V0(r_v0);
                                    RR_GTE14_NOP2();
                                    RR_GTE14_LIGHT0();
                                    r_v0 = (int)(t3 + 0x2C);
                                    RR_GTE14_SWC2_RGB2(r_v0);
                                    r_v0 = (int)(t3 + 0x64);
                                    RR_GTE14_SWC2_RGB2(r_v0);
                                    RR_NOP_DEP();
                                    RR_GTE_OP(0x4A48E012);
                                    /* ASM_BARRIER: read IR1 and IR2 from GTE $9..$10 */
                                    __asm__ volatile("mfc2 %0, $9; mfc2 %1, $10" : "=r"(t1), "=r"(t2));
                                    RR_NOP_DEP();
                                    RR_GTE_OP(0x4A496012);

                                    r_v0 = t1 >> 6;
                                    t1 = r_v0 + 0xC0;
                                    r_v0 = t2 >> t8;
                                    t2 = r_v0 + 0xC0;
                                    r_v0 = t2 << 8;
                                    r_v1 = t0[0x10 / 4];
                                    /* ASM_BARRIER: load t0[4] before ORing t1 and v1 */
                                    __asm__ volatile("" : "+r"(r_v0), "+r"(r_v1), "+r"(t1));
                                    r_v0 |= t1;
                                    r_v0 |= r_v1;
                                    *(int *)(r_a3 - 0x18) = r_v0;

                                    /* ASM_BARRIER: read IR1 and IR2 from GTE $9..$10 */
                                    __asm__ volatile("mfc2 %0, $9; mfc2 %1, $10" : "=r"(t1), "=r"(t2));
                                    r_v0 = t1 >> 6;
                                    t1 = r_v0 + 0xC0;
                                    r_v0 = t2 >> t8;
                                    t2 = r_v0 + 0xC0;
                                    /* ASM_BARRIER: compute t1 and t2 before MVMVA V0 */
                                    __asm__ volatile("" : "+r"(t1), "+r"(t2));
                                    RR_GTE_OP(0x4A486012);
                                    r_v0 = (t2 << 8) | t1;
                                    *(int *)(r_a3 - 0x0C) = r_v0;

                                    /* ASM_BARRIER: read IR1 and IR2 from GTE $9..$10 */
                                    __asm__ volatile("mfc2 %0, $9; mfc2 %1, $10" : "=r"(t1), "=r"(t2));
                                    r_v0 = t1 >> 6;
                                    t1 = r_v0 + 0xC0;
                                    r_v0 = t2 >> t8;
                                    t2 = r_v0 + 0xC0;
                                    /* ASM_BARRIER: compute t1 and t2 before final UV store */
                                    __asm__ volatile("" : "+r"(t1), "+r"(t2));
                                    r_v0 = (t2 << 8) | t1;
                                    *(int *)(r_a3 + 0x00) = r_v0;
                                    r_a3 += 0x78;
                                    t3 += 0x78;
                                }
                            }
                        }
                    }
                }
            }
            t4 += 0x20;
            s0 += 0x20;
        } while (t9-- > 0);
    }
    *(char **)SCRATCHPAD = t3;
    return s0;
}

void SetModelTexturePage(s32 arg0) {
    ModelTexturePage = arg0;
}

int DrawRroModel(int a0, int a1, int a2, int a3)
{
  int *s0 = (int *) (SCRATCHPAD + 0x7C);
  int *p;
  int *g;
  int w;
  unsigned new_var3;
  unsigned new_var2;
  int t;
  /* ASM_BARRIER: pin a to $5 to match original register allocation */
  register unsigned a __asm__("$5");
  unsigned long new_var;
  unsigned b;
  if (ModelTexturePage < 0)
  {
    return 0;
  }
  SetGteObjectTransform((SCRATCHPAD + 0x7C), a0, a1);
  new_var = 3;
  s0[2] = a3;
  a = ModelTexturePage & 0xF;
  a <<= 6;
  b = ModelTexturePage & 0x10;
  b <<= 4;
  s0[3] = (((b + 0x7F) << 6) | ((a >> 4) | new_var)) << 16;
  new_var3 = b >> 4;
  new_var2 = (a >> 6) | 0x20;
  s0[4] = (new_var3 | new_var2) << 16;
  g = (int *) SCRATCHPAD;
  s0[0] = g[0x54 / 4];
  s0[1] = g[0x58 / 4];
  s0[5] = g[1] + 0x200;
  p = *((int **) ((a2 * 4) + g[0x50 / 4]));
  while ((w = *(p++)) != 0)
  {
    t = w >> 16;
    w &= 0xFFFF;
    switch (w)
    {
      case 0:
        p = (int *) (((char *) p) + (t * 0x10));
        break;

      case 1:
        p = (int *) (((char *) p) + (t * 0x18));
        break;

      case 2:
        p = (int *) EmitRroLitQuadsG4(p, s0, t);
        break;

      case 3:
        asm volatile("mtc2 %0, $6" : : "r"(0x3C808080));
        p = (int *) EmitRroEnvMapQuads(p, s0, t);
        break;

      default:
        return DebugPrintStub(&UnknownRroTypeFmt, w, t);

    }

  }

  return 1;
}

void DrawSparkLines(char *a0_in, char *a1_in, s32 a2_in) {
    s32 sp[6];
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 v0 __asm__("$2");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 v1 __asm__("$3");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 a0 __asm__("$4");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 a1 __asm__("$5");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 a2 __asm__("$6");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 a3 __asm__("$7");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 s0 __asm__("$16");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 s1 __asm__("$17");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *s2 __asm__("$18");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 s3 __asm__("$19");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *s4 __asm__("$20");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *s5 __asm__("$21");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *s6 __asm__("$22");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *s7 __asm__("$23");
    /* ASM_BARRIER: PsyQ-style register allocation */
    s32 fp;

    s7 = a0_in;
    s5 = a1_in;
    s0 = a2_in;
    v0 = (s32)(s5 + 0x174);
    /* ASM_BARRIER: load far color from s5+0x174 into GTE FC registers */
    __asm__ volatile(
        "lbu $12, 0(%0); lbu $13, 1(%0); lbu $14, 2(%0); sll $12, $12, 4; sll $13, $13, 4; sll $14, $14, 4; ctc2 $12, $21; ctc2 $13, $22; ctc2 $14, $23"
        : : "r"(v0), "r"(s0) : "$12", "$13", "$14");

    a3 = 0x42000000;
    /* ASM_BARRIER: load RGB code into GTE $6 */
    __asm__ volatile("mtc2 %0, $6" : : "r"(a3));

    a3 = 0xE1000220;
    fp = 0x10;
    v0 = SCRATCHPAD;
    /* ASM_BARRIER: order setup before sp[2] store and scratchpad loads */
    __asm__ volatile("" : "+r"(a3), "+r"(fp), "+r"(v0));
    sp[2] = a3;
    s4 = *(char **)(v0 + 0x0);
    v1 = *(s32 *)(v0 + 0x4);
    v0 = *(s32 *)(v0 + 0x60);
    /* ASM_BARRIER: order 3 scratchpad loads before additions */
    __asm__ volatile("" : "+r"(s4), "+r"(v1), "+r"(v0));
    v1 += 0x1B0;
    v0 += 2;
    /* ASM_BARRIER: order additions before sp[0]/sp[1] stores and jal */
    __asm__ volatile("" : "+r"(v1), "+r"(v0));
    sp[0] = v1;
    sp[1] = v0;
    v0 = PsyqRand() & 3;
    s0 = -(s0 + 0xA0);
    a3 = (s32)(s5 + 0xE0);
    s2 = s7 + 8;
    s6 = s4 + 4;
    /* ASM_BARRIER: order post-call setup before sp[3..5] stores */
    __asm__ volatile("" : "+r"(v0), "+r"(s0), "+r"(a3), "+r"(s2), "+r"(s6));
    sp[3] = v0;
    sp[4] = s0;
    sp[5] = a3;
    /* ASM_BARRIER: keep sp[3..5] stores before loop entry */
    __asm__ volatile("" : "+r"(s5) : : "memory");

    do {
        v0 = *(s32 *)s7;
        /* ASM_BARRIER: load-use delay on *(s32 *)s7 */
        __asm__ volatile("" : "+r"(v0));
        s3 = v0 - 0xAA;
        if (s3 <= 0) {
            /* ASM_BARRIER: prevent delay-slot filling into bgtz */
            __asm__ volatile("" : "+r"(s3));
            a3 = sp[3];
            /* ASM_BARRIER: load-use delay on sp[3] */
            __asm__ volatile("" : "+r"(a3));
            if (a3 != 0) {
                goto L_next;
            }
            /* ASM_BARRIER: prevent a3 = 1 from filling bnez delay slot */
            __asm__ volatile("" : "+r"(a3));
            a3 = 1;
            sp[3] = a3;
            s3 = 0x1000;
            v0 = PsyqRand() & 0x1FF;
            s0 = v0 - 0x100;
            v0 = PsyqRand() & 0x1FF;
            s1 = v0 - 0x100;
            if (s0 == 0) {
                /* ASM_BARRIER: keep s1 assignment in bnez delay slot */
                __asm__ volatile("" : "+r"(s1));
                s0 = 0x100;
            }
            /* ASM_BARRIER: join after s0 check */
            __asm__ volatile("" : "+r"(s0), "+r"(s1));
            if (s1 == 0) {
                s1 = 0x100;
            }
            v0 = s1 + 0x140;
            /* ASM_BARRIER: order v0 before stores and jal */
            __asm__ volatile("" : "+r"(v0), "+r"(s0), "+r"(s1));
            *(s16 *)(s2 - 4) = s0;
            *(s16 *)(s2 - 2) = v0;
            v0 = (PsyqRand() & 0x7F) - 0xC30;
            *(s16 *)(s2 + 0) = v0;
            v0 = s0 / 2;
            *(s16 *)(s2 + 4) = v0;
            v0 = s1 / 2;
            *(s16 *)(s2 + 6) = v0;
            a3 = *(u16 *)&sp[4];
            /* ASM_BARRIER: load-use delay on sp[4] */
            __asm__ volatile("" : "+r"(a3));
            *(s16 *)(s2 + 2) = a3;
        }
        /* ASM_BARRIER: join at L80014DA0 */
        __asm__ volatile("" : "+r"(s3));
        *(s32 *)s7 = s3;
        RR_GTE_MTC2_IR0(s3);
        RR_GTE14_NOP2();
        RR_GTE_OP(0x4A780010);
        RR_GTE14_SWC2_RGB2(s4 + 8);

        s0 = *(s32 *)(s2 - 4);
        a2 = *(s32 *)(s2 + 0);
        a0 = *(s32 *)(s2 + 4);
        /* ASM_BARRIER: order 3 word loads from s2 */
        __asm__ volatile("" : "+r"(s0), "+r"(a2), "+r"(a0));
        s1 = s0 >> 16;
        v0 = s0 << 16;
        /* ASM_BARRIER: order s1 and v0 before s0 sign-extend */
        __asm__ volatile("" : "+r"(s1), "+r"(v0));
        s0 = v0 >> 16;
        a1 = a2 >> 16;
        v1 = a2 << 16;
        /* ASM_BARRIER: order s0, a1, v1 before a2 sign-extend */
        __asm__ volatile("" : "+r"(s0), "+r"(a1), "+r"(v1));
        a2 = v1 >> 16;
        v0 >>= 20;
        /* ASM_BARRIER: order a2 and v0 before sh E0 */
        __asm__ volatile("" : "+r"(a2), "+r"(v0));
        *(s16 *)(s5 + 0xE0) = v0;
        v0 = s1 >> 4;
        v1 >>= 20;
        /* ASM_BARRIER: order v0 and v1 before sh E4 */
        __asm__ volatile("" : "+r"(v0), "+r"(v1));
        *(s16 *)(s5 + 0xE4) = v1;
        v1 = a0 >> 16;
        a0 <<= 16;
        /* ASM_BARRIER: order v1 and a0 before sh E2 */
        __asm__ volatile("" : "+r"(v1), "+r"(a0), "+r"(v0));
        *(s16 *)(s5 + 0xE2) = v0;

        a3 = sp[5];
        /* ASM_BARRIER: load-use delay on sp[5] */
        __asm__ volatile("" : "+r"(a3));
        /* ASM_BARRIER: load V1 from a3 */
        __asm__ volatile("lwc2 $2, 0(%0); lwc2 $3, 4(%0)" : : "r"(a3) : "memory");

        v0 = (s0 + (a0 >> 19)) >> 4;
        *(s16 *)(s5 + 0xE0) = v0;
        v0 = (s1 + (v1 >> 3)) >> 4;
        *(s16 *)(s5 + 0xE2) = v0;
        v0 = (a2 + (a1 >> 3)) >> 4;
        *(s16 *)(s5 + 0xE4) = v0;

        /* ASM_BARRIER: load V2 from a3 and run RTPT */
        __asm__ volatile("lwc2 $4, 0(%0); lwc2 $5, 4(%0); nop; nop; .word 0x4A280030" : : "r"(a3) : "memory");

        a0 = s0 + (a0 >> 20);
        v1 = s1 + (v1 >> 4);
        a1 = a2 + (a1 >> 4);
        v0 = (s32)(s5 + 0x70);
        /* ASM_BARRIER: order additions and v0 address before stores */
        __asm__ volatile("" : "+r"(a0), "+r"(v1), "+r"(a1), "+r"(v0));
        *(s16 *)(s2 - 4) = a0;
        *(s16 *)(s2 - 2) = v1;
        *(s16 *)(s2 + 0) = a1;
        /* ASM_BARRIER: store SZ3 to s5+0x70 */
        __asm__ volatile("swc2 $19, 0(%0)" : : "r"(v0) : "memory");

        v0 = *(s32 *)(s5 + 0x70);
        /* ASM_BARRIER: load-use delay on s5+0x70 */
        __asm__ volatile("" : "+r"(v0));
        if (v0 > 0) {
            /* ASM_BARRIER: prevent delay-slot filling into blez */
            __asm__ volatile("" : "+r"(v0));
            a3 = sp[1];
            /* ASM_BARRIER: load-use delay on sp[1] */
            __asm__ volatile("" : "+r"(a3));
            v1 = v0 >> a3;
            if (v1 < 0x1C0) {
                v0 = (s32)(s4 + 0xC);
                /* ASM_BARRIER: store SXY1 to s4+0xC */
                __asm__ volatile("swc2 $13, 0(%0)" : : "r"(v0) : "memory");
                v0 = (s32)(s4 + 0x10);
                /* ASM_BARRIER: store SXY2 to s4+0x10 */
                __asm__ volatile("swc2 $14, 0(%0)" : : "r"(v0) : "memory");

                a3 = sp[0];
                /* ASM_BARRIER: order sp[0] load before v1 shift to fill load delay slot */
                __asm__ volatile("" : "+r"(a3), "+r"(v1));
                v1 <<= 2;
                /* ASM_BARRIER: order v1 shift before addition */
                __asm__ volatile("" : "+r"(a3), "+r"(v1));
                v1 += a3;
                v0 = *(s32 *)v1;
                /* ASM_BARRIER: load-use delay on OT entry */
                __asm__ volatile("" : "+r"(v0), "+r"(v1));
                *(s32 *)s4 = v0;
                *(u8 *)(s6 - 1) = 4;
                *(char **)v1 = s4;
                *(u8 *)((char *)v1 + 3) = 0;
                /* ASM_BARRIER: order byte store before sp[2] load */
                __asm__ volatile("" : : : "memory");

                a3 = sp[2];
                /* ASM_BARRIER: order sp[2] load before s4 increment in its load delay slot */
                __asm__ volatile("" : "+r"(a3), "+r"(s4));
                s4 += 0x14;
                /* ASM_BARRIER: order s4 increment before s6 store */
                __asm__ volatile("" : "+r"(a3), "+r"(s4));
                *(s32 *)s6 = a3;
                s6 += 0x14;
                /* ASM_BARRIER: keep s6 increment inside conditional block */
                __asm__ volatile("" : "+r"(s6));
            }
        }
L_next:
        /* ASM_BARRIER: prevent delay-slot filling from L_next into bnez/blez/beqz */
        __asm__ volatile("" : "+r"(s2));
        s2 += 0x10;
        fp--;
        /* ASM_BARRIER: order s2 and fp updates before bgtz */
        __asm__ volatile("" : "+r"(s2), "+r"(fp));
        s7 += 0x10;
    } while (fp > 0);

    *(char **)SCRATCHPAD = s4;
}

void func_80014F34(s32 *a0_in, char *a1_in) {
    /* ASM_BARRIER: keep a3 in $7 */
    register char *a3 __asm__("$7") = a1_in;
    /* ASM_BARRIER: keep a0 in $4 */
    register char *a0 __asm__("$4");
    /* ASM_BARRIER: keep t0 in $8 */
    register char *t0 __asm__("$8");
    /* ASM_BARRIER: keep t1 in $9 */
    register s32 *t1 __asm__("$9");
    /* ASM_BARRIER: keep v0 in $2 */
    register s32 v0 __asm__("$2");
    /* ASM_BARRIER: keep v1 in $3 */
    register s32 v1 __asm__("$3");
    /* ASM_BARRIER: keep a1 in $5 */
    register s32 a1 __asm__("$5");
    /* ASM_BARRIER: keep a2 in $6 */
    register s32 a2 __asm__("$6");

    v0 = a0_in[2];
    a0 = (char *)a0_in[0];
    /* ASM_BARRIER: order loads from a0_in before t0 addition */
    __asm__ volatile("" : "+r"(v0), "+r"(a0), "+r"(a3));
    t0 = (char *)v0 + 4;
    a1 = *(u16 *)((char *)v0 + 4);
    v1 = *(u16 *)(t0 + 2);
    v0 = *(u16 *)(t0 + 4);
    /* ASM_BARRIER: order 3 index loads before shifts */
    __asm__ volatile("" : "+r"(a1), "+r"(v1), "+r"(v0), "+r"(t0));
    a1 = (s32)(a0 + (a1 << 3));
    v1 = (s32)(a0 + (v1 << 3));
    v0 = (s32)(a0 + (v0 << 3));
    RR_GTE_RTPT_LDV(a1, v1, v0);
    RR_GTE14_NOP2();
    RR_GTE14_NCLIP();
    RR_GTE14_SWC2_SXYZ(a3 + 0x1A0);
    RR_GTE14_SWC2_MAC0(a3 + 0x74);

    v0 = *(u16 *)(t0 + 6);
    /* ASM_BARRIER: load-use delay before shift */
    __asm__ volatile("" : "+r"(v0));
    v0 <<= 3;
    a0 += v0;
    RR_GTE_RTPS_LDV(a0);
    RR_GTE14_AVSZ4();
    RR_GTE14_NCLIP();
    RR_GTE14_SWC2_SXY2(a3 + 0x1AC);
    RR_GTE14_SWC2_MAC0(a3 + 0x78);
    RR_GTE14_SWC2_OTZ(a3 + 0x70);

    v1 = *(s32 *)(a3 + 0x70);
    t1 = (s32 *)SCRATCHPAD;
    if (v1 <= 0) {
        return;
    }
    /* ASM_BARRIER: place lui $t1, 0x1F80 in blez delay slot */
    __asm__ volatile("" : "+r"(t1), "+r"(v1));
    v0 = t1[0x60 / 4];
    /* ASM_BARRIER: load-use delay on shift count */
    __asm__ volatile("" : "+r"(v0));
    a1 = v1 >> v0;
    *(s32 *)(a3 + 0x70) = a1;
    if (a1 >= 0x1C0) {
        return;
    }

    v0 = *(s32 *)(a3 + 0x74);
    v1 = *(s32 *)(a3 + 0xBC);
    a2 = *(s32 *)(a3 + 0x78);
    a0 = (char *)a2;
    /* ASM_BARRIER: order 3 loads before xor instructions */
    __asm__ volatile("" : "+r"(v0), "+r"(v1), "+r"(a0));
    v0 ^= v1;
    a2 = (s32)a0 ^ v1;
    /* ASM_BARRIER: keep second xor in $a0 for bgtz delay slot */
    a0 = (char *)a2;
    if (v0 <= 0 && (s32)a0 >= 0) {
        return;
    }

    v0 = a1 << 2;
    v1 = *(s8 *)(t0 + 0x19);
    a0 = (char *)t1[1];
    /* ASM_BARRIER: order lb and lw before sll v1 */
    __asm__ volatile("" : "+r"(v0), "+r"(v1), "+r"(a0));
    v1 <<= 2;
    v0 += (s32)a0;
    v1 += v0;
    a0 = (char *)t1[0];
    v0 = *(s32 *)((char *)v1 + 0x200);
    /* ASM_BARRIER: order lw a0 and lw v0 before sw v0 */
    __asm__ volatile("" : "+r"(a0), "+r"(v0));
    *(s32 *)a0 = v0;
    *(u8 *)(a0 + 3) = 9;

    v1 = *(s8 *)(t0 + 0x19);
    v0 = *(s32 *)(a3 + 0x70);
    a1 = t1[1];
    /* ASM_BARRIER: order reloads before shifts */
    __asm__ volatile("" : "+r"(v1), "+r"(v0), "+r"(a1));
    v1 <<= 2;
    v0 <<= 2;
    v0 += a1;
    v1 += v0;
    *(char **)((char *)v1 + 0x200) = a0;

    v1 = *(s8 *)(t0 + 0x19);
    v0 = *(s32 *)(a3 + 0x70);
    a1 = t1[1];
    /* ASM_BARRIER: order reloads before shifts */
    __asm__ volatile("" : "+r"(v1), "+r"(v0), "+r"(a1));
    v1 <<= 2;
    v0 <<= 2;
    v0 += a1;
    v1 += v0;
    *(u8 *)((char *)v1 + 0x203) = 0;

    a1 = *(s32 *)(a3 + 0x1A0);
    a2 = *(s32 *)(a3 + 0x1A4);
    v1 = *(s32 *)(a3 + 0x1A8);
    v0 = *(s32 *)(a3 + 0x1AC);
    /* ASM_BARRIER: order 4 SXY loads before 4 packet stores */
    __asm__ volatile("" : "+r"(a1), "+r"(a2), "+r"(v1), "+r"(v0));
    *(s32 *)(a0 + 0x8) = a1;
    *(s32 *)(a0 + 0x10) = a2;
    *(s32 *)(a0 + 0x18) = v1;
    *(s32 *)(a0 + 0x20) = v0;

    v0 = *(s32 *)(a3 + 0x170);
    /* ASM_BARRIER: load-use delay on 0x170(a3) */
    __asm__ volatile("" : "+r"(v0));
    *(s32 *)(a0 + 0x4) = v0;
    *(u8 *)(a0 + 0x7) = 0x2E;

    a1 = *(s32 *)(t0 + 0xC);
    v1 = *(s32 *)(t0 + 0x14);
    a2 = *(s32 *)(t0 + 0x10);
    /* ASM_BARRIER: order 3 UV/clut loads before sra */
    __asm__ volatile("" : "+r"(a1), "+r"(v1), "+r"(a2));
    v0 = v1 >> 16;
    /* ASM_BARRIER: order sra before 4 stores */
    __asm__ volatile("" : "+r"(v0));
    *(s32 *)(a0 + 0xC) = a1;
    *(s32 *)(a0 + 0x14) = a2;
    *(s32 *)(a0 + 0x1C) = v1;
    *(s32 *)(a0 + 0x24) = v0;
    a0 += 0x28;
    t1[0] = (s32)a0;
}

void func_80015110(s32 *a0_in, char *a1_in) {
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 v0 __asm__("$2");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 v1 __asm__("$3");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 *a0 __asm__("$4") = a0_in;
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *a1 __asm__("$5") = a1_in;
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *a2 __asm__("$6");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 a3 __asm__("$7");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *t0 __asm__("$8");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *t1 __asm__("$9");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 t2 __asm__("$10");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *t3 __asm__("$11");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *t4 __asm__("$12");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 t5 __asm__("$13");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 t6 __asm__("$14");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 t7 __asm__("$15");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 t8 __asm__("$24");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 t9 __asm__("$25");

    a2 = (char *)a0[0];
    /* ASM_BARRIER: schedule lw a2, 0(a0) before addu t0, a1, zero */
    __asm__ volatile("" : "+r"(a2), "+r"(a1));
    t0 = a1;
    /* ASM_BARRIER: order a2 and t0 initialization */
    __asm__ volatile("" : "+r"(a2), "+r"(t0));

    v0 = *(s32 *)(a2 + 0x0);
    v1 = *(s32 *)(a2 + 0x4);
    /* ASM_BARRIER: order first pair load before t1 setup */
    __asm__ volatile("" : "+r"(v0), "+r"(v1), "+r"(t0));
    t1 = t0 + 0x188;
    /* ASM_BARRIER: order t1 setup before stores */
    __asm__ volatile("" : "+r"(v0), "+r"(v1), "+r"(t1));
    *(s32 *)(t0 + 0xE0) = v0;
    *(s32 *)(t0 + 0xE4) = v1;
    /* ASM_BARRIER: order first pair stores before second pair load */
    __asm__ volatile("" : "+r"(a2), "+r"(t0) : "r"(v0), "r"(v1) : "memory");

    v0 = *(s32 *)(a2 + 0x8);
    v1 = *(s32 *)(a2 + 0xC);
    /* ASM_BARRIER: order second pair load before t2 setup */
    __asm__ volatile("" : "+r"(v0), "+r"(v1), "+r"(t0));
    t2 = (s32)(t0 + 0xE0);
    /* ASM_BARRIER: order t2 setup before stores */
    __asm__ volatile("" : "+r"(v0), "+r"(v1), "+r"(t2));
    *(s32 *)(t0 + 0xE8) = v0;
    *(s32 *)(t0 + 0xEC) = v1;
    /* ASM_BARRIER: order second pair stores before third pair load */
    __asm__ volatile("" : "+r"(a2), "+r"(t0) : "r"(v0), "r"(v1) : "memory");

    v0 = *(s32 *)(a2 + 0x10);
    v1 = *(s32 *)(a2 + 0x14);
    /* ASM_BARRIER: order third pair load before a1 setup */
    __asm__ volatile("" : "+r"(v0), "+r"(v1), "+r"(a2));
    a1 = a2 + 0x14;
    /* ASM_BARRIER: order a1 setup before stores */
    __asm__ volatile("" : "+r"(v0), "+r"(v1), "+r"(a1));
    *(s32 *)(t0 + 0xF0) = v0;
    *(s32 *)(t0 + 0xF4) = v1;

    a3 = a0[1];
    /* ASM_BARRIER: order a3 load before loop entry */
    __asm__ volatile("" : "+r"(a3));

    do {
        RR_GTE14_LWC2_V012((char *)t2);
        a1 += 0x18;
        a2 += 0x18;
        a3 -= 3;
        t1 += 0x18;
        /* ASM_BARRIER: order pointer increments before RTPT */
        __asm__ volatile("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(t1));
        RR_GTE14_NOP2();
        RR_GTE14_RTPT();

        v0 = *(s32 *)(a2 + 0x0);
        v1 = *(s32 *)(a1 - 0x10);
        /* ASM_BARRIER: order pair 1 load before store */
        __asm__ volatile("" : "+r"(v0), "+r"(v1));
        *(s32 *)(t0 + 0xE0) = v0;
        *(s32 *)(t0 + 0xE4) = v1;

        v0 = *(s32 *)(a1 - 0xC);
        v1 = *(s32 *)(a1 - 0x8);
        /* ASM_BARRIER: order pair 2 load before store */
        __asm__ volatile("" : "+r"(v0), "+r"(v1));
        *(s32 *)(t0 + 0xE8) = v0;
        *(s32 *)(t0 + 0xEC) = v1;

        v0 = *(s32 *)(a1 - 0x4);
        v1 = *(s32 *)(a1 + 0x0);
        /* ASM_BARRIER: order pair 3 load before store */
        __asm__ volatile("" : "+r"(v0), "+r"(v1));
        *(s32 *)(t0 + 0xF0) = v0;
        *(s32 *)(t0 + 0xF4) = v1;

        /* ASM_BARRIER: store SXY0..SXY2 and SZ1..SZ3 from GTE */
        __asm__ volatile("swc2 $12, 0(%0); swc2 $17, 4(%0); swc2 $13, 8(%0); swc2 $18, 12(%0); swc2 $14, 16(%0); swc2 $19, 20(%0)" : : "r"(t1) : "memory");
    } while (a3 > 0);

    t6 = 0xE1000220;
    /* ASM_BARRIER: keep t6 initialization immediately after loop so lui fills bgtz delay slot */
    __asm__ volatile("" : "+r"(t6));

    v0 = a0[2];
    a0 = (s32 *)SCRATCHPAD;
    /* ASM_BARRIER: order v0 load and a0 lui before t3/t2 setup */
    __asm__ volatile("" : "+r"(v0), "+r"(a0));
    t3 = (char *)v0 + 4;
    t2 = *(u16 *)((char *)v0 + 2);
    /* ASM_BARRIER: order t3 and t2 before (SCRATCHPAD + 0x60) load */
    __asm__ volatile("" : "+r"(t3), "+r"(t2), "+r"(a0));
    v0 = a0[0x60 / 4];
    v1 = *(s32 *)(t0 + 0xA0);
    /* ASM_BARRIER: order v0 and v1 loads before blez */
    __asm__ volatile("" : "+r"(v0), "+r"(v1));
    t5 = v0 + 4;
    if (v1 > 0) {
        /* ASM_BARRIER: keep t5 in blez delay slot */
        __asm__ volatile("" : "+r"(t5), "+r"(a0));
        v0 = a0[1];
        t4 = (char *)v0 + 0x1C4;
    } else {
        /* ASM_BARRIER: keep t5 in blez delay slot */
        __asm__ volatile("" : "+r"(t5), "+r"(a0));
        v0 = a0[1];
        /* ASM_BARRIER: load-use delay on a0[1] in else branch */
        __asm__ volatile("" : "+r"(v0));
        t4 = (char *)v0 + 0x1B0;
    }
    /* ASM_BARRIER: join after t4 selection */
    __asm__ volatile("" : "+r"(t4), "+r"(t5));

    t1 = (char *)SCRATCHPAD;
    /* ASM_BARRIER: force lui $t1, 0x1F80 */
    __asm__ volatile("" : "+r"(t1));
    t1 = *(char **)t1;
    /* ASM_BARRIER: order t1 load before loop count check */
    __asm__ volatile("" : "+r"(t1), "+r"(t2));

    v0 = t2;
    t2--;
    if (v0 > 0) {
        /* ASM_BARRIER: keep t2-- in blez delay slot */
        __asm__ volatile("" : "+r"(t2), "+r"(t3), "+r"(t1));
        a3 = (s32)(t3 + 6);
        a2 = t1 + 4;
        /* ASM_BARRIER: order loop pointer setup before loop body */
        __asm__ volatile("" : "+r"(a3), "+r"(a2));
        do {
            v1 = *(u16 *)(t3 + 0);
            v0 = *(u16 *)((char *)a3 - 4);
            a1 = (char *)(s32)*(u16 *)((char *)a3 - 2);
            /* ASM_BARRIER: order 3 vertex index loads before shifts */
            __asm__ volatile("" : "+r"(v1), "+r"(v0), "+r"(a1));
            v1 = (s32)(t0 + (v1 << 3));
            v0 = (s32)(t0 + (v0 << 3));
            a1 = t0 + ((s32)a1 << 3);
            /* ASM_BARRIER: order 3 vertex pointers before SXY loads */
            __asm__ volatile("" : "+r"(v1), "+r"(v0), "+r"(a1));
            t7 = *(s32 *)((char *)v1 + 0x1A0);
            t8 = *(s32 *)((char *)v0 + 0x1A0);
            t9 = *(s32 *)(a1 + 0x1A0);
            /* ASM_BARRIER: load SXY0..SXY2 and run NCLIP */
            __asm__ volatile("mtc2 %0, $12\n\tmtc2 %2, $14\n\tmtc2 %1, $13\n\tnop\n\tnop\n\t.word 0x4B400006" : : "r"(t7), "r"(t8), "r"(t9));

            v1 = *(s32 *)((char *)v1 + 0x1A4);
            a0 = (s32 *)*(s32 *)((char *)v0 + 0x1A4);
            v0 = *(u16 *)((char *)a3 + 0);
            /* ASM_BARRIER: order SZ1, SZ2, and index 3 loads */
            __asm__ volatile("" : "+r"(v1), "+r"(a0), "+r"(v0));
            v1 += (s32)a0;
            v0 <<= 3;
            v0 = (s32)(t0 + v0);
            /* ASM_BARRIER: order v0 pointer computation before SZ3, SZ4 loads */
            __asm__ volatile("" : "+r"(v1), "+r"(v0), "+r"(a1));
            a0 = (s32 *)*(s32 *)(a1 + 0x1A4);
            v0 = *(s32 *)((char *)v0 + 0x1A4);
            /* ASM_BARRIER: order SZ3 and SZ4 loads before additions */
            __asm__ volatile("" : "+r"(a0), "+r"(v0), "+r"(v1));
            v1 += (s32)a0;
            v1 += v0;
            if (v1 > 0) {
                v1 >>= t5;
                if (v1 < 0x1C0) {
                    RR_GTE14_SWC2_MAC0(t0 + 0x74);
                    v0 = *(s32 *)(t0 + 0x74);
                    if (v0 != 0) {
                        v1 <<= 2;
                        v1 += (s32)t4;
                        v0 = *(s32 *)v1;
                        /* ASM_BARRIER: load-use delay on OT entry */
                        __asm__ volatile("" : "+r"(v0), "+r"(v1));
                        *(s32 *)t1 = v0;
                        *(u8 *)(a2 - 1) = 9;
                        *(char **)v1 = t1;
                        *(u8 *)((char *)v1 + 3) = 0;

                        v0 = *(u16 *)((char *)a3 + 2);
                        /* ASM_BARRIER: load-use delay on color index 0 */
                        __asm__ volatile("" : "+r"(v0));
                        v0 = (s32)(t0 + (v0 << 2));
                        v0 = *(s32 *)((char *)v0 + 0x160);
                        /* ASM_BARRIER: load-use delay on color 0 */
                        __asm__ volatile("" : "+r"(v0));
                        *(s32 *)(a2 + 0x4) = v0;

                        v0 = *(u16 *)((char *)a3 + 4);
                        /* ASM_BARRIER: load-use delay on color index 1 */
                        __asm__ volatile("" : "+r"(v0));
                        v0 = (s32)(t0 + (v0 << 2));
                        v0 = *(s32 *)((char *)v0 + 0x160);
                        /* ASM_BARRIER: load-use delay on color 1 */
                        __asm__ volatile("" : "+r"(v0));
                        *(s32 *)(a2 + 0xC) = v0;

                        v0 = *(u16 *)((char *)a3 + 6);
                        /* ASM_BARRIER: load-use delay on color index 2 */
                        __asm__ volatile("" : "+r"(v0));
                        v0 = (s32)(t0 + (v0 << 2));
                        v0 = *(s32 *)((char *)v0 + 0x160);
                        /* ASM_BARRIER: load-use delay on color 2 */
                        __asm__ volatile("" : "+r"(v0));
                        *(s32 *)(a2 + 0x14) = v0;

                        v0 = *(u16 *)((char *)a3 + 8);
                        /* ASM_BARRIER: load-use delay on color index 3 */
                        __asm__ volatile("" : "+r"(v0));
                        v0 = (s32)(t0 + (v0 << 2));
                        v0 = *(s32 *)((char *)v0 + 0x160);
                        /* ASM_BARRIER: load-use delay on color 3 */
                        __asm__ volatile("" : "+r"(v0));
                        *(s32 *)(a2 + 0x1C) = v0;

                        v0 = *(u16 *)(t3 + 0);
                        /* ASM_BARRIER: load-use delay on vertex index 0 */
                        __asm__ volatile("" : "+r"(v0));
                        v0 = (s32)(t0 + (v0 << 3));
                        v0 = *(s32 *)((char *)v0 + 0x1A0);
                        /* ASM_BARRIER: load-use delay on SXY0 */
                        __asm__ volatile("" : "+r"(v0));
                        *(s32 *)(a2 + 0x8) = v0;

                        v0 = *(u16 *)((char *)a3 - 4);
                        /* ASM_BARRIER: load-use delay on vertex index 1 */
                        __asm__ volatile("" : "+r"(v0));
                        v0 = (s32)(t0 + (v0 << 3));
                        v0 = *(s32 *)((char *)v0 + 0x1A0);
                        /* ASM_BARRIER: load-use delay on SXY1 */
                        __asm__ volatile("" : "+r"(v0));
                        *(s32 *)(a2 + 0x10) = v0;

                        v0 = *(u16 *)((char *)a3 - 2);
                        /* ASM_BARRIER: load-use delay on vertex index 2 */
                        __asm__ volatile("" : "+r"(v0));
                        v0 = (s32)(t0 + (v0 << 3));
                        v0 = *(s32 *)((char *)v0 + 0x1A0);
                        /* ASM_BARRIER: load-use delay on SXY2 */
                        __asm__ volatile("" : "+r"(v0));
                        *(s32 *)(a2 + 0x18) = v0;

                        v0 = *(u16 *)((char *)a3 + 0);
                        /* ASM_BARRIER: load-use delay on vertex index 3 */
                        __asm__ volatile("" : "+r"(v0));
                        v0 = (s32)(t0 + (v0 << 3));
                        v0 = *(s32 *)((char *)v0 + 0x1A0);
                        /* ASM_BARRIER: order SXY3 load before t1 increment */
                        __asm__ volatile("" : "+r"(v0), "+r"(t1));
                        t1 += 0x28;
                        /* ASM_BARRIER: schedule t1 increment in load delay slot of SXY3 */
                        __asm__ volatile("" : "+r"(v0), "+r"(t1));
                        *(s32 *)(a2 + 0x0) = t6;
                        *(s32 *)(a2 + 0x20) = v0;
                        a2 += 0x28;
                        /* ASM_BARRIER: keep a2 increment inside conditional block */
                        __asm__ volatile("" : "+r"(a2));
                    }
                }
            }
            a3 += 0x14;
            t3 += 0x14;
            /* ASM_BARRIER: order pointer advances before loop condition */
            __asm__ volatile("" : "+r"(a3), "+r"(t3), "+r"(t2));
            v0 = t2;
            t2--;
        } while (v0 > 0);
    }
    *(char **)SCRATCHPAD = t1;
}

/* legacy typing of shared globals in this part of the unit */
#define PsyqRand ((int (*)())PsyqRand)

void func_800153FC(a0, a1) char *a0; char *a1;
{
    int s0;
    int *p;
    char *q;
    int *o;
    int n;
    int a2;
    int t;
    int v;
    RR_GTE_SETTR(a1 + 0x14);
    if (RacePaused != 0) {
        s0 = *(int *)(a0 + 0x64);
    } else {
        s0 = (*(short *)(*(int *)(a1 + 0x90) + 0x1D8) - 0x3E0) * 2;
        /* clamp s0 to [0, 0x1000]; out-of-line arm for the upper clamp reproduces block layout */
        if (s0 < 0) goto L1;
        if (s0 > 0x1000) goto E;
    L1:
        v = s0;
        if (s0 < 0) v = 0;
        goto L2;
    E:
        v = 0x1000;
    L2:
        s0 = v;
        s0 += PsyqRand() & 0xF;
        *(int *)(a0 + 0x64) = s0;
    }
    RR_GTE_MTC2_IR0(s0);
    p = D_8009E0B0;
    q = (char *)D_8009E0B0 + 0x18;
    o = (int *)(a1 + 0x15C);
    n = 6;
    do {
        RR_GTE_LWC2_RGB(p);
        RR_GTE_DPCS_FC4(q);
        p++;
        q += 4;
        o++;
        RR_GTE_SWC2_RGB2(o);
        n--;
    } while (n > 0);
    a2 = *(short *)(*(int *)(a1 + 0x90) + 0x1D8);
    DrawSparkLines(*(int *)(a0 + 0xC4), a1, a2 / 4);
    t = (ScreenFrame & 3) * 12 + 0x64;
    func_80015110(*(int *)a0 + t, a1);
    {
        int u = (PsyqRand() & 3) - 1;
        *(int *)(a1 + 0xAC) = *(int *)(a1 + 0x14) + u;
    }
    {
        int w = (PsyqRand() & 3) - 1;
        *(int *)(a1 + 0xB0) = *(int *)(a1 + 0x18) + w;
    }
    *(int *)(a1 + 0xB4) = *(int *)(a1 + 0x1C);
    RR_GTE_SETTR(a1 + 0xAC);
    func_80014F34(*(int *)a0 + 0x58, a1);
}

#undef PsyqRand

void DrawGlowFan(char *a0, char *a1) {
    int otz;
    int *p;
    int *dst;
    int i;
    int col;
    int sxy0;
    int *pkt;
    int *ot;
    int *spd;
    int e1;
    int t;

    RR_GTE_SETTR(a1 + 0x14);
    *(short *)(a1 + 0xC8) = 0;
    *(short *)(a1 + 0xCA) = -0x5C;
    *(short *)(a1 + 0xCC) = 0x98;
    RR_GTE_RTPS_LDV(a1 + 0xC8);
    RR_GTE_SWC2_SXY2(a1 + 0x1A0);
    RR_GTE_TR_FROM_IR();
    /* ASM_BARRIER: read GTE IR3 ($11) with pipeline delay nop */
    __asm__ volatile("mfc2 %0, $11\n\tnop" : "=r"(otz));
    if (otz >= 0) {
        otz >>= *(int *)(SCRATCHPAD + 0x60) + 2;
        if (otz < 0x1C0) {
            /* ASM_BARRIER: set identity GTE rotation matrix ($0..$4) and clear VXY/VZ ($1,$3,$5) */
            __asm__ volatile(
                "ctc2 %0, $0\n\t"
                "ctc2 $0, $1\n\t"
                "ctc2 %0, $2\n\t"
                "ctc2 $0, $3\n\t"
                "ctc2 %0, $4\n\t"
                "mtc2 $0, $1\n\t"
                "mtc2 $0, $3\n\t"
                "mtc2 $0, $5"
                :
                : "r"(0x1000)
            );
            p = D_8009E0E0;
            dst = (int *)(a1 + 0x198);
            i = 0x11;
            do {
                /* ASM_BARRIER: load VXY0/VXY1/VXY2 and run RTPT */
                __asm__ volatile(
                    "lwc2 $0, 0(%0)\n\t"
                    "lwc2 $2, 4(%0)\n\t"
                    "lwc2 $4, 8(%0)\n\t"
                    "nop\n\t"
                    "nop\n\t"
                    ".word 0x4A280030"
                    :
                    : "r"(p)
                );
                p += 3;
                dst += 3;
                RR_GTE_SWC2_SXYZ(dst);
                i -= 3;
            } while (i > 0);
            e1 = 0xE1000220;
            dst = (int *)(a1 + 0x1A4);
            i = 0x10;
            col = *(int *)(a0 + 8);
            spd = (int *)SCRATCHPAD;
            t = 0x32000000;
            sxy0 = *(int *)(a1 + 0x1A0);
            pkt = (int *)spd[0];
            col = (col << 8) | (col | t);
            ot = (int *)((otz << 2) + spd[1] + 0x1A4);
            do {
                pkt[3] = sxy0;
                pkt[5] = dst[0];
                pkt[7] = dst[1];
                i--;
                pkt[2] = col;
                pkt[6] = 0;
                pkt[4] = 0;
                pkt[1] = e1;
                dst++;
                pkt[0] = *ot;
                ((char *)pkt)[3] = 7;
                *ot = (int)pkt;
                pkt += 8;
                ((char *)ot)[3] = 0;
            } while (i > 0);
            *(int **)SCRATCHPAD = pkt;
        }
    }
}

int *EmitModelQuadsFT4(int *a0, int a1, char *a2) {
    int verts = (int)(a2 + 0x1A0);
    volatile int *hw = (volatile int *)SCRATCHPAD;
    int shift = hw[0x60 / 4] + 4;
    int *pkt = (int *)hw[0];
    int xor_val = *(int *)(a2 + 0xBC);

    while (a1-- > 0) {
        int v01 = a0[0];
        int v23 = a0[1];
        int i1 = v01 >> 16;
        int i0 = v01 & 0xFFFF;
        int i3 = v23 >> 16;
        int i2 = v23 & 0xFFFF;
        int *p0 = (int *)((i0 << 3) + verts);
        int *p1 = (int *)((i1 << 3) + verts);
        int *p2 = (int *)((i2 << 3) + verts);
        int *p3 = (int *)((i3 << 3) + verts);
        int sxy0 = p0[0];
        int sxy1 = p1[0];
        int sxy2 = p2[0];
        int sxy3 = p3[0];
        int z;

        /* ASM_BARRIER: load SXY0..SXY2 and run NCLIP */
        __asm__ volatile(
            "mtc2 %0, $12\n\t"
            "mtc2 %2, $14\n\t"
            "mtc2 %1, $13\n\t"
            "nop\n\t"
            "nop\n\t"
            ".word 0x4B400006"
            :
            : "r"(sxy0), "r"(sxy1), "r"(sxy2)
        );
        pkt[2] = sxy0;
        pkt[4] = sxy1;
        pkt[6] = sxy2;
        pkt[8] = sxy3;
        z = p0[1] + p1[1] + p2[1] + p3[1];
        RR_GTE14_SWC2_MAC0(a2 + 0x74);
        if ((*(int *)(a2 + 0x74) ^ xor_val) > 0 && z > 0) {
            z >>= shift;
            if (z < 0x1C0) {
                int *ot = (int *)(*(int *)(a2 + 0x6C) + (z << 2) + (((signed char *)a0)[0x19] << 2));
                pkt[0] = *ot;
                ((char *)pkt)[3] = 9;
                *ot = (int)pkt;
                ((char *)ot)[3] = 0;
                pkt[3] = a0[3];
                pkt[5] = a0[4];
                pkt[7] = a0[5];
                pkt[9] = *(short *)((char *)a0 + 0x16);
                /* ASM_BARRIER: make a0+0x14 the last giv in the loop so loop strength reduction picks a0+0x14 as the base */
                __asm__ volatile("" : : "m"(a0[5]));
                pkt[1] = *(int *)(a2 + 0x160);
                pkt += 10;
            }
        }
        a0 += 7;
    }
    *(int **)SCRATCHPAD = pkt;
    return a0;
}

void func_80015918(Tri0C *a0, int a1, char *a2) {
    int *spd = (int *)SCRATCHPAD;
    int *pkt = (int *)spd[0];
    int c160 = *(int *)(a2 + 0x160) | 0x3A000000;
    int shift = spd[0x18] + 3;
    int v_base = *(int *)(a2 + 0xD0);
    int v_alt = *(int *)(a2 + 0xD4);
    int c164 = *(int *)(a2 + 0x164);
    int xor_val = *(int *)(a2 + 0xBC);
    int e1 = 0xE1000220;

    while (a1-- > 0) {
        int sxy0 = *(int *)((a0->i0 << 3) + v_base);
        int sxy1 = *(int *)((a0->i1 << 3) + v_base);
        int sxy2 = *(int *)((a0->i2 << 3) + v_base);
        int z;

        /* ASM_BARRIER: load SXY0..SXY2 into GTE */
        __asm__ volatile(
            "mtc2 %0, $12\n\t"
            "mtc2 %2, $14\n\t"
            "mtc2 %1, $13"
            :
            : "r"(sxy0), "r"(sxy1), "r"(sxy2)
        );
        z = *(int *)((a0->i3 << 3) + v_base + 4) >> shift;
        *(int *)(a2 + 0x70) = z;
        if ((unsigned int)(z - 1) < 0x1BF) {
            int x0, x1, x2, x3, x4, x5;
            int y0, y1, y2, y3, y4, y5;

            RR_GTE14_NCLIP();
            pkt[4] = sxy0;
            pkt[13] = sxy1;
            pkt[22] = sxy2;
            x0 = *(short *)((a0->i0 << 3) + v_base);
            x1 = *(short *)((a0->i1 << 3) + v_base);
            x2 = *(short *)((a0->i2 << 3) + v_base);
            x3 = *(short *)((a0->i0 << 3) + v_alt);
            x4 = *(short *)((a0->i1 << 3) + v_alt);
            x5 = *(short *)((a0->i2 << 3) + v_alt);
            RR_GTE14_SWC2_MAC0(a2 + 0x74);
            if ((*(int *)(a2 + 0x74) ^ xor_val) > 0) {
                if ((x0 >= 0 || x1 >= 0 || x2 >= 0 || x3 >= 0 || x4 >= 0 || x5 >= 0) &&
                    (x0 < 0x140 || x1 < 0x140 || x2 < 0x140 || x3 < 0x140 || x4 < 0x140 || x5 < 0x140)) {
                    y0 = *(short *)((a0->i0 << 3) + v_base + 2);
                    y1 = *(short *)((a0->i1 << 3) + v_base + 2);
                    y2 = *(short *)((a0->i2 << 3) + v_base + 2);
                    y3 = *(short *)((a0->i0 << 3) + v_alt + 2);
                    y4 = *(short *)((a0->i1 << 3) + v_alt + 2);
                    y5 = *(short *)((a0->i2 << 3) + v_alt + 2);
                    if ((y0 >= 0 || y1 >= 0 || y2 >= 0 || y3 >= 0 || y4 >= 0 || y5 >= 0) &&
                        (y0 < 0x100 || y1 < 0x100 || y2 < 0x100 || y3 < 0x100 || y4 < 0x100 || y5 < 0x100)) {
                        int s3_0, s3_1;
                        int *ot;

                        sxy0 = *(int *)((a0->i0 << 3) + v_alt);
                        sxy1 = *(int *)((a0->i1 << 3) + v_alt);
                        sxy2 = *(int *)((a0->i2 << 3) + v_alt);
                        pkt[8] = sxy0;
                        pkt[17] = sxy1;
                        pkt[26] = sxy2;
                        s3_0 = *(int *)((a0->i3 << 3) + v_base);
                        pkt[20] = s3_0;
                        pkt[11] = s3_0;
                        pkt[2] = s3_0;
                        s3_1 = *(int *)((a0->i3 << 3) + v_alt);
                        pkt[19] = c160;
                        pkt[10] = c160;
                        pkt[1] = c160;
                        pkt[23] = c164;
                        pkt[14] = c164;
                        pkt[5] = c164;
                        pkt[25] = 0;
                        pkt[16] = 0;
                        pkt[7] = 0;
                        pkt[21] = 0;
                        pkt[12] = 0;
                        pkt[3] = 0;
                        pkt[24] = s3_1;
                        pkt[15] = s3_1;
                        pkt[6] = s3_1;
                        ot = (int *)(*(int *)(a2 + 0x6C) + (*(int *)(a2 + 0x70) << 2) + (a0->unk09 << 2));
                        pkt[0] = *ot;
                        ((char *)pkt)[3] = 8;
                        *ot = (int)pkt;
                        ((char *)ot)[3] = 0;
                        pkt[9] = *ot;
                        ((char *)pkt)[0x27] = 8;
                        *ot = (int)(pkt + 9);
                        ((char *)ot)[3] = 0;
                        pkt[18] = *ot;
                        ((char *)pkt)[0x4B] = 8;
                        *ot = (int)(pkt + 18);
                        ((char *)ot)[3] = 0;
                        pkt += 27;
                        pkt[1] = e1;
                        pkt[0] = *ot;
                        ((char *)pkt)[3] = 1;
                        *ot = (int)pkt;
                        pkt += 2;
                        ((char *)ot)[3] = 0;
                    }
                }
            }
        }
        a0++;
    }
    *(int **)SCRATCHPAD = pkt;
}

char *func_80015D00(char *a0_in, s32 a1_in, char *a2_in) {
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 v0 __asm__("$2");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 v1 __asm__("$3");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 a0 __asm__("$4");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 a1 __asm__("$5") = a1_in;
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *a2 __asm__("$6") = a2_in;
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *a3 __asm__("$7");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *t0 __asm__("$8");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 t1 __asm__("$9");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *t2 __asm__("$10");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 t3 __asm__("$11");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 t4 __asm__("$12");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 t5 __asm__("$13");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 t6 __asm__("$14");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 t7 __asm__("$15");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register char *t8 __asm__("$24");
    /* ASM_BARRIER: PsyQ-style register allocation */
    register s32 t9 __asm__("$25");
    s32 s0;

    v0 = *(s32 *)(a2 + 0x98);
    /* ASM_BARRIER: load-use delay on *(s32 *)(a2 + 0x98) */
    __asm__ volatile("" : "+r"(v0));
    v1 = -v0;
    t8 = a0_in;
    if (v1 < 0) {
        /* ASM_BARRIER: place t8 = a0 in bgez delay slot */
        __asm__ volatile("" : "+r"(v1), "+r"(t8));
        v1 += 0x3F;
    }

    v0 = *(s32 *)(a2 + 0x9C);
    /* ASM_BARRIER: load-use delay on *(s32 *)(a2 + 0x9C) */
    __asm__ volatile("" : "+r"(v0));
    a0 = -v0;
    t5 = v1 >> 6;
    if (a0 < 0) {
        /* ASM_BARRIER: place t5 = v1 >> 6 in bgez delay slot */
        __asm__ volatile("" : "+r"(a0), "+r"(t5));
        a0 += 0x3F;
    }

    t2 = a2 + 0x1A0;
    t1 = a1;
    v0 = SCRATCHPAD;
    /* ASM_BARRIER: order pre-loop pointer/count setup */
    __asm__ volatile("" : "+r"(t2), "+r"(t1), "+r"(v0));
    v0 = *(s32 *)(v0 + 0x60);
    t4 = a0 >> 6;
    /* ASM_BARRIER: order scratchpad load and t4 shift */
    __asm__ volatile("" : "+r"(v0), "+r"(t4));
    t3 = v0 + 2;
    if (a1 > 0) {
        /* ASM_BARRIER: place t3 = v0 + 2 in blez delay slot */
        __asm__ volatile("" : "+r"(t3));
        t0 = a2 + 0x70;
        a3 = a2 + 0x1B0;
        /* ASM_BARRIER: order t0 and a3 loop pointers */
        __asm__ volatile("" : "+r"(t0), "+r"(a3));
        do {
            /* ASM_BARRIER: GTE RTPS on vertex at t8 */
            RR_GTE_RTPS_LDV(t8);
            /* ASM_BARRIER: store SZ3 to 0(t0) */
            __asm__ volatile("swc2 $19, 0(%0)" : : "r"(t0) : "memory");
            v0 = *(s32 *)(a2 + 0x70);
            /* ASM_BARRIER: load-use delay on *(s32 *)(a2 + 0x70) */
            __asm__ volatile("" : "+r"(v0));
            a0 = v0 >> t3;
            if (v0 <= 0)
                goto L_zero;
            /* ASM_BARRIER: keep srav a0 in blez delay slot */
            __asm__ volatile("" : "+r"(a0));
            v0 = a0 < 0x1C0;
            /* ASM_BARRIER: materialize slti into v0 */
            __asm__ volatile("" : "+r"(v0));
            if (v0 != 0)
                goto L_valid;
L_zero:
            /* ASM_BARRIER: keep srav a0 live on L_zero path for blez delay slot */
            __asm__ volatile("" : "+r"(a0));
            *(s32 *)a3 = 0;
            goto L_next;
L_valid:
            /* ASM_BARRIER: store GTE IR1..IR3 to t2 */
            RR_GTE_STORE_IR(t2);
            v0 = (s32)(t2 + 0xC);
            /* ASM_BARRIER: store SXY2 to t2+0xC */
            RR_GTE_SWC2_SXY2(v0);
            v0 = *(s32 *)(t2 + 0x0);
            /* ASM_BARRIER: load-use delay on *(s32 *)(t2 + 0x0) */
            __asm__ volatile("" : "+r"(v0));
            v0 += t5;
            *(s32 *)(t2 + 0x0) = v0;
            /* ASM_BARRIER: order t2[0] update before a3[-3] update */
            __asm__ volatile("" : "+r"(a0) : : "memory");
            v0 = *(s32 *)(a3 - 0xC);
            a0 <<= 2;
            v0 += t4;
            *(s32 *)(a3 - 0xC) = v0;
            /* ASM_BARRIER: order a3[-3] store before OT pointer calculation */
            __asm__ volatile("" : "+r"(a0) : : "memory");
            v1 = *(s32 *)(a2 + 0x6C);
            v0 = *(s8 *)(t8 + 7);
            /* ASM_BARRIER: order v1 and v0 loads */
            __asm__ volatile("" : "+r"(v1), "+r"(v0));
            v1 += a0;
            v0 <<= 2;
            /* ASM_BARRIER: order v1 add and v0 shift */
            __asm__ volatile("" : "+r"(v1), "+r"(v0));
            v1 += v0;
            *(s32 *)a3 = v1;
L_next:
            t1 -= 1;
            a3 += 0x14;
            t2 += 0x14;
            /* ASM_BARRIER: order loop increments before t8 += 8 in bgtz delay slot */
            __asm__ volatile("" : "+r"(t1), "+r"(a3), "+r"(t2));
            t8 += 8;
        } while (t1 > 0);
    }

    v0 = 0x1000;
    /* ASM_BARRIER: materialize 0x1000 in v0 */
    __asm__ volatile("" : "+r"(v0));
    *(s32 *)(a2 + 0x58) = v0;
    *(s32 *)(a2 + 0x50) = v0;
    *(s32 *)(a2 + 0x48) = v0;
    /* ASM_BARRIER: order diagonal stores before v0 = a2 + 0x48 */
    __asm__ volatile("" : : : "memory");
    v0 = (s32)(a2 + 0x48);
    /* ASM_BARRIER: compute v0 = a2 + 0x48 before zeroing off-diagonals */
    __asm__ volatile("" : "+r"(v0));
    *(s32 *)(a2 + 0x54) = 0;
    *(s32 *)(a2 + 0x4C) = 0;
    /* ASM_BARRIER: load identity rotation matrix into GTE */
    RR_GTE_SETROT(v0);

    s0 = 0xE1000220;
    a0 = SCRATCHPAD;
    /* ASM_BARRIER: order s0 and a0 setup */
    __asm__ volatile("" : "+r"(s0), "+r"(a0));
    a0 = *(s32 *)a0;
    /* ASM_BARRIER: order scratchpad load before blez a1 */
    __asm__ volatile("" : "+r"(a0));
    t2 = a2 + 0x1A0;
    if (a1 > 0) {
        /* ASM_BARRIER: place t2 = a2 + 0x1A0 in blez delay slot */
        __asm__ volatile("" : "+r"(t2));
        do {
            t9 = 6;
            t3 = (s32)(a2 + 0x1AC);
            /* ASM_BARRIER: keep t9 and t3 setup before loop label */
            __asm__ volatile("" : "+r"(t9), "+r"(t3));
            do {
                a3 = *(char **)(t3 + 4);
                /* ASM_BARRIER: load-use delay on *(char **)(t3 + 4) */
                __asm__ volatile("" : "+r"(a3));
                if (a3 != 0) {
                    /* ASM_BARRIER: prevent delay-slot filling into beqz */
                    __asm__ volatile("" : : : "memory");
                    t5 = (s32)D_8009E124;
                    /* ASM_BARRIER: load translation vector from t2 into GTE */
                    RR_GTE_SETTR(t2);
                    t0 = D_8009E124;
                    /* ASM_BARRIER: compute t0 = D_8009E124 before lw t5, 0(t3) */
                    __asm__ volatile("" : "+r"(t0));
                    t5 = *(s32 *)(t3 + 0);
                    /* ASM_BARRIER: order lw t5 before GTE RTPS */
                    __asm__ volatile("" : "+r"(t5));
                    /* ASM_BARRIER: GTE RTPS on first offset at t0 */
                    RR_GTE_RTPS_LDV(t0);
                    /* ASM_BARRIER: read SXY2 into t7 */
                    __asm__ volatile("mfc2 %0, $14" : "=r"(t7));
                    t1 = 8;
                    v1 = a0 + 0x18;
                    t4 = *(s32 *)(a2 + 0x160);
                    /* ASM_BARRIER: order inner loop setup */
                    __asm__ volatile("" : "+r"(t1), "+r"(v1), "+r"(t4));
                    do {
                        t0 += 8;
                        /* ASM_BARRIER: advance t0 before GTE RTPS */
                        __asm__ volatile("" : "+r"(t0));
                        /* ASM_BARRIER: GTE RTPS on next offset at t0 */
                        RR_GTE_RTPS_LDV(t0);
                        *(s32 *)(v1 - 0x10) = t5;
                        *(s32 *)(v1 - 0x8) = t7;
                        *(s32 *)(v1 - 0x14) = t4;
                        *(s32 *)(v1 - 0x4) = 0;
                        *(s32 *)(v1 - 0xC) = 0;
                        /* ASM_BARRIER: order packet stores before OT link load */
                        __asm__ volatile("" : : : "memory");
                        v0 = *(s32 *)a3;
                        /* ASM_BARRIER: load-use delay on *(s32 *)a3 */
                        __asm__ volatile("" : "+r"(v0));
                        *(s32 *)a0 = v0;
                        *(s8 *)(v1 - 0x15) = t9;
                        *(s32 *)a3 = a0;
                        *(s8 *)(a3 + 3) = 0;
                        /* ASM_BARRIER: read SXY2 into t7 with nop delay */
                        __asm__ volatile("mfc2 %0, $14; nop" : "=r"(t7) : : "memory");
                        *(s32 *)v1 = t7;
                        v1 += 0x1C;
                        t1 -= 1;
                        /* ASM_BARRIER: order v1 and t1 updates before bgtz */
                        __asm__ volatile("" : "+r"(v1), "+r"(t1) : : "memory");
                        a0 += 0x1C;
                    } while (t1 > 0);
                    /* ASM_BARRIER: keep a0 increment in bgtz delay slot */
                    __asm__ volatile("" : "+r"(a0));
                    *(s32 *)(a0 + 4) = s0;
                    /* ASM_BARRIER: order s0 store before OT link load */
                    __asm__ volatile("" : : : "memory");
                    v0 = *(s32 *)a3;
                    /* ASM_BARRIER: load-use delay on *(s32 *)a3 */
                    __asm__ volatile("" : "+r"(v0));
                    *(s32 *)a0 = v0;
                    v0 = 1;
                    /* ASM_BARRIER: materialize 1 in v0 */
                    __asm__ volatile("" : "+r"(v0) : : "memory");
                    *(s8 *)(a0 + 3) = v0;
                    *(s32 *)a3 = a0;
                    a0 += 8;
                    /* ASM_BARRIER: advance a0 before final sb zero, 3(a3) */
                    __asm__ volatile("" : "+r"(a0) : : "memory");
                    *(s8 *)(a3 + 3) = 0;
                    /* ASM_BARRIER: keep sb before loop tail */
                    __asm__ volatile("" : : : "memory");
                }
                a1 -= 1;
                t3 += 0x14;
                /* ASM_BARRIER: order a1 and t3 updates before bgtz */
                __asm__ volatile("" : "+r"(a1), "+r"(t3));
                t2 += 0x14;
            } while (a1 > 0);
        } while (a1 > 0);
        /* ASM_BARRIER: place t2 += 0x14 in bgtz delay slot */
        __asm__ volatile("" : "+r"(t2));
    }

    *(s32 *)SCRATCHPAD = a0;
    return t8;
}

void DrawWheelTrailEffects(void *arg0, s32 *arg1, void *arg2, void *arg3, s32 arg4, s32 arg5, s32 arg6) {
    char *s2 = (char *)(SCRATCHPAD + 0x7C);
    char *s4 = (char *)&CarSlotBuffers[arg4 * 0x190];
    s32 s0;
    s32 a0;
    TrailEntry8 *a2;
    char *vdst;
    s32 *p;
    char *vsrc;
    s32 vcnt;

    SP->w[0x23] = arg5;
    if (*(s32 *)(SCRATCHPAD + 0x64) != 0) {
        s4 += 0xC8;
        SP->w[0x2F] = 0x80000000;
    } else {
        SP->w[0x2F] = 0;
    }
    SP->p[0x24] = arg0;
    SP->w[0x1B] = *(s32 *)(SCRATCHPAD + 0x4) + 0x200 + arg6 * 4;
    SP->p[0x25] = CarBoundsTable[arg4 / 2];

    if (arg1 == 0 || (SP->w[0x23] & 1) == 0) {
        S4->w[0x3] = 0;
        S4->w[0xD] = 0;
        S4->w[0x1A] = 0;
        S4->w[0x17] = 0;
    }

    if (*(u8 *)(SP->w[0x25] + 0xA) == 3) {
        if (RacePaused == 0) {
            s32 v;
            s0 = S4->w[0x18];
            if (SP->w[0x23] & 1) {
                a0 = (*(s16 *)(SP->w[0x24] + 0x1D8) - 0x358) / 4;
                if (s0 < a0) {
                    s0 = a0;
                } else if (a0 < s0) {
                    s0 -= 4;
                }
            } else if (s0 != 0) {
                s0 -= 4;
            }
            if (s0 < 0) goto L_clamp_low;
            if (s0 > 0xF0) goto L_clamp_high;
        L_clamp_low:
            v = s0;
            if (s0 < 0) v = 0;
            goto L_clamp_done;
        L_clamp_high:
            v = 0xF0;
        L_clamp_done:
            s0 = v;
            S4->w[0x18] = s0;
            if (SP->w[0x23] & 1) {
                s0 += PsyqRand() & 0xF;
            }
            S4->w[0x2] = s0;
        }
        if (S4->w[0x2] <= 0) {
            return;
        }
    } else {
        s0 = 0x80;
        if ((SP->w[0x23] & 1) == 0) {
            s0 = 0;
            a0 = 0x80;
            S4->w[0x3] = 0;
            S4->w[0xD] = 0;
            S4->w[0x1A] = 0;
            S4->w[0x17] = 0;
        } else {
            a0 = 0xF0;
        }
        if (RacePaused == 0) {
            s32 v1;
            s32 d = a0 - s0;
            if (SP->w[0x23] & 2) {
                v1 = S4->w[0x18] + d / 4;
                if (v1 < a0) {
                    S4->w[0x18] = v1;
                } else {
                    S4->w[0x18] = a0;
                }
                v1 = S4->w[0x19] + a0 / 4;
                if (v1 < a0) {
                    S4->w[0x19] = v1;
                } else {
                    S4->w[0x19] = a0;
                }
            } else {
                v1 = S4->w[0x18] - d / 8;
                if (v1 > s0) {
                    S4->w[0x18] = v1;
                } else {
                    S4->w[0x18] = s0;
                }
                v1 = S4->w[0x19] - a0 / 8;
                if (v1 > 0) {
                    S4->w[0x19] = v1;
                } else {
                    S4->w[0x19] = 0;
                }
            }
            /* ASM_BARRIER: keep a0/d live (see KEEP_LIVE2) */
            KEEP_LIVE2(a0, d);
        }
    }

    if (arg1 == 0) {
        return;
    }

    SP->w[0xE] = arg1[0] - 0x8000;
    SP->w[0xF] = arg1[1];
    SP->w[0x10] = arg1[2] - 0x8000;
    {
        s32 *g = (s32 *)SCRATCHPAD;
        SP->h[0x64] = arg1[0] - g[2];
        SP->h[0x65] = arg1[1] - g[3];
        SP->h[0x66] = arg1[2] - g[4];
    }
    RR_GTE_SETROT((void *)SCRATCH_ROT_MATRIX);
    RR_GTE_MVMVA_ROT_V0_NOCV(s2 + 0xC8);
    RR_GTE_SWC2_MAC123(s2 + 0x14);
    GTE_SETROT_AND_SAVE(arg2, s2);

    SP->h[0x65] = 0;
    SP->h[0x64] = 0;
    SP->h[0x66] = -0x1000;

    RR_GTE_MVMVA_ROT_V0_NOCV(s2 + 0xC8);
    RR_GTE_SWC2_MAC123(s2 + 0x98);

    if (*(u8 *)(SP->w[0x25] + 0xA) == 3 && RaceType == 0) {
        DrawGlowFan(s4, s2);
        return;
    }

    if (S4->w[0x1C] != 0 && (SP->w[0x23] & 3) == 3) {
        if (S4->w[0x17] == 0) {
            S4->w[0x1D] = SP->w[0xE];
            S4->w[0x1E] = SP->w[0xF];
            S4->w[0x1F] = SP->w[0x10];
            S4->w[0x17] = 1;
        }
    } else if (S4->w[0x17] == 1) {
        S4->w[0x17] = 2;
    }

    if (*(u8 *)(SP->w[0x25] + 0xA) == 2 && (RaceType == 6 || RaceType == 0)) {
        if (CurrentViewIndex != 6) {
            func_800153FC(s4, s2);
        }
    }

    RR_GTE_SETFC(0, 0, 0);
    GTE_CTC2_28(0);

    if (SP->w[0x23] & 1) {
        s32 v1;
        RR_GTE_LWC2_RGB((char *)(SP->w[0x25] + 0xC));
        v1 = SP->w[0x28];
        if (v1 >= -0xEC8) {
            char *a0;
            char *a1;
            if (RacePaused != 0) {
                SP->w[0x30] = S4->w[0x2];
            } else {
                s32 a0_val;
                s32 v0_val;
                SP->w[0x31] = v1;
                if (v1 < 0) {
                    SP->w[0x31] = 0;
                } else if (v1 >= 0x1000) {
                    SP->w[0x31] = 0xFFF;
                }
                v0_val = PsyqRand();
                v1 = (SP->w[0x28] + 0xEC8) / 2;
                a0_val = v0_val & SP->w[0x31];
                SP->w[0x30] = a0_val;
                if (v1 > 0x200) {
                    v1 = 0x200;
                }
                v0_val = (a0_val >> 2) + v1;
                SP->w[0x30] = v0_val;
                if (v0_val > 0xE00) {
                    SP->w[0x30] = 0xE00;
                }
                v0_val = 0x1000 - SP->w[0x30];
                SP->w[0x30] = v0_val;
                S4->w[0x2] = v0_val;
            }
            RR_GTE_SETTR(s2 + 0x14);
            SP->w[0x1F] = *(s32 *)(S4->w[0] + 0x10);
            a1 = *(char * volatile *)(s2 + 0x7C);
            SP->w[0x22] = *(s32 *)(S4->w[0] + 0x14);
            v1 = *(volatile s32 *)(s2 + 0x88);
            SP->w[0x21] = *(s32 *)(S4->w[0] + 0x18);
            a0 = s2 + 0x188;
            while (v1 > 0) {
                RR_GTE_SWC2_IR0(s2 + 0xDC);
                RR_GTE14_LWC2_V012(a1);
                RR_GTE14_NOP2();
                RR_GTE14_RTPT();
                a1 += 0x18;
                a0 += 0x18;
                GTE_STORE_SXY_SZ(a0);
                v1 -= 3;
            }
            if (RacePaused == 0) {
                s32 idx = S4->w[0x4];
                s32 n = S4->w[0x3];
                s32 off = (idx << 3) + 0x14;
                TrailEntry4 *a2 = (TrailEntry4 *)(s4 + off);
                s32 rem = 4 - idx;
                VertScreen *dst;
                VertScreen *src;
                if (n > 0) {
                    do {
                        a2->unk0 += a2->unk0 / 2;
                        if (a2->unk0 > 0x1000) {
                            a2->unk0 = 0x1000;
                        }
                        rem--;
                        a2++;
                        if (rem <= 0) {
                            a2 = (TrailEntry4 *)(s4 + 0x14);
                            rem = 4;
                        }
                        n--;
                    } while (n > 0);
                }
                if (S4->w[0x3] < 4) {
                    S4->w[0x3] += 1;
                }
                S4->w[0x4] = (S4->w[0x4] + 3) % 4;
                a2 = (TrailEntry4 *)(s4 + ((S4->w[0x4] << 3) + 0x14));
                v1 = 0x1000 - SP->w[0x31];
                if (v1 >= 0x200) {
                    a2->unk0 = v1;
                } else {
                    a2->unk0 = 0x200;
                }
                a2->unk2 = SP->uh[0x6E];
                n = SP->w[0x22];
                dst = a2->verts;
                src = (VertScreen *)(s2 + 0x1A0);
                while (n > 0) {
                    s32 x = src->sxy;
                    s32 y = src->sz;
                    src++;
                    n--;
                    dst->sxy = x;
                    dst->sz = y;
                    dst++;
                }
            }
            if (S4->w[0x3] >= 2) {
                s32 *out = (s32 *)(s2 + 0x160);
                s32 idx = S4->w[0x4];
                s32 s3 = S4->w[0x3] - 1;
                s32 off = (idx << 3) + 0x14;
                TrailEntry4 *s0 = (TrailEntry4 *)(s4 + off);
                s32 s1 = 4 - idx;
                while (s3 > 0) {
                    s32 unk0 = s0->unk0;
                    RR_GTE_MTC2_IR0(unk0);
                    RR_GTE14_NOP2();
                    RR_GTE_OP(0x4A780010);
                    RR_GTE_SWC2_RGB2(out);
                    out++;
                    s1--;
                    s0++;
                    if (s1 <= 0) {
                        s0 = (TrailEntry4 *)(s4 + 0x14);
                        s1 = 4;
                    }
                    s3--;
                }
                *out = 0;
                SP->p[0x35] = s2 + 0x1A0;
                {
                s32 i2 = S4->w[0x4];
                SP->p[0x36] = s2 + 0x168;
                s1 = 4 - i2;
                off = (i2 << 3) + 0x14;
                }
                s3 = S4->w[0x3] - 1;
                s0 = (TrailEntry4 *)(s4 + off);
                while (s3 > 0) {
                    p = SP->pw[0x21];
                    SP->p[0x34] = SP->p[0x35];
                    SP->p[0x35] = s0->verts;
                    func_80015918(p + 1, *(volatile s32 *)p >> 16, s2);
                    SP->w[0x58] = SP->w[0x59];
                    s1--;
                    SP->w[0x59] = *SP->pw[0x36]++;
                    s0++;
                    if (s1 <= 0) {
                        s0 = (TrailEntry4 *)(s4 + 0x14);
                        s1 = 4;
                    }
                    s3--;
                }
            }
            {
                char *v0 = *(char **)(S4->w[0] + 0x1C);
                v1 = *(s32 *)(S4->w[0] + 0x20);
                a0 = s2 + 0x188;
                while (v1 > 0) {
                    RR_GTE14_LWC2_V012(v0);
                    RR_GTE14_NOP2();
                    RR_GTE14_RTPT();
                    v0 += 0x18;
                    a0 += 0x18;
                    GTE_STORE_SXY_SZ(a0);
                    v1 -= 3;
                }
            }
            SP->w[0x58] = 0x2E808080;
            p = *(s32 **)(S4->w[0] + 0x24);
            EmitModelQuadsFT4(p + 1, *(volatile s32 *)p >> 16, s2);
            GTE_LWC2_IR0(s2 + 0xC0);
            RR_GTE14_NOP2();
            RR_GTE_OP(0x4A780010);
            RR_GTE_SWC2_RGB2(s2 + 0x160);
            SP->w[0x59] = 0;
            SP->w[0x58] = (SP->w[0x58] & 0xFFFFFF) | 0x32000000;
            p = *(s32 **)(S4->w[0] + 0xC);
            func_80015D00(p + 1, *(volatile s32 *)p >> 16, s2);
        } else {
            S4->w[0x3] = 0;
        }
    }

    if (SP->w[0x28] >= 0x7D0) {
        s32 v1 = S4->w[0x17];
        S4->w[0xD] = 0;
        if (v1 == 0) {
            return;
        }
        if (v1 == 1) {
            S4->w[0x17] = 2;
        }
    } else {
        s32 v1;
        char *a0;
        char *a1;
        RR_GTE_SETROT(s2);
        RR_GTE_SETTR(s2 + 0x14);
        GTE_CTC2_27(0x88);
        SP->w[0x1F] = *(s32 *)(S4->w[0] + 0x34);
        a1 = *(char * volatile *)(s2 + 0x7C);
        SP->w[0x22] = *(s32 *)(S4->w[0] + 0x38);
        v1 = *(volatile s32 *)(s2 + 0x88);
        SP->w[0x21] = *(s32 *)(S4->w[0] + 0x3C);
        a0 = s2 + 0x188;
        while (v1 > 0) {
            RR_GTE_SWC2_IR0(s2 + 0xDC);
            RR_GTE14_LWC2_V012(a1);
            RR_GTE14_NOP2();
            RR_GTE14_RTPT();
            a1 += 0x18;
            a0 += 0x18;
            GTE_STORE_SXY_SZ(a0);
            v1 -= 3;
        }
        if (RacePaused == 0) {
            s32 idx;
            TrailEntry4 *entry;
            s32 n;
            VertScreen *dst;
            VertScreen *src;
            if (S4->w[0xD] < 4) {
                S4->w[0xD] += 1;
            }
            idx = (S4->w[0xE] + 3) % 4;
            S4->w[0xE] = idx;
            entry = (TrailEntry4 *)(s4 + ((idx << 3) + 0x3C));
            entry->unk0 = S4->h[0x30];
            n = SP->w[0x22];
            dst = entry->verts;
            src = (VertScreen *)(s2 + 0x1A0);
            while (n > 0) {
                s32 x = src->sxy;
                s32 y = src->sz;
                src++;
                n--;
                dst->sxy = x;
                dst->sz = y;
                dst++;
            }
        }
        if (S4->w[0xD] >= 2) {
            s32 cnt = S4->w[0xD];
            s32 col = S4->w[0x18];
            s32 s0;
            s32 *out;
            s32 s1;
            GTE_DPCS_COL(col, SP->w[0x37]);
            out = (s32 *)(s2 + 0x160);
            s0 = cnt - 1;
            while (s0 > 0) {
                if (col > 0) {
                    *out = col;
                } else {
                    *out = 0;
                }
                col -= col / 4;
                s0--;
                out++;
            }
            *out = 0;
            SP->p[0x35] = s2 + 0x1A0;
            SP->p[0x36] = s2 + 0x168;
            s0 = S4->w[0xD] - 1;
            s1 = S4->w[0xE];
            while (s0 > 0) {
                SP->p[0x34] = SP->p[0x35];
                s1 = (s1 + 1) % 4;
                p = SP->pw[0x21];
                SP->p[0x35] = ((TrailEntry4 *)(s4 + (s1 << 3) + 0x3C))->verts;
                func_80015918(p + 1, *(volatile s32 *)p >> 16, s2);
                if (SP->w[0x59] <= 0) {
                    break;
                }
                do {
                    SP->w[0x58] = SP->w[0x59];
                } while (0);
                {
                    s32 *q;
                    q = SP->pw[0x36];
                    s0--;
                    col = (s32) q;
                    q = (s32 *)*q;
                    col += 4;
                    SP->pw[0x36] = (s32 *)col;
                    SP->w[0x59] = (s32)q;
                }
            }
        }
        {
            char *v0 = *(char **)(S4->w[0] + 0x40);
            v1 = *(s32 *)(S4->w[0] + 0x44);
            a0 = s2 + 0x188;
            while (v1 > 0) {
                RR_GTE14_LWC2_V012(v0);
                RR_GTE14_NOP2();
                RR_GTE14_RTPT();
                v0 += 0x18;
                a0 += 0x18;
                GTE_STORE_SXY_SZ(a0);
                v1 -= 3;
            }
        }
        {
            s32 c = S4->w[0x18];
            SP->w[0x58] = 0x2E000000 | (c << 16) | (c << 8) | c;
            p = *(s32 **)(S4->w[0] + 0x48);
            EmitModelQuadsFT4(p + 1, *(volatile s32 *)p >> 16, s2);
        }
        {
            char *v0 = *(char **)(S4->w[0] + 0x28);
            v1 = *(s32 *)(S4->w[0] + 0x2C);
            a0 = s2 + 0x188;
            while (v1 > 0) {
                RR_GTE14_LWC2_V012(v0);
                RR_GTE14_NOP2();
                RR_GTE14_RTPT();
                v0 += 0x18;
                a0 += 0x18;
                GTE_STORE_SXY_SZ(a0);
                v1 -= 3;
            }
        }
        {
            s32 c = S4->w[0x19];
            SP->w[0x58] = 0x2E000000 | (c << 16) | (c << 8) | c;
            p = *(s32 **)(S4->w[0] + 0x30);
            EmitModelQuadsFT4(p + 1, *(volatile s32 *)p >> 16, s2);
        }
    }

    GTE_CTC2_27(0x88);
    if (S4->w[0x17] == 0) {
        return;
    }

    SP->w[0x1F] = *(s32 *)(S4->w[0] + 0x34);
    SP->w[0x22] = *(s32 *)(S4->w[0] + 0x38);
    SP->w[0x21] = *(s32 *)(S4->w[0] + 0x3C);
    SP->p[0x2A] = ((TrailEntry8 *)(s4 + (S4->w[0x1B] << 3) + 0x84))->verts;

    if (S4->w[0x17] == 1) {
        RR_GTE_SETROT(arg3);
        SP->w[0x2B] = SP->w[0xE] - S4->w[0x1D];
        SP->w[0x2C] = SP->w[0xF] - S4->w[0x1E];
        SP->w[0x2D] = SP->w[0x10] - S4->w[0x1F];
        RR_GTE_SETTR(s2 + 0xAC);
        vdst = (SP->c[0x2A] = s2 + 0xE0);
        vsrc = SP->c[0x1F];
        vcnt = SP->w[0x22];
        while (vcnt > 0) {
            RR_GTE_MVMVA_ROT_TR_V0(vsrc);
            RR_GTE_STORE_IR_S16(vdst);
            vsrc += 8;
            vcnt--;
            vdst += 8;
        }
        if (RacePaused == 0) {
            S4->w[0x1A] += 1;
            if (S4->w[0x1A] == 8) {
                S4->w[0x17] = 2;
            }
            S4->w[0x1B] = (S4->w[0x1B] + 7) % 8;
            a2 = (TrailEntry8 *)(s4 + ((S4->w[0x1B] << 3) + 0x84));
            vdst = (char *)a2->verts;
            a2->unk0 = S4->h[0x30];
            vcnt = SP->w[0x22];
            vsrc = SP->c[0x2A];
            while (vcnt > 0) {
                *(SVector *)vdst = *(SVector *)vsrc;
                vsrc += 8;
                vcnt--;
                vdst += 8;
            }
        }
    }

    if (S4->w[0x1A] != 0) {
        /* ASM_BARRIER: pin the fade counter to $a1 (original register choice) */
        register s32 nleft __asm__("$5");
        s32 dec8;
        s32 i8;
        s32 s5;
        nleft = S4->w[0x1A];
        dec8 = (S4->w[0x17] == 2) ? 8 : 4;
        i8 = S4->w[0x1B];
        while (nleft > 0) {
            a2 = (TrailEntry8 *)(s4 + ((i8 << 3) + 0x84));
            i8 = (i8 + 1) % 8;
            s5 = a2->unk0;
            s5 -= dec8;
            if (s5 < 0) {
                s5 = 0;
            }
            nleft--;
            a2->unk0 = s5;
        }
        if (s5 == 0) {
            S4->w[0x1A] -= 1;
        }
        if (S4->w[0x1A] == 0) {
            S4->w[0x17] = 0;
            return;
        }
        if (S4->w[0x1A] >= 2) {
            /* ASM_BARRIER: pin the RTPT vertex counter to $v1 (original register choice) */
            register s32 nvert __asm__("$3");
            s32 s0;
            s32 s1;
            RR_GTE_SETROT((void *)SCRATCH_ROT_MATRIX);
            {
                char *t = s2 + 0x1A0;
                s32 *g;
                s32 m;
                do {
                    m = -0x8000;
                } while (0);
                g = (s32 *)SCRATCHPAD;
                SP->p[0x34] = t;
                SP->h[0x64] = S4->w[0x1D] + m - g[2];
                SP->h[0x65] = S4->w[0x1E] - g[3];
                SP->h[0x66] = S4->w[0x1F] + m - g[4];
            }
            RR_GTE_MVMVA_ROT_V0_NOCV(s2 + 0xC8);
            RR_GTE_SWC2_MAC123(s2 + 0x38);
            RR_GTE_SETTR_AT20(s2 + 0x24);
            vsrc = SP->c[0x2A];
            vdst = SP->c[0x34] - 0x18;
            nvert = SP->w[0x22];
            while (nvert > 0) {
                RR_GTE14_LWC2_V012(vsrc);
                RR_GTE14_NOP2();
                RR_GTE14_RTPT();
                vsrc += 0x18;
                vdst += 0x18;
                GTE_STORE_SXY_SZ(vdst);
                nvert -= 3;
            }
            SP->c[0x35] = vdst + 0x18;
            s0 = S4->w[0x1B];
            SP->w[0x58] = 0;
            s1 = S4->w[0x1A] - 1;
            while (s1 > 0) {
                s32 off;
                void *d0;
                void *d4;
                s0 = (s0 + 1) % 8;
                off = (s0 << 3) + 0x84;
                a2 = (TrailEntry8 *)(s4 + off);
                vsrc = (char *)a2->verts;
                vdst = SP->c[0x35] - 0x18;
                nvert = SP->w[0x22];
                while (nvert > 0) {
                    RR_GTE_SWC2_IR0(s2 + 0xDC);
                    RR_GTE14_LWC2_V012(vsrc);
                    RR_GTE14_NOP2();
                    RR_GTE14_RTPT();
                    vsrc += 0x18;
                    vdst += 0x18;
                    GTE_STORE_SXY_SZ(vdst);
                    nvert -= 3;
                }
                if (s1 == 1) {
                    SP->w[0x59] = 0;
                } else {
                    u16 c;
                    GTE_LWC2_IR0_MEM(s2 + 0xDC);
                    c = a2->unk0;
                    GTE_MTC2_RGB_DPCS(c);
                    RR_GTE_SWC2_RGB2(s2 + 0x164);
                }
                p = SP->pw[0x21];
                func_80015918(p + 1, *(volatile s32 *)p >> 16, s2);
                do {
                    d4 = SP->p[0x35];
                    d0 = SP->p[0x34];
                } while (0);
                nvert = SP->w[0x59];
                SP->p[0x34] = d4;
                SP->p[0x35] = d0;
                if (nvert <= 0) {
                    break;
                }
                s1--;
                SP->w[0x58] = nvert;
            }
        }
    }
}
