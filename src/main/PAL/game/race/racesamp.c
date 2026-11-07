/* RODATA 0x80010700 0x1c */
#include "types.h"
#include "game/car/car.h"
#include "game/car/grid.h"
#include "scratchpad.h"
#include "rr/track_sample_types.h"
#include "m2c_macros.h"
#include "game/car/carmove1.h"
#include "game/menu/menu8.h"
#include "game/course/trkintp.h"
#include "game/ui/uipart1.h"
#include "game/car/carparam.h"
#include "game/car/carsetup.h"
#include "rr/audio_loop.h"
#include "game/car/aipath2.h"
#include "game/data/selection.h"
#include "game/gfx/drawbuf.h"

typedef struct {
    int a, b, c, d;
} Vec4;

typedef struct {
    unsigned short w0;
    short w2;
    unsigned short w4;
    short w6;
} Bounds08;

typedef struct {
    int f0;
    int f4;
    int f8;
    char pad0C[0x20 - 0x0C];
    int f20;
    int f24;
    int f28;
    int f2C;
    char pad30[0x50 - 0x30];
    int f50;
    int f54;
    int f58;
    char pad5C[0x70 - 0x5C];
    Vec4 vec70;
    short s80;
    short s82;
    short s84;
    char pad86[2];
    struct {
        int f0;
        int f4;
        int f8;
        int fC;
    } sub88[4];
    char padC8[0xDC - 0xC8];
    short sDC;
    short sDE;
    short sE0;
    short sE2;
    short sE4;
    short sE6;
    short sE8;
    short sEA;
    char padEC[4];
    int fF0;
    int fF4;
    short sF8;
    short sFA;
    short sFC;
    short sFE;
    char pad100[8];
    int f108;
    short s10C;
    short s10E;
    short s110;
    short s112;
    short s114;
    short s116;
    short s118;
    short s11A;
    short s11C;
    short s11E;
    short s120;
    short s122;
    short arr124[16];
    short arr144[16];
    unsigned short s164;
    unsigned short s166;
    unsigned short s168;
    unsigned short s16A;
    int f16C;
    char pad170[0x18C - 0x170];
    int f18C;
    int f190;
    char pad194[0x1A8 - 0x194];
    short s1A8;
    short s1AA;
    short s1AC;
    short s1AE;
    short s1B0;
    short s1B2;
    short s1B4;
    short s1B6;
    char pad1B8[4];
    short arr1BC[4];
    short arr1C4[4];
    char pad1CC[0x1D4 - 0x1CC];
    short s1D4;
    short s1D6;
    char pad1D8[0x1E4 - 0x1D8];
    short s1E4;
    short s1E6;
    char pad1E8[0x290 - 0x1E8];
    int f290;
} Car37618;

extern int SetModelTexturePage();
extern int InitBadgeRow();
extern int InitRect16Row();
extern int InitImageRow();
extern s32 MaskBitSetting;
extern int AddDrawModePrim();
extern int AddSpritePrim();
extern unsigned char D_8009F7CC[];
extern unsigned char D_8009F7D0[];
extern unsigned char D_8009F7D4[];
extern unsigned char D_8009F7D8[];
extern unsigned char D_8009F7DC[];
extern struct { char pad[0x1D2]; short h1D2; char pad2[0x1F]; unsigned char b1F3; } *CarRecordPtrs[];
extern Car *CarTable[];
extern Car PlayerCars[];
extern Car D_80106B58[];
extern Car D_80106E78[];
extern Car ComCars[];
extern short NumCars;
extern short LinkRoleSlot;
extern int FindTrackNode();
extern int Atan2Table();
extern int FindWheelTrackNodes();
extern short ReverseCourse;
extern struct {
    char pad[0x53C];
    Grid53C *p53C;
} RaceContext;
extern Bounds08 *CarBoundsTable[];

void InitPreviewImageRows(void);
void DrawSelectionIconGrid(int arg0);
void InitCarTable(int a);
void PlaceCarOnGrid(Car37618 *p, int arg1);

