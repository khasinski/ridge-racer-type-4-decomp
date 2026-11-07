#include "rr/overlay667_packet.h"
s32 serial_link_func_80114D30(void) {
    u8 csum;
    u32 i;
    if (_sio_control(3, 1, 0) == 0) {
        return 0;
    }
    serial_link_D_80118F80.packet.flags00 = serial_link_D_80118F09;
    if (serial_link_D_80118EEF == 1) {
        serial_link_D_80118F80.packet.flags00 |= 0x80;
    }
    csum = 0;
    i = 0;
    serial_link_D_80118F80.packet.first04 = LinkLocalInputs.choices[0];
    serial_link_D_80118F80.packet.second08 = LinkLocalInputs.choices[1];
    do {
        csum ^= ((u8 *)&serial_link_D_80118F80)[i + 4];
        i++;
    } while (i < 8);
    serial_link_D_80118F80.packet.checksum01 = csum;
    serial_link_D_80118F80.packet.status02 = RaceState;
    serial_link_D_80118F80.packet.phase03 = serial_link_D_80118EEC;
    if (FileWrite(LinkWriteHandle, &serial_link_D_80118F80, 12) != 12) {
        return 2;
    }
    return 1;
}
