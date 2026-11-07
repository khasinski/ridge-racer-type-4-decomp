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

extern u8 CdReadyResult[];

extern u8 CdDataEndResult[];

extern s32 VSync(s32);

extern s32 CheckCallback(void);

extern char *CD_intstr[];

extern s32 CD_timeout_deadline;

extern s32 CD_timeout_count;

extern const char *CdWaitCallerName[];

extern const char CdReadyNameStr[];

static inline int check_alarm_1(char **ac, char **sc, volatile u8 *st) {
    if (VSync(-1) > CD_timeout_deadline || CD_timeout_count++ > 0x3C0000) {
        PsyqPuts(CdTimeoutPrefixStr);
        {
            u8 s0_ = st[0];
            u8 s1_ = st[1];
            char *x5 = sc[s1_];
            DebugPrintStub(
                CdTimeoutStatusFmt,
                CdWaitCallerName[0],
                ac[CD_com],
                sc[s0_],
                x5
            );
        }
        CD_flush();
        return -1;
    }
    return 0;
}

static inline void cd_poll_cb_1(volatile u8 *st, volatile u8 *st1) {
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

s32 CD_ready(s32 mode, u8 *result) {
    s32 ret;
    u8 *src;
    u8 *dst;
    s32 i;
    char **ac;
    char **sc;
    volatile u8 *st;
    volatile u8 *st1;
    volatile u8 *st2;

    CD_timeout_deadline = VSync(-1) + 0x3C0;
    CD_timeout_count = 0;
    CdWaitCallerName[0] = CdReadyNameStr;
    ac = CD_comstr;
    sc = CD_intstr;
    st = CdIntrStatusBytes;
    st1 = st + 1;
    st2 = st + 2;
    do {
        if (check_alarm_1(ac, sc, st) != 0) {
            return -1;
        }
        if (CheckCallback() != 0) {
            cd_poll_cb_1(st, st1);
        }
        ret = *st2;
        if (ret != 0) {
            st[2] = 0;
            src = CdDataEndResult;
            if (result != 0) {
                dst = result;
                for (i = 7; i != -1; i--) {
                    *dst++ = *src++;
                }
            }
            return ret;
        }
        ret = st2[-1];
        if (ret != 0) {
            st[1] = 0;
            dst = result;
            src = CdReadyResult;
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
