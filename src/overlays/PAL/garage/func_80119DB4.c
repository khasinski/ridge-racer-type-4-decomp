#include "rr/garage_transition.h"

void garage_enter_color_select(void) {
    garage_init_color_select();
    ModeGroup = 1;
    ModeId = 0x46;
    ModeSubState = 0;
    UpdateScreenFade(0x10002);
}
