#include "rr/overlay667_packet.h"
void serial_link_init_connection(unsigned long configuration) {
    s32 work;
    u32 *base;
    /* Three word lanes in the genuine24-word ring. Keep cursor arithmetic
     * at full pointer width: after the last store, the unused second/third
     * integer cursors advance beyond the ring without forming C pointers. */
    unsigned long first;
    unsigned long second;
    unsigned long third;
    SetDispMask(0);
    _sio_control(1,3,configuration);
    _sio_control(1,4,4);
    _sio_control(4,0,(unsigned long)serial_link_sio_timeout_check);
    work=0;
    base=serial_link_rx_ring.words;
    third=(unsigned long)base+8;
    second=(unsigned long)base+4;
    first=(unsigned long)base;
    serial_link_checksum_error=0;
    serial_link_rx_cursor=0;
    serial_link_last_rx_cursor=7;
    serial_link_rx_sequence=0;
    serial_link_tx_sequence=0;
    serial_link_rx_miss_count=0;
    serial_link_tx_fail_count=0;
    serial_link_rx_count=0;
    serial_link_tx_count=0;
    serial_link_rx_pending=0;
    serial_link_sync_state=0;
    serial_link_remote_menu_buttons=0;
    serial_link_sent_menu_buttons=0;
    ((u8 *)&serial_link_local_menu_buttons)[0]=0;
    do {
        *(u32 *)first=0;
        *(u32 *)second=0;
        *(u32 *)third=0;
        third+=12;
        second+=12;
        work++;
        first+=12;
    } while (work<8);
    serial_link_tx_packet.words[0]=0;
    serial_link_tx_packet.words[1]=0;
    serial_link_tx_packet.words[2]=0;
    serial_link_rx_packet.words[0]=0;
    serial_link_rx_packet.words[1]=0;
    serial_link_rx_packet.words[2]=0;
    LinkLocalInputs.words[0]=0;
    LinkLocalInputs.words[1]=0;
    LinkSentLocalInputs.words[0]=0;
    LinkSentLocalInputs.words[1]=0;
    LinkRemoteInputs.words[0]=0;
    LinkRemoteInputs.words[1]=0;
    serial_link_stop_race_state=0;
    serial_link_shutdown_state=0;
    if (_sio_control(3,1,0) != 0) {
        _sio_control(3,0,1);
    } else {
        _sio_control(3,0,1);
        while (_sio_control(3,1,0)==0) {
            work=((u32)_sio_control(0,0,0)>>7)&3;
            if (work==0 || work==3) break;
        }
    }
    _sio_control(5,0,0);
    VSync(0);
    _sio_control(3,0,0);
}
