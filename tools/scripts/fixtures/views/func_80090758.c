/* GCC_VERSION: 2.7.2 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "types.h"

extern int Lzc();

int CordicSqrt(a0)
int a0;
{
    int t, s, a;
    if (a0 == 0) return 0;
    t = 8 - Lzc(a0);
    if (t >= 0) {
        s = t >> 1;
        a = a0 >> (s * 2);
    } else {
        s = (t >> 1) + 1;
        a = a0 << -(s * 2);
    }
    s -= 6;
    if (s >= 0) return csqrt_1(a) << s;
    return csqrt_1(a) >> -s;
}
