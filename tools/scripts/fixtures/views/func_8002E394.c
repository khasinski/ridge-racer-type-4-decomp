#include "rr/race.h"

#define I(o) (*(int *)(a0 + (o)))

#define U(o) (*(unsigned short *)(a0 + (o)))

#define S(o) (*(short *)(a0 + (o)))

typedef struct {
    unsigned short halfW00, pad02, halfL04;
    s16 height06;
} RrVehicleBounds08;

extern short LinkRoleSlot;

extern RrVehicleBounds08 *CarBoundsTable[];

void SetCarBounds(RrRaceVehicle *a)
{
    int n;
    RrVehicleBounds08 **pp;
    int b;
    if (LinkRoleSlot != 0) {
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
