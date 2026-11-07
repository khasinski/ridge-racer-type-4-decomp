#ifndef RR_OVERLAY668_DMA_H
#define RR_OVERLAY668_DMA_H
#include "rr/overlay668_wait.h"

extern u32 *movie_playback_D_80116E58; /* DMA priority/control */
extern u32 *movie_playback_D_80116E24; /* input block count */
extern u32 *movie_playback_D_80116E28; /* input channel control */
extern u32 *movie_playback_D_80116E30; /* output block count */
extern u32 *movie_playback_D_80116E50; /* MDEC command */
/* The address registers hold pointers; their width is the target pointer width. */
extern const void *volatile *movie_playback_D_80116E20;
extern void *volatile *movie_playback_D_80116E2C;
/* Native tests override accesses while PAL retains volatile hardware lvalues. */
#ifndef RR_STR_READ32
#define RR_STR_READ32(port) (*(volatile u32 *)(port))
#endif
#ifndef RR_STR_WRITE32
#define RR_STR_WRITE32(port,value) (*(volatile u32 *)(port)=(value))
#endif
#endif
