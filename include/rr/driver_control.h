#ifndef RR_DRIVER_CONTROL_H
#define RR_DRIVER_CONTROL_H
#include "rr/timing.h"

s32 ResetGraph(s32);
/* The tagged dispatcher takes a target machine word as its last argument.
 * This caller uses only scalar control values0 and1. */
s32 _sio_control(s32 command,s32 operation,unsigned long argument);
void SetDispMask(s32 mode);
void serial_link_reset_graph_and_sio(void);
#endif
