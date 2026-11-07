#include "rr/overlay667_packet.h"

void ovl667_func_80115068(void) {
    if (ovl667_D_80118ED4 > 0) {
        if (ovl667_D_80118ED4 == 1) {
            ovl667_D_80118ED4 = 2;
        }
    } else {
        ovl667_D_80118F0C = 1;
        if (ovl667_func_80114E34() == 1) {
            ovl667_D_80118EFC = 0;
            ovl667_D_80118EF4 = 0;
            ovl667_D_80118EF8 = (s32)((u32)ovl667_D_80118EF8 + 1U);
        } else {
            ovl667_D_80118EFC = (s32)((u32)ovl667_D_80118EFC + 1U);
            if (ovl667_D_80118F09 == ovl667_D_80118F08) {
                ovl667_D_80118F8D = (ovl667_D_80118F08 + 7) & 7;
            } else if (ovl667_D_80118F08 == ovl667_D_80118F09 + 1) {
                ovl667_D_80118F8D = ovl667_D_80118F09;
                ovl667_D_80118EC8 = 3;
            } else if (ovl667_D_80118F09 == ovl667_D_80118F08 + 2) {
                ovl667_D_80118F8D = ovl667_D_80118F08;
                ovl667_D_80118F09 = (ovl667_D_80118F08 + 9) & 7;
            } else {
                ovl667_D_80118F8D = ovl667_D_80118F08;
            }
        }
        LinkRemoteInputs.choices[0] = ovl667_D_80118F20.packets[ovl667_D_80118F8D].first04;
        LinkRemoteInputs.choices[1] = ovl667_D_80118F20.packets[ovl667_D_80118F8D].second08;
        ovl667_D_80118EED = ovl667_D_80118F20.packets[ovl667_D_80118F8D].phase03;
    }
}
