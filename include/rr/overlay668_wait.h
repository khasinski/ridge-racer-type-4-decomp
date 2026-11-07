#ifndef RR_OVERLAY668_WAIT_H
#define RR_OVERLAY668_WAIT_H
#include "rr/overlay668_control.h"

/* Pointer cells for MDEC status and output DMA control. Access the hardware
 * words through volatile-qualified lvalues; the pointer cells are ordinary RAM. */
extern u32 *movie_playback_D_80116E54;
extern u32 *movie_playback_D_80116E34;
extern char movie_playback_D_801148DC[];
extern char movie_playback_D_801148EC[];
s32 movie_playback_func_801168A4(const char *message);
#endif
