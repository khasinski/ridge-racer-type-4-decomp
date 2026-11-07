#include "rr/garage_garage_fade.h"

void garage_update_cursor_glide(void)
{
    RrMenuItemState5C *pad;

    if ((garage_logo_work.words[0] & 2) ||
        ((pad = &PlayerControllers[0])->type01 != 0x12 && (pad->buttons02 & 4))) {
        /* ASM_BARRIER: GCC local-alloc assigns state base to $a0 so controller base gets $a1 */
        register u32 *s __asm__("$4") = garage_logo_work.words;
        s[10] += s[11];
        if ((s32)s[10] > 4096) {
            s[10] = 4096;
        }
    } else {
        if (garage_logo_work.words[10] == 4096) {
            PlaySfx(2);
        }
        garage_logo_work.words[10] -= garage_logo_work.words[11];
        if ((s32)garage_logo_work.words[10] < 0) {
            garage_logo_work.words[10] = 0;
            garage_logo_work.words[1] = 48;
            garage_logo_work.words[0] &= ~4;
        }
    }
    RR_GARAGE_GTE_LERP(garage_logo_work.words[10],
                       &garage_logo_work.words[8],
                       &garage_logo_work.words[5]);
}
