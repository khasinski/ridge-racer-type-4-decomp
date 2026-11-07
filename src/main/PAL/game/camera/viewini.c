#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "game/gfx/model.h"
#include "game/gfx/poly1.h"
#include "rr/menu_overlay_runtime.h"
#include "rr/overlay668_camera.h"
#include "game/data/globals.h"

typedef struct { int a, b, c, d, e, f, g, h; } w3g6_32;

typedef struct { int x, y; } __attribute__((packed)) w3g6_U;

extern w3g6_32 D_800A3B00;
extern w3g6_U D_800A3B20[];
extern w3g6_U D_800A3B28[];
extern w3g6_U D_800A3B30[];
extern int CourseFarColorTable[];
extern w3g6_32 ColorMatrix;
extern short D_800AD5BC[];
extern w3g6_U D_800F38F8[];
extern int ViewUsesCarTrackNode;
extern char D_800F4C38[];
extern unsigned int *D_800ADAD0;
extern int RelFileViewRecords;
extern unsigned int D_8010E290[];
extern unsigned int *D_8010E690;

void InitViewLighting(void);
void BuildViewSegmentObjectList(void);
void func_8006E8F8();
void func_8006E9CC(void);
void func_8006E9D4(void);

void InitViewLighting(void)
{
    int *t;
    int *b;
    int i;
    ColorMatrix = D_800A3B00;
    D_800F38F8[0] = D_800A3B20[0];
        D_800F38F8[1] = D_800A3B28[0];
    D_800F38F8[2] = D_800A3B30[0];
    D_800AD5BC[0] = 0x1000;
    D_800AD5BC[1] = 0x1000;
    D_800AD5BC[2] = 0x1000;
    BackColorR = 0x13;
    BackColorG = 0x1D;
    BackColorB = 0x1C;
    b = CourseFarColorTable;
    i = CourseIndex;
    t = b + i * 3;
    FarColorR = t[0];
    FarColorG = t[1];
    FarColorB = t[2];
}

void InitViewState(void) {
    int i;
    char *p;
    char *g = (char *)SCRATCHPAD;
    InitViewLighting();
    ViewUsesCarTrackNode = 1;
    D_800AD5B8 = 0x1400;
    *(int *)(g + 0x78) = 0;
    *(short *)(g + 0x5E) = 0;
    if (CourseIndex == 7) {
        for (i = 0; i < 5; i++) {
            func_80070934(i, 6, 0, 0);
        }
    }
    i = 0x1F;
    p = D_800F4C38 + i;
    do {
        *p-- = 0;
        i--;
    } while (i >= 0);
}

void BuildViewSegmentObjectList(void) {
    unsigned int *t;
    unsigned int i;
    D_8010E690 = D_8010E290;
    t = GetSegmentViewTable();
    D_800ADAD0 = t;
    D_8010E290[0] = *t;
    if (D_8010E290[0] >= 0x100) {
        D_8010E290[0] = 0xFF;
    }
    t++;
    i = 0;
    if (D_8010E290[0] != 0) {
        do {
            D_8010E290[i + 1] = RelFileViewRecords + ((unsigned short *)t)[i] * 0x50;
            i++;
        } while (i < D_8010E290[0]);
    }
}

/* legacy typing of shared globals in this part of the unit */
#define D_800F4C38 (*(unsigned char (*)[])&D_800F4C38)

void func_8006E8F8() {
    int *t1;
    int v;
    int t0;
    unsigned int i;
    unsigned int t2;
    int off;
    unsigned char *pb;

    t1 = (int *)D_8010E690;
    t2 = *t1;
    off = t2 << 2;
    t1 = (int *)((char *)t1 + (off + 4));
    for (t0 = 0; t0 < D_800F4C38[0]; t0++) {
        v = RelFileViewRecords + (pb = D_800F4C38 + t0)[1] * 0x50;
        for (i = 0; i < *D_8010E690; i++) {
            if (v == D_8010E690[i + 1]) {
                break;
            }
        }
        if (i == *D_8010E690) {
            *t1 = v;
            t1++;
            t2++;
        }
    }
    *D_8010E690 = t2;
}

#undef D_800F4C38

void func_8006E9CC(void) {
}

void func_8006E9D4(void) {
    *(s32 *)(SCRATCHPAD + 0x128) = 0x1F;
}
