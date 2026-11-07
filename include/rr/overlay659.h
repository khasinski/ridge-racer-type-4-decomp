#ifndef RR_OVERLAY_659_H
#define RR_OVERLAY_659_H

#include "rr/menu_overlay_runtime.h"

extern u8 race_results_finish_rank_limits[4];
extern s32 race_results_result_labels[34];
extern s32 race_results_preview_timer;

void race_results_update_race(void);
void race_results_update_results(void);
void race_results_rank_standings(void);
void race_results_init_preview(void);
void race_results_update_preview(void);
void race_results_exit_race(void);
void race_results_init_results(void);

#endif
