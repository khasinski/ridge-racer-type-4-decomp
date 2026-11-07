#include "scratchpad.h"
#include "game/car/carmove1.h"
#include "game/car/carparam.h"
#include "game/car/carsetup.h"
#include "rr/overlay662_distance.h"
#include "game/car/aipath.h"
#include "game/car/carphys2.h"
#include "game/car/aipath2.h"
#include "game/car/carsteer.h"
extern int AngleDiffSigned();
extern int FindWheelTrackNodes();
extern int Atan2Table();
extern int rsin();
extern int rcos();
extern int ViewUsesCarTrackNode;
extern struct w3g2o { int pad0[4]; int f10, f14, f18, f1C; char pad1[0x10]; int f30; int pad34; int f38; char pad2[0x18]; int f54; } PlayerCars;
typedef struct { int a, b, c, d; } w3g2V;
struct w3g2q { int pad0[4]; int f10, f14, f18, f1C; char pad1[0x10]; int f30; int pad34; int f38; char pad2[0x18]; int f54;
  char pad3[0x1D4 - 0x58]; short s1D4, s1D6; short s1D8; short s1DA, s1DC, s1DE; unsigned short u1E0; short s1E2, s1E4, s1E6, s1E8, s1EA; };

void func_80033808(struct w3g2q *p) {
    int sp[3];
    int v, t, u, v2, v3, v4;
    int *g = (int *)SCRATCHPAD;
    int *q = (int *)(SCRATCHPAD + 0x7C);
    ViewUsesCarTrackNode = 0;
    u = p->s1D8;
    p->s1D8 = (unsigned short)p->s1D8 - (u * 6) / 100;
    p->s1D8 = p->s1D8 + p->u1E0;
    v = p->s1D8 * rsin(p->s1D4 + (p->s1EA << 11));
    if (v < 0) v += 0xFF;
    p->f30 = v >> 8;
    v2 = p->s1D8 * rcos(p->s1D4 + (p->s1EA << 11));
    if (v2 < 0) v2 += 0xFF;
    p->f38 = v2 >> 8;
    SteerCarTowardTrack(p);
    t = AngleDiffSigned(p->f54, p->s1D4);
    *q = 0;
    p->f54 = p->f54 + t / 2;
    func_8002679C(p);
    UpdateCarSteering(p);
    IntegrateCarMotion(p);
    UpdateCarTrackNode(p);
    ComputeWheelOffsets(p);
    ConstrainCarToTrack(p);
    FindWheelTrackNodes(p);
    UpdateWheelBumps(p);
    v3 = rsin(PlayerCars.f54);
    if (v3 < 0) v3 += 0x7F;
    sp[0] = PlayerCars.f10 + (v3 >> 7) - g[2];
    sp[1] = PlayerCars.f14 - g[3];
    v4 = rcos(PlayerCars.f54);
    if (v4 < 0) v4 += 0x7F;
    sp[2] = PlayerCars.f18 + (v4 >> 7) - g[4];
    *(w3g2V *)(SCRATCHPAD + 0x8) = *(w3g2V *)&p->f10;
    g[3] -= 0x100;
    g[7] = 0x400 - Atan2Table(sp[0], sp[2]);
    g[6] = 0x400 - Atan2Table(sp[1], CordicSqrt(sp[0] * sp[0] + sp[2] * sp[2]) >> 6);
    g[8] = 0;
}
