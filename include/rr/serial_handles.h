#ifndef RR_SERIAL_HANDLES_H
#define RR_SERIAL_HANDLES_H
#include "rr/driver_control.h"

/* BIOS event IDs are opaque 32-bit handles; file descriptors remain signed. */
extern u32 serial_link_D_80118ED8,serial_link_D_80118EDC;
extern s32 LinkWriteHandle,LinkReadHandle;
s32 func_80096EC4(u32 handle);
s32 CloseEvent(u32 handle);
s32 FileClose(s32 descriptor);
void serial_link_func_80114A40(void);
#endif
