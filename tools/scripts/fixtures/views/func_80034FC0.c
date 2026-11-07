#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define PlayerCars (*(S2 (*)[])&PlayerCars)

#define PlayerControllers (*(S1 (*)[])&PlayerControllers)

#undef PlayerCars

#undef PlayerControllers

/* legacy typing of shared globals in this part of the unit */
#define _sio_control ((unsigned int (*)())_sio_control)

#define PlayerControllers (*(T38 (*))&PlayerControllers)

#define LinkState (*(unsigned short (*))&LinkState)

#undef _sio_control

#undef PlayerControllers

#undef LinkState

/* legacy typing of shared globals in this part of the unit */
#define _sio_control ((unsigned (*)())_sio_control)

#undef _sio_control

extern short ModeId;

extern short ModeGroup;

extern short ModeSubState;

extern int _sio_control();

extern s16 LinkState;

int ExitOnLinkLost(void) {
    if (LinkState == -1) {
        ModeGroup = 1;
        ModeId = 0x2D;
        ModeSubState = 0;
        _sio_control(1, 1, 0);
        return 1;
    }
    return 0;
}
