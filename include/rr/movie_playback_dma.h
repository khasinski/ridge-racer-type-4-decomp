#ifndef RR_MOVIE_PLAYBACK_DMA_H
#define RR_MOVIE_PLAYBACK_DMA_H
#include "rr/movie_playback_wait.h"

extern u32 *movie_playback_dma_dpcr; /* DMA priority/control */
extern u32 *movie_playback_mdec_in_dma_bcr; /* input block count */
extern u32 *movie_playback_mdec_in_dma_chcr; /* input channel control */
extern u32 *movie_playback_mdec_out_dma_bcr; /* output block count */
extern u32 *movie_playback_mdec_cmd_reg; /* MDEC command */
/* The address registers hold pointers; their width is the target pointer width. */
extern const void *volatile *movie_playback_mdec_in_dma_madr;
extern void *volatile *movie_playback_mdec_out_dma_madr;
/* Native tests override accesses while PAL retains volatile hardware lvalues. */
#ifndef RR_STR_READ32
#define RR_STR_READ32(port) (*(volatile u32 *)(port))
#endif
#ifndef RR_STR_WRITE32
#define RR_STR_WRITE32(port,value) (*(volatile u32 *)(port)=(value))
#endif
#endif
