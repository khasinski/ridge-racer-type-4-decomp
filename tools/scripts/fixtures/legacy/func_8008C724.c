/* MASPSX_FLAGS: --assembler-profile=gnu */
/* CC1_FLAGS: -mno-split-addresses */
/* RODATA 0x80011A88 0x14 */

#include "types.h"

extern s32 DebugPrintStub();
extern s32 PsyqPuts(const char *);

extern const char DiskErrorPrefixStr[];
extern const char StrCdComCodeFmt[];
extern const char CdUnknownIntrMsg[];
extern const char CD_fmt_intr_code[];

extern s32 CD_debug;
extern s32 CD_status;
extern s32 CD_status1;
extern s32 CD_nopen;
extern u8 D_800A64A5;
extern char *CD_comstr[];
extern s32 CD_comHasCompleteInt[];
extern s32 D_800A664C[];
extern volatile u8 *CD_reg0;
extern volatile u8 *CD_REG1;
extern volatile u8 *CD_REG2;
extern volatile u8 *CD_reg3;
extern volatile u8 D_800A6764[];

extern u8 CD_syncResult[];
extern u8 CdReadyResult[];
extern u8 D_80110328[];

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

s32 getintr(void) {
    volatile u8 intr;
    u8 res[8];
    s32 i;
    s32 j;
    s32 err;

    *CD_reg0 = 1;
    err = 0;
    if ((intr = *CD_reg3 & 7) == 0) {
        return 0;
    }
    while (intr != (*CD_reg3 & 7)) {
        intr = *CD_reg3 & 7;
    }
    for (i = 0; i < 8; i++) {
        if ((*CD_reg0 & 0x20) == 0) {
            break;
        }
        res[i] = *CD_REG1;
    }
    for (j = i; j < 8; j++) {
        res[j] = 0;
    }
    *CD_reg0 = 1;
    *CD_reg3 = 7;
    *CD_REG2 = 7;
    {
        s32 t = intr;
        if (t != 3 || D_800A664C[D_800A64A5] != 0) {
            if ((CD_status & 0x10) == 0 && (res[0] & 0x10) != 0) {
                CD_nopen++;
            }
            CD_status = *(volatile u8 *)res;
            err = CD_status & 0x1D;
            CD_status1 = res[1];
        }
    }
    {
        s32 t = intr;
        if (t == 5) {
            if (CD_debug > 0) {
                DebugPrintStub(DiskErrorPrefixStr);
            }
            if (CD_debug > 0) {
                DebugPrintStub(StrCdComCodeFmt, CD_comstr[D_800A64A5], CD_status, CD_status1);
            }
        }
    }
    switch (intr) {
    case 3:
        if (err) {
            D_800A6764[0] = 5;
            COPY8(CD_syncResult, res);
            return 2;
        }
        if (CD_comHasCompleteInt[D_800A64A5]) {
            D_800A6764[0] = 3;
            COPY8(CD_syncResult, res);
            return 1;
        }
        D_800A6764[0] = 2;
        COPY8(CD_syncResult, res);
        return 2;
    case 2:
        D_800A6764[0] = err ? 5 : 2;
        COPY8(CD_syncResult, res);
        return 2;
    case 1:
        if (err && i == 1) {
            err = 0;
        }
        {
            u8 v = err ? 5 : 1;
            volatile u8 *p = D_800A6764;
            p[1] = v;
        }
        COPY8(CdReadyResult, res);
        *CD_reg0 = 0;
        *CD_reg3 = 0;
        return 4;
    case 4:
        {
            volatile u8 *p = D_800A6764;
            p[1] = p[2] = 4;
        }
        COPY8(D_80110328, res);
        COPY8(CdReadyResult, res);
        return 4;
    case 5:
        D_800A6764[0] = D_800A6764[1] = 5;
        COPY8(CD_syncResult, res);
        COPY8(CdReadyResult, res);
        return 6;
    default:
        PsyqPuts(CdUnknownIntrMsg);
        DebugPrintStub(CD_fmt_intr_code, intr);
        return 0;
    }
}
