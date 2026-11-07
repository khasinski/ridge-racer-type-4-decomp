#ifndef RR_OVERLAY662_SCROLL_DRAG_H
#define RR_OVERLAY662_SCROLL_DRAG_H
#include "rr/overlay662_image_drag.h"
/* Saved vertical drag metrics: image thumb/travel and track thumb/travel.
 * They are separate original scalar owners at27280..2728C. */
extern s32 garage_D_80127280,garage_D_80127284,garage_D_80127288,garage_D_8012728C;
extern u16 garage_D_8012727E; /* Grab offset within the projected image thumb. */
extern s16 garage_D_8012767E; /* Real alias of working.points.cursor14.y. */
/* Starting requires zoom>=4 and a hit in216,87,4,224. Services may change
 * state;after the clip service zoom must remain4..255. Active updates require
 * previously initialized metrics: image travel32..64,track travel112..223;
 * grab offset is an unsigned halfword. These bounds keep products signed32
 * and divisions nonzero. Release restores clip13,63,294,346 before returning.
 * Vertical scroll narrows to signed16 before clamping to0..imageTravel. */
void garage_func_80123F90(void);
/* Horizontal peer uses hit90,337,112,8 and tool46. Its saved image travel
 * is32..64 and track travel56..112;other service/narrowing rules are shared. */
extern u16 garage_D_80127268;
extern s32 garage_D_8012726C,garage_D_80127270,garage_D_80127274,garage_D_80127278;
extern s16 garage_D_8012767C; /* Alias of working.points.cursor14.x. */
void garage_func_80123CC8(void);
#endif
