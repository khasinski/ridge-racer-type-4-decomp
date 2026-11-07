/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "rr/trig.h"

s32 sin_1(s32 a) {
    if (a < 0x801) {
        if (a < 0x401) return SinTable[a];
        return SinTable[0x800 - a];
    }
    if (a < 0xC01) return -SinTableBiasedBase800[a];
    return -SinTable[0x1000 - a];
}
