#ifndef RR_OVERLAY662_ACTION_LOOP_H
#define RR_OVERLAY662_ACTION_LOOP_H
#include "rr/overlay662_actions.h"
#include "rr/overlay662_buttons.h"
/* The existing callback-only view is the first member of each complete record.
 * A pointer to it can be converted back to its genuine containing record. */
typedef struct {
    RrGarageActionPrefix prefix;
    RrGarageStep render;
    s32 argument,key;
} RrGarageActionRecord16;
extern RrGarageActionRecord16 garage_action_records[53];
void garage_draw_color_panel(void);
/* Nonzero mode installs53 action and46 button pointers by their bounded keys.
 * Mode0 requires all53 entries to address complete records with both callbacks.
 * A dispatched single action requires working slot4 to be0..52.
 * After step callbacks, config1E
 * must be0..4; table slots4/9/11/12/14 point at complete button owners.
 * Callbacks may replace later entries and change working flags or render mode.
 * The render-enable byte is checked once, before all53 render callbacks. */
void garage_update_actions(s32 mode);
RR_ASSERT_SIZE(RrGarageActionRecord16,16);
RR_ASSERT_OFFSET(RrGarageActionRecord16,prefix,0);
RR_ASSERT_OFFSET(RrGarageActionRecord16,render,4);
RR_ASSERT_OFFSET(RrGarageActionRecord16,argument,8);
RR_ASSERT_OFFSET(RrGarageActionRecord16,key,12);
#endif
