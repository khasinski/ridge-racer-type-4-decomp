/* RODATA 0x80010884 0x54 */
#include "game/ui/hudmap.h"
#include "game/gfx/gfxmtx.h"
#include "game/menu/mnrows.h"
#include "game/ui/uidraw.h"
#include "rr/bios_devices.h"
#include "rr/audio_cd_controller.h"
#include "game/gfx/drawbuf.h"
#define EP ((E_2 *)((a0 * 16) + Rect16Rows))

typedef struct {
    short p0, f2, f4, f6, f8, fA, fC, fE, f10, f12, f14, f16, f18, f1A;
} E;

typedef struct { short f0, f2, f4, f6, f8, fA, fC, fE; } E_1;

typedef struct {
    short s0, s2, x, y, w, h, st, cnt;
} E_2;

extern int DrawBadgeRow();
extern int rsin();
extern int rcos();
extern E *BadgeRows;
extern int Rect16Rows;
extern unsigned char Rect16EdgeCornerPairs[];
extern unsigned char Rect16EdgeShadeIndex[];
extern int GlyphRows;
extern char GlyphRowTemplates[];

void UpdateBadgeRowAnim();
int AllocRect16Rows(int a0);
void InitRect16Row(int i, short a, short b, short c, short d, short e, short f);
s32 UpdateRect16Row(int a0);
int AllocGlyphRows(int a);
void InitGlyphRow();

void UpdateBadgeRowAnim(a0) int a0; {
    int s2;
    if (a0 < BadgeRowCount) {
        s2 = BadgeRows[a0].f12;
        switch (BadgeRows[a0].f10) {
        case 0:
            BadgeRows[a0].f1A = 2;
            BadgeRows[a0].f10 = 0xA;
            BadgeRows[a0].f18 = 0;
            BadgeRows[a0].f12 = 0;
            BadgeRows[a0].f4 = BadgeRows[a0].f8;
            BadgeRows[a0].f6 = (BadgeRows[a0].fA + BadgeRows[a0].fE) - 2;
            BadgeRows[a0].f14 = BadgeRows[a0].f8;
            BadgeRows[a0].f16 = (BadgeRows[a0].fA + BadgeRows[a0].fE) - 2;
            break;
        case 1:
            BadgeRows[a0].f18 = ((BadgeRows[a0].fC + 2) * rsin((s2 << 10) / 15)) / 4096;
            if (s2 < 0xF) {
                BadgeRows[a0].f12++;
            } else {
                BadgeRows[a0].f10 = 2;
                BadgeRows[a0].f12 = 0;
            }
            break;
        case 2:
            BadgeRows[a0].f1A = (BadgeRows[a0].fE * rsin((s2 << 10) / 15)) / 4096;
            BadgeRows[a0].f16 = (BadgeRows[a0].fA + BadgeRows[a0].fE) - BadgeRows[a0].f1A;
            BadgeRows[a0].f6 = BadgeRows[a0].f16;
            if (s2 < 0xF) {
                BadgeRows[a0].f12++;
            } else {
                BadgeRows[a0].f10 = 3;
                BadgeRows[a0].f12 = 0;
            }
            break;
        case 3:
            BadgeRows[a0].f4 = BadgeRows[a0].f8;
            BadgeRows[a0].f6 = BadgeRows[a0].fA;
            BadgeRows[a0].f14 = BadgeRows[a0].f8;
            BadgeRows[a0].f16 = BadgeRows[a0].fA;
            BadgeRows[a0].f18 = BadgeRows[a0].fC + 2;
            BadgeRows[a0].f1A = BadgeRows[a0].fE;
            break;
        case 4:
            BadgeRows[a0].f1A = (BadgeRows[a0].fE * rcos((s2 << 10) / 15)) / 4096;
            BadgeRows[a0].f16 = BadgeRows[a0].fA + (BadgeRows[a0].fE - BadgeRows[a0].f1A);
            BadgeRows[a0].f6 = BadgeRows[a0].f16;
            if (s2 < 0xF) {
                BadgeRows[a0].f12++;
            } else {
                BadgeRows[a0].f10 = 5;
                BadgeRows[a0].f12 = 0;
            }
            break;
        case 5:
            BadgeRows[a0].f18 = ((BadgeRows[a0].fC + 2) * rcos((s2 << 10) / 15)) / 4096;
            if (s2 < 0xF) {
                BadgeRows[a0].f12++;
            } else {
                BadgeRows[a0].f10 = 0xB;
                BadgeRows[a0].f12 = 0;
            }
            break;
        case 10:
            if (BadgeRows[a0].f2 != 0) {
                BadgeRows[a0].f10 = 1;
            }
            break;
        case 7:
            BadgeRows[a0].f18 = ((BadgeRows[a0].fC + 2) * rsin((s2 << 10) / 15)) / 4096;
            BadgeRows[a0].f1A = BadgeRows[a0].fE;
            BadgeRows[a0].f14 = BadgeRows[a0].f8;
            BadgeRows[a0].f16 = BadgeRows[a0].fA;
            if (s2 < 0xF) {
                BadgeRows[a0].f12++;
            } else {
                BadgeRows[a0].f10 = 3;
                BadgeRows[a0].f12 = 0;
            }
            break;
        case 6:
        case 9:
        case 11:
            break;
        case 8:
            BadgeRows[a0].f18 = BadgeRows[a0].fC + 2;
            BadgeRows[a0].f1A = (BadgeRows[a0].fE * rsin((s2 << 10) / 15)) / 4096;
            BadgeRows[a0].f14 = BadgeRows[a0].f8;
            BadgeRows[a0].f16 = BadgeRows[a0].fA + ((BadgeRows[a0].fE / 2) - (BadgeRows[a0].f1A / 2));
            if (s2 < 0xF) {
                BadgeRows[a0].f12++;
            } else {
                BadgeRows[a0].f10 = 3;
                BadgeRows[a0].f12 = 0;
            }
            break;
        }
        DrawBadgeRow(a0);
    }
}

