/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "types.h"

#define R(o) (*(unsigned short *)(CdSpuRegBase + (o)))

/* legacy typing of shared globals in this part of the unit */
#define CD_cbsync (*(void (*(*))(u8, u8 *))&CD_cbsync)

#define CD_cbready (*(void (*(*))(u8, u8 *))&CD_cbready)

#undef CD_cbsync

#undef CD_cbready

/* legacy typing of shared globals in this part of the unit */
#define CD_reg0 ((unsigned char *)CD_reg0)

#define CD_REG1 ((unsigned char *)CD_REG1)

#define CD_REG2 ((unsigned char *)CD_REG2)

#undef CD_reg0

#undef CD_REG1

#undef CD_REG2

/* legacy typing of shared globals in this part of the unit */
#define CD_reg0 ((signed char *)CD_reg0)

#define CD_REG2 ((signed char *)CD_REG2)

#undef CD_reg0

#undef CD_REG2

/* legacy typing of shared globals in this part of the unit */
#define CD_reg0 ((unsigned char *)CD_reg0)

#define CD_REG1 ((unsigned char *)CD_REG1)

#define CD_REG2 ((unsigned char *)CD_REG2)

#undef CD_reg0

#undef CD_REG1

#undef CD_REG2

extern s32 DebugPrintStub();

extern s32 VSync(s32);

extern s32 CheckCallback(void);

extern s32 CD_sync(s32, u8 *);

extern const char D_80011AB0[];

extern const char StrCdNoParamFmt[];

extern const char CdCwLabel[];

extern s32 CD_debug;

extern u8 CD_pos[4];

extern u8 D_800A64A4;

extern u8 D_800A64A5;

extern char *CD_comstr[];

extern char *CD_intstr[];

extern s32 CdCommandParamTable[];

extern s32 D_800A66CC[];

extern volatile u8 *CD_reg0;

extern volatile u8 *CD_REG1;

extern volatile u8 *CD_REG2;

extern u8 D_800A6764[];

extern u8 CD_syncResult[];

extern s32 CD_timeout_deadline;

extern s32 CD_timeout_count;

extern const char *CdWaitCallerName;

static inline int check_alarm(char **ac, char ** sc, volatile u8 *st) {
    if (VSync(-1) > CD_timeout_deadline || CD_timeout_count++ > 0x3C0000) {
        PsyqPuts(CdTimeoutPrefixStr);
        { u8 s0_ = st[0]; u8 s1_ = st[1]; char *x5 = sc[s1_];
        /* ASM_BARRIER: Load CdWaitCallerName directly into $a1. */
        register const char *a1 __asm__("$5") = CdWaitCallerName;
        DebugPrintStub(
            CdTimeoutStatusFmt,
            a1,
            ac[D_800A64A5],
            sc[s0_],
            x5
        ); }
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

s32 CD_cw(u8 arg0, u8 *arg1, u8 *arg2, s32 arg3) {
    s32 i;
    u8 *src;
    u8 *dst;
    u8 *p;

    if (CD_debug >= 2) {
        DebugPrintStub(D_80011AB0, CD_comstr[arg0]);
    }
    if (D_800A66CC[arg0] != 0 && arg1 == 0) {
        if (CD_debug > 0) {
            DebugPrintStub(StrCdNoParamFmt, CD_comstr[arg0]);
        }
        return -2;
    }
    p = arg1;
    {
        /* ASM_BARRIER: Materialize first zero argument in $a0 before the asm barrier. */
        register s32 a0_zero __asm__("$4") = 0;
        /* ASM_BARRIER: Invalidate reload_cse_regs tracking on $a0 so $a1=0 stays move $a1,$zero. */
        __asm__("" : "+r"(a0_zero));
        CD_sync(a0_zero, 0);
    }
    if (arg0 == 2) {
        for (i = 0; i < 4; i++) {
            CD_pos[i] = p[i];
        }
    }
    if (arg0 == 0xE) {
        D_800A64A4 = *p;
    }
    D_800A6764[0] = 0;
    if (((s32 (*)[64])CdCommandParamTable)[0][arg0] != 0) {
        D_800A6764[1] = 0;
    }
    *CD_reg0 = 0;
    for (i = 0; i < ((s32 (*)[64])CdCommandParamTable)[1][arg0]; i++) {
        *CD_REG2 = p[i];
    }
    D_800A64A5 = arg0;
    *CD_REG1 = arg0;
    if (arg3 != 0) {
        return 0;
    }
    CD_timeout_deadline = VSync(-1) + 0x3C0;
    CD_timeout_count = 0;
    CdWaitCallerName = CdCwLabel;
    if (D_800A6764[0] == 0) {
        char **ac = CD_comstr;
        char **sc = CD_intstr;
        volatile u8 *st = D_800A6764;
        volatile u8 *st1 = st + 1;
        do {
            if (check_alarm(ac, sc, st)) {
                return -1;
            }
            if (CheckCallback() != 0) {
                cd_poll_cb(st, st1);
            }
        } while (*st == 0);
    }
    dst = arg2;
    src = CD_syncResult;
    if (dst != 0) {
        s32 n;
        for (n = 7; n != -1; n--) {
            *dst++ = *src++;
        }
    }
    return -(((volatile u8 *)D_800A6764)[0] == 5);
}
