/* GCC_VERSION: 2.7.2 */
#include "rr/trig.h"

s32 rsin(s32 a) {
    int r;
    if (a >= 0) {
        r = sin_1(a & 0xFFF);
    } else {
        r = -sin_1((0U - (u32)a) & 0xFFFU);
    }
    return r;
}

s32 sin_1(s32 a) {
    if (a < 0x801) {
        if (a < 0x401) return SinTable[a];
        return SinTable[0x800 - a];
    }
    if (a < 0xC01) return -SinTableBiasedBase800[a];
    return -SinTable[0x1000 - a];
}

