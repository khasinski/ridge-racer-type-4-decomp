#include "types.h"
#include "scratchpad.h"
#include "game/car/car.h"
#include "game/car/carsp.h"
#include "rr_asm.h"
#include "game/car/carstate.h"
#include "game/car/carwheel.h"
#include "game/car/carmove1.h"
#include "rr/menu_overlay_runtime.h"
#include "rr/movie_playback_camera.h"

#define W9C w8c[4]
#define WAC w8c[8]
#define WBC w8c[12]

typedef struct Node {
    s32 pad0[2];
    Z52 z8;
} Node;

typedef struct { char p0[8]; int s8; char p1[0x34 - 0xC]; int w34, w38, w3c, w40; char p2[0x114 - 0x44]; int w114[3][4]; int w144[3]; short h150[4]; char p3[0x250 - 0x158]; short h250[64]; } T;

typedef struct {
    char p0[0x10]; int w10, w14, w18; char p1[0x20 - 0x1C]; int w20, w24, w28;
    char p2[0x40 - 0x2C]; int w40, w44, w48; char p3[0x50 - 0x4C]; int w50; unsigned short h54; short h56; int w58;
    char p5[0x70 - 0x5C]; int w70, w74, w78; char p6[0x80 - 0x7C]; short h80; short h82; short h84; char p7[0x8C - 0x86];
    int w8c[13]; char p8[0xC8 - 0xC0]; int wc8, wcc, wd0;
    char p9[0xF8 - 0xD4]; unsigned short hf8[4]; unsigned short h100[4]; char pa[0x10C - 0x108]; short h10c, h10e;
    char pb[0x124 - 0x110]; short h124[16]; short h144[16]; unsigned short h164, h166, h168, h16a; int w16c;
    char pc[0x1A8 - 0x170]; unsigned short h1a8, h1aa, h1ac, h1ae;
    char pd[0x1B8 - 0x1B0]; unsigned short h1b8, h1ba; unsigned short h1bc[4]; short h1c4[4]; short h1cc[4];
} A;

struct B4 { char b[4]; };

extern short TrackLengthDiv1024;
extern short TrackLengthDiv256Plus1;

void FindAxleTrackNode(void);
void FindWheelTrackNodes(Car *arg0);
void UpdateWheelBumps();
void SetTrackNodeData(char *a);

