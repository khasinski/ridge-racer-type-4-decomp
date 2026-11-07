#ifndef RR_OVERLAY662_MODAL_H
#define RR_OVERLAY662_MODAL_H
#include "rr/overlay662_tool_action.h"
#include "rr/menu_text_state.h"
#include "rr/tables.h"
extern RrMenuItemState5C PlayerControllers[2];
extern s16 ModeId,ModeGroup,D_800F4D8A;
extern s32 ovl662_D_801272E8;
/* Negative modes reset the first option index. Modes1/2/4/8 configure
 * navigation; other positive modes preserve state. Mode0 enters or operates
 * the modal tool using two live UI rows and complete badge/text owners.
 * Services may replace those roots and mutate later selectors and input.
 * Modal state is captured after sound before publishing row states. */
void ovl662_func_8011E824(s32 mode);
#endif
