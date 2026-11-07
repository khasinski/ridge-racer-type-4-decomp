#include "rr/overlay667_packet.h"

void serial_link_update_receive(void) {
    if (serial_link_shutdown_state > 0) {
        if (serial_link_shutdown_state == 1) {
            serial_link_shutdown_state = 2;
        }
    } else {
        serial_link_tx_state = 1;
        if (serial_link_receive_packet() == 1) {
            serial_link_rx_miss_count = 0;
            serial_link_rx_idle_flag = 0;
            serial_link_rx_count = (s32)((u32)serial_link_rx_count + 1U);
        } else {
            serial_link_rx_miss_count = (s32)((u32)serial_link_rx_miss_count + 1U);
            if (serial_link_tx_sequence == serial_link_rx_sequence) {
                serial_link_rx_cursor = (serial_link_rx_sequence + 7) & 7;
            } else if (serial_link_rx_sequence == serial_link_tx_sequence + 1) {
                serial_link_rx_cursor = serial_link_tx_sequence;
                serial_link_sync_state = 3;
            } else if (serial_link_tx_sequence == serial_link_rx_sequence + 2) {
                serial_link_rx_cursor = serial_link_rx_sequence;
                serial_link_tx_sequence = (serial_link_rx_sequence + 9) & 7;
            } else {
                serial_link_rx_cursor = serial_link_rx_sequence;
            }
        }
        LinkRemoteInputs.choices[0] = serial_link_rx_ring.packets[serial_link_rx_cursor].first04;
        LinkRemoteInputs.choices[1] = serial_link_rx_ring.packets[serial_link_rx_cursor].second08;
        serial_link_remote_menu_buttons = serial_link_rx_ring.packets[serial_link_rx_cursor].phase03;
    }
}
