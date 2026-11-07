#include "types.h"
extern u32 *movie_playback_mdec_ctrl_reg;
u32 movie_playback_mdec_status(void) { return *movie_playback_mdec_ctrl_reg; }
