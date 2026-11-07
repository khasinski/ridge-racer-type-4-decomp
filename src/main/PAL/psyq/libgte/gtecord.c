/* GCC_VERSION: 2.7.2 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "types.h"
#include "psyq/libgte/gtecolo4.h"
#include "rr/garage_distance.h"

s32 csqrt_1(s32 arg0);

s32 csqrt_1(s32 arg0) {
    s32 x[8];
    s32 y[8];
    s32 i;
    s32 k;
    s32 *q;
    s32 *p;
    s32 a;

    x[1] = arg0 + 0x5D50AD;
    y[1] = arg0 - 0x5D50AD;
    for (i = 1, k = 4, q = &y[1], p = &y[2]; i < 7; p++, i++) {
        if (i != k) {
            if (y[i] >= 0) {
                x[i + 1] = x[i] - (y[i] >> i);
                *p = y[i] - (x[i] >> i);
            } else {
                x[i + 1] = x[i] + (y[i] >> i);
                *(s32 *)((u8 *)q + (i << 2)) = y[i] + (x[i] >> i);
            }
        } else {
            if (y[4] >= 0) {
                a = x[4];
                x[4] = a - (y[4] >> 4);
                y[4] = y[4] - (a >> 4);
                if (y[4] >= 0) {
                    x[5] = x[4] - (y[4] >> 4);
                    y[5] = y[4] - (x[4] >> 4);
                } else {
                    x[5] = x[4] + (y[4] >> 4);
                    y[5] = y[4] + (x[4] >> 4);
                }
            } else {
                a = x[4];
                x[4] = a + (y[4] >> 4);
                y[4] = y[4] + (a >> 4);
                if (y[4] >= 0) {
                    x[5] = x[4] - (y[4] >> 4);
                    y[5] = y[4] - (x[4] >> 4);
                } else {
                    x[5] = x[4] + (y[4] >> 4);
                    y[5] = y[4] + (x[4] >> 4);
                }
            }
        }
    }
    return x[7];
}

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
