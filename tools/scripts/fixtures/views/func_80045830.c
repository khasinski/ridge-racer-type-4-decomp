#include "m2c_macros.h"

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(Context650 (*))&RaceContext)

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(int (*)[])&RaceContext)

#define D_800AC6B0 (*(char (*)[])&D_800AC6B0)

#define D_800AC6D8 (*(int (*))&D_800AC6D8)

#define CurrentDrawBuf ((char *)CurrentDrawBuf)

#undef RaceContext

#undef D_800AC6B0

#undef D_800AC6D8

#undef CurrentDrawBuf

extern int PsyqBzero();

extern M2C_UNK D_800AC6D8;

void func_80045830(void) {
    PsyqBzero(&D_800AC6D8, 0x20);
    M2C_FIELD(&D_800AC6D8, s32 *, 0x1C) = -1;
}
