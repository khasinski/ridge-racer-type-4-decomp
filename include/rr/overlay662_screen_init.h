#ifndef RR_OVERLAY662_SCREEN_INIT_H
#define RR_OVERLAY662_SCREEN_INIT_H
#include "rr/menu_arenas.h"
#include "rr/menu_setup.h"
#include "rr/menu_transition.h"
#include "rr/overlay_menu.h"
#include "rr/renderer_setup.h"
extern s16 garage_D_801272D4;
void InitPreviewImageRows(void);
/* Loader and layout callbacks may replace the published arenas. */
void garage_init_car_select(void);
#endif
