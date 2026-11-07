#include "rr/overlay667_packet.h"

s32 serial_link_func_80114E34(void) {
    u8 checksum;
    u32 i;
    if (TestEvent(serial_link_D_80118ED8) == 1) {
        serial_link_D_80118EF4 = 0;
        serial_link_D_80118EEF = 0;
        checksum = 0;
        i = 0;
        do {
            checksum ^= ((u8 *)&serial_link_D_80118F10.packet)[i+4];
            i++;
        } while (i < 8);
        if (checksum != serial_link_D_80118F10.packet.checksum01) serial_link_D_80118EEF = 1;
        if (serial_link_D_80118F10.packet.flags00 >> 7) {
            if ((serial_link_D_80118F10.packet.flags00 >> 6) & 1) serial_link_D_80118EF0 = 2;
            else serial_link_D_80118EF0 = 1;
        }
        serial_link_D_80118F08 = serial_link_D_80118F10.packet.flags00;
        if (serial_link_D_80118F09 == serial_link_D_80118F08) {
            serial_link_D_80118F8D = (serial_link_D_80118F08 + 7) & 7;
        } else if (serial_link_D_80118F08 == serial_link_D_80118F09 + 1) {
            serial_link_D_80118F8D = serial_link_D_80118F09;
            serial_link_D_80118EC8 = 3;
        } else if (serial_link_D_80118F09 == serial_link_D_80118F08 + 2) {
            serial_link_D_80118F8D = serial_link_D_80118F08;
            serial_link_D_80118F09 = (serial_link_D_80118F08 + 9) & 7;
        } else serial_link_D_80118F8D = serial_link_D_80118F08;
        serial_link_D_80118F20.packets[serial_link_D_80118F08] = serial_link_D_80118F10.packet;
        FileRead(LinkReadHandle, &serial_link_D_80118F10.packet, 12);
        return 1;
    }
    if (serial_link_D_80118EE8 != 0) return -1;
    _sio_control(2,0,0);
    FileRead(LinkReadHandle, &serial_link_D_80118F10.packet, 12);
    serial_link_D_80118EE8 = 1;
    serial_link_D_80118EF8 = (s32)((u32)serial_link_D_80118EF8 + 1U);
    return 0;
}
