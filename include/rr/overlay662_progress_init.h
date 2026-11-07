#ifndef RR_OVERLAY662_PROGRESS_INIT_H
#define RR_OVERLAY662_PROGRESS_INIT_H
#include "rr/overlay_menu_init.h"
#include "rr/save_progress.h"
/* Resident save prefix: the sixteen progress words start at offset10.
 * Only bits0..19 of each word contribute to these menu counts. */
extern u16 garage_D_80127328[4][4];
extern u16 garage_D_801272D8[4],garage_D_801272FC;
void garage_func_80114998(void);
#endif
