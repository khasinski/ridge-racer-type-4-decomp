#include "types.h"

#define G ((Scratch *)0x1F800000)

#define S16(p, o) (*(s16 *)((char *)(p) + (o)))

#define S32(p, o) (*(s32 *)((char *)(p) + (o)))

#define VEC(p, o) (*(VECTOR *)((char *)(p) + (o)))

extern s32 ModelBankFormat;

extern int *ModelBankTable[];

void SelectModelBank(int a0) {
    int *v = ModelBankTable[a0];
    int *w;
    int a2;
    int *g = (int *)0x1F800000;
    /* ASM_BARRIER: legacy, reason not recorded (forces reg copy/CSE/scheduling); retire & retest with fd.py */
    __asm__("" : "=r"(w) : "0"(v));
    a2 = v[0];
    g[0x15] = w[1];
    g[0x16] = ModelBankTable[a0][2];
    ModelBankFormat = a2;
    g[0x14] = (int)ModelBankTable[a0] + 0xC;
}
