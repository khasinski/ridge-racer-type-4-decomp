#ifndef RR_MOVIE_PLAYBACK_WAIT_H
#define RR_MOVIE_PLAYBACK_WAIT_H
#include "rr/movie_playback_control.h"

/* Pointer cells for MDEC status and output DMA control. Access the hardware
 * words through volatile-qualified lvalues; the pointer cells are ordinary RAM. */
extern u32 *movie_playback_mdec_ctrl_reg;
extern u32 *movie_playback_mdec_out_dma_chcr;
extern char movie_playback_mdec_in_sync_name[];
extern char movie_playback_mdec_out_sync_name[];
s32 movie_playback_mdec_timeout(const char *message);
#endif