void FindAxleTrackNode(void)
{
    SP *g = (SP *)SCRATCH_CAR_WORK;
    s32 a, n, t, x;
    char *p, *q;
    s32 junk[2];
    /* ASM_BARRIER: GTE operand temps live in $t2/$t3/$t4 in the original PsyQ-style sequence */
    register s32 r1 __asm__("$10");
    register s32 r2 __asm__("$11");
    register s32 r3 __asm__("$12");
    g->cnt = 0;
    for (;;) {
        g->w34 = (g->s150[g->idx] + 1) % TrackNodeCount;
        a = g->w34;
        p = (char *)TrackNodes + g->s150[g->idx] * 0x3C;
        g->w78 = *(s32 *)(p + 8);
        g->w80 = *(s32 *)(p + 12);
        q = (char *)TrackNodes + a * 0x3C;
        g->h74 = g->h70 = *(u16 *)(q + 8) - (u16)g->w78;
        g->h76 = g->h72 = *(u16 *)(q + 12) - (u16)g->w80;
        g->h64 = (g->t110[g->idx].a + g->t110[g->idx + 1].a) / 2 - (u16)g->w78;
        g->h66 = (g->t110[g->idx].c + g->t110[g->idx + 1].c) / 2 - (u16)g->w80;
        g->h70 += *(u16 *)(q + 0);
        g->h72 += *(u16 *)(q + 2);
        g->h74 += *(u16 *)(q + 4);
        g->h76 += *(u16 *)(q + 6);
        g->h68 = *(u16 *)(p + 0);
        g->h6a = *(u16 *)(p + 2);
        g->h6c = *(u16 *)(p + 4);
        g->h6e = *(u16 *)(p + 6);
        r3 = *(s32 *)&g->h64; r1 = *(s32 *)&g->h68; r2 = *(s32 *)&g->h6c;
        RR_GTE_NCLIP3(r1, r2, r3, &g->r3c);
        r1 = *(s32 *)&g->h74; r2 = *(s32 *)&g->h70; r3 = *(s32 *)&g->h64;
        RR_GTE_NCLIP3(r1, r2, r3, &g->r40);
        if (g->r3c >= 0 && g->r40 >= 0)
            return;
        n = g->cnt + 1;
        g->cnt = n;
        x = g->s150[g->idx] + TrackNodeCount;
        g->s150[g->idx] = ((n & 1) ? x + n : x - n) % TrackNodeCount;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define TrackNodes ((Node *)TrackNodes)

void FindWheelTrackNodes(Car *arg0) {
    SP_1 *g = (SP_1 *)SCRATCH_CAR_WORK;
    register s32 t0 asm("$8"); /* ASM_BARRIER: pin inner loop counter to $t0 */
    register s32 *q asm("$19"); /* ASM_BARRIER: pin (SCRATCHPAD + 0x7C) pointer to $s3 */

    arg0->u1F0.w1F0 = 0;
    arg0->u20.s.w20 = arg0->posX6 >> 2;
    arg0->u20.s.w28 = arg0->posZ6 >> 2;
    *(struct Q4 *)(SCRATCHPAD + 0x190) = arg0->q88;
    *(struct Q4 *)(SCRATCHPAD + 0x1A0) = arg0->q98;
    *(struct Q4 *)(SCRATCHPAD + 0x1B0) = arg0->qA8;
    *(struct Q4 *)(SCRATCHPAD + 0x1C0) = arg0->qB8;
    g->sC0 = 0;
    g->sBA = 0;
    g->sC8 = 0;
    g->sC6 = 0;
    g->sC4 = 0;
    g->sE6 = 0;
    g->sE4 = 0;
    g->sE2 = 0;
    g->sDE = 0;
    g->sDC = 0;
    g->sDA = 0;
    *(struct Q4 *)(SCRATCHPAD + 0x180) = arg0->u20.q20;
    q = (s32 *)(SCRATCHPAD + 0x7C);
    {
        s32 a0 = arg0->wF0;
        s32 v0 = a0 - 2;
        s32 v1 = (v0 + TrackNodeCount) % TrackNodeCount;
        g->w14 = a0;
        g->w30 = arg0->s10C;
        g->w8 = 0;
        g->w20 = v1;
    }
    do {
        g->t110[g->w8].a = g->w100 + (g->t110[g->w8].a >> 2);
        g->t110[g->w8].c = g->w108 + (g->t110[g->w8].c >> 2);
        g->t110[g->w8 + 1].a = g->w100 + (g->t110[g->w8 + 1].a >> 2);
        g->t110[g->w8 + 1].c = g->w108 + (g->t110[g->w8 + 1].c >> 2);
        g->s150[g->w8] = arg0->sF8[g->w8];
        FindAxleTrackNode();
        g->z180[g->w8] = TrackNodes[g->s150[g->w8]].z8;
        g->z180[g->w8 + 1] = TrackNodes[(g->s150[g->w8] + 1) % TrackNodeCount].z8;
        arg0->sF8[g->w8 + 1] = arg0->sF8[g->w8] = g->s150[g->w8];
        g->w8 += 2;
    } while (g->w8 < 3);
    g->w8 = 0;
    do {
        s32 a0 = g->w8;
        g->wC = a0;
        if (g->s150[0] != g->s150[2] || g->w8 < 2) {
            g->s174 = g->z180[a0].s1A;
            g->s176 = g->z180[g->wC].s18;
            g->s24 = rsin(g->s174);
            g->s26 = rcos(g->s174);
            g->s28 = rsin(g->s176);
            g->s2A = rcos(g->s176);
            if (g->s174 != g->s176) {
                g->w38 = 1;
                g->s174 = g->s162 = g->z180[g->wC].s14;
                g->s160 = g->z180[g->wC].sA + 0x400;
            } else {
                g->w38 = 0;
                g->s174 = g->s162 = (g->z180[g->wC].s14 * g->s26) >> 12;
                g->s160 = g->z180[g->wC].s16 + 0x400;
                g->s176 = (g->s24 * g->z180[g->wC].s14) >> 12;
            }
            g->sB8 = g->sC2 = rcos(g->s160);
            {
                s16 s = rsin(g->s160);
                g->sBE = -s;
                g->sBC = s;
            }
            RR_GTE_SETROT(&g->sB8);
            g->s16C = g->z180[g->wC + 1].s8 - g->z180[g->wC].s8;
            g->s168 = g->z180[g->wC + 1].sE - g->z180[g->wC].sE;
            g->s164 = g->z180[g->wC + 1].s12 - g->z180[g->wC].s12;
            g->s166 = g->z180[g->wC + 1].s10 - g->z180[g->wC].s10;
        }
        for (t0 = 0; t0 < 2; t0++) {
            g->sF8 = g->t110[g->w8 + t0].a - g->z180[g->wC].w0;
            g->sFC = g->t110[g->w8 + t0].c - g->z180[g->wC].w4;
            RR_GTE_MVMVA_ROT_V0_NOCV(&g->sF8);
            g->w34 = 0;
            g->s16E = 0;
            g->w3C = g->s174;
            g->w40 = g->s176;
            RR_GTE_STORE_IR(&g->w14);
            __asm__ volatile("" : : : "memory"); /* ASM_BARRIER: RR_GTE_STORE_IR writes g->w14..w1C in memory */
            if (g->w38 == 1) {
                g->w40 = (g->w18 * g->s24) / g->s26;
                g->w14 -= g->w40;
                g->w3C -= g->w40;
                g->w3C += (g->w18 * g->s28) / g->s2A;
                g->w14 = (g->w14 * g->s162) / g->w3C;
            } else {
                g->w18 -= (g->w40 * g->w14) / g->s162;
            }
            g->w10 = (g->w14 << 10) / g->s162;
            g->s158 = ((g->s168 * g->w10) >> 10) + g->z180[g->wC].sE;
            g->s15A = ((g->s164 * g->w10) >> 10) + g->z180[g->wC].s12;
            g->s15C = ((g->s166 * g->w10) >> 10) + g->z180[g->wC].s10;
            if (g->w18 > g->s15A) {
                g->s16A = g->w18 - g->s15A;
                g->w34 = 1;
                g->w18 = g->s15A;
            } else if (g->w18 < -g->s15C) {
                g->s16A = -(g->w18 + g->s15C);
                g->w34 = 2;
                g->w18 = -g->s15C;
            }
            if (g->w34 > 0) {
                g->w34--;
                g->s164 = g->z180[g->wC + 1].s2C[g->w34] - g->z180[g->wC].s2C[g->w34];
                g->s164 = ((g->s164 * g->w10) >> 10) + g->z180[g->wC].s2C[g->w34];
                g->s170 = g->z180[g->wC + 1].s30[g->w34] - g->z180[g->wC].s30[g->w34];
                g->s170 = ((g->s170 * g->w10) >> 10) + g->z180[g->wC].s30[g->w34];
                if (g->s16A < g->s164) {
                    g->s172 = g->s16A;
                    g->s16A = 0;
                } else {
                    g->s16A -= (g->s172 = g->s164);
                    g->s16E = g->z180[g->wC + 1].s28[g->w34] - g->z180[g->wC].s28[g->w34];
                    g->s16E = ((g->s16E * g->w10) >> 10) + g->z180[g->wC].s28[g->w34];
                    {
                        s32 v = (g->w34 != 0) ? -(((g->s16E * g->s16A) << 11) >> 16) : (((g->s16E * g->s16A) << 11) >> 16);
                        g->s16E = v;
                    }
                    arg0->u1F0.b1F0[g->w8 + t0] = g->z180[g->wC].a20[g->w34].b0;
                }
                {
                    s32 v = (g->w34 != 0) ? -(((g->s170 * g->s172) << 11) >> 16) : (((g->s170 * g->s172) << 11) >> 16);
                    g->s16E += (g->s170 = v);
                }
            }
            g->s250[*q][g->w8 + t0] = g->s16E + ((((g->s16C * g->w10) >> 10) + g->z180[g->wC].s8) + ((g->s158 * g->w18) >> 5));
        }
        g->w8 += 2;
    } while (g->w8 < 4);
}

#undef TrackNodes

void UpdateWheelBumps(a)
A *a;
{
    int *q7c = (int *)(SCRATCHPAD + 0x7C);
    T *SC = (T *)SCRATCH_CAR_WORK;
    int t, t2, t3, v;
    A *p2;
    int *pw;
    SC->s8 = 0;
    do {
        t = SC->s8;
        SC->w114[t][0] = *(short *)((char *)SC + (*q7c * 8 + t * 2) + 0x250);
        if ((short)a->h100[SC->s8] != (short)a->hf8[SC->s8]) {
            if ((short)a->h1bc[SC->s8] == 0) {
                if (TriggerWheelBump(a, SC->s8) != 0)
                    a->h1cc[SC->s8] = SC->w114[SC->s8][0];
            }
        }
        t2 = *(volatile int *)&SC->s8;
        p2 = (A *)((char *)a + t2 * 2);
        /* ASM_BARRIER: pin c to a0 */
        {
            register int c asm("a0") = (short)p2->h1bc[0];
            if (c > 0) {
                int lo = p2->h1c4[0] * c;
            a->w8c[t2 * 4] = p2->h1cc[0] + lo + ((c * c << 11) / 900);
            t3 = SC->s8;
            v = SC->w114[t3][0];
            if (a->w8c[t3 * 4] >= v) {
                if ((short)a->h1bc[t3] >= 2) {
                    a->w8c[t3 * 4] = v;
                    if (SC->s8 == 0) {
                        a->h1ba = a->h1bc[0];
                        SetCarImpactState(1, a);
                    }
                    a->h1bc[SC->s8] = 0;
                    a->h1b8 ^= 1 << SC->s8;
                } else goto b11;
            } else {
b11:
                a->h1bc[SC->s8]++;
            }
            SC->h150[SC->s8] = a->w8c[SC->s8 * 4] - SC->w114[SC->s8][0];
            } else {
                a->w8c[t2 * 4] = SC->w114[t2][0];
                SC->h150[SC->s8] = 0;
            }
        }
        a->h100[SC->s8] = a->hf8[SC->s8];
    } while (++SC->s8 < 4);
    a->h1a8 += Atan2Table(a->h10e, (SC->h150[0] + SC->h150[1] - (SC->h150[2] + SC->h150[3])) >> 3);
    {
        short r = Atan2Table(a->h10c, (SC->h150[0] + SC->h150[2] - (SC->h150[1] + SC->h150[3])) >> 3);
        int wac = a->w8c[8];
        int wbc = a->w8c[12];
        int m = (wac + wbc) >> 1;
        a->w14 = m;
        a->h1aa += r;
        a->w14 = ((((a->w8c[0] + a->w8c[4]) - (a->w8c[8] + a->w8c[12])) >> 1) * 0x14) / a->h10e + m;
        /* ASM_BARRIER: keep wbc live past h1aa update so local-alloc picks a2 */
        __asm__ volatile("" : : "r"(wbc));
    }
    {
        int m = (SC->w114[2][0] + SC->w114[3][0]) >> 1;
        int v, d;
        a->wcc = m;
        d = ((((SC->w114[0][0] + SC->w114[1][0]) - (SC->w114[2][0] + SC->w114[3][0])) >> 1) * 0x14) / a->h10e;
        v = a->w16c & 0xF;
        a->wcc = d + m;
        a->w16c = v;
        SC->w34 = v;
        SC->w38 = (v + 6) & 0xF;
        SC->w3c = (SC->w34 + 0xC) & 0xF;
        SC->w40 = (SC->w34 + 2) & 0xF;
    }
    a->h124[SC->w34] = Atan2Table(a->h10e, (a->w8c[0] + a->w8c[4] - (a->w8c[8] + a->w8c[12])) >> 3);
    a->h144[SC->w34] = Atan2Table(a->h10c, (a->w8c[0] + a->w8c[8] - (a->w8c[4] + a->w8c[12])) >> 3);
    a->h164 += a->h124[SC->w34];
    a->h166 += a->h144[SC->w34];
    a->h164 = a->h164 - a->h124[SC->w38];
    a->h166 = a->h166 - a->h144[SC->w38];
    a->w50 = (short)((short)a->h164 / 10);
    a->w58 = (short)((short)a->h166 / 10);
    a->h168 += a->h124[SC->w3c];
    a->h16a += a->h144[SC->w3c];
    a->h168 -= a->h124[SC->w40];
    a->h16a -= a->h144[SC->w40];
    a->h80 = (short)a->h168 / 10 + ((short)a->h1ac >> 1);
    a->h82 = a->h54;
    a->h84 = a->h1ae + (short)a->h16a / 10;
    a->w70 += a->w10;
    a->w74 += a->w14;
    a->w78 += a->w18;
    a->w10 += a->w40;
    a->w14 += a->w44;
    a->w18 += a->w48;
    a->w70 += a->w40;
    a->w74 += a->w44;
    {
        /* ASM_BARRIER: pin w78 to v0 */
        register int w78 asm("v0") = a->w78;
        /* ASM_BARRIER: pin w48 to a0 */
        register int w48 asm("a0") = a->w48;
        int w10 = a->w10;
        int w18 = a->w18;
        /* ASM_BARRIER: pin wcc to v1 */
        register int wcc asm("v1");
        /* ASM_BARRIER: pin w44 to a1 */
        register int w44 asm("a1") = a->w44;
        /* ASM_BARRIER: pin v2 to v1 */
        register int v2 asm("v1");
        /* ASM_BARRIER: schedule w10/w18 loads before wcc load */
        __asm__ volatile("" : "+r"(w78) : "r"(w10), "r"(w18), "r"(w48), "r"(w44));
        wcc = a->wcc;
        w78 += w48;
        wcc += w44;
        /* ASM_BARRIER: schedule both addu before w78 store */
        __asm__ volatile("" : "+r"(w78), "+r"(wcc));
        a->w78 = w78;
        /* ASM_BARRIER: schedule w78 store before wcc store */
        __asm__ volatile("" : "+r"(wcc));
        a->wcc = wcc;
        v2 = w10;
        /* ASM_BARRIER: force addu v1, a2, zero */
        __asm__ volatile("" : "+r"(v2));
        v2 >>= 2;
        a->w20 = v2;
        a->w16c += 1;
        v2 = w18;
        /* ASM_BARRIER: force addu v1, a3, zero */
        __asm__ volatile("" : "+r"(v2));
        v2 >>= 2;
        a->wc8 = w10;
        a->wd0 = w18;
        a->w28 = v2;
        a->w24 = a->w14 >> 2;
        /* ASM_BARRIER: keep w48 and w44 live across tail so local-alloc picks a2/a3 for w10/w18 */
        __asm__ volatile("" : : "r"(w48));
    }
}

/* legacy typing of shared globals in this part of the unit */
#define TrackNodes ((char *)TrackNodes)

void SetTrackNodeData(char *a)
{
    struct B4 sp;
    int i = 0;
    int n;
    register char *p asm("$8");
    TrackNodeGrid = a + 4;
    sp = *(struct B4 *)a;
    n = *(int *)(a + 0x804);
    TrackNodes = p = a + 0x808;
    TrackLength = 0;
    TrackNodeCount = n;
    if (n > 0) {
        int m = n;
        register char *q asm("$4") = p;
        do {
            TrackLength += *(short *)(q + 0x1C);
            q += 0x3C;
            i++;
        } while (i < m);
    }
    TrackLengthDiv1024 = TrackLength / 1024;
    TrackLengthDiv256Plus1 = (TrackLength >> 8) + 1;
}

#undef TrackNodes
