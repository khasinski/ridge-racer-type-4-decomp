#include "rr/race.h"

#define I(o) (*(int *)(a0 + (o)))

#define U(o) (*(unsigned short *)(a0 + (o)))

#define S(o) (*(short *)(a0 + (o)))

struct V { int a, b, c, d; };

extern int PsyqBzero();

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