void InitPreviewImageRows(void) {
    SetModelTexturePage(0x19);
    MaskBitSetting = 0;
    InitBadgeRow(0, 0x10, 0x158, 0x120, 0x3C, 0, 0x103);
    InitRect16Row(0, 0x10, 0x48, 0x120, 0x120, 0, 0);
    InitImageRow(0, -1, 0, 1);
}

void DrawSelectionIconGrid(int arg0) {
    int v[9];
    char buf[4];
    int s0;
    int a1;
    int b;
    unsigned char *t7 = D_8009F7CC;
    unsigned char (*q)[4] = SelectionRows;
    unsigned char *p = q[arg0];
    v[0] = t7[p[1]];
    v[1] = D_8009F7D0[p[1]];
    v[2] = CarRecordPtrs[arg0]->h1D2 > 0 ? 0x18 : 0x19;
    v[3] = D_8009F7D4[p[0]];
    v[4] = D_8009F7D8[p[0]];
    v[5] = D_8009F7DC[p[0]];
    v[6] = CarRecordPtrs[arg0]->b1F3 + 0x14;
    b = p[3];
    v[7] = b + 0x1A;
    v[8] = b + 0x1C;
    if (p[2] == 0x27) {
        v[5] = 0x1E;
        v[4] = 0x1E;
        v[3] = 0x1E;
        v[1] = 0x1E;
        v[0] = 0x1E;
    }
    if (p[3] == 2) {
        v[8] = 0x1E;
        v[7] = 0x1E;
    }
    s0 = 0;
    a1 = *(int *)SCRATCHPAD;
    buf[2] = 0x10;
    buf[3] = 0x20;
    do {
        int t = v[s0];
        buf[0] = (t % 16) * 16;
        buf[1] = (t / 16) << 5;
        a1 = AddSpritePrim(CurrentDrawBuf + 0x70, a1, (s0 % 3) * 0x14 + 0xF0, (s0 / 3) * 0x28 + 0x58, buf, 0x7FA4, 1);
        s0++;
    } while (s0 < 9);
    *(int *)SCRATCHPAD = AddDrawModePrim(CurrentDrawBuf + 0x70, a1, 0x39);
}

void InitCarTable(int a) {
    int i;
    int n;
    int v;
    Car **p;
    Car *q;
    switch (a) {
    case 1:
        NumCars = 8;
        i = 0;
        for (; i < NumCars; i++) {
            if (i == 0) {
                CarTable[0] = PlayerCars;
            } else {
                CarTable[i] = D_80106E78 + i;
            }
        }
        break;
    case 2:
        NumCars = 2;
        CarTable[0] = PlayerCars;
        CarTable[1] = PlayerCars + 1;
        break;
    case 3:
        i = 2;
        CarTable[0] = PlayerCars;
        CarTable[1] = PlayerCars + 1;
        v = NumCars;
        if (i < v) {
            n = v;
            do {
                CarTable[i] = D_80106B58 + i;
                i++;
            } while (i < n);
        }
        break;
    case 4:
        NumCars = 4;
        if (LinkRoleSlot == 1) {
            CarTable[0] = PlayerCars;
            CarTable[1] = PlayerCars + 1;
            CarTable[2] = ComCars;
            CarTable[3] = ComCars + 1;
        } else {
            CarTable[2] = PlayerCars;
            CarTable[3] = PlayerCars + 1;
            CarTable[0] = ComCars;
            CarTable[1] = ComCars + 1;
        }
        break;
    case 5:
        NumCars = 7;
        i = 0;
        n = 7;
        for (; i < n; i++) {
            CarTable[i] = ComCars + i;
        }
        break;
    default:
    case 0:
        CarTable[0] = PlayerCars;
        NumCars = 1;
        break;
    case 6:
        CarTable[0] = PlayerCars;
        CarTable[1] = ComCars;
        NumCars = 2;
        break;
    }
    ResetRankEntriesForCars();
}

