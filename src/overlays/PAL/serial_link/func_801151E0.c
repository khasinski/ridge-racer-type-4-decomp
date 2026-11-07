#include "rr/overlay667_packet.h"
void serial_link_func_801151E0(void) {
    s32 state;
    if (serial_link_D_80118ED0 == RaceState && serial_link_D_80118ED0 >= 4) {
        if (serial_link_D_80118ED4 == 0) serial_link_D_80118ED4 = 1;
    } else if (serial_link_D_80118ED4 <= 0) {
        state = serial_link_D_80118F0C;
        if (state == 1) {
            if (serial_link_func_80114D30() == state) {
                serial_link_D_80118F04 = 0;
                LinkSentLocalInputs.choices[0] = LinkLocalInputs.choices[0];
                LinkSentLocalInputs.choices[1] = LinkLocalInputs.choices[1];
                serial_link_D_80118F00 = (s32)((u32)serial_link_D_80118F00 + 1U);
                serial_link_D_80118F09 = (serial_link_D_80118F09 + 1) & 7;
                serial_link_D_80118EEE = ((u8 *)&serial_link_D_80118EEC)[0];
                if (serial_link_D_80118EC8 == state) serial_link_D_80118EC8 = 0;
                serial_link_D_80118F0C = 0;
            } else serial_link_D_80118F0C = 2;
        } else if (state == 2) {
            if (serial_link_func_80114D30() == 1) {
                serial_link_D_80118F04 = 0;
                LinkSentLocalInputs.choices[0] = LinkLocalInputs.choices[0];
                LinkSentLocalInputs.choices[1] = LinkLocalInputs.choices[1];
                serial_link_D_80118F00 = (s32)((u32)serial_link_D_80118F00 + 1U);
                serial_link_D_80118F09 = (serial_link_D_80118F09 + 1) & 7;
                serial_link_D_80118EEE = ((u8 *)&serial_link_D_80118EEC)[0];
                if (serial_link_D_80118EC8 == 1) serial_link_D_80118EC8 = 0;
                serial_link_D_80118F0C = 0;
            } else {
                serial_link_D_80118F0C = 0;
                serial_link_D_80118F04 = (s32)((u32)serial_link_D_80118F04 + 1U);
                if (serial_link_D_80118EC8 == 0) serial_link_D_80118EC8 = 1;
                else if (serial_link_D_80118EC8 == 1) serial_link_D_80118EC8 = state;
            }
        }
    }
}