int AllocRect16Rows(int a0) {
    int n = Rect16RowCount << 4;
    Rect16Rows = a0;
    PsyqBzero(a0, n);
    DebugPrintStub(StrSizeEqLdFmt, n);
    return n;
}

/* legacy typing of shared globals in this part of the unit */
#define Rect16Rows ((E_1 *)Rect16Rows)

void InitRect16Row(int i, short a, short b, short c, short d, short e, short f) {
    if (i < Rect16RowCount) {
        E_1 *p = (E_1 *)((i << 4) + (int)Rect16Rows);
        p->f0 = f;
        p->f2 = 0;
        p->f4 = a;
        p->f6 = b;
        p->f8 = c;
        p->fA = d;
        p->fC = e;
    }
}

#undef Rect16Rows

int UpdateRect16Row(int a0) {
    int lx[4];
    int ly[4];
    int lv[4];
    int r;
    int hh;
    int v;
    int k;
    int a1, a3;
    E_2 *e0, *e1, *e2, *e3, *e4, *e5;
    unsigned char *p;
    int *q;
    int *q2;
    int *xs, *ys;
    unsigned char *tb;

    v = a0 < Rect16RowCount;
    if (v != 0) {
        r = 0;
        e0 = EP;
        a1 = e0->st;
        a3 = e0->cnt;
        k = 3;
        q = &lv[3];
        do {
            *q = 0;
            k--;
            q--;
        } while (k >= 0);
        e1 = EP;
        lx[0] = e1->x;
        ly[0] = e1->y;
        lx[1] = e1->x + e1->w - 1;
        ly[1] = e1->y;
        lx[2] = e1->x;
        ly[2] = e1->y + e1->h - 1;
        lx[3] = e1->x + e1->w - 1;
        ly[3] = e1->y + e1->h - 1;
        switch (a1) {
        case 0:
            e2 = EP;
            e2->st = 1;
            e2->cnt = 0;
            break;
        case 5:
            r = 1;
            hh = 0x100;
            k = 3;
            q2 = &lv[3];
            do {
                *q2 = hh;
                k--;
                q2--;
            } while (k >= 0);
            break;
        case 4:
            lv[2] = 0x100;
        case 3:
            lv[1] = 0x100;
        case 2:
            lv[0] = 0x100;
        case 1:
            lv[a1 - 1] = (a3 << 8) / 6;
            if (a3 < 6) {
                e3 = EP;
                e3->cnt = e3->cnt + 1;
            } else {
                e4 = EP;
                e4->cnt = 0;
                e4->st = e4->st + 1;
            }
            break;
        case 8:
            e5 = EP;
            if (e5->s2 != 0) {
                e5->st = 1;
            }
            break;
        default:
            break;
        }
        k = 0;
        xs = lx;
        ys = ly;
        tb = Rect16EdgeShadeIndex;
        p = Rect16EdgeCornerPairs;
    loop:
        AddPartialLine(CurrentDrawBuf + 0x70, *(int *)((char *)xs + (p[0] << 2)), *(int *)((char *)ys + (p[0] << 2)), *(int *)((char *)xs + (p[1] << 2)), *(int *)((char *)ys + (p[1] << 2)), lv[*(unsigned char *)(k + (int)tb)], 1, 8, 8, 8, -1);
        p += 2;
        k++;
        if (k < 6) goto loop;
        v = r;
    }
    return v;
}

int AllocGlyphRows(int a) {
    int sz = GlyphRowCount * 28;
    GlyphRows = a;
    PsyqBzero(a, sz);
    DebugPrintStub(SizeLdFmt, sz);
    return sz;
}

/* legacy typing of shared globals in this part of the unit */
#define GlyphRows ((char *)GlyphRows)

void InitGlyphRow(a0, a1, a2, a3)
int a0;
int a1;
int a2;
int a3;
{
  char *s;
  char *new_var;
  char *q;
  if (a0 < GlyphRowCount)
  {
    *((char *) (((a0 * 0x1C) + ((int) GlyphRows)) + 2)) = a1;
    *((char *) (((a0 * 0x1C) + ((int) GlyphRows)) + 7)) = a2;
    s = GlyphRowTemplates + (a3 * 0x14);
    new_var = (char *) ((a0 * 0x1C) + ((int) GlyphRows));
    q = new_var;
    *((short *) (q + 8)) = *((short *) (s + 0));
    *((short *) (q + 0xA)) = *((short *) (s + 2));
    *((short *) (q + 0xC)) = *((short *) (s + 4));
    *((short *) (q + 0xE)) = *((short *) (s + 6));
    q[1] = s[0xE];
    *((char *) (((a0 * 0x1C) + ((int) GlyphRows)) + 5)) = s[0xF];
    *((char *) (((a0 * 0x1C) + ((int) GlyphRows)) + 6)) = s[0x10];
    q = (char *) ((a0 * 0x1C) + ((int) GlyphRows));
    *((short *) (q + 0x10)) = *((short *) (s + 8));
    *((short *) (q + 0x12)) = *((short *) (s + 0xA));
    q[0x14] = s[0xC];
    *((char *) (((a0 * 0x1C) + ((int) GlyphRows)) + 0x15)) = s[0xD];
  }
}

#undef GlyphRows
