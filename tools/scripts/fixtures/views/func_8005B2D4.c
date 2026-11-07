#include "rr/game_motion.h"

#include "m2c_macros.h"

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(char (*)[])&RaceContext)

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define MotionController (*(int (*))&MotionController)

#undef MotionController

/* legacy typing of shared globals in this part of the unit */
#define PsyqBzero ((int (*)())PsyqBzero)

#undef PsyqBzero

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(struct { char pad[0x3D8]; struct Q_1 *q; } (*))&RaceContext)

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define MotionController ((void *)MotionController)

#undef MotionController

/* legacy typing of shared globals in this part of the unit */
#define MotionController ((void *)MotionController)

#undef MotionController

/* legacy typing of shared globals in this part of the unit */
#define MotionController ((void *)MotionController)

#undef MotionController

void func_8005B2D4(s32 arg0) {
    M2C_FIELD(MotionController, s32 *, 0x54) = arg0;
}
