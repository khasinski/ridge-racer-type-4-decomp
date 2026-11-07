#include "rr/overlay662_tim.h"

void garage_build_save_tim(void) {
    u32 *p;
    RrGarageTimBytes *src2C = garage_logo_state.tim2C[garage_logo_state.active21];
    u32 *img38 = garage_logo_state.image38[garage_logo_state.active21];
    RrPaletteBytes32 *pal48 = (RrPaletteBytes32 *)(RrPalette16 *)garage_logo_state.palette48[garage_logo_state.active21];

    p = SaveDataBackup.words;
    *(RrGarageTimBytes *)p = *src2C;
    *p++ = 0x10;
    *p++ = 8;
    *p++ = 44;
    *p++ = 0x00400280;
    *p++ = 0x00010010;
    *(RrPaletteBytes32 *)p = *pal48;
    p += 8;
    *p++ = 0x80C;
    *p++ = 0x280;
    *p++ = 0x00400010;
    garage_blend_image_nibbles(img38, p);
}
