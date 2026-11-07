/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "types.h"
#include "m2c_macros.h"
#include "rr/channels.h"

#define VQ (*(struct S2 * volatile *)&a0->q)

struct S2 { char p0[2]; unsigned char b2, b3, b4, b5, b6, b7; };

struct S {
    char p0[0x3C];
    struct S2 *q;
    char p1[6];
    unsigned char st, cnt;
    char p2[1];
    unsigned char f49;
    char p3[0x99];
    unsigned char c3, c4;
    unsigned short s6;
    char p4[1];
    unsigned char c9, ca, cb;
    unsigned short sc, se;
};

struct S_1 { char pad[0xE3]; unsigned char a; char pad1[5]; unsigned char b; char pad2[2]; unsigned short c; };

typedef struct { int f0; int f4; int f8; char p0[8]; int f14; int f18; char p1[0x2A]; char f46; char f47; char p2[1]; char f49; char p3[0x99]; unsigned char fE3; char p4[5]; unsigned char fE9; } O;

extern int _padSendActInfoCmd();
extern int _padRecvActInfoReply();

void _padSetAct(void *arg0, s32 arg1, s8 arg2);
void _padSetCmd(void *arg0, s8 arg1, s32 arg2, s8 arg3);
int _padRecvAtLoadInfo();
int _padGetActSize();
int _padLoadActInfo();

void _padSetAct(void *arg0, s32 arg1, s8 arg2) {
    M2C_FIELD(arg0, s32 *, 0x28) = arg1;
    M2C_FIELD(arg0, s8 *, 0x34) = arg2;
}

void _padSetCmd(void *arg0, s8 arg1, s32 arg2, s8 arg3) {
    M2C_FIELD(arg0, s8 *, 0x37) = arg1;
    M2C_FIELD(arg0, s32 *, 0x2C) = arg2;
    M2C_FIELD(arg0, s8 *, 0x36) = arg3;
}

void _padSendAtLoadInfo(u8 *channel) {
    RrChannelStatusView *state=(RrChannelStatusView *)channel;
    switch(state->phase) {
    case 2:_padCmd45(state);break;
    case 3:_padCmd4C(state,state->argumentE4);break;
    case 4:_padCmd47(state,state->state47);break;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define PadResetChannelFn (*(void (*(*))(struct S *))&PadResetChannelFn)

int _padRecvAtLoadInfo(a0) struct S *a0; {
    struct S2 *q;
    int t, n;
    /* ASM_BARRIER: pin b4 to $v1 so the load/add chain takes the original register allocation */
    register int b __asm__("$3");
    switch (a0->st) {
    case 2:
        q = a0->q;
        if (q->b7 != 0) goto r0;
        if (a0->c3 == q->b3 && a0->c4 == q->b4 && a0->c9 == q->b5 && a0->ca == q->b6) {
            a0->se = 0;
        } else {
            a0->se = 0xFFFF;
        }
        a0->c3 = VQ->b3;
        a0->c4 = VQ->b4;
        a0->s6 = 0;
        a0->c9 = VQ->b5;
        a0->ca = VQ->b6;
        a0->sc = 0;
        if (a0->se != 0) goto r0;
        a0->cb = 0;
        goto r1;
    case 3:
        if (a0->q->b2 != 0) goto r0;
        if (a0->q->b3 != 0) goto r0;
        t = a0->q->b5 + (a0->q->b4 << 8);
        a0->s6 = t;
        if (a0->se == (t & 0xFFFF)) goto eq;
        a0->se = t;
    r0:
        return 0;
    eq:
        a0->se = 0xFFFF;
        a0->cb = 0;
        a0->cnt = 0;
        goto r1;
    case 4:
        if (a0->q->b2 != 0) goto r0;
        if (a0->q->b3 != 0) goto r0;
        t = a0->sc;
        b = a0->q->b4;
        t += 8;
        t += (b + 3) & 0x1FC;
        n = a0->cnt + 1;
        a0->cnt = n;
        a0->sc = t;
        if ((unsigned char)n < a0->ca) goto r0;
        if (_padGetActSize(a0) >= 0x81) {
            PadResetChannelFn(a0);
            a0->st = 0xFE;
            a0->f49 = 2;
            goto r0;
        }
        if (a0->se != a0->sc) {
            a0->se = a0->sc;
            a0->cnt = 0;
            a0->sc = 0;
            return 0;
        }
        a0->se = 0;
        a0->cb = 0;
        a0->st = 0xFF;
        _padLoadActInfo(a0, (char *)a0 + 0x63);
        a0->st = 2;
        goto r0;
    default:
        return 1;
    }
r1:
    return 1;
}

#undef PadResetChannelFn

int _padGetActSize(s) struct S_1 *s; {
    int a = s->a, b = s->b, c = s->c;
    int x = ((a + 1) >> 1) * 4;
    int y = ((b * 5 + 3) & 0xFFC) + 4;
    return x + y + c;
}

/* legacy typing of shared globals in this part of the unit */
#define PadChannelBusyFn (*(int (*(*))())&PadChannelBusyFn)

int _padLoadActInfo(o, a1) O *o; int a1; {
    int r;
    if (a1 != 0) {
        if (o->f4 != 0) return 0;
        if (PadChannelBusyFn() != 0) return 0;
    } else return 0;
    /* ASM_BARRIER: return value 1 must be a separate li (not CSE'd with the constant 1 used by the stores/shift) */
    asm volatile("" : "=r"(r) : "0"(1));
    o->f49 = 4;
    o->f0 = (a1 + 3) >> 2 << 2;
    o->f4 = o->f0 + ((o->fE3 + 1) >> 1 << 2); o->f8 = o->f4 + ((o->fE9 * 5 + 3) & 0xFFC);
    o->f46 = 1;
    o->f14 = (int)_padSendActInfoCmd;
    o->f47 = 0;
    o->f18 = (int)_padRecvActInfoReply;
    return r;
}

#undef PadChannelBusyFn
