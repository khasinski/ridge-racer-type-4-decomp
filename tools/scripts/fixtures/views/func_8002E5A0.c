#include "types.h"

#define I(o) (*(int *)(a0 + (o)))

#define U(o) (*(unsigned short *)(a0 + (o)))

#define S(o) (*(short *)(a0 + (o)))

typedef struct { int a, b, c, d; } V4;

extern int BuildRotMatrixY();

extern int BuildRotMatrixX();

extern int BuildRotMatrixZ();

extern int MulMatrix();

extern int MulMatrix2();

extern int ApplyMatrix();

void PrepareCarDrawPose(a0) char *a0; {
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
