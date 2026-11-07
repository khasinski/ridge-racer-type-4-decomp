/* RODATA 0x80010488 0x1c */
#include "types.h"
#include "rr/race.h"
#include "m2c_macros.h"
#include "game/gfx/gtepoly1.h"
#include "game/race/raceinit.h"
#include "game/car/carbound.h"
#include "rr/audio_loop.h"
#include "rr/race_setup.h"
#include "rr/bios_devices.h"
#include "rr/matrix_workspace.h"
#include "game/race/racesamp.h"
#include "game/data/selection.h"

#define I(o) (*(int *)(a0 + (o)))
#define U(o) (*(unsigned short *)(a0 + (o)))
#define S(o) (*(short *)(a0 + (o)))

typedef struct { char pad[0x1E4]; unsigned short f1E4; short f1E6; short f1E8; } T;

typedef struct {
    unsigned short halfW00, pad02, halfL04;
    s16 height06;
} RrVehicleBounds08;

struct V { int a, b, c, d; };

typedef struct { int a, b, c, d; } V4;

extern T *CarTable[];
extern short NumCars;
extern short D_800AD508;
extern int DrawPlayerCar();
extern int DrawOpponentCar();
extern RrVehicleBounds08 *CarBoundsTable[];
extern int BuildRotMatrixY();
extern int BuildRotMatrixX();
extern int BuildRotMatrixZ();
extern int MulMatrix();
extern int ApplyMatrix();

void DrawCars();
void SetCarBounds(RrRaceVehicle *a);
void ResetCarState(RrRaceVehicle *a);
void func_8002E5A0();
void func_8002E828(s32 arg0, void *arg1);

void DrawCars(a0) int a0; {
    int z; 
    int i, s2, y;
    T **p;
    D_800ABEC4 = 0;
    switch (a0) {
    case 0:
        z = 0;
        if (!(CarTable[0]->f1E4 & 0x100)) {
            DrawPlayerCar(CarTable[0], z);
        } else {
            __asm__("" : "=r"(y) : "0"(z)); /* ASM_BARRIER: hide that z==0 from cse so the else-arm copies a1 (move a2,a1) while the then-arm keeps the single hoisted a1=0 */
            DrawWheelTrailEffects(CarTable[0], y, y, y, 0, 0, 0);
        }
    case 1:
        if (RaceType != 4) {
            for (i = 1; i < NumCars; i++) {
                if (CarTable[i]->f1E8 != -1) DrawOpponentCar(CarTable[i], a0);
            }
        } else {
            if (D_80107178 == 2) {
                { char *bb = (char *)CarTable; DrawOpponentCar(*(T **)(bb + (D_800AD508 << 3)), a0); }
            }
        }
        break;
    case 3:
    case 5:
        D_800ABEC4 = 1;
    case 2:
    case 4:
        s2 = 3;
        if (RaceType == 4) s2 = 4;
        if (D_80107178 == 2) s2 = 0;
        if (NumCars > 0) {
            i = 0;
            p = CarTable;
            do {
                if (p[0]->f1E8 != -1) {
                    if (a0 - 2 == i) {
                        z = 0;
                        if (!(p[0]->f1E4 & 0x100)) {
                            DrawPlayerCar(p[0], z);
                        } else {
                            __asm__("" : "=r"(y) : "0"(z)); /* ASM_BARRIER: hide that z==0 from cse so the else-arm copies a1 (move a2,a1) while the then-arm keeps the single hoisted a1=0 */
                            DrawWheelTrailEffects(p[0], y, y, y, p[0]->f1E6 + D_800ABEC4, 0, 0);
                        }
                    } else {
                        DrawOpponentCar(p[0], s2);
                    }
                }
                p++;
                i++;
            } while (i < NumCars);
        }
        break;
    case 6:
        if (NumCars > 0) {
            i = 0;
            do {
                if (CarTable[i]->f1E8 != -1) DrawOpponentCar(CarTable[i], 2);
                i++;
            } while (i < NumCars);
        }
        break;
    }
}

void SetCarBounds(RrRaceVehicle *a)
{
    int n;
    RrVehicleBounds08 **pp;
    int b;
    if (D_800AD508 != 0) {
        n = a != PlayerCars;
    } else {
        n = 3;
        if (a == PlayerCars) {
            n = 2;
        }
    }
    a->slotIndex1E6 = n * 2;
    b = (int)CarBoundsTable;
    pp = (RrVehicleBounds08 **)(b + n * 4);
    a->height108 = (*pp)->height06 * 4;
    a->width10C = ((short)(*pp)->halfW00 + 5) * 2;
    a->halfSpan10E = 0x28 - (*pp)->halfL04;
    a->corner110 = -(((*pp)->halfW00 + 5) * 4);
    a->corner112 = -(((*pp)->halfL04 - 0x14) * 4);
    a->corner114 = ((*pp)->halfW00 + 5) * 4;
    a->corner116 = -(((*pp)->halfL04 - 0x14) * 4);
    a->corner118 = -(((*pp)->halfW00 + 5) * 4);
    a->corner11A = -0x50;
    a->corner11C = ((*pp)->halfW00 + 5) * 4;
    a->corner11E = -0x50;
}

