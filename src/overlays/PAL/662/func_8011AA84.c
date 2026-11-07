#include "rr/overlay662_transition.h"

void ovl662_func_8011AA84(void) {
    ovl662_func_8011B484();
    ovl662_func_8011B400();
    ovl662_func_8011CF8C();
    {
    union {u32 bits;s32 value;} count;
    count.bits=(u32)ovl662_D_801276A8.word04+1U;
    ovl662_D_801276A8.word04=count.value;
    if (count.value >= 28) {
        ModeGroup = 3;
        ModeId = 5;
        D_800F4D8A = 0;
    }
    }
}
