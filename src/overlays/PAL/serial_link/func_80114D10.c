#include "rr/overlay667_events.h"

s32 serial_link_func_80114D10(void) {
    union {u32 bits;s32 value;} count;
    count.bits=(u32)serial_link_D_80118EE0+1U;
    serial_link_D_80118EE0=count.value;
    serial_link_D_80118EE8 = 0;
    return 0;
}
