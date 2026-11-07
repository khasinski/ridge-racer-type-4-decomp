#ifndef RR_GARAGE_CONFIRM_H
#define RR_GARAGE_CONFIRM_H
#include "rr/garage_input.h"
#include "rr/garage_tool_action.h"
#include "rr/menu_text_state.h"
#include "rr/menu_transition.h"
extern s16 MenuResultCode;
/* Active confirmation reads first badge.state10 and text status. Sound may
 * replace text/badge roots; acceptance writes the fresh owners afterward. */
void garage_load_dialog(s32 mode);
#endif
