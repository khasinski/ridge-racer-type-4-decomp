#include "rr/overlay662_frame_step.h"

void ovl662_func_8011AAF4(void) {
    ScreenFrame=(s32)((u32)ScreenFrame+1U);
    if (++PlayTimeFrames > 0x014996FFU) {
        PlayTimeFrames = 0x014996FFU;
    }
    ovl662_func_8011D208();
    ovl662_func_8011E070();
    ovl662_func_8011E36C(0);
    ovl662_func_8011B8B4();
    ovl662_func_8011B484();
    ovl662_func_8011B400();
    ovl662_func_8011CF8C();
    ovl662_func_8011CD18();
    D_800F39A0 = ovl662_D_801276A8.buffers24[ovl662_D_801276A8.active21];
    D_800F399C = ovl662_D_801276A8.tim2C[ovl662_D_801276A8.active21];
}
