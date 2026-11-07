/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "types.h"

#define COPY8(dst_arr, src_arr) \
    { \
        u8 *d = (dst_arr); \
        u8 *s = (src_arr); \
        s32 n; \
        if (d != 0) { \
            for (n = 7; n != -1; n--) { \
                *d++ = *s++; \
            } \
        } \
    }

extern char *CD_comstr[];

extern volatile u8 CdIntrStatusBytes[];

extern u8 CD_syncResult[];

extern s32 VSync(s32);

extern s32 CheckCallback(void);

extern const char StrCdSync[];

extern char *CD_intstr[];

extern s32 CD_timeout_deadline;

extern s32 CD_timeout_count;

extern const char *CdWaitCallerName[];

static inline int check_alarm(char **ac, char **sc, volatile u8 *st) {
    if (VSync(-1) > CD_timeout_deadline || CD_timeout_count++ > 0x3C0000) {
        PsyqPuts(CdTimeoutPrefixStr);
        {
            u8 s0_ = st[0];
            u8 s1_ = st[1];
            char *x5 = sc[s1_];
            DebugPrintStub(
                CdTimeoutStatusFmt,
                CdWaitCallerName[0],
                ac[D_800A64A5],
                sc[s0_],
                x5
            );
        }
        CD_flush();
        return -1;
    }
    return 0;
}

static inline void cd_poll_cb(volatile u8 *st, volatile u8 *st1) {
    u8 saved = *CD_reg0 & 3;
    s32 res;
    while ((res = getintr()) != 0) {
        if ((res & 4) && CD_cbready != 0) {
            CD_cbready(*st1, CdReadyResult);
        }
        if ((res & 2) && CD_cbsync != 0) {
            CD_cbsync(*st, CD_syncResult);
        }
    }
    *CD_reg0 = saved;
}

s32 CD_sync(s32 mode, u8 *result) {
    s32 ret;
    u8 *src;
    u8 *dst;
    s32 i;
    char **ac;
    char **sc;
    volatile u8 *st;
    volatile u8 *st1;

    s32 t = VSync(-1);
    CD_timeout_deadline = t + 0x3C0;
    CD_timeout_count = 0;
    CdWaitCallerName[0] = StrCdSync;
    ac = CD_comstr;
    sc = CD_intstr;
    st = CdIntrStatusBytes;
    st1 = st + 1;
    do {
        if (check_alarm(ac, sc, st) != 0) {
            return -1;
        }
        if (CheckCallback() != 0) {
            cd_poll_cb(st, st1);
        }
        ret = *st;
        if (ret == 2 || ret == 5) {
            *st = 2;
            dst = result;
            src = CD_syncResult;
            if (dst != 0) {
                for (i = 7; i != -1; i--) {
                    *dst++ = *src++;
                }
            }
            return ret;
        }
    } while (mode == 0);
    return 0;
}
