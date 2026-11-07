#ifndef RR_OVERLAY667_EVENTS_H
#define RR_OVERLAY667_EVENTS_H
#include "rr/bios_events.h"
#include "rr/driver_control.h"
#include "rr/overlay667_status.h"
extern RrBiosEventHandle serial_link_rx_event,serial_link_error_event;
extern s32 serial_link_error_count,serial_link_rx_pending;
extern s32 LinkWriteHandle,LinkReadHandle;
extern const char serial_link_sio_device_name[];
/* The callback wraps its counter modulo2^32 and clears the pending word. */
s32 serial_link_error_callback(void);
/* Register two events, open sio: twice, then enable freshly read event handles.
 * Service boundaries may invoke the registered callback and mutate state. */
s32 serial_link_open_sio(void);
void serial_link_func_80114950(void);
#endif
