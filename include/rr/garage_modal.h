#ifndef RR_GARAGE_MODAL_H
#define RR_GARAGE_MODAL_H
#include "rr/garage_tool_action.h"
#include "rr/menu_text_state.h"
#include "rr/tables.h"
extern RrMenuItemState5C PlayerControllers[2];
extern s16 ModeId,ModeGroup,ModeSubState;
extern s32 garage_returned_from_logo_editor;
/* Negative modes reset the first option index. Modes1/2/4/8 configure
 * navigation; other positive modes preserve state. Mode0 enters or operates
 * the modal tool using two live UI rows and complete badge/text owners.
 * Services may replace those roots and mutate later selectors and input.
 * Modal state is captured after sound before publishing row states. */
void garage_car_color_dialog(s32 mode);
#endif
