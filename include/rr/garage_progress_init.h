#ifndef RR_GARAGE_PROGRESS_INIT_H
#define RR_GARAGE_PROGRESS_INIT_H
#include "rr/overlay_menu_init.h"
#include "rr/save_progress.h"
/* Resident save prefix: the sixteen progress words start at offset10.
 * Only bits0..19 of each word contribute to these menu counts. */
extern u16 garage_team_maker_car_counts[4][4];
extern u16 garage_team_car_counts[4],garage_total_car_count;
void garage_init_main_menu(void);
#endif