void PlaceCarOnGrid(Car37618 *p, int arg1) {
    int i;
    int t;
    int *g = (int *)(SCRATCHPAD + 0x7C);

    ResetVehicleShake();
    if (RaceType == 0 && ReverseCourse == 1) {
        p->fF0 = *TrackStartNode;
        t = TrackNodeCount - 5;
        p->f20 = TrackNodes[(p->fF0 + t) % TrackNodeCount].x08;
        p->f24 = 0;
        p->f28 = TrackNodes[(p->fF0 + t) % TrackNodeCount].z0C;
    } else {
        p->f20 = RaceContext.p53C->pos[arg1].x;
        p->f24 = 0;
        p->f28 = RaceContext.p53C->pos[arg1].z;
        p->fF0 = RaceContext.p53C->f0;
    }
    p->f0 = p->f20 << 8;
    p->f4 = 0;
    p->f8 = p->f28 << 8;
    p->fF0 = FindTrackNode(&p->f20, p->fF0, TrackNodeCount + 1);
    p->sF8 = p->sFA = p->sFC = p->sFE = p->fF0;
    p->f50 = 0;
    p->f54 = ((0x800 - TrackNodes[p->fF0].heading12) - (ReverseCourse << 11)) & 0xFFF;
    p->f58 = 0;
    p->s1AE = 0;
    p->s1D4 = p->s1D6 = p->f54;
    if (p->s1E4 >= 0x10) {
        p->f290 = p->s1D4;
    }
    i = p->s1E6 >> 1;
    p->f108 = CarBoundsTable[i]->w6 * 4;
    p->s10C = ((short)CarBoundsTable[i]->w0 + 5) * 2;
    p->s10E = 0x28 - CarBoundsTable[i]->w4;
    p->s110 = -((CarBoundsTable[i]->w0 + 5) * 4);
    p->s112 = -((CarBoundsTable[i]->w4 - 0x14) * 4);
    p->s114 = (CarBoundsTable[i]->w0 + 5) * 4;
    p->s116 = -((CarBoundsTable[i]->w4 - 0x14) * 4);
    p->s118 = -((CarBoundsTable[i]->w0 + 5) * 4);
    p->s11A = -0x50;
    p->s11C = (CarBoundsTable[i]->w0 + 5) * 4;
    p->s11E = -0x50;
    p->s120 = 0x400 - Atan2Table(p->s114 * 2, p->s116 + 0x50);
    p->s122 = 0x400 - Atan2Table(p->s11C * 2, p->s11E - p->s116);
    p->f16C = 0x10;
    p->sDC = p->sDE = 0;
    p->sE0 = p->sE2 = 0;
    p->sE4 = p->sE6 = p->f108 << 6;
    p->sE8 = p->sEA = p->f108 << 6;
    p->s80 = p->f50;
    p->s82 = p->f54;
    p->s84 = p->f58;
    p->vec70 = *(Vec4 *)&p->f20;
    p->s1AC = p->s1A8 = p->s1B4 = p->s1B0 = 0;
    p->s1AE = p->s1AA = p->s1B6 = p->s1B2 = 0;
    p->s164 = p->s166 = 0;
    for (i = 0; i < 10; i++) {
        p->s164 += p->f50;
        p->s166 += p->f58;
    }
    p->s168 = p->s16A = 0;
    for (i = 0; i < 10; i++) {
        p->s168 += p->f50;
        p->s16A += p->f58;
    }
    for (i = 0; i < 16; i++) {
        p->arr124[i] = p->f50;
        p->arr144[i] = p->f58;
    }
    for (i = 0; i < 4; i++) {
        p->sub88[i].f4 = p->f24;
        p->arr1BC[i] = 0;
        p->arr1C4[i] = 0;
    }
    ComputeStartDistance(p, 0);
    p->f190 = p->f18C;
    IntegrateCarMotion(p);
    ComputeWheelOffsets(p);
    ConstrainCarToTrack(p);
    *g = 0;
    FindWheelTrackNodes(p);
    UpdateWheelBumps(p);
}
