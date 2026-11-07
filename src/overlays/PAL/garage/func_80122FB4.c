#include "rr/overlay662_scroll_actions.h"

void garage_func_80122FB4(void) {
    if (garage_D_80127668.words[0] & 8) {
        if (garage_D_80127668.words[1] & 4) {
            garage_D_80127668.words[0] &= ~8;
            garage_func_8011E7C0(13, 63, 294, 346);
            return;
        }
    } else {
        if (garage_func_8011E6C8(215, 69, 6, 18) != 0) {
            return;
        }
        if (!(garage_D_80127668.words[1] & 1)) {
            return;
        }
        garage_D_80127668.words[4] = 36;
        garage_D_80127668.words[0] |= 8;
        garage_func_8011E7DC(36);
    }
    if (garage_D_80127668.words[1] & 0x40) {
        ((RrGarageButton *)garage_D_801273B8[36])->timer0E = 7;
        if (--garage_D_801276A8.reset08[1] < 0) {
            garage_D_801276A8.reset08[1] = 0;
            PlaySfx(4);
        } else {
            PlaySfx(0);
        }
    }
}
