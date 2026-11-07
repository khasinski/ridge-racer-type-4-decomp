#ifndef RR_OVERLAY662_CURSOR_CONTROL_H
#define RR_OVERLAY662_CURSOR_CONTROL_H
#include "rr/overlay662_input.h"
#include "rr/overlay662_actions.h"
#include "rr/overlay662_buttons.h"
#include "rr/overlay662_distance.h"
void garage_func_8011DA90(void);
void garage_func_8011DE70(void);
/* Flags1 disables input;2 animates to cursor14,4 operates an action,8 bypasses
 * animation/input activation. Animation progress is a signed word. Movement
 * wraps to signed16 before independent lower/upper clipping (also for empty
 * rectangles). Selected button and action indices must name valid initialized
 * owners. Services can change later state,indices and tables; both point
 * arguments to the distance helper are complete shared working-owner fields.
 * Snapshots and subsequent action callbacks use fresh post-service state. */
void garage_func_8011E070(void);
#endif
