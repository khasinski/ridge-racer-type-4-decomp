#include "types.h"
#include "scratchpad.h"
#include "rr/game_motion.h"
#include "m2c_macros.h"
#include "game/demo/opening.h"
#include "game/camera/camrun.h"
#include "game/demo/opening.h"
#include "game/gfx/motion1.h"
#include "game/gfx/ring.h"
#include "game/camera/campath.h"
#include "rr/bios_devices.h"

typedef struct {
    s16 vx, vy, vz, p0;
    s32 sxy;
    s32 p1;
    s32 e0, e1, e2;
    s32 p2;
    s32 z0, z1, z2;
    s32 p3;
    s16 r0, r1, r2, p4;
    char m1[0x20];
    char m2[0x20];
    s32 fl;
    s32 dep;
} w3t1_F;

struct Q {
    char pad0[8]; int f8; char pad1[0x20 - 0xC]; unsigned char c20, c21, c22; char pad23; unsigned char c24, c25, c26; char pad27;
    unsigned char c28, c29, c2a; char pad2b; short h2c, h2e, h30; char pad32[2]; short h34, h36, h38; char pad3a[2]; short h3c, h3e, h40; char pad42[2];
    unsigned char c44, c45, c46; char pad47; int f48; char pad4c[0x60 - 0x4C]; int f60; short *f64; short f68, f6a; int f6c; int f70;
};

struct Q_1 { char pad[0x60]; int f60; int *f64; short f68, f6a; int f6c; int f70; };

extern u16 MarkerDiscBitmap16x16[];
extern s32 SetupObjectLightMatrices();
extern s32 ScaleMatrix();
extern s32 SetGteObjectTransform();
extern char MotionTrackTable[];
extern struct { char pad[0x250]; char pad2[0x3D8 - 0x250]; struct Q *q; } RaceContext;
extern short MotionCameraScript[];
extern int *MotionSequenceTable[];

void DrawMarkerBitmapGrid(void);
void InitMotionTrack(int a0);
void InitMotionController(void);
void RestartMotionSequence(void);
void SetMotionControllerVisible(s32 arg0);
void SetMotionControllerField50(s32 arg0);
void SetMotionControllerField58(s32 arg0);

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(char (*)[])&RaceContext)

void DrawMarkerBitmapGrid(void) {
    w3t1_F f;
    char *b = RaceContext;
    char *c;
    u16 *row;
    s32 i, j, x, y, v;
    c = *(char **)(b + 0x3D8);
    f.z0 = 0;
    f.z1 = 0;
    f.z2 = 0;
    f.r0 = *(u16 *)(c + 0x170);
    f.r1 = *(u16 *)(c + 0x172);
    f.r2 = *(u16 *)(c + 0x174);
    SetupObjectLightMatrices(&f.z0, c + 0x20, f.m1, f.m2);
    v = *(s32 *)(*(char **)(b + 0x3D8) + 0x178);
    f.e2 = v;
    f.e1 = v;
    f.e0 = v;
    ScaleMatrix(f.m1, &f.e0);
    SetGteObjectTransform((SCRATCHPAD + 0x7C), &f.z0, f.m1);
    row = MarkerDiscBitmap16x16; i = 0; y = -0x438;
    do {
        j = 0;
        x = -0x438;
        do {
            if (*row & (0x8000 >> j)) {
                f.vx = x;
                f.vy = y;
                f.vz = 0;
                __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(&f.vx));
                __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001");
                __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(&f.sxy) : "memory");
                __asm__ volatile ("swc2 $8, 0(%0)\n\tcfc2 $12, $31\n\tnop\n\tsw $12, 0(%0)" : : "r"(&f.fl) : "$12", "memory");
                __asm__ volatile ("mfc2 $12, $19\n\tnop\n\tsra $12, $12, 2\n\tsw $12, 0(%0)" : : "r"(&f.dep) : "$12", "memory");
                f.dep = f.dep >> 3;
                EmitProjectedMarkerSprt(((s16 *)&f.sxy)[0], ((s16 *)&f.sxy)[1], f.dep);
            }
            j++;
            x += 0x90;
        } while (j < 16);
        y += 0x90;
        i++;
        row++;
    } while (i < 16);
    AddDrawModeTagToOt();
}

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define MotionController (*(int (*))&MotionController)

