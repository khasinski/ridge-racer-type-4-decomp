#ifndef RR_OVERLAY667_EVENTS_H
#define RR_OVERLAY667_EVENTS_H
#include "rr/bios_events.h"
#include "rr/driver_control.h"
#include "rr/overlay667_status.h"
extern RrBiosEventHandle serial_link_D_80118ED8,serial_link_D_80118EDC;
extern s32 serial_link_D_80118EE0,serial_link_D_80118EE8;
extern s32 LinkWriteHandle,LinkReadHandle;
extern const char serial_link_D_8011487C[];
/* The callback wraps its counter modulo2^32 and clears the pending word. */
s32 serial_link_func_80114D10(void);
/* Register two events, open sio: twice, then enable freshly read event handles.
 * Service boundaries may invoke the registered callback and mutate state. */
s32 serial_link_func_80114988(void);
void serial_link_func_80114950(void);
#endif
