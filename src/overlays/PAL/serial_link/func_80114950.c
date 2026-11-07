#include "rr/serial_link_events.h"

void serial_link_poll_sio_status(void) {
    _sio_control(0, 0, 0);
    _sio_control(0, 1, 0);
}
