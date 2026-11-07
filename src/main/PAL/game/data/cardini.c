/* RODATA 0x800113f0 0x1c */
#include "types.h"
#include "m2c_macros.h"
#include "rr/bios_devices.h"
#include "psyq/libcard/cardirq.h"
#include "psyq/libcard/cardsts.h"
#include "psyq/libcard/init.h"
#include "psyq/libc/printf.h"
#include "psyq/libcard/cardc162.h"
#include "psyq/libcard/init.h"
#include "psyq/libgpu/gpuctl.h"
#include "game/data/cardini.h"
#include "rr/bios_events.h"
#include "rr/serial_handles.h"
#include "rr/overlay667_packet.h"
#include "rr/card_format.h"
#include "game/data/globals.h"

extern int _bu_init();
extern int EnterCriticalSection();
extern int ExitCriticalSection();
extern int HwCardEvIoe;
extern int HwCardEvError;
extern int HwCardEvTimeout;
extern int HwCardEvNew;
extern int SwCardEvIoe;
extern int SwCardEvError;
extern int SwCardEvTimeout;
extern int SwCardEvNew;
extern unsigned char CardSlotState[];
extern char SharedWorkBuffer[];
extern int CardPortTimer[];
extern char StrCardDirPatternFmt[];
extern char CardDirEntries[];
extern char *nextfile();
extern char CardFilePathFmt[];
extern int CardFileHandle;
extern int FileSeek();
extern int _card_info();
extern int _card_load();
extern int CardPortState[];
extern int CardPortLastResult[];
extern int CardPortResult[];
extern int CardPortLoaded[];
extern char CardPortCmd5ABuffers[];

void CardSystemShutdown(void);
int PollHwCardEvents(int arg);
void ClearHwCardEvents(void);
int WaitSwCardEvent(void);
void ClearSwCardEvents(void);
int CountCardFiles(int a, int b);
int CardHasFile(int a0, unsigned char *a1);
int CardReadFileAt(int a0, int a1, int a2, int a3, int a4, int a5);
int CardWriteFileAt(int a0, int a1, int a2, int a3, int a4, int a5);
int CardUpdate(int p, char *a1);
void CardFormatSlot(s32 arg0, s32 arg1);

void CardSystemInit()
{
    InitCARD();
    StartCARD();
    _bu_init();
    _cardStartCom();
    EnterCriticalSection();
    HwCardEvIoe = OpenEvent(0xF4000001, 4, 0x2000, 0);
    HwCardEvError = OpenEvent(0xF4000001, 0x8000, 0x2000, 0);
    HwCardEvTimeout = OpenEvent(0xF4000001, 0x100, 0x2000, 0);
    HwCardEvNew = OpenEvent(0xF4000001, 0x2000, 0x2000, 0);
    SwCardEvIoe = OpenEvent(0xF0000011, 4, 0x2000, 0);
    SwCardEvError = OpenEvent(0xF0000011, 0x8000, 0x2000, 0);
    SwCardEvTimeout = OpenEvent(0xF0000011, 0x100, 0x2000, 0);
    SwCardEvNew = OpenEvent(0xF0000011, 0x2000, 0x2000, 0);
    ExitCriticalSection();
    EnableEvent(HwCardEvIoe);
    EnableEvent(HwCardEvError);
    EnableEvent(HwCardEvTimeout);
    EnableEvent(HwCardEvNew);
    EnableEvent(SwCardEvIoe);
    EnableEvent(SwCardEvError);
    EnableEvent(SwCardEvTimeout);
    EnableEvent(SwCardEvNew);
    CardSlotState[1] = 0x5A;
    CardSlotState[0] = 0x5A;
    SaveBuffer = (int)SharedWorkBuffer;
}

void CardSystemShutdown(void) {
    EnterCriticalSection();
    CloseEvent(HwCardEvIoe);
    CloseEvent(HwCardEvError);
    CloseEvent(HwCardEvTimeout);
    CloseEvent(HwCardEvNew);
    CloseEvent(SwCardEvIoe);
    CloseEvent(SwCardEvError);
    CloseEvent(SwCardEvTimeout);
    CloseEvent(SwCardEvNew);
    ExitCriticalSection();
    _cardStopCom();
    StopCARD();
}

int PollHwCardEvents(int arg) {
    int r;
    int t;
    r = TestEvent(HwCardEvIoe) == 1;
    if (TestEvent(HwCardEvError) == 1) r = 2;
    if (TestEvent(HwCardEvTimeout) == 1) r = 3;
    if (TestEvent(HwCardEvNew) == 1) r = 4;
    t = CardPortTimer[arg]++;
    if (t >= 0x78) r = 2;
    return r;
}

s32 GetHwCardEvent(void) {
    if (TestEvent(HwCardEvIoe) == 1) {
        return 1;
    }
    if (TestEvent(HwCardEvError) == 1) {
        return 2;
    }
    if (TestEvent(HwCardEvTimeout) != 1) {
        return (TestEvent(HwCardEvNew) == 1) * 4;
    }
    return 3;
}

