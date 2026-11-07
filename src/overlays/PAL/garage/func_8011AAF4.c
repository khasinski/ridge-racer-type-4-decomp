#include "rr/overlay662_frame_step.h"

void garage_func_8011AAF4(void) {
    ScreenFrame=(s32)((u32)ScreenFrame+1U);
    if (++PlayTimeFrames > 0x014996FFU) {
        PlayTimeFrames = 0x014996FFU;
    }
    garage_func_8011D208();
    garage_func_8011E070();
    garage_func_8011E36C(0);
    garage_func_8011B8B4();
    garage_func_8011B484();
    garage_func_8011B400();
    garage_func_8011CF8C();
    garage_func_8011CD18();
    GarageActiveBuffer = garage_D_801276A8.buffers24[garage_D_801276A8.active21];
    GarageLogoTimPtr = garage_D_801276A8.tim2C[garage_D_801276A8.active21];
}
