#include "rr/overlay662_transition.h"

void garage_func_8011AA84(void) {
    garage_func_8011B484();
    garage_func_8011B400();
    garage_func_8011CF8C();
    {
    union {u32 bits;s32 value;} count;
    count.bits=(u32)garage_D_801276A8.word04+1U;
    garage_D_801276A8.word04=count.value;
    if (count.value >= 28) {
        ModeGroup = 3;
        ModeId = 5;
        ModeSubState = 0;
    }
    }
}
