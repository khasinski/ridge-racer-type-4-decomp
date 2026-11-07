#include "rr/overlay662_scroll_actions.h"

void garage_func_80123228(void) {
    if (garage_D_80127668.words[0] & 8) {
        if (garage_D_80127668.words[1] & 4) {
            garage_D_80127668.words[0] &= ~8;
            garage_func_8011E7C0(13, 63, 294, 346);
            return;
        }
    } else {
        if (garage_func_8011E6C8(81, 335, 9, 12) != 0) {
            return;
        }
        if (!(garage_D_80127668.words[1] & 1)) {
            return;
        }
        garage_D_80127668.words[4] = 38;
        garage_D_80127668.words[0] |= 8;
        garage_func_8011E7DC(38);
    }
    if (garage_D_80127668.words[1] & 0x40) {
        ((RrGarageButton *)garage_D_801273B8[38])->timer0E = 7;
        if (--garage_D_801276A8.reset08[0] < 0) {
            garage_D_801276A8.reset08[0] = 0;
            PlaySfx(4);
        } else {
            PlaySfx(0);
        }
    }
}
