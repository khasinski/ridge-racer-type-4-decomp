#include "rr/serial_handles.h"
void serial_link_close_sio(void){
 _sio_control(2,3,0);_sio_control(2,0,0);
 DisableEvent(serial_link_rx_event);DisableEvent(serial_link_error_event);
 FileClose(LinkWriteHandle);FileClose(LinkReadHandle);
 CloseEvent(serial_link_rx_event);CloseEvent(serial_link_error_event);
 _sio_control(4,0,0);_sio_control(1,1,1);
}
