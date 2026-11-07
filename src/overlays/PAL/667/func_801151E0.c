#include "rr/overlay667_packet.h"
void ovl667_func_801151E0(void) {
    s32 state;
    if (ovl667_D_80118ED0 == RaceState && ovl667_D_80118ED0 >= 4) {
        if (ovl667_D_80118ED4 == 0) ovl667_D_80118ED4 = 1;
    } else if (ovl667_D_80118ED4 <= 0) {
        state = ovl667_D_80118F0C;
        if (state == 1) {
            if (ovl667_func_80114D30() == state) {
                ovl667_D_80118F04 = 0;
                D_800ACBF0.choices[0] = LinkLocalInputs.choices[0];
                D_800ACBF0.choices[1] = LinkLocalInputs.choices[1];
                ovl667_D_80118F00 = (s32)((u32)ovl667_D_80118F00 + 1U);
                ovl667_D_80118F09 = (ovl667_D_80118F09 + 1) & 7;
                ovl667_D_80118EEE = ((u8 *)&ovl667_D_80118EEC)[0];
                if (ovl667_D_80118EC8 == state) ovl667_D_80118EC8 = 0;
                ovl667_D_80118F0C = 0;
            } else ovl667_D_80118F0C = 2;
        } else if (state == 2) {
            if (ovl667_func_80114D30() == 1) {
                ovl667_D_80118F04 = 0;
                D_800ACBF0.choices[0] = LinkLocalInputs.choices[0];
                D_800ACBF0.choices[1] = LinkLocalInputs.choices[1];
                ovl667_D_80118F00 = (s32)((u32)ovl667_D_80118F00 + 1U);
                ovl667_D_80118F09 = (ovl667_D_80118F09 + 1) & 7;
                ovl667_D_80118EEE = ((u8 *)&ovl667_D_80118EEC)[0];
                if (ovl667_D_80118EC8 == 1) ovl667_D_80118EC8 = 0;
                ovl667_D_80118F0C = 0;
            } else {
                ovl667_D_80118F0C = 0;
                ovl667_D_80118F04 = (s32)((u32)ovl667_D_80118F04 + 1U);
                if (ovl667_D_80118EC8 == 0) ovl667_D_80118EC8 = 1;
                else if (ovl667_D_80118EC8 == 1) ovl667_D_80118EC8 = state;
            }
        }
    }
}