void ClearHwCardEvents(void) {
    TestEvent(HwCardEvIoe);
    TestEvent(HwCardEvError);
    TestEvent(HwCardEvTimeout);
    TestEvent(HwCardEvNew);
}

int WaitSwCardEvent(void) {
    for (;;) {
        if (TestEvent(SwCardEvIoe) == 1) return 1;
        if (TestEvent(SwCardEvError) == 1) return 2;
        if (TestEvent(SwCardEvTimeout) == 1) return 3;
        if (TestEvent(SwCardEvNew) == 1) return 4;
    }
}

void ClearSwCardEvents(void) {
    TestEvent(SwCardEvIoe);
    TestEvent(SwCardEvError);
    TestEvent(SwCardEvTimeout);
    TestEvent(SwCardEvNew);
}

int CountCardFiles(int a, int b) {
    char sp[32];
    char *s0;
    int n = 0;
    PsyqSprintf(sp, StrCardDirPatternFmt, a, b);
    s0 = CardDirEntries;
    if (firstfile(sp, s0) == s0) {
        do { n++; s0 += 0x28; } while (nextfile(s0) == s0);
        n = n;
    }
    return n;
}

int CardFreeBlocks(int a0) {
    int a2 = 0, a1 = 0;
    char *v1;
    char *p = CardDirEntries;
    int v;
    if (a0 > 0) { v1 = p; do { a1 += *(int *)(v1 + 0x18); v1 += 0x28; } while (++a2 < a0); }
    v = a1;
    if (v < 0) v += 0x1FFF;
    a1 = v >> 13;
    return 15 - a1;
}

int CardHasFile(int a0, unsigned char *a1) {
    int t1 = 0, t2 = 0;
    char *t0;
    int a3, v1;
    unsigned char a2;
    char *b = CardDirEntries;
    if (a0 <= 0) goto end;
    t0 = b;
outer:
    a3 = 0;
    v1 = a3;
top:
    a2 = a1[v1];
    if (a2 == 0) goto l64;
    if (a2 != (unsigned char)t0[v1]) goto l74;
    v1++;
    if (v1 < 0x15) goto top;
l64:
    t1++;
    if (a3 != 0) goto l7c;
    t2 = 1;
    goto end;
l74:
    a3++;
    goto l64;
l7c:
    t0 += 0x28;
    if (t1 < a0) goto outer;
end:
    return t2;
}

int CardReadFile(a0, a1, a2, a3, a4)
int a0, a1, a2, a3, a4;
{
    int sp[8];
    int r;
    int v;
    int ret;
    PsyqSprintf(sp, CardFilePathFmt, a0, a1, a2);
    v = a4 << 13;
    r = FileOpen(sp, 0x8001);
    if (r != -1) {
        CardFileHandle = r;
        ClearHwCardEvents();
        while (FileRead(r, a3, v) == 0) {
        }
        ret = 0;
        FileClose(r);
    } else {
        ret = -1;
    }
    return ret;
}

int CardReadFileAt(int a0, int a1, int a2, int a3, int a4, int a5) {
    char sp18[32];
    int h; register int r asm("$17"); int t;
    PsyqSprintf(sp18, CardFilePathFmt, a0, a1, a2);
    t = a4;
    h = FileOpen(sp18, 0x8001);
    /* ASM_BARRIER: loads -1 into r after the call, not hoisted before it */
    __asm__ volatile("" : "=r"(r) : "0"(-1));
    if (h != r) {
        if (FileSeek(h, a5, 0) != r) {
            CardFileHandle = h;
            ClearHwCardEvents();
            while (FileRead(h, a3, t) == 0) ;
            r = 0;
        }
        FileClose(h);
        return r;
    }
    /* ASM_BARRIER: legacy, reason not recorded (forces reg copy); retire & retest with fd.py */
    __asm__ volatile("" : "=r"(r) : "0"(r));
    return r;
}

int CardWriteFile(a0, a1, a2, a3, a4, a5)
int a0, a1, a2, a3, a4, a5;
{
    int sp[8];
    int r;
    int v;
    int ret;
    PsyqSprintf(sp, CardFilePathFmt, a0, a1, a2);
    if (a5 == 1) {
        r = FileOpen(sp, (a4 << 16) | 0x200);
        if (r == -1) {
            return -1;
        }
        FileClose(r);
    }
    v = a4 << 13;
    r = FileOpen(sp, 0x8002);
    if (r != -1) {
        CardFileHandle = r;
        ClearHwCardEvents();
        while (FileWrite(r, a3, v) == 0) {
        }
        ret = 0;
        FileClose(r);
    } else {
        ret = -1;
    }
    return ret;
}

