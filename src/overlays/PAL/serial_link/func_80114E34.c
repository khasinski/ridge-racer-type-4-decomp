#include "rr/overlay667_packet.h"

s32 serial_link_receive_packet(void) {
    u8 checksum;
    u32 i;
    if (TestEvent(serial_link_rx_event) == 1) {
        serial_link_rx_idle_flag = 0;
        serial_link_checksum_error = 0;
        checksum = 0;
        i = 0;
        do {
            checksum ^= ((u8 *)&serial_link_rx_packet.packet)[i+4];
            i++;
        } while (i < 8);
        if (checksum != serial_link_rx_packet.packet.checksum01) serial_link_checksum_error = 1;
        if (serial_link_rx_packet.packet.flags00 >> 7) {
            if ((serial_link_rx_packet.packet.flags00 >> 6) & 1) serial_link_rx_flag_kind = 2;
            else serial_link_rx_flag_kind = 1;
        }
        serial_link_rx_sequence = serial_link_rx_packet.packet.flags00;
        if (serial_link_tx_sequence == serial_link_rx_sequence) {
            serial_link_rx_cursor = (serial_link_rx_sequence + 7) & 7;
        } else if (serial_link_rx_sequence == serial_link_tx_sequence + 1) {
            serial_link_rx_cursor = serial_link_tx_sequence;
            serial_link_sync_state = 3;
        } else if (serial_link_tx_sequence == serial_link_rx_sequence + 2) {
            serial_link_rx_cursor = serial_link_rx_sequence;
            serial_link_tx_sequence = (serial_link_rx_sequence + 9) & 7;
        } else serial_link_rx_cursor = serial_link_rx_sequence;
        serial_link_rx_ring.packets[serial_link_rx_sequence] = serial_link_rx_packet.packet;
        FileRead(LinkReadHandle, &serial_link_rx_packet.packet, 12);
        return 1;
    }
    if (serial_link_rx_pending != 0) return -1;
    _sio_control(2,0,0);
    FileRead(LinkReadHandle, &serial_link_rx_packet.packet, 12);
    serial_link_rx_pending = 1;
    serial_link_rx_count = (s32)((u32)serial_link_rx_count + 1U);
    return 0;
}
