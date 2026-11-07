#include "psyq/libgte/gtemat7.h"
#include "game/gfx/gfxmtx.h"
#include "game/menu/mnrows.h"
#include "game/ui/uidraw.h"
#include "rr/course_choices.h"
#include "rr/badge_rows.h"
#include "rr/bios_devices.h"
#include "rr/audio_cd_controller.h"
typedef struct { unsigned short a, b, c, d; } W8;

typedef struct { unsigned short a, b, c; } W6;

typedef struct { unsigned char a, b, c, d; } B4;

extern unsigned char D_8009F924[];
extern B4 D_8009F928[];
extern W8 D_8009F934[];
extern W6 LightMatrix[];
extern char *TrackLightTable;

void SetLightColorsScaled(int a0);
void SetLightColorsFromRow();
int func_8003D108();
int AllocBadgeRows(int a0);

void SetLightColorsScaled(int a0)
{
    short sp[3][3];
    int pad[2];
    int i;
    int x;

    for (i = 0; i < 3; i++) {
        LightMatrix[i].a = D_8009F934[i].a;
        LightMatrix[i].b = D_8009F934[i].b;
        LightMatrix[i].c = D_8009F934[i].c;
        sp[0][i] = D_8009F928[i].a << 4;
        sp[1][i] = D_8009F928[i].b << 4;
        sp[2][i] = D_8009F928[i].c << 4;
    }
    x = D_8009F924[0] * a0;
    {
        int y = D_8009F924[1] * a0;
        x >>= 8;
        y >>= 8;
        SetBackColor(x, y, (D_8009F924[2] * a0) >> 8);
    }
    SetColorMatrix(sp);
}

/* legacy typing of shared globals in this part of the unit */
#define LightMatrix (*(unsigned short (*)[])&LightMatrix)

void SetLightColorsFromRow(a0) int a0;
{
    short sp[14];
    short *s0;
    int i = 0;
    char *base;
    unsigned short *a3 = LightMatrix;
    unsigned char *a1;
    unsigned char *a2;
    base = TrackLightTable;
    a1 = (unsigned char *)(a0 * 0x2C + (int)base);
    a2 = a1;
L:
    {
        short *v;
        s0 = sp;
        v = (short *)((int)s0 + i * 2);
        i++;
        a3[0] = *(unsigned short *)(a2 + 0x10);
        a3[1] = *(unsigned short *)(a2 + 0x12);
        a3[2] = *(unsigned short *)(a2 + 0x14);
        a2 += 8;
        v[0] = a1[4] << 4;
        v[3] = a1[5] << 4;
        v[6] = a1[6] << 4;
        a3 += 3;
        a1 += 4;
    }
    if (i < 3) goto L;
    {
        char *b2 = TrackLightTable;
        unsigned char *q = (unsigned char *)(a0 * 0x2C + (int)b2);
        SetBackColor(q[0x28], q[0x29], q[0x2A], a3);
    }
    SetColorMatrix(s0);
}

#undef LightMatrix

int func_8003D108(a0) int a0;
{
    int r = 3;
    int i;
    int n;
    char *p;
    if (a0 & 1) {
        i = 0;
        n = BadgeRowCount;
        if (n > 0) {
            int c = 0xB;
            int m = n;
            p = BadgeRows;
            do {
                short t = *(short *)(p + 0x10);
                if (t != 0 && t != c) {
                    r &= 0xFFFE;
                    break;
                }
                i++;
                p += 0x1C;
            } while (i < m);
        }
    }
    if (a0 & 2) {
        i = 0;
        n = ImageRowCount;
        if (n > 0) {
            int c = 5;
            int m = n;
            a0 = (int)ImageRows;
            do {
                signed char t = *(char *)(a0 + 1);
                if (t != 0) {
                    i++;
                    if (t != c) {
                        r &= 0xFFFD;
                        break;
                    }
                } else i++;
                a0 += 0x14;
            } while (i < m);
        }
    }
    return r;
}

/* legacy typing of shared globals in this part of the unit */
#define BadgeRows (*(int (*))&BadgeRows)

int AllocBadgeRows(int a0) {
    int n = BadgeRowCount * 28;
    BadgeRows = a0;
    PsyqBzero(a0, n);
    DebugPrintStub(StrSizeEqLdFmt, n);
    return n;
}

#undef BadgeRows
