#include "rr/overlay662_garage.h"
#include "rr/tables.h"

extern RrMenuItemState5C PlayerControllers[];
s32 garage_step_button_repeat(s32 mode);

/* Repeat-key handling for the two garage scroll directions. */
void garage_update_button_state(void)
{
    s32 first;
    s32 second;

    if (PlayerControllers[0].type01 == 0x23) {
        first = PlayerControllers[0].buttons02 & 0x840;
        second = PlayerControllers[0].buttons02 & 0x80;
    } else if (PlayerControllers[0].type01 != 0x12) {
        first = PlayerControllers[0].buttons02 & 0x820;
        second = PlayerControllers[0].buttons02 & 0x40;
    }
    if (PlayerControllers[1].type01 == 0x12) {
        first |= PlayerControllers[1].buttons02 & 8;
        second |= PlayerControllers[1].buttons02 & 4;
    }

    if (garage_logo_work.words[1] & 0x10) {
        if (first) {
            garage_logo_work.words[1] = (garage_logo_work.words[1] | 0x10) & ~5;
            if (garage_step_button_repeat(1)) {
                garage_logo_work.words[1] |= 0x40;
            } else {
                garage_logo_work.words[1] &= ~0x40;
            }
        } else {
            garage_logo_work.words[1] = (garage_logo_work.words[1] & ~0x51) | 4;
        }
    } else {
        if (first) {
            garage_logo_work.words[1] = (garage_logo_work.words[1] | 0x11) & ~4;
            garage_logo_work.words[2] = 0;
            if (garage_step_button_repeat(1)) {
                garage_logo_work.words[1] |= 0x40;
            } else {
                garage_logo_work.words[1] &= ~0x40;
            }
        } else {
            garage_logo_work.words[1] &= ~0x55;
        }
    }

    if (garage_logo_work.words[1] & 0x20) {
        if (second) {
            garage_logo_work.words[1] = (garage_logo_work.words[1] | 0x20) & ~0xA;
            if (garage_step_button_repeat(0)) {
                garage_logo_work.words[1] |= 0x80;
            } else {
                garage_logo_work.words[1] &= ~0x80;
            }
        } else {
            garage_logo_work.words[1] = (garage_logo_work.words[1] & ~0xA2) | 8;
        }
    } else {
        if (second) {
            garage_logo_work.words[1] = (garage_logo_work.words[1] | 0x22) & ~8;
            garage_logo_work.words[3] = 0;
            if (garage_step_button_repeat(0)) {
                garage_logo_work.words[1] |= 0x80;
            } else {
                garage_logo_work.words[1] &= ~0x80;
            }
        } else {
            garage_logo_work.words[1] &= ~0xAA;
        }
    }
}
