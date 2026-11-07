#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define LinkRoleSlot (*(short (*))&LinkRoleSlot)

#undef LinkRoleSlot

/* legacy typing of shared globals in this part of the unit */
#define LinkEntryState (*(char (*)[])&LinkEntryState)

#undef LinkEntryState

/* legacy typing of shared globals in this part of the unit */
#define LinkRoleSlot (*(s16 (*))&LinkRoleSlot)

#undef LinkRoleSlot

/* legacy typing of shared globals in this part of the unit */
#define ModeFrame (*(unsigned int (*))&ModeFrame)

#define ModeId (*(unsigned short (*))&ModeId)

#define ModeSubState (*(unsigned short (*))&ModeSubState)

#undef ModeFrame

#undef ModeId

#undef ModeSubState

/* legacy typing of shared globals in this part of the unit */
#define ModeSubState (*(unsigned short (*))&ModeSubState)

#define ModeFrame (*(unsigned int (*))&ModeFrame)

#undef ModeSubState

#undef ModeFrame

/* legacy typing of shared globals in this part of the unit */
#define ModeSubState (*(unsigned short (*))&ModeSubState)

#undef ModeSubState

/* legacy typing of shared globals in this part of the unit */
#define ModeFrame (*(u32 (*))&ModeFrame)

#undef ModeFrame

/* legacy typing of shared globals in this part of the unit */
#define SelectionState (*(struct w3g2s (*))&SelectionState)

#undef SelectionState

extern int PadClearPulse();

extern int PadRumbleStop();

void StopRaceRumble(void) {
    PadRumbleStop(0);
    PadRumbleStop(1);
    PadClearPulse(0);
    PadClearPulse(1);
}
