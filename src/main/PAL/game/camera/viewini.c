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

extern w3g6_32 DefaultColorMatrix;
extern w3g6_U DefaultLightAngle0[];
extern w3g6_U DefaultLightAngle1[];
extern w3g6_U DefaultLightAngle2[];
extern int CourseFarColorTable[];
extern w3g6_32 ColorMatrix;
extern short LightDirMagnitude[];
extern w3g6_U LightAngles[];
extern int ViewUsesCarTrackNode;
extern char ViewSegmentIndices[];
extern unsigned int *SegmentViewTablePtr;
extern int RelFileViewRecords;
extern unsigned int ViewSegmentObjects[];
extern unsigned int *ViewSegmentObjectList;

void InitViewLighting(void);
void BuildViewSegmentObjectList(void);
void AddUniqueViewSegmentRecords();
void ViewInitEmptyStub(void);
void SetViewClipMask(void);

void InitViewLighting(void)
{
    int *t;
    int *b;
    int i;
    ColorMatrix = DefaultColorMatrix;
    LightAngles[0] = DefaultLightAngle0[0];
        LightAngles[1] = DefaultLightAngle1[0];
    LightAngles[2] = DefaultLightAngle2[0];
    LightDirMagnitude[0] = 0x1000;
    LightDirMagnitude[1] = 0x1000;
    LightDirMagnitude[2] = 0x1000;
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
    DefaultSegmentOctantValue = 0x1400;
    *(int *)(g + 0x78) = 0;
    *(short *)(g + 0x5E) = 0;
    if (CourseIndex == 7) {
        for (i = 0; i < 5; i++) {
            ConfigureTrackObjectParticles(i, 6, 0, 0);
        }
    }
    i = 0x1F;
    p = ViewSegmentIndices + i;
    do {
        *p-- = 0;
        i--;
    } while (i >= 0);
}

void BuildViewSegmentObjectList(void) {
    unsigned int *t;
    unsigned int i;
    ViewSegmentObjectList = ViewSegmentObjects;
    t = GetSegmentViewTable();
    SegmentViewTablePtr = t;
    ViewSegmentObjects[0] = *t;
    if (ViewSegmentObjects[0] >= 0x100) {
        ViewSegmentObjects[0] = 0xFF;
    }
    t++;
    i = 0;
    if (ViewSegmentObjects[0] != 0) {
        do {
            ViewSegmentObjects[i + 1] = RelFileViewRecords + ((unsigned short *)t)[i] * 0x50;
            i++;
        } while (i < ViewSegmentObjects[0]);
    }
}

/* legacy typing of shared globals in this part of the unit */
#define ViewSegmentIndices (*(unsigned char (*)[])&ViewSegmentIndices)

void AddUniqueViewSegmentRecords() {
    int *t1;
    int v;
    int t0;
    unsigned int i;
    unsigned int t2;
    int off;
    unsigned char *pb;

    t1 = (int *)ViewSegmentObjectList;
    t2 = *t1;
    off = t2 << 2;
    t1 = (int *)((char *)t1 + (off + 4));
    for (t0 = 0; t0 < ViewSegmentIndices[0]; t0++) {
        v = RelFileViewRecords + (pb = ViewSegmentIndices + t0)[1] * 0x50;
        for (i = 0; i < *ViewSegmentObjectList; i++) {
            if (v == ViewSegmentObjectList[i + 1]) {
                break;
            }
        }
        if (i == *ViewSegmentObjectList) {
            *t1 = v;
            t1++;
            t2++;
        }
    }
    *ViewSegmentObjectList = t2;
}

#undef ViewSegmentIndices

void ViewInitEmptyStub(void) {
}

void SetViewClipMask(void) {
    *(s32 *)(SCRATCHPAD + 0x128) = 0x1F;
}
