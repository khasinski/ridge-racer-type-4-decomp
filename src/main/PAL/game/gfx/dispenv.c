#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "rr/overlay662_distance.h"

struct S {
    char p0[0x1C];
    int f1C, f20, f24, f28;
    char p2C[0x0C];
    int f38, f3C, f40, f44;
};

extern int GetTrackPointAhead();
extern struct S RaceContext;
/* same symbol under a 2nd C name: cse cannot reuse the earlier address, so the else arm re-materialises lui/addiu like the original */
extern struct S D_800AC650b __asm__("RaceContext");
extern int RaceContextCameraDistance;
extern int RaceContextCameraDistanceFlag;
extern int InterpolateLanePointXYZ();
extern int TrackNodes;
extern int TrackNodeCount;
extern void ApplyMatrix();
extern void RotMatrixZYX();
extern void *TrackEntries;
extern int SetSceneColorParams();
extern int SetSceneOverlayEnabled();
extern int SceneTintPresetTable;
extern char CarSlotBuffers[];

int UpdateCameraTargetDistance();
void GetTrackLaneWorldPos();
void SetCamAnglesRotateNegZ(short *a, short b, int c);
void RotateNegZByEulerZYX(short *a, short b, int c);
short LerpAngleWrap4096(short *p, int t);
s32 CountTrackEntries(void);
void ApplySceneTintPreset(int a0);

int UpdateCameraTargetDistance(a0, a1)
char *a0;
int *a1;
{
    int d[3];
    int sp20[4];
    int s;
    struct S *q;
    int t;
    d[0] = (RaceContext.f20 / 4 - a1[0]) / 4;
    d[1] = (RaceContext.f24 / 4 - a1[1]) / 4;
    d[2] = (RaceContext.f28 / 4 - a1[2]) / 4;
    s = CordicSqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]) * 2;
    if (RaceContext.f1C != 0) {
        t = s >> 2;
        RaceContext.f3C = t;
        RaceContext.f38 = s - t;
        RaceContext.f44 = 0;
        RaceContext.f40 = s;
        if (s > 0xFFFF) {
            if (*(short *)(a0 + 0x1EA) != 0) goto L;
        } else {
L:
            RaceContext.f44 = 1;
        }
        if (*(short *)((char *)a1 + 0x32) - *(int *)(a0 + 0xF0) >= 0) RaceContextCameraDistanceFlag = 1;
    }
    t = s - RaceContext.f3C;
    if (RaceContext.f44 == 0) {
        if (t < 0) {
            RaceContext.f44 = 1;
        } else {
            q = &D_800AC650b;
            if (q->f40 - s < 0) q->f44 = 1;
        }
    }
    GetTrackPointAhead(a0, sp20, a1[5]);
    return RaceContextCameraDistance = s;
}

void GetTrackLaneWorldPos(a0, a1, a2) char *a0; int *a1; int *a2; {
    int d;
    int q2;
    int r;
    int t;
    int j;
    d = (*(int *)(a0 + 0xF0) << 10) + *(short *)(a0 + 0x170) - a2[2];
    t = d;
    if (d < 0) t = d + 0x3FF;
    r = t >> 10;
    q2 = r;
    r = d - (q2 << 10);
    d = q2;
    if (d < 0) d += TrackNodeCount;
    j = d + 1;
    if (TrackNodeCount < j) j -= TrackNodeCount;
    InterpolateLanePointXYZ(TrackNodes + d * 60, TrackNodes + j * 60, a2[0], r, a1);
    a1[0] = a1[0] << 2;
    a1[2] = a1[2] << 2;
    a1[1] = a1[1] + (a2[1] << 2);
}

void SetCamAnglesRotateNegZ(short *a, short b, int c) {
    int m[8];
    unsigned short v[3];
    short r[3];
    int *s = (int *)SCRATCHPAD;
    s[6] = -a[0];
    s[7] = a[1];
    s[8] = a[2];
    v[0] = a[0];
    v[1] = a[1];
    v[2] = 0;
    r[0] = 0;
    r[1] = 0;
    r[2] = -b;
    RotMatrixZYX(v, m);
    ApplyMatrix(m, r, c);
}

void RotateNegZByEulerZYX(short *a, short b, int c) {
    int m[8];
    unsigned short v[3];
    short r[3];
    v[0] = a[0];
    v[1] = a[1];
    v[2] = 0;
    r[0] = 0;
    r[1] = 0;
    r[2] = -b;
    RotMatrixZYX(v, m);
    ApplyMatrix(m, r, c);
}

short LerpAngleWrap4096(short *p, int t) {
    int n = p[0x24 / 2] * 5;
    int v;
    if (n > 0 && t < n) {
        v = (p[0x22 / 2] - p[0x20 / 2]) * t / n + p[0x20 / 2];
    } else {
        v = p[0x22 / 2];
    }
    return v - (v / 4096 * 4096);
}

s32 CountTrackEntries(void) {
    s32 var_a1;
    void *var_a0;

    var_a0 = TrackEntries;
    var_a1 = 0;
    if (M2C_FIELD(var_a0, s16 *, 0x32) != -1) {
        do {
            var_a0 += 0x44;
            var_a1 += 1;
        } while (M2C_FIELD(var_a0, s16 *, 0x32) != -1);
    }
    return var_a1;
}

void ApplySceneTintPreset(int a0) {
    int s2, s0, x, s1, v1, y;
    int *v0;
    s1 = SceneTintPresetTable;
    s2 = 0;
    s0 = -1;
    a0 = ((unsigned)a0 >> 4) & 0xFF;
    /* ASM_BARRIER: legacy, reason not recorded (forces reg copy/CSE/scheduling); retire & retest with fd.py */
    __asm__("" : "=r"(x) : "0"(a0));
    if (a0 < 8) {
        s0 = (a0 - 1) % 4;
    }
    if (x >= 5) {
        s2 = 1;
    }
    if (s0 >= 0) {
        SetSceneOverlayEnabled(1, x);
        s1 += s0 * 8;
        SetSceneColorParams(*(int *)s1, *(unsigned char *)(s1 + 4), *(unsigned char *)(s1 + 5), *(unsigned char *)(s1 + 6));
        v1 = 7;
    } else {
        SetSceneOverlayEnabled(0, x);
        v1 = 7;
    }
    v0 = (int *)CarSlotBuffers;
    v0 = (int *)((char *)v0 + 0x578);
    do {
        v0[0x70 / 4] = s2;
        v1--;
        v0 = (int *)((char *)v0 - 0xC8);
    } while (v1 >= 0);
}
