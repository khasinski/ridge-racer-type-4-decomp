#include "m2c_macros.h"

#define I(o) (*(int *)(a0 + (o)))

#define U(o) (*(unsigned short *)(a0 + (o)))

#define S(o) (*(short *)(a0 + (o)))

void ScaleMatrixRow1(s32 arg0, void *arg1) {
    M2C_FIELD(arg1, s16 *, 6) = (s16) (M2C_FIELD(arg1, s16 *, 6) * arg0);
    M2C_FIELD(arg1, s16 *, 8) = (s16) (M2C_FIELD(arg1, s16 *, 8) * arg0);
    M2C_FIELD(arg1, s16 *, 0xA) = (s16) (M2C_FIELD(arg1, s16 *, 0xA) * arg0);
}