int CardWriteFileAt(int a0, int a1, int a2, int a3, int a4, int a5) {
    char sp18[32];
    int h; register int r asm("$17"); int t;
    PsyqSprintf(sp18, CardFilePathFmt, a0, a1, a2);
    t = a4;
    h = FileOpen(sp18, 0x8002);
    /* ASM_BARRIER: loads -1 into r after the call, not hoisted before it */
    __asm__ volatile("" : "=r"(r) : "0"(-1));
    if (h != r) {
        if (FileSeek(h, a5, 0) != r) {
            CardFileHandle = h;
            ClearHwCardEvents();
            while (FileWrite(h, a3, t) == 0) ;
            r = 0;
        }
        FileClose(h);
        return r;
    }
    /* ASM_BARRIER: re-launders r so the -1 return uses s1 (not the equal h in s0) */
    __asm__ volatile("" : "=r"(r) : "0"(r));
    return r;
}

int CardUpdate(int p, char *a1) {
    char *s2;
    int a0;
    int sp[2];
    int x;
    s2 = (char *)((p << 4) + (int)a1);
    switch (CardPortState[p]) {
    case 0:
        _card_info(s2);
        CardPortState[p] = 1;
        CardPortTimer[p] = 0;
        CardPortLastResult[p] = 0;
        break;
    case 1:
        a0 = PollHwCardEvents(p);
        if (a0 == 0) break;
        switch (a0) {
        case 1:
            CardPortResult[p] = a0;
            CardPortState[p] = 2;
            CardPortTimer[p] = 0;
            break;
        case 3:
            CardPortResult[p] = -1;
            CardPortState[p] = 6;
            CardPortLoaded[p] = 0;
            break;
        case 4:
            CardPortResult[p] = 2;
            CardPortState[p] = 2;
            CardPortTimer[p] = 0;
            break;
        case 2:
        default:
            CardPortResult[p] = -4;
            CardPortState[p] = 6;
            CardPortLoaded[p] = 0;
            break;
        }
        break;
    case 2:
        if (_cardCmd5ABegin(s2, p * 17 + (int)CardPortCmd5ABuffers)) {
            CardPortState[p] = 3;
            CardPortTimer[p] = 0;
        } else {
            if (CardPortTimer[p]++ >= 0x78) {
                CardPortResult[p] = -4;
                CardPortState[p] = 6;
                CardPortLoaded[p] = 0;
            }
        }
        break;
    case 3:
        a0 = MemCardSync(1, &sp[0], &sp[1]);
        if (a0 != 0) {
            if (a0 > 0) {
                a0 = sp[1];
                switch (a0) {
                case 0:
                    CardPortResult[p] = 3;
                    if (CardPortLoaded[p] == 1) {
                        CardPortState[p] = 6;
                    } else {
                        CardPortState[p] = 4;
                    }
                    break;
                case 2:
                    if (CardPortResult[p] == 1) {
                        if (CardPortLoaded[p] == 1) {
                            CardPortState[p] = 6;
                        } else {
                            CardPortState[p] = 4;
                        }
                    } else {
                        CardPortResult[p] = 2;
                        ClearSwCardEvents();
                        _card_clear(s2);
                        WaitSwCardEvent();
                        CardPortState[p] = 4;
                        CardPortLoaded[p] = 0;
                    }
                    break;
                case 1:
                    CardPortResult[p] = -1;
                    CardPortState[p] = 6;
                    CardPortLoaded[p] = 0;
                    break;
                case 3:
                    CardPortResult[p] = 4;
                    ClearSwCardEvents();
                    _card_clear(s2);
                    WaitSwCardEvent();
                    CardPortState[p] = 4;
                    CardPortLoaded[p] = 0;
                    break;
                }
            } else {
                CardPortResult[p] = -4;
                CardPortState[p] = 6;
                CardPortLoaded[p] = 0;
            }
        } else {
            if (CardPortTimer[p]++ >= 0x78) {
                CardPortResult[p] = -4;
                CardPortState[p] = 6;
                CardPortLoaded[p] = 0;
            }
        }
        break;
    case 4:
        ClearHwCardEvents();
        _card_load(s2);
        CardPortState[p] = 5;
        CardPortTimer[p] = 0;
        break;
    case 5:
        a0 = PollHwCardEvents(p);
        if (a0 == 0) break;
        CardPortState[p] = 6;
        switch (a0) {
        case 1:
            CardPortLoaded[p] = a0;
            break;
        case 3:
            CardPortResult[p] = -1;
            CardPortLoaded[p] = 0;
            break;
        case 4:
            x = CardPortResult[p];
            if ((unsigned)(x - 1) < 2) {
                CardPortResult[p] = -2;
            } else if ((unsigned)(x - 3) < 2) {
                CardPortResult[p] = -3;
            }
            CardPortLoaded[p] = 0;
            break;
        case 2:
        default:
            CardPortResult[p] = -4;
            CardPortLoaded[p] = 0;
            break;
        }
        break;
    case 6:
        CardPortLastResult[p] = CardPortResult[p];
        CardPortState[p] = 0;
        break;
    }
    return CardPortLastResult[p];
}

void CardFormatSlot(s32 arg0, s32 arg1) {
    _card_format((arg0 * 0x10) + arg1);
}
