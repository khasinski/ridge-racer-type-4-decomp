#include "rr/overlay667_events.h"

s32 serial_link_func_80114988(void) {
    serial_link_D_80118ED8 = OpenEvent(0xF000000B, 0x400, 0x2000, 0);
    serial_link_D_80118EDC = OpenEvent(0xF000000B, 0x8000, 0x1000, serial_link_func_80114D10);
    LinkWriteHandle = FileOpen(serial_link_D_8011487C, 2);
    LinkReadHandle = FileOpen(serial_link_D_8011487C, 0x8001);
    EnableEvent(serial_link_D_80118EDC);
    EnableEvent(serial_link_D_80118ED8);
    return 0;
}