void InitMotionTrack(int a0) {
    int *p = (int *)(MotionController + 0x15C);
    PsyqBzero(p, 0x2C);
    p[1] = 0;
    p[3] = (int)(MotionTrackTable + (a0 << 5));
    p[7] = 0x1000;
}

#undef MotionController

void UpdateMotionTrack(RrMotionClock20 *clock) {
    s16 counter, next;
    s32 start, code, delta, units;
    s32 rate;
    register s32 velocity __asm__("$4");
    u32 tempo;
    RrMotionKey32 *key;
    RrMotionKey32 *finishKey;
    RrMotionKey32 *restKey;
    register RrMotionKey32 *scaleKey __asm__("$4");
    RrMotionTrack44 *track;
    RrMotionControllerPrefix *controller;

    controller = MotionController;
    track = &controller->track;
    if (track->disabled == 0) {
        switch (controller->track.state) {
        case 0:
            if (clock->active != 0) {
                controller->track.position = track->key->from;
                track->scale = track->key->fromScale;
                code = track->key->timeCode;
                units = (code >> 8) * 12 + (code & 255);
                tempo = clock->tempo;
                /* Capture the clock before doubling the decoded time. */
                __asm__("" :: "r"(tempo));
                units *= 2;
                track->elapsed = 0;
                track->duration = (s16)((tempo * (u32)units) / 1200U);
                controller->track.state = (s32)((u32)controller->track.state + 1U);
                /* Fall through: the first interpolation step is immediate. */
        case 1:
                key = track->key;
                counter = track->elapsed;
                track->position.x = (u16)key->from.x +
                    ((key->to.x - key->from.x) * counter) / track->duration;
                track->position.y = (u16)key->from.y +
                    ((key->to.y - key->from.y) * counter) / track->duration;
                track->position.z = (u16)key->from.z +
                    ((key->to.z - key->from.z) * counter) / track->duration;
                scaleKey = key;
                delta = scaleKey->toScale;
                start = scaleKey->fromScale;
                delta = (s32)(((u32)delta - (u32)start) *
                              (u32)(s32)track->elapsed) / track->duration;
                next = (u16)track->elapsed + 1;
                track->elapsed = next;
                track->scale = (s32)((u32)delta + (u32)start);
                finishKey = key;
                if (track->duration < next) {
                    /* Keep the first destination write before the key reload. */
                    __asm__("" : "+r"(finishKey));
                    track->position.x = (u16)finishKey->to.x;
                    restKey = key;
                    __asm__("" : "+r"(restKey) :: "memory");
                    track->position.y = (u16)restKey->to.y;
                    track->position.z = (u16)restKey->to.z;
                    track->scale = restKey->toScale;
                    if (restKey->continueAfter == 0) {
                        track->state = 3;
                        return;
                    }
                    track->state = 2;
                    track->velocity = 800;
                    return;
                }
            } else {
                return;
            }
            break;
        case 2:
            /* Publish scale, damped velocity, then x/z/y in wire order. */
            velocity = track->velocity;
            rate = (s32)((u32)velocity << 5);
            {
                s32 current = track->scale;
                s32 carry = velocity;
                __asm__("" :: "r"(current));
                __asm__("" : "+r"(carry) : "r"(rate));
                rate = (s32)((u32)rate - (u32)velocity);
                track->scale = (s32)((u32)current + (u32)carry);
            }
            if (rate < 0) {
                rate += 31;
            }
            {
                u16 x, y, z;
                s32 damped;
                x = (u16)track->position.x;
                __asm__("" : "+r"(rate), "+r"(x));
                damped = rate >> 5;
                __asm__("" :: "r"(rate), "r"(damped));
                track->velocity = damped;
                z = (u16)track->position.z;
                track->position.x = x + 16;
                y = (u16)track->position.y;
                track->position.z = z + 8;
                __asm__("" :: "r"(y));
                track->position.y = y + 8;
            }
            break;
        }
    }
}

