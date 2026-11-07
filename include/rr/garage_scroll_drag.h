#ifndef RR_GARAGE_SCROLL_DRAG_H
#define RR_GARAGE_SCROLL_DRAG_H
#include "rr/garage_image_drag.h"
/* Saved vertical drag metrics: image thumb/travel and track thumb/travel.
 * They are separate original scalar owners at27280..2728C. */
extern s32 garage_vscroll_image_thumb,garage_vscroll_image_travel,garage_vscroll_track_thumb,garage_vscroll_track_travel;
extern u16 garage_vscroll_grab_offset; /* Grab offset within the projected image thumb. */
extern s16 garage_logo_cursor_y; /* Real alias of working.points.cursor14.y. */
/* Starting requires zoom>=4 and a hit in216,87,4,224. Services may change
 * state;after the clip service zoom must remain4..255. Active updates require
 * previously initialized metrics: image travel32..64,track travel112..223;
 * grab offset is an unsigned halfword. These bounds keep products signed32
 * and divisions nonzero. Release restores clip13,63,294,346 before returning.
 * Vertical scroll narrows to signed16 before clamping to0..imageTravel. */
void garage_drag_vscrollbar(void);
/* Horizontal peer uses hit90,337,112,8 and tool46. Its saved image travel
 * is32..64 and track travel56..112;other service/narrowing rules are shared. */
extern u16 garage_hscroll_grab_offset;
extern s32 garage_hscroll_image_thumb,garage_hscroll_image_travel,garage_hscroll_track_thumb,garage_hscroll_track_travel;
extern s16 garage_logo_cursor_x; /* Alias of working.points.cursor14.x. */
void garage_drag_hscrollbar(void);
#endif
