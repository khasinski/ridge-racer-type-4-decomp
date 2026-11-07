#include "rr/serial_link_events.h"

s32 serial_link_open_sio(void) {
    serial_link_rx_event = OpenEvent(0xF000000B, 0x400, 0x2000, 0);
    serial_link_error_event = OpenEvent(0xF000000B, 0x8000, 0x1000, serial_link_error_callback);
    LinkWriteHandle = FileOpen(serial_link_sio_device_name, 2);
    LinkReadHandle = FileOpen(serial_link_sio_device_name, 0x8001);
    EnableEvent(serial_link_error_event);
    EnableEvent(serial_link_rx_event);
    return 0;
}
