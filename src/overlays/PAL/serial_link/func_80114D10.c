#include "rr/serial_link_events.h"

s32 serial_link_error_callback(void) {
    union {u32 bits;s32 value;} count;
    count.bits=(u32)serial_link_error_count+1U;
    serial_link_error_count=count.value;
    serial_link_rx_pending = 0;
    return 0;
}
