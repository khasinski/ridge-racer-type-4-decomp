#include "rr/serial_link_packet.h"
s32 serial_link_send_packet(void) {
    u8 csum;
    u32 i;
    if (_sio_control(3, 1, 0) == 0) {
        return 0;
    }
    serial_link_tx_packet.packet.flags00 = serial_link_tx_sequence;
    if (serial_link_checksum_error == 1) {
        serial_link_tx_packet.packet.flags00 |= 0x80;
    }
    csum = 0;
    i = 0;
    serial_link_tx_packet.packet.first04 = LinkLocalInputs.choices[0];
    serial_link_tx_packet.packet.second08 = LinkLocalInputs.choices[1];
    do {
        csum ^= ((u8 *)&serial_link_tx_packet)[i + 4];
        i++;
    } while (i < 8);
    serial_link_tx_packet.packet.checksum01 = csum;
    serial_link_tx_packet.packet.status02 = RaceState;
    serial_link_tx_packet.packet.phase03 = serial_link_local_menu_buttons;
    if (FileWrite(LinkWriteHandle, &serial_link_tx_packet, 12) != 12) {
        return 2;
    }
    return 1;
}
