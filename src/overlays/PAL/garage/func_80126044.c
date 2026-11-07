#include "rr/overlay662_tim.h"

void garage_func_80126044(void) {
    u32 *p;
    RrGarageTimBytes *src2C = garage_D_801276A8.tim2C[garage_D_801276A8.active21];
    u32 *img38 = garage_D_801276A8.image38[garage_D_801276A8.active21];
    RrPaletteBytes32 *pal48 = (RrPaletteBytes32 *)(RrPalette16 *)garage_D_801276A8.palette48[garage_D_801276A8.active21];

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
    garage_func_80125F60(img38, p);
}
