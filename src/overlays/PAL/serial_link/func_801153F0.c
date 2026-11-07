#include "rr/serial_link_reset.h"
void serial_link_check_link_lost(void){
 if(serial_link_tx_fail_count>=7 || serial_link_rx_miss_count>=7 || serial_link_checksum_error==1){
  serial_link_tx_fail_count=0;serial_link_rx_miss_count=0;serial_link_rx_count=0;
  serial_link_tx_count=0;serial_link_rx_pending=0;ModeFrame=0;
  serial_link_close_sio();ModeGroup=1;ModeId=44;ModeSubState=0;
 }
}
