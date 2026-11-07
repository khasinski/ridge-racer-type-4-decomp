/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "psyq/libpad/padpoll.h"
#include "psyq/libpad/padwait.h"
#include "psyq/libpad/padenab.h"
#include "psyq/libpad/padopen.h"
struct HW { unsigned char b0; char p1[3]; volatile unsigned short s4; short p6; volatile short s8; volatile unsigned short sA; char pC[2]; volatile short sE; };

struct A { char p0[0xC]; char *f0C; char p1[0x27]; unsigned char b37; char p2[0x18]; unsigned char b50; char p3[0x97]; unsigned char bE8; };

struct W { int x; };

extern int EnterCriticalSection();
extern int ChangeClearRCnt();
extern int SysDeqIntRP();
extern int ExitCriticalSection();
extern struct HW *PadSio0Base;
extern volatile int *I_STAT_Pad;
extern int PadActiveChannel;
extern struct W PadChannelPendingPackets[];
extern void (*PadPacketDoneCallback)();
extern void (*PadSaveCommandCallback)();

int _padInitSioMode();

void _padStopCom() { EnterCriticalSection(); ChangeClearRCnt(3, 1); SysDeqIntRP(2, &PadIntQueueEntry); ExitCriticalSection(); }

int _padInitSioMode(a0) struct A *a0; {
    struct W *p;
    struct HW *hw;
    int v;
    int t;
    int cur;
    PadSio0Base->sA = 0x40;
    PadSio0Base->sA = 0;
    PadSio0Base->s8 = 0xD;
    PadSio0Base->sE = 0x88;
    setRC2wait(a0->bE8 == 8 ? 0x50 : 0x91);
    hw = PadSio0Base;
    t = 0x1003;
    if (PadActiveChannel != 0) t = 0x3003;
    hw->sA = t;
    if ((cur = (&PadChannelPendingPackets[PadActiveChannel])->x) >= 0) {
        struct W *base;
        if (cur > 0) {
            base = PadChannelPendingPackets;
        lp:
            v = base[PadActiveChannel].x - 1;
            base[PadActiveChannel].x = v;
            PadPacketDoneCallback(a0->f0C + v * 0xF0);
            if (base[PadActiveChannel].x > 0) goto lp;
        }
        p = &PadChannelPendingPackets[PadActiveChannel];
        if (p->x == 0) {
            p->x = -1;
            PadPacketDoneCallback(a0);
            PadSaveCommandCallback(a0);
        }
    }
    if (PadSio0Base->s4 & 0x200) {
        PadSio0Base->sA |= 0x10;
        if (PadSio0Base->s4 & 0x200) {
            while (chkRC2wait() == 0);
            PadSio0Base->b0 = 1;
            setRC2wait(0x64);
            if (_padClrIntSio0() == 0) return 0;
            _padWaitRXready();
            *(volatile unsigned char *)&PadSio0Base->b0;
            setRC2wait(0x1AE);
            goto l1;
        l1b:
            if (chkRC2wait() != 0) return 0;
        l1:
            if (!(*I_STAT_Pad & 0x80)) goto l1b;
            PadSio0Base->b0 = 0x42;
            setRC2wait(0x3C);
            if (_padClrIntSio0() == 0) return 0;
            _padWaitRXready();
            *(volatile unsigned char *)&PadSio0Base->b0;
            setRC2wait(0x1AE);
            goto l2;
        l2b:
            if (chkRC2wait() != 0) return 0;
        l2:
            if (!(*I_STAT_Pad & 0x80)) goto l2b;
            PadSio0Base->b0 = 1;
            setRC2wait(0x3C);
            if (_padClrIntSio0() == 0) return 0;
            _padWaitRXready();
            *(volatile unsigned char *)&PadSio0Base->b0;
            return 0;
        }
        *I_STAT_Pad = -0x81;
    }
    if (a0->b50 != 0 && a0->b37 != 0) return 0;
    return 1;
}