void ResetCarState(RrRaceVehicle *a)
{
    int *p = (int *)a;
    PsyqBzero(a, sizeof(RrRaceVehicle));
    SetCarBounds(a);
    a->ang80 = (unsigned short)a->rot50;
    a->ang82 = (unsigned short)a->rot54;
    a->ang84 = (unsigned short)a->rot58;
    *(struct V *)a->ir70 = *(struct V *)a->pos20;
    p[0] = p[8] << 8;
    p[1] = p[9] << 8;
    p[2] = p[10] << 8;
    *(struct V *)a->savedRotC8 = *(struct V *)&a->rot50;
}

void func_8002E5A0(a0) char *a0; {
    struct { int m1[8]; int m2[8]; short v[3]; } L;
    int t1, t2;
    unsigned short u1, u2, u3;
    I(0x10) = I(0x20) * 4;
    I(0x14) = I(0x24) * 4;
    I(0x18) = I(0x28) * 4;
    U(0x80) = U(0x50);
    U(0x82) = U(0x54);
    U(0x84) = U(0x58);
    BuildRotMatrixY(L.m1, 0x800 - I(0x54));
    BuildRotMatrixX(L.m2, I(0x50));
    MulMatrix(L.m1, L.m2);
    BuildRotMatrixZ(L.m2, I(0x58));
    MulMatrix2(L.m1, L.m2);
    L.v[0] = 0;
    L.v[1] = -U(0x108);
    L.v[2] = (S(0x10E) * 4) / 3;
    ApplyMatrix(L.m2, L.v, a0 + 0x40);
    BuildRotMatrixY(L.m1, 0x800 - S(0x82));
    BuildRotMatrixX(L.m2, S(0x80));
    MulMatrix(L.m1, L.m2);
    BuildRotMatrixZ(L.m2, S(0x84));
    MulMatrix2(L.m1, L.m2);
    L.v[0] = 0;
    L.v[1] = (S(0xE8) + S(0xEA)) >> 7;
    L.v[2] = 0;
    ApplyMatrix(L.m2, L.v, a0 + 0x70);
    L.v[0] = 0;
    L.v[1] = -((S(0xE0) + S(0xE2)) >> 3);
    L.v[2] = 0;
    ApplyMatrix(L.m2, L.v, a0 + 0x60);
    I(0x70) = I(0x70) + I(0x10);
    I(0x74) = I(0x74) + I(0x14);
    I(0x78) = I(0x78) + I(0x18);
    U(0x80) = U(0x50);
    U(0x82) = U(0x54);
    U(0x84) = U(0x58);
    I(0x14) = I(0x14) + I(0x44);
    I(0x10) = I(0x10) + I(0x40);
    U(0x80) = U(0x80) + U(0x1AC);
    U(0x84) = U(0x84) + U(0x1AE);
    I(0x74) = I(0x74) + I(0x44);
    I(0x70) = I(0x70) + I(0x40);
    I(0x18) = I(0x18) + I(0x48);
    I(0x78) = I(0x78) + I(0x48);
    *(V4 *)(a0 + 0xC8) = *(V4 *)(a0 + 0x10);
}

void func_8002E828(s32 arg0, void *arg1) {
    M2C_FIELD(arg1, s16 *, 6) = (s16) (M2C_FIELD(arg1, s16 *, 6) * arg0);
    M2C_FIELD(arg1, s16 *, 8) = (s16) (M2C_FIELD(arg1, s16 *, 8) * arg0);
    M2C_FIELD(arg1, s16 *, 0xA) = (s16) (M2C_FIELD(arg1, s16 *, 0xA) * arg0);
}

void func_8002E868(a0, a1, a2) int a0; int a1; unsigned char *a2; {
    register RrRaceVehicle *p asm("$7"); /* ASM_BARRIER: pin p to a3 */
    int t;
    int v;
    int b;
    register int c asm("$3"); /* ASM_BARRIER: pin c to v1 */
    register int r asm("$4"); /* ASM_BARRIER: pin r to a0 */
    p = (RrRaceVehicle *)a0; /* ASM_BARRIER: pin p to a3 */
    r = 0;
    if (RaceType == 1) {
        a2 = SelectionRows;
        t = p->flags1E4 & 7;
        c = a2[0];
        r = t < 4;
        b = a2[t * 4];
        r ^= 1;
        if (c == b) {
            p->slotIndex1E6 = r;
        } else if (a2[0x14] == b) {
            v = r | 2; p->slotIndex1E6 = v;
        } else if (a2[0x18] == b) {
            v = r | 4; p->slotIndex1E6 = v;
        } else if (a2[0x1C] == b) {
            v = r | 6; p->slotIndex1E6 = v;
        }
    } else if (RaceType == 5) {
        int u = p->flags1E4;
        if ((u & 7) >= 5) r = 1;
        v = (((u - 1) & 3) << 1) | r; p->slotIndex1E6 = v;
    } else if (RaceType == 6) {
        p->slotIndex1E6 = 2;
    } else {
        int w = (short)p->flags1E4 - 1;
        p->slotIndex1E6 = (w % 2 + 2) << 1;
    }
    v = 0x100; p->maxSpeed2EC = v;
    PlaceCarOnGrid(p, ((short)p->flags1E4 - 1) & 7, a2, p);
}
