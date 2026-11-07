#ifndef RR_AUDIO_PAN_H
#define RR_AUDIO_PAN_H

#include "rr/audio_loop.h"

extern s16 PanGainTable[128];
extern s16 SoundGain; /* Alias of SoundState.gain18. */
extern u16 D_800F4DDC, D_800F4DDE, D_800F4DEA;
s32 SsUtKeyOn(s32 bank,s32 program,s32 mode,s32 note,
                 s32 fine,s32 left,s32 right);
/* Bank selection uses the full program word; helpers receive signed low16.
 * Pitch addition wraps in32 bits. Both helper calls reuse captured parameters. */
void TriggerSfx(s32 program,s32 level,s32 pan,s32 pitch,s32 voice);

#endif
