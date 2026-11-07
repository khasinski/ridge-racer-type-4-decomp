#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define D_800AD508 (*(short (*))&D_800AD508)

#undef D_800AD508

/* legacy typing of shared globals in this part of the unit */
#define LinkEntryState (*(char (*)[])&LinkEntryState)

#undef LinkEntryState

/* legacy typing of shared globals in this part of the unit */
#define D_800AD508 (*(s16 (*))&D_800AD508)

#undef D_800AD508

/* legacy typing of shared globals in this part of the unit */
#define ModeFrame (*(unsigned int (*))&ModeFrame)

#define ModeId (*(unsigned short (*))&ModeId)

#define D_800F4D8A (*(unsigned short (*))&D_800F4D8A)

#undef ModeFrame

#undef ModeId

#undef D_800F4D8A

/* legacy typing of shared globals in this part of the unit */
#define D_800F4D8A (*(unsigned short (*))&D_800F4D8A)

#define ModeFrame (*(unsigned int (*))&ModeFrame)

#undef D_800F4D8A

#undef ModeFrame

/* legacy typing of shared globals in this part of the unit */
#define D_800F4D8A (*(unsigned short (*))&D_800F4D8A)

#undef D_800F4D8A

/* legacy typing of shared globals in this part of the unit */
#define ModeFrame (*(u32 (*))&ModeFrame)

#undef ModeFrame

/* legacy typing of shared globals in this part of the unit */
#define SelectionState (*(struct w3g2s (*))&SelectionState)

#undef SelectionState

extern int SoundSetMasterVolume();

extern s32 MaskBitSetting;

void StopRaceAudioAndRumble(void) {
    SoundSetMasterVolume(0, 0);
    StopRaceRumble();
    MaskBitSetting = 0;
}
