#include "rr/overlay662_transition.h"

void garage_update_editor_intro(void) {
    garage_draw_animated_panels();
    garage_add_editor_draw_env();
    garage_update_editor_overlays();
    {
    union {u32 bits;s32 value;} count;
    count.bits=(u32)garage_logo_state.word04+1U;
    garage_logo_state.word04=count.value;
    if (count.value >= 28) {
        ModeGroup = 3;
        ModeId = 5;
        ModeSubState = 0;
    }
    }
}
