#ifndef RR_GARAGE_CURSOR_CONTROL_H
#define RR_GARAGE_CURSOR_CONTROL_H
#include "rr/garage_input.h"
#include "rr/garage_actions.h"
#include "rr/garage_buttons.h"
#include "rr/garage_distance.h"
void garage_update_cursor_glide(void);
void garage_update_button_state(void);
/* Flags1 disables input;2 animates to cursor14,4 operates an action,8 bypasses
 * animation/input activation. Animation progress is a signed word. Movement
 * wraps to signed16 before independent lower/upper clipping (also for empty
 * rectangles). Selected button and action indices must name valid initialized
 * owners. Services can change later state,indices and tables; both point
 * arguments to the distance helper are complete shared working-owner fields.
 * Snapshots and subsequent action callbacks use fresh post-service state. */
void garage_update_cursor(void);
#endif
