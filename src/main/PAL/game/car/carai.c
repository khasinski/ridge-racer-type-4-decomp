#include "scratchpad.h"
#include "rr/track_projection.h"
#include "rr/race.h"
#include "game/car/carspec3.h"
#include "game/race/racedist.h"
#include "game/car/carai.h"
#include "game/course/trkproj.h"
#include "rr/track_lateral.h"
#include "rr/vehicle_drive.h"

typedef struct { char p0[0x54]; unsigned short s54, s56; char p1[4]; short s5C; unsigned short s5E; short s60; } W;
typedef struct { char p0[0x1E4]; short s1E4; } V;
extern int RaceStandings;
extern char ComCars[];
extern int SoundStateRaceTick;
extern int RaceState;
extern struct { char pad[0x34]; int s34; } SoundState;

extern int UpdateComCarLaneChange();
extern int UpdateComCarSpeed();
extern int PsyqRand();

void UpdateComCars(void) {
    int i;
    char *s0;
    W *s1;
    /* ASM_BARRIER: pin rand() results to $v0 as in the original */
    register int r __asm__("$2");
    int q;

    RaceStandings = (SCRATCHPAD + 0x7C);
    UpdateRaceStandings();
    switch (RaceState) {
    case 0:
        SoundStateRaceTick = 0;
        break;
    case 1:
        SoundState.s34 = SoundState.s34 + 1;
        break;
    }
    for (i = 0; i < 0xB; i++) {
        s0 = ComCars + i * 0x320;
        s1 = (W *)(s0 + 0x1F4);
        if (((V *)s0)->s1E4 > 0) {
            CalcDistanceToPlayer((RrRaceVehicle *)s0);
            if (RaceState < 2) {
                if (RaceState == 0) {
                    r = PsyqRand();
                    q = r / 1000;
                    s1->s5C = r - q * 1000 + 0x1D4C;
                    r = PsyqRand(q);
                    s1->s54 = 1;
                    s1->s56 = 0x400;
                    s1->s60 = 0;
                    s1->s5E = r % 100 + 0x190;
                } else if (s1->s54 == 1) {
                    /* ASM_BARRIER: pin the sum to $v0 like the original */
                    register int u __asm__("$2");
                    /* ASM_BARRIER: pin the masked sum to $v1 like the original (s5C load goes to $v0) */
                    register int m __asm__("$3");
                    u = s1->s56;
                    u += s1->s5E;
                    s1->s56 = u;
                    /* ASM_BARRIER: keep the sh before the andi as in original */
                    __asm__ volatile("" : "=r"(u) : "0"(u));
                    m = u & 0xFFFF;
                    /* ASM_BARRIER: keep andi before the s5C load as in original */
                    __asm__ volatile("" : "=r"(m) : "0"(m));
                    if (s1->s5C < m) {
                        s1->s54 = 0;
                        r = PsyqRand();
                        q = r / 1000;
                        s1->s5C = r - q * 1000 + 0x157C;
                        r = PsyqRand(q);
                        s1->s5E = r % 100 + 0xC8;
                    }
                } else {
                    s1->s56 = s1->s56 - s1->s5E;
                    if (s1->s56 < s1->s5C) {
                        s1->s54 = 1;
                        r = PsyqRand();
                        q = r / 1000;
                        s1->s5C = r - q * 1000 + 0x1D4C;
                        r = PsyqRand(q);
                        s1->s5E = r % 100 + 0x190;
                    }
                }
            } else {
                ApplyRubberBandSpec(s0);
                UpdateComCarCurrentLane(s0);
                UpdateComCarOvertake(s0);
                UpdateComCarTargetPoint(s0);
                UpdateComCarDrive(s0);
                UpdateComCarLaneChange(s0);
                UpdateComCarSpeed(s0);
            }
        }
    }
}
