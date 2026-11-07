/* CC1_FLAGS: -fno-cse-follow-jumps -fno-rerun-cse-after-loop */
#include "rr/overlay662_scrollbar.h"

void garage_func_8011C174(s32 arg0, RrGpuRectangleFields *arg1)
{
    /* ASM_BARRIER: match GCC local-alloc assignment of $v0/$v1/$a0 so global-alloc keeps base in $a1 */
    register s32 q __asm__("$2");
    /* ASM_BARRIER: match GCC local-alloc assignment of 64 to $v1 */
    register s32 c64 __asm__("$3");
    /* ASM_BARRIER: match GCC local-alloc assignment of extent diff to $a0 */
    register s32 diff __asm__("$4");

    if (arg0 == 0) {
        arg1->w = 112 / (garage_D_801276A8.config1C >> 1);
        arg1->h = 8;
        arg1->y = 337;
        if (garage_D_801276A8.config1C == 2) {
            arg1->x = 90;
            return;
        }
        c64 = 64;
        q = c64 / (garage_D_801276A8.config1C >> 1);
        diff = 112 - arg1->w;
        c64 -= q;
        q = garage_D_801276A8.reset08[0] * diff / c64;
        arg1->x = q + 90;
    } else {
        arg1->w = 4;
        arg1->h = 224 / (garage_D_801276A8.config1C >> 1);
        arg1->x = 216;
        if (garage_D_801276A8.config1C == 2) {
            arg1->y = 87;
            return;
        }
        c64 = 64;
        q = c64 / (garage_D_801276A8.config1C >> 1);
        diff = 224 - arg1->h;
        c64 -= q;
        q = garage_D_801276A8.reset08[1] * diff / c64;
        arg1->y = q + 87;
    }
}
