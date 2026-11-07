#include "rr/driver_control.h"
void serial_link_func_80115494(void)
{
    ResetGraph(1);
    _sio_control(3,0,1);
    while(_sio_control(3,1,0)==0){}
    _sio_control(5,0,0);
    _sio_control(3,0,0);
    VSync(0);
    SetDispMask(1);
}
