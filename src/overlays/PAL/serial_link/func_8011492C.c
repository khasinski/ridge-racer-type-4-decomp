#include "rr/overlay667_status.h"
s32 serial_link_func_8011492C(s32 arg0, u32 arg1) {
    /* ASM_BARRIER: Hold constant 2 and return value in $2 so dbr_schedule fills bne delay slot */
    register s32 ret __asm__("$2") = 2;
    if (arg0 == ret) {
        ret = 0;
        if (arg1 < 25000U) {
            /* ASM_BARRIER: Prevent GCC jump-optimization from folding the branch into sltiu */
            __asm__("");
            ret = 1;
            /* ASM_BARRIER: Prevent dbr_schedule from duplicating jr ra for the inner branch */
            __asm__("" : "=r"(ret) : "0"(ret));
        }
    } else {
        ret = 1;
    }
    return ret;
}