/* legacy typing of shared globals in this part of the unit */
#define PsyqBzero ((int (*)())PsyqBzero)

void InitMotionController(void) {
    RaceContext.q = (struct Q *)((char *)&RaceContext + 0x250);
    PsyqBzero(RaceContext.q, 0x188);
    RaceContext.q->f8 = -0x1388;
    RaceContext.q->f48 = 0x1B8;
    RaceContext.q->h2c = 0x1C1;
    RaceContext.q->h2e = -0xDF8;
    RaceContext.q->h30 = -0x79A;
    RaceContext.q->c20 = 0xC0;
    RaceContext.q->c21 = 0xC0;
    RaceContext.q->c22 = 0xC0;
    RaceContext.q->h34 = -0x64;
    RaceContext.q->h36 = 0;
    RaceContext.q->h38 = 0x64;
    RaceContext.q->c24 = 0xC0;
    RaceContext.q->c25 = 0xC0;
    RaceContext.q->c26 = 0xC0;
    RaceContext.q->h3c = -0xA4D;
    RaceContext.q->h3e = -0x642;
    RaceContext.q->h40 = 0xA86;
    RaceContext.q->c28 = 0x20;
    RaceContext.q->c29 = 0x20;
    RaceContext.q->c2a = 0x20;
    RaceContext.q->c44 = 0;
    RaceContext.q->c45 = 0;
    RaceContext.q->c46 = 0;
    RaceContext.q->f64 = MotionCameraScript;
    RaceContext.q->f6c = RaceContext.q->f64[RaceContext.q->f68 * 4];
    RaceContext.q->f70 = RaceContext.q->f64[RaceContext.q->f68 * 4 + 1];
    switch (RaceContext.q->f6c) {
    case 0:
        InitCameraMotion(RaceContext.q->f70);
        break;
    case 1:
        InitMotionControllerPanel(RaceContext.q->f70);
        break;
    case 2:
        InitScrollTrack(RaceContext.q->f70);
        break;
    case 3:
        InitMotionTrack(RaceContext.q->f70);
        break;
    }
}

#undef PsyqBzero

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(struct { char pad[0x3D8]; struct Q_1 *q; } (*))&RaceContext)

void RestartMotionSequence(void) {
    RaceContext.q->f68 = 0;
    RaceContext.q->f6a = 0;
    RaceContext.q->f60 = 0;
    RaceContext.q->f64 = MotionSequenceTable[MotionSequenceIndex];
    RaceContext.q->f6c = ((short *)RaceContext.q->f64)[RaceContext.q->f68 * 4];
    RaceContext.q->f70 = ((short *)RaceContext.q->f64)[RaceContext.q->f68 * 4 + 1];
    switch (RaceContext.q->f6c) {
    case 0:
        InitCameraMotion(RaceContext.q->f70);
        break;
    case 1:
        InitMotionControllerPanel(RaceContext.q->f70);
        break;
    case 2:
        InitScrollTrack(RaceContext.q->f70);
        break;
    case 3:
        InitMotionTrack(RaceContext.q->f70);
        break;
    }
}

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define MotionController ((void *)MotionController)

void SetMotionControllerVisible(s32 arg0) {
    M2C_FIELD(MotionController, s32 *, 0x54) = arg0;
}

#undef MotionController

/* legacy typing of shared globals in this part of the unit */
#define MotionController ((void *)MotionController)

void SetMotionControllerField50(s32 arg0) {
    M2C_FIELD(MotionController, s32 *, 0x50) = arg0;
}

#undef MotionController

/* legacy typing of shared globals in this part of the unit */
#define MotionController ((void *)MotionController)

void SetMotionControllerField58(s32 arg0) {
    M2C_FIELD(MotionController, s32 *, 0x58) = arg0;
}

#undef MotionController
