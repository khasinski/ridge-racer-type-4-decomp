#include "rr/overlay667_packet.h"

void serial_link_func_80115068(void) {
    if (serial_link_D_80118ED4 > 0) {
        if (serial_link_D_80118ED4 == 1) {
            serial_link_D_80118ED4 = 2;
        }
    } else {
        serial_link_D_80118F0C = 1;
        if (serial_link_func_80114E34() == 1) {
            serial_link_D_80118EFC = 0;
            serial_link_D_80118EF4 = 0;
            serial_link_D_80118EF8 = (s32)((u32)serial_link_D_80118EF8 + 1U);
        } else {
            serial_link_D_80118EFC = (s32)((u32)serial_link_D_80118EFC + 1U);
            if (serial_link_D_80118F09 == serial_link_D_80118F08) {
                serial_link_D_80118F8D = (serial_link_D_80118F08 + 7) & 7;
            } else if (serial_link_D_80118F08 == serial_link_D_80118F09 + 1) {
                serial_link_D_80118F8D = serial_link_D_80118F09;
                serial_link_D_80118EC8 = 3;
            } else if (serial_link_D_80118F09 == serial_link_D_80118F08 + 2) {
                serial_link_D_80118F8D = serial_link_D_80118F08;
                serial_link_D_80118F09 = (serial_link_D_80118F08 + 9) & 7;
            } else {
                serial_link_D_80118F8D = serial_link_D_80118F08;
            }
        }
        LinkRemoteInputs.choices[0] = serial_link_D_80118F20.packets[serial_link_D_80118F8D].first04;
        LinkRemoteInputs.choices[1] = serial_link_D_80118F20.packets[serial_link_D_80118F8D].second08;
        serial_link_D_80118EED = serial_link_D_80118F20.packets[serial_link_D_80118F8D].phase03;
    }
}
