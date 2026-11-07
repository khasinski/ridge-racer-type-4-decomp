/* CC1_FLAGS: -mno-split-addresses */

#include "types.h"

extern int CdReadStartTime[];

extern int VSync(), cd_read_retry(), CdReady();

int CdReadSync(a0, a1) int a0, a1; {
    int s0;
    int *p = CdReadStartTime;
    int *q = CdReadStartTime + 2;
    for (;;) {
        if (VSync(-1) > p[0] + 0x4B0) {
            s0 = -1;
        } else if (p[-2] < 0 || VSync(-1) > p[-1] + 0x3C) {
            cd_read_retry(1);
            s0 = p[-7];
        } else {
            s0 = p[-2];
        }
        if (a0 == 0) {
            if (q[0] == 0 || s0 != 0) { if (s0 > 0) continue; break; }
            continue;
        }
        break;
    }
    CdReady(1, a1);
    if (q[0] != 0 && s0 == 0) s0 = 1;
    return s0;
}
