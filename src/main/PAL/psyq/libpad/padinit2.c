/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "rr/layout.h"
#include "psyq/libpad/padpoll.h"
#include "psyq/libpad/padwait.h"
#include "rr/serial.h"
#include "psyq/libpad/padrecv.h"

typedef struct { char pad[0x3C]; char *f3c; } T;

typedef struct { char p0[0xC]; int fC; char p1[0x27]; unsigned char f37; } O;

typedef struct Rec8009991C {
    u8 pad00[0xC];
    struct Rec8009991C *unk0C;
    u8 pad10[0x27];
    u8 unk37;
    u8 pad38[4];
    u8 *unk3C;
    u8 pad40[4];
    u8 unk44;
    u8 pad45[0xF0 - 0x45];
} Rec8009991C;

typedef struct {
    u8 unk00;
    u8 pad01[0xD];
    u16 unk0E;
} Unk800ABCE4;

extern int (*PadChannelDispatchPtr)();
extern int PadTapDispatchFlag;
extern int PadActiveChannel;
extern int PadFirstChannel;
extern int PadFrameHookEnabled;
extern int (*PadFirstChannelHook)();
extern int (*PadFirstChannelHook2)();
extern int PadMultitapEnabled;
extern int PadPacketBytesLeft;
/* Partial transfer state; field purposes beyond this routine are still unknown. */
typedef struct {
    u8 pad00[0x37];
    u8 flag37;
    u8 pad38[4];
    u8 *packet;
} RrTransferState;
RR_ASSERT_SIZE(RrTransferState, 0x40);
RR_ASSERT_OFFSET(RrTransferState, flag37, 0x37);
extern s32 (*PadTxByteCallback)(RrTransferState *, s32);
extern void (*PadAdvanceChannelFn)(s32);
extern void (*PadAllocGatesFn)(Rec8009991C *);
extern void (*PadPacketDoneCallback)(Rec8009991C *);
extern void (*PadSaveCommandCallback)(Rec8009991C *);
extern Rec8009991C *PadChannelRecords;
extern s32 PadChannelPendingPackets[];
extern Unk800ABCE4 *JOY_DATA;

void PadRecvStageStart(T *a0);
void PadRecvStageCommand();
int _padRecvPadId();
s32 _padTxByteChkAck(RrTransferState *state);
s32 _padTransferPacket(Rec8009991C *arg0);

void PadRecvStageStart(T *a0) {
    int r = PadChannelDispatchPtr(a0);
    char *p = a0->f3c;
    PadTapDispatchFlag = r;
    *p = 0;
    _padSioRW(a0, -2);
}

/* legacy typing of shared globals in this part of the unit */
#define _padSioRW2 ((int (*)())_padSioRW2)

void PadRecvStageCommand(o) O *o; {
    if (PadActiveChannel == PadFirstChannel) {
        if (PadFrameHookEnabled != 0) {
            PadFirstChannelHook();
            PadFirstChannelHook2();
        }
    }
    if (PadTapDispatchFlag != 0) {
        PadChannelDispatchPtr(o->fC);
        PadChannelDispatchPtr(o->fC + 0xF0);
    }
    if (o->f37 == 0) _padSioRW2(o, 0x42); else _padSioRW2(o, o->f37);
}

#undef _padSioRW2

/* legacy typing of shared globals in this part of the unit */
#define _padSioRW2 ((int (*)())_padSioRW2)

int _padRecvPadId(a0) unsigned char *a0; {
  int a1, v, t;
  if (PadTapDispatchFlag != 0) {
    PadChannelDispatchPtr(*(int *)(a0 + 0xC) + 0x1E0);
    PadChannelDispatchPtr(*(int *)(a0 + 0xC) + 0x2D0);
  }
  a1 = 0;
  if (a0[0x37] == 0) a1 = PadMultitapEnabled;
  v = _padSioRW2(a0, a1);
  if (v < 0) return v;
  t = (v & 0xF) * 2;
  PadPacketBytesLeft = t;
  if (t == 0) PadPacketBytesLeft = 0x20;
  return 0;
}

#undef _padSioRW2

s32 _padTxByteChkAck(RrTransferState *state) {
    s32 mode = 0;
    s32 result;

    if ((state->packet[0] >> 4) == 8) {
        mode = state->flag37 == 0;
    }
    result = _padSioRW2(state, (u8)PadTxByteCallback(state, mode));
    if (result == 0x5A || result == 0 || result < 0) return result;
    return -4;
}

/* legacy typing of shared globals in this part of the unit */
#define PadTxByteCallback (*(s32 (*(*))(Rec8009991C *, s32))&PadTxByteCallback)
#define _padSioRW ((s32 (*)(Rec8009991C *, u8))_padSioRW)
#define _padSioRW2 ((s32 (*)(Rec8009991C *, u8))_padSioRW2)

s32 _padTransferPacket(Rec8009991C *arg0) {
    s32 i;
    s32 res;
    s32 idx;
    Rec8009991C *ptr;
    s32 mode;
    s32 b;

    PadAllocGatesFn(arg0);
    mode = 0;
    if (PadMultitapEnabled != 0 && ((b = arg0->unk3C[0]) >> 4) == 8) {
        mode = (arg0->unk37 == 0);
    }

    if (mode != 0) {
        for (i = -1; i < 4; i++) {
            if (--PadPacketBytesLeft <= 0) {
                break;
            }
            if (i >= 0) {
                PadAllocGatesFn(&arg0->unk0C[i]);
            }
            res = _padSioRW2(arg0, (u8)PadTxByteCallback(arg0, 1));
            if (res < 0) {
                return res;
            }
            setRC2wait(0x3C);
            if (_padClrIntSio0() == 0) {
                return -3;
            }
        }
    }

    ptr = 0;
    idx = (PadActiveChannel == 0);
    while (PadPacketBytesLeft >= 2) {
        if (PadChannelPendingPackets[idx] < 0) {
            break;
        }
        if (PadChannelPendingPackets[idx] > 0) {
            ptr = &PadChannelRecords[idx].unk0C[PadChannelPendingPackets[idx]] - 1;
            PadPacketDoneCallback(ptr);
            {
                void *a0r;
            }
        }
        switch (PadChannelPendingPackets[idx]) {
        case 4:
            PadChannelPendingPackets[idx] = 3;
            break;
        case 3:
            PadPacketDoneCallback(ptr - 1);
            PadChannelPendingPackets[idx] = 1;
            break;
        case 0:
        case 1:
            ptr = &PadChannelRecords[idx];
            PadPacketDoneCallback(ptr);
            PadSaveCommandCallback(ptr);
            PadChannelPendingPackets[idx] = -1;
            break;
        }
        res = _padSioRW(arg0, (u8)PadTxByteCallback(arg0, mode));
        if (res < 0) {
            return res;
        }
        setRC2wait(0x3C);
        if (_padClrIntSio0() == 0) {
            return -3;
        }
        PadPacketBytesLeft--;
    }

    while (--PadPacketBytesLeft > 0) {
        res = _padSioRW(arg0, (u8)PadTxByteCallback(arg0, mode));
        if (res < 0) {
            return res;
        }
        if (JOY_DATA->unk0E != 0x22) {
            setRC2wait(0x3C);
            if (_padClrIntSio0() == 0) {
                res = -3;
                goto end;
            }
        }
    }

    _padWaitRXready();
    arg0->unk3C[arg0->unk44++] = JOY_DATA->unk00;
    PadAdvanceChannelFn(0);
    res = 0;
end:
    return res;
}

#undef PadTxByteCallback
#undef _padSioRW
#undef _padSioRW2
