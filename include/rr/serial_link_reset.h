#ifndef RR_SERIAL_LINK_RESET_H
#define RR_SERIAL_LINK_RESET_H
#include "rr/frame_loop.h"

extern s32 serial_link_tx_fail_count,serial_link_rx_miss_count;
extern s32 serial_link_rx_count,serial_link_tx_count,serial_link_rx_pending;
extern u8 serial_link_checksum_error;
void serial_link_close_sio(void);
void serial_link_check_link_lost(void);
#endif
