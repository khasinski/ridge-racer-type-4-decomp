#include "rr/overlay667_packet.h"
void serial_link_update_send(void) {
    s32 state;
    if (serial_link_D_80118ED0 == RaceState && serial_link_D_80118ED0 >= 4) {
        if (serial_link_shutdown_state == 0) serial_link_shutdown_state = 1;
    } else if (serial_link_shutdown_state <= 0) {
        state = serial_link_tx_state;
        if (state == 1) {
            if (serial_link_send_packet() == state) {
                serial_link_tx_fail_count = 0;
                LinkSentLocalInputs.choices[0] = LinkLocalInputs.choices[0];
                LinkSentLocalInputs.choices[1] = LinkLocalInputs.choices[1];
                serial_link_tx_count = (s32)((u32)serial_link_tx_count + 1U);
                serial_link_tx_sequence = (serial_link_tx_sequence + 1) & 7;
                serial_link_sent_menu_buttons = ((u8 *)&serial_link_local_menu_buttons)[0];
                if (serial_link_sync_state == state) serial_link_sync_state = 0;
                serial_link_tx_state = 0;
            } else serial_link_tx_state = 2;
        } else if (state == 2) {
            if (serial_link_send_packet() == 1) {
                serial_link_tx_fail_count = 0;
                LinkSentLocalInputs.choices[0] = LinkLocalInputs.choices[0];
                LinkSentLocalInputs.choices[1] = LinkLocalInputs.choices[1];
                serial_link_tx_count = (s32)((u32)serial_link_tx_count + 1U);
                serial_link_tx_sequence = (serial_link_tx_sequence + 1) & 7;
                serial_link_sent_menu_buttons = ((u8 *)&serial_link_local_menu_buttons)[0];
                if (serial_link_sync_state == 1) serial_link_sync_state = 0;
                serial_link_tx_state = 0;
            } else {
                serial_link_tx_state = 0;
                serial_link_tx_fail_count = (s32)((u32)serial_link_tx_fail_count + 1U);
                if (serial_link_sync_state == 0) serial_link_sync_state = 1;
                else if (serial_link_sync_state == 1) serial_link_sync_state = state;
            }
        }
    }
}
