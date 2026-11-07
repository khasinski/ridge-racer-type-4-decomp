#include "rr/overlay667_packet.h"
s32 ovl667_func_80114D30(void) {
    u8 csum;
    u32 i;
    if (_sio_control(3, 1, 0) == 0) {
        return 0;
    }
    ovl667_D_80118F80.packet.flags00 = ovl667_D_80118F09;
    if (ovl667_D_80118EEF == 1) {
        ovl667_D_80118F80.packet.flags00 |= 0x80;
    }
    csum = 0;
    i = 0;
    ovl667_D_80118F80.packet.first04 = LinkLocalInputs.choices[0];
    ovl667_D_80118F80.packet.second08 = LinkLocalInputs.choices[1];
    do {
        csum ^= ((u8 *)&ovl667_D_80118F80)[i + 4];
        i++;
    } while (i < 8);
    ovl667_D_80118F80.packet.checksum01 = csum;
    ovl667_D_80118F80.packet.status02 = RaceState;
    ovl667_D_80118F80.packet.phase03 = ovl667_D_80118EEC;
    if (FileWrite(LinkWriteHandle, &ovl667_D_80118F80, 12) != 12) {
        return 2;
    }
    return 1;
}
