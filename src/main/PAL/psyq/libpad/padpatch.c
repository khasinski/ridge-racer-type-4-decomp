/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "types.h"
#include "m2c_macros.h"
#include "rr/layout.h"
#include "psyq/libpad/padpatch.h"

struct S { char pad[0x46]; unsigned char c46, c47, c48; };

struct S2 {
    char p0[2];
    unsigned char b2, b3, b4, b5, b6, b7;
};

struct E4 {
    unsigned char b0, b1, b2, b3, b4;
};

struct E8 {
    unsigned char b0;
    char pad[3];
    unsigned char *p4;
};

struct S_1 {
    unsigned short *f0;
    struct E4 *f4;
    struct E8 *f8;
    char p0[0x30];
    struct S2 *q;
    char p1[6];
    unsigned char st, cnt, c48, f49;
    char p3[0x99];
    unsigned char c3, c4;
    unsigned short s6;
    char p4[1];
    unsigned char c9, ca, cb;
    unsigned short sc, se;
};

extern int _padCmd4C();
extern int _padCmd46();
extern int _padCmd47();
extern int _padCmd4B();
extern unsigned char *PadPatchWritePtr;
extern int (*PadChannelBusyFn)();
/* Partial layout: six input slot IDs, per-group limits, six output IDs. */
typedef struct {
    char pad0[4];
    u8 *group_data;
    char pad8[0x20 - 8];
    u8 *slot_groups;
    char pad24[0x46 - 0x24];
    u8 status;
    char pad47[0x5D - 0x47];
    u8 assignments[6];
    char pad63[0xE9 - 0x63];
    u8 group_count;
} RrSlotAssignmentState;
RR_ASSERT_SIZE(RrSlotAssignmentState, 0xEC);
RR_ASSERT_OFFSET(RrSlotAssignmentState, group_data, 4);

void _padSendActInfoCmd();
int _padRecvActInfoReply(struct S_1 *s);
void _padActAlignUpdate(void *arg0);
s32 _padActAlignComplete(RrSlotAssignmentState *state);

void _padSendActInfoCmd(s) struct S *s; {
    switch (s->c46) {
    case 2: _padCmd4C(s, s->c47); break;
    case 3: _padCmd46(s, s->c47); break;
    case 4:
        if (s->c48 == 0) _padCmd47(s, s->c47);
        else _padCmd4B(s);
        break;
    }
}

int _padRecvActInfoReply(struct S_1 *s) {
    struct S2 *q;
    struct E4 *p5;
    struct E8 *e8;
    unsigned char *src;
    int i;
    int t;

    switch (s->st) {
    case 2:
        if (s->q->b2 != 0) return 0;
        if (s->q->b3 != 0) return 0;
        s->f0[s->cnt] = s->q->b5 + (s->q->b4 << 8);
        if (s->se != s->f0[s->cnt]) {
            s->se = s->f0[s->cnt];
            return 0;
        }
        s->se = 0;
        s->cb = 0;
        if (++s->cnt < s->c3) return 0;
        s->cnt = 0;
        break;

    case 3:
        q = s->q;
        if (q->b2 != 0) return 0;
        if (q->b3 != 0) return 0;
        p5 = &s->f4[s->cnt];
        if (p5->b0 == q->b4 && p5->b1 == (*(volatile unsigned char *)&q->b5 & 0x7F) && p5->b2 == q->b6 && p5->b3 == q->b7 && p5->b4 == ((t = q->b5) >> 7)) {
            s->se = 0;
        } else {
            s->se = 0xFFFF;
        }
        p5->b0 = s->q->b4;
        p5->b1 = s->q->b5 & 0x7F;
        p5->b2 = s->q->b6;
        p5->b3 = s->q->b7;
        t = s->q->b5;
        p5->b4 = t >> 7;
        if (s->se != 0) return 0;
        s->cb = 0;
        if (++s->cnt < s->c9) return 0;
        s->cnt = 0;
        s->c48 = 0;
        break;

    case 4:
        q = s->q;
        if (q->b2 != 0) {
            s->c48 = 0;
            return 0;
        }
        e8 = &s->f8[s->cnt];
        if (s->c48 == 0) {
            i = 3;
            e8->b0 = s->c48 = q->b4;
            src = &s->q->b5;
            if (s->cnt == 0) {
                PadPatchWritePtr = e8->p4 = (unsigned char *)(s->f8 + s->ca);
            } else {
                PadPatchWritePtr = e8->p4 = e8[-1].p4 + ((e8[-1].b0 + 3) & 0x1FC);
            }
        } else {
            i = 5;
            src = &q->b3;
        }
        while (i-- != 0) {
            if (s->c48 == 0) break;
            if (PadPatchWritePtr >= &s->c3) {
                s->cnt = 0;
                s->c48 = 0;
                return 0;
            }
            if (*PadPatchWritePtr != *src) {
                s->se = 0xFFFF;
            }
            {
                unsigned char **pp = &PadPatchWritePtr;
                unsigned char *p = *pp;
                *p = *src++;
                *pp = p + 1;
            }
            s->c48--;
        }
        if (s->c48 == 0) {
            if (s->se != 0) {
                s->se = 0;
                s->c48 = 0;
                return 0;
            }
            if (++s->cnt >= s->ca) {
                s->f49 = 6;
                s->st = 0xFE;
                s->cb = 0;
                return 0;
            }
            s->c48 = 0;
            s->cb = 0;
            return 0;
        }
        return 0;

    default:
        return 1;
    }
    return 1;
}

int _padSetActAlign(a0, a1)
char *a0; int a1;
{
    int r;
    if (PadChannelBusyFn(a0, a1) == 0) {
        /* ASM_BARRIER: keep separate li 1 for return value (original has two li 1) */
        __asm__ volatile("" : "=r"(r) : "0"(1));
        a0[0x46] = 1;
        *(int *)(a0 + 0x14) = (int)_padActAlignUpdate;
        *(int *)(a0 + 0x20) = a1;
        *(int *)(a0 + 0x18) = (int)_padActAlignComplete;
        return r;
    }
    return 0;
}

void _padActAlignUpdate(void *arg0) {
    M2C_FIELD(arg0, s8 *, 0x37) = 0x4D;
    M2C_FIELD(arg0, s8 *, 0x36) = 6;
    M2C_FIELD(arg0, s32 *, 0x2C) = (s32) M2C_FIELD(arg0, s32 *, 0x20);
}

s32 _padActAlignComplete(RrSlotAssignmentState *state)
{
    s32 group, minimum, remaining, i;
    u8 *slots;

    /* Groups with fewer slots than their minimum are marked unassigned. */
    group = 0;
    if (state->group_count != 0) {
        do {
            slots = state->slot_groups;
            remaining = 0;
            for (i = 5; i >= 0; i--) {
                if (*slots++ == group) remaining++;
            }
            minimum = state->group_data[group * 5 + 2];
            slots = state->slot_groups;
            if (minimum == 0) minimum = 1;
            i = 0;
            do {
                if (*slots++ == group) {
                    if (remaining < minimum) {
                        state->assignments[i] = 0xFF;
                        remaining--;
                    } else {
                        state->assignments[i] = group;
                    }
                }
                i++;
            } while (i < 6);
            group++;
        } while (group < state->group_count);
    }
    state->status = 0xFE;
    return 0;
}
