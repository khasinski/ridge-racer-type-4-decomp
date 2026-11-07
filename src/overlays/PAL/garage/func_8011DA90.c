#include "rr/overlay662_garage_fade.h"

void garage_func_8011DA90(void)
{
    RrMenuItemState5C *pad;

    if ((garage_D_80127668.words[0] & 2) ||
        ((pad = &PlayerControllers[0])->type01 != 0x12 && (pad->buttons02 & 4))) {
        /* ASM_BARRIER: GCC local-alloc assigns state base to $a0 so controller base gets $a1 */
        register u32 *s __asm__("$4") = garage_D_80127668.words;
        s[10] += s[11];
        if ((s32)s[10] > 4096) {
            s[10] = 4096;
        }
    } else {
        if (garage_D_80127668.words[10] == 4096) {
            PlaySfx(2);
        }
        garage_D_80127668.words[10] -= garage_D_80127668.words[11];
        if ((s32)garage_D_80127668.words[10] < 0) {
            garage_D_80127668.words[10] = 0;
            garage_D_80127668.words[1] = 48;
            garage_D_80127668.words[0] &= ~4;
        }
    }
    RR_GARAGE_GTE_LERP(garage_D_80127668.words[10],
                       &garage_D_80127668.words[8],
                       &garage_D_80127668.words[5]);
}
