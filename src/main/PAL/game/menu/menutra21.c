/* RODATA 0x8001160c 0x74 */
/* Candidate module: menu transitions, 8007BD18..8007BFF4.
 * Contiguous functions indexed by the same menu state and entry table. */
#include "rr/tables.h"
#include "game/data/cardchk.h"
#include "game/data/cardini.h"
#include "game/menu/loadmenu.h"
#include "game/data/cardini.h"
#include "game/data/cardchk.h"
#include "game/menu/menumode.h"
#include "game/menu/menutra21.h"
#include "game/menu/menutra22.h"
#include "rr/course_choices.h"
#include "rr/menu_selection.h"
#include "game/gfx/drawbuf.h"
#include "game/data/menuvars.h"

struct E { char pad[4]; unsigned char a; char b; unsigned char c; signed char d; };

typedef struct { char b[0x17]; signed char b17; char r[0x2C - 0x18]; } w8c5S;

typedef struct { char pad[4]; unsigned char b; char pad2[3]; } E8;

typedef struct { s16 a, b; } P2;

typedef struct 
{
  char b[0x2C];
} W10_R;

typedef struct 
{
  char p[0x10];
  short chk;
} W10_H;

extern RrEntry800A4258 MenuStateTable[];
extern RrMenuChoice D_800A44F4[];
extern s32 D_800A4510[];
extern s16 D_800A4514[];
extern RrMenuChoice D_800A4520[];
extern s8 *DialogueState;
extern s16 MenuPlayerIndex;
extern s32 ModeFrame;
extern s16 MenuStateIndex;
extern u8 CardSlotState[];
extern RrMenuItemState5C PlayerControllers[];
extern s16 CardSlot;
extern s16 D_8010948C;
extern unsigned char D_800A4524[][4];
extern unsigned short D_800F3AF8;
extern unsigned short D_800F4D8A;
extern unsigned short D_800FFC18;
extern short D_80109478[];
extern short D_8010948E;
extern int StepMenuValue();
extern int PlaySfx();
extern int SeqFadeVolume();
extern int AddScreenFadeTiles();
extern int LoadDialogueTableStep();
extern int ClearScreenOverlayState();
extern int SeqMuteVolume();
extern int func_80114920();
extern int func_80115598();
extern int func_801161A8();
extern int func_8011512C();
extern w8c5S SaveSlotSelections[];
extern w8c5S SaveSlotSelectionsBackup[];
extern struct { char pad[0x500]; unsigned char b500; } *UiRows;
extern struct { char pad[0x10]; short h10; char pad2[0x1A]; short h2C; } *BadgeRows;
extern short MenuFadeTimer;
extern short ModeId;
extern short LinkState;
extern unsigned short D_800F3A1E;
extern unsigned short RaceType;
extern int D_800F4CA8;
extern short D_800F4D3C;
extern int D_801272F0[];
extern s16 D_800A4540[][2];
extern char *SaveBuffer;
extern int D_8010EFA8;
extern int CopyStringClearTail();
extern short D_800A45BC;
extern short D_800A45BE;
extern unsigned char CardFileCounts;
extern char CardSaveFileName[];
extern char CardTargetFileName[];
extern int D_800A45C0;
extern int D_800A45C4;
extern unsigned char D_800A4478[];
extern int UpdateDialogue();

void func_8007BD18(void);
void func_8007BDF8(void);
void func_8007BEF8(void);
void MenuStepFormatCard(void);
void func_8007BFF4();
void func_8007C308(void);
void func_8007C76C();
void func_8007C848(void);
void func_8007C8D4(void);
void func_8007C944(void);
int func_8007CB44();
int func_8007CFC0();

void func_8007BD18(void) {
    s32 i = MenuStateTable[MenuStateIndex].i;
    s32 a = D_800A44F4[i].a;
    s32 c = D_800A44F4[i].c;
    s32 b = D_800A44F4[i].b;
    s32 r;
    if (c == 1) r = CardSaveStep(0, CardSlot);
    else r = CardLoadStep(0, CardSlot);
    if (r == 4) {
        MenuStateIndex = a;
    } else if (r == 5) {
        MenuStateIndex = b;
    } else if (r == 6) {
        MenuStateIndex = b;
    }
    if (c == 0xD) {
        if (r == 4) D_8010948C = 1;
    }
}

void func_8007BDF8(void)
{
  s32 a1 = *(D_800A4510 + MenuStateTable[MenuStateIndex].i);
  if ((&PlayerControllers[MenuPlayerIndex])->a != 0)
  {
    if (a1 == 0)
    {
      PlaySfx(1);
      if (MenuStateIndex == 0xC)
      {
        MenuStateIndex = D_80109458;
        CardSlotState[CardSlot] = 0x5A;
        PollCardStatus(CardSlot, 0, 1);
        return;
      }
      MenuStateIndex = 0;
    }
  }
  else
    if ((&PlayerControllers[MenuPlayerIndex])->b != 0)
  {
    if (a1 == 0)
    {
      PlaySfx(2);
      MenuStateIndex = 0;
    }
  }
}

void func_8007BEF8(void) {
    s32 i = MenuStateTable[MenuStateIndex].i;
    s32 v = D_800A4514[i * 2];
    if (DialogueState[6] == 4) {
        MenuStateIndex = v;
        ModeFrame -= 1;
    }
}

void MenuStepFormatCard(void) {
    s32 k;
    s32 i;
    s32 a, b;
    for (k = 0x63; k >= 0; k--) ;
    i = MenuStateTable[MenuStateIndex].i;
    a = D_800A4520[i].a;
    b = D_800A4520[i].c;
    if (CardFormatSlot(CardSlot, 0) == 1) {
        MenuStateIndex = a;
        return;
    }
    MenuStateIndex = b;
}

/* legacy typing of shared globals in this part of the unit */
#define MenuStateTable (*(struct E (*)[])&MenuStateTable)
#define PlayerControllers (*(char (*)[])&PlayerControllers)
#define PollCardStatus ((int (*)())PollCardStatus)

void func_8007BFF4()
{
    int arr[4];
    int i, c, flag, n;
    struct E *base, *p;
    unsigned char *q;
    char *r, *b3;
    signed char sb;

    D_801094A0 = 0;
    D_800F3AF8 = D_800FFC18;
    switch ((short)D_800F3AF8) {
    case 0: MenuStateIndex = 0; break;
    case 1: case 5: MenuStateIndex = 0x20; break;
    case 2: MenuStateIndex = 0x21; break;
    case 3: MenuStateIndex = 0x18; break;
    case 6: MenuStateIndex = 0x2A; D_800F3AF8 = 0x10; break;
    case 7: MenuStateIndex = 0x2B; D_800F3AF8 = 0x10; break;
    }
    base = MenuStateTable;
    p = base + MenuStateIndex;
    i = p->a;
    c = p->c;
    D_8010948E = c;
    if (D_800F4D8A == 0) {
        sb = p->d;
        if (sb >= 0) {
            D_80109478[c] = sb;
        }
        D_800F4D8A = 1;
    }
    flag = 0;
    q = D_800A4524[i];
    i = 0;
    arr[0] = q[0];
    arr[1] = q[1];
    arr[2] = q[2];
    arr[3] = q[3];
    n = 4;
    for (; i < 4; i++) {
        if (arr[i] == 0xFF) { n = i; break; }
        if (i == 1 && (arr[1] & 0x80)) {
            flag = 1;
            arr[1] ^= 0x80;
        }
    }
    if (flag) {
        StepMenuValue(((unsigned short)MenuPlayerIndex & 1) << 2, 2, D_80109478 + D_8010948E, n, 0);
    } else {
        StepMenuValue((((unsigned short)MenuPlayerIndex & 1) << 2) | 1, 2, D_80109478 + D_8010948E, n, 0);
    }
    b3 = PlayerControllers;
    r = b3 + MenuPlayerIndex * 0x5C;
    if (*(int *)(r + 0x28) != 0) {
        PlaySfx(1);
        D_800F4D8A = 0;
        MenuStateIndex = arr[D_80109478[D_8010948E]];
        if (MenuStateIndex != 0x43) {
            CardSlotState[CardSlot] = 0x5A;
            PollCardStatus(CardSlot, 0, 1);
        }
    } else if (*(int *)(r + 0x2C) != 0) {
        PlaySfx(2);
        D_800F4D8A = 0;
        MenuStateIndex = arr[n - 1];
    }
}

#undef MenuStateTable
#undef PlayerControllers
#undef PollCardStatus

/* legacy typing of shared globals in this part of the unit */
#define D_800FFC18 (*(short (*))&D_800FFC18)

void func_8007C308(void) {
    int i;
    char *q;
    if (UiRows->b500 == 2) {
        for (i = 0; i < 4; i++) {
            q = (char *)(i * 0x24 + (int)UiRows) + 0x4C8;
            if (q[0x14] == 2) {
                q[0x14] = 3;
            }
        }
        BadgeRows->h10 = 4;
        if (D_800FFC18 == 3 || D_800FFC18 == 6 || D_800FFC18 == 7) {
            BadgeRows->h2C = 4;
        }
        MenuFadeTimer = 0x40;
        if (D_800FFC18 == 0) {
            SeqFadeVolume(0, 0x3C);
        }
        ResetScreenCounters();
        D_800F4CA8 = 2;
        return;
    }
    if (MenuFadeTimer > 0) {
        MenuFadeTimer = MenuFadeTimer - 1;
    }
    AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x40 - MenuFadeTimer) << 2, 0x45);
    if (LoadDialogueTableStep() != 1) return;
    if (MenuFadeTimer > 0) return;
    if (UiRows->b500 != 5) return;
    if (BadgeRows->h10 != 0xB) return;
    for (i = 0; i < 3; i++) {
        SaveSlotSelections[i] = SaveSlotSelectionsBackup[i];
    }
    ClearScreenOverlayState();
    switch (D_800FFC18) {
    case 0:
    default:
    case 4:
        SeqMuteVolume();
        ModeGroup = 1;
        ModeId = 3;
        D_800F4D8A = 0;
        break;
    case 1:
        if (D_800F3A1E == 0x132) {
            func_80114920();
            D_800F4D8A = 0;
            ModeGroup = D_800F3A1E >> 8;
            ModeId = D_800F3A1E & 0xFF;
        }
        break;
    case 2:
        if ((unsigned)(RaceType - 2) < 2) {
            func_80115598(SelectionPlayer);
            ModeGroup = 1;
            ModeId = 0x38;
            D_800F4D8A = 0;
        } else if ((short)RaceType == 4) {
            if (LinkState != -1) {
                func_801161A8(SelectionPlayer);
                ModeGroup = 1;
                ModeId = 0x3C;
            } else {
                SeqMuteVolume();
                ModeGroup = 1;
                ModeId = 0x2D;
            }
            D_800F4D8A = 0;
        }
        break;
    case 3:
        ModeGroup = 1;
        ModeId = 0xF;
        D_800F4D8A = 0;
        break;
    case 5:
        func_8011512C();
        ModeGroup = 1;
        ModeId = 0x40;
        D_800F4D8A = 0;
        break;
    case 6:
        if (D_8010948C != 0) {
            ModeGroup = 3;
            ModeId = 3;
        } else {
            ModeGroup = 3;
            ModeId = 2;
        }
        D_800F4D8A = 0;
        break;
    case 7:
        ModeGroup = 1;
        ModeId = 0x27;
        D_800F4D8A = 0;
        break;
    }
}

#undef D_800FFC18

/* legacy typing of shared globals in this part of the unit */
#define D_800FFC18 (*(short (*))&D_800FFC18)

void func_8007C76C()
{
    short t;
    if (D_800F4D8A == 0) {
        D_801272F0[0] = -1;
        D_801272F0[1] = -1;
        D_801272F0[2] = -1;
        D_800F4D3C = 0;
        func_8007CFC0(0, 0);
        D_800F4D8A = 1;
        return;
    }
    if (func_8007CFC0(1, D_800F4D3C) != 0) {
        t = (unsigned short)D_800F4D3C + 1;
        D_800F4D3C = t;
        if (t == 3) {
            if (D_800FFC18 == 7) {
                MenuStateIndex = 0x2B;
            } else {
                MenuStateIndex = 0x2A;
            }
            D_800F4D3C = 0;
            D_800F4D8A = 0;
            return;
        }
        func_8007CFC0(0, t);
    }
}

#undef D_800FFC18

/* legacy typing of shared globals in this part of the unit */
#define MenuStateTable (*(E8 (*)[])&MenuStateTable)

void func_8007C848(void) {
 E8 *e = MenuStateTable; int s = MenuStateIndex;
 unsigned char *q = D_800A44A8 + e[s].b*4;
 int x = q[0], y = q[1];
 unsigned c = CardSlotState[CardSlot];
 register int d asm("$3"); /* ASM_BARRIER: pin reload value to v1 */
 if (c < 0x10 || x == (d = *(volatile unsigned char *)&CardSlotState[CardSlot])) { MenuStateIndex = y; return; }
 if (d == 0x14 && s == 2) MenuStateIndex = 8;
}

#undef MenuStateTable

void func_8007C8D4(void) {
    s32 i = MenuStateTable[MenuStateIndex].i;
    s32 a = D_800A4540[i][0];
    s32 b = D_800A4540[i][1];
    if (MenuStateIndex == 0x3B) {
        if (D_8010B2DC[CardSlot] == 1) {
            MenuStateIndex = a;
            return;
        }
        MenuStateIndex = b;
        return;
    }
    MenuStateIndex = a;
}

/* legacy typing of shared globals in this part of the unit */
#define SaveSlotSelections (*(W10_R (*)[])&SaveSlotSelections)

void func_8007C944(void)
{
  int r;
  int t0;
  int t1;
  W10_R *t2;
  short a1;
  unsigned v1;
  char *a0;
  int new_var;
  char *a2;
  W10_H *a3;
  signed char *e;
  char pad[0x38];
  if (D_800F4D8A == 0)
  {
    func_8007CB44(0);
    D_8010EFA8 = 0;
    D_800F4D8A = 1;
    return;
  }
  r = func_8007CB44(1);
  t1 = 0;
  if (r > 0)
  {
    new_var = 8;
    do
    {
      a1 = 0;
      v1 = 0;
      a0 = ((char *) (t1 * 0x2C)) + ((int) SaveBuffer);
      do
      {
        if (v1 != new_var)
        {
          a1 += *((unsigned short *) (a0 + 0x1F60));
        }
        v1++;
        a0 = a0 + 2;
      }
      while (v1 < 0x16U);
      a2 = ((char *) (t1 * 0x2C)) + ((int) SaveBuffer);
      a3 = (W10_H *) (a2 + 0x1F60);
      if (a1 == a3->chk)
      {
        *(t2 = SaveSlotSelections + t1) = *((W10_R *) a3);
      }
      else
      {
        e = ((signed char *) SaveSlotSelections) + (t1 * 0x2C);
        e[0x17] = -1;
      }
      t1++;
    }
    while (t1 < 3);
    MenuStateIndex = 0x18;
    D_800F4D8A = 0;
  }
  else
    if (r < 0)
  {
    for (t1 = 2; t1 >= 0; t1--)
    {
      e = ((signed char *) SaveSlotSelections) + (t1 * 0x2C);
      e[0x17] = -1;
    }

    MenuStateIndex = 0x18;
    D_800F4D8A = 0;
  }
}

#undef SaveSlotSelections

/* legacy typing of shared globals in this part of the unit */
#define PollCardStatus ((int (*)())PollCardStatus)
#define SaveBuffer (*(int (*))&SaveBuffer)

int func_8007CB44(a0)
int a0;
{
    int pad[14];
    int ret;
    int i;
    int r;
    int q;
    int v;
    if (a0 == 0) {
        D_800A45BC = 0;
    }
    ret = 0;
    switch (D_800A45BC) {
    case 0:
        CardSlotState[1] = 0x5A;
        CardSlotState[0] = 0x5A;
        PollCardStatus(CardSlot, 0, 1);
        CopyStringClearTail(CardTargetFileName, CardSaveFileName);
        D_800A45BC = 1;
        break;
    case 1:
        PollCardStatus(CardSlot, 0, 0);
        if (CardSlotState[0] < 0x10U) {
            if (CardHasFile(CardFileCounts, CardTargetFileName) == 1) {
                D_800A45BE = 0;
                D_800A45BC = 2;
            } else {
                D_800A45BC = 7;
            }
            break;
        }
        v = CardSlotState[0];
        switch (v) {
        case 0x14:
        case 0x1E:
        case 0x28:
        case 0x3C:
            D_800A45BC = 7;
            break;
        case 0x32:
        case 0x5A:
            ret = 0;
            break;
        }
        break;
    case 2:
        D_800A45BC = 3;
        break;
    case 3:
        for (i = 0x63; i >= 0; i--) {
        }
        r = CardReadFileAt(CardSlot, 0, CardTargetFileName, SaveBuffer + 0x1F00, 0x100, 0x300);
        if (r == -1) {
            if (D_800A45BE < 3) {
                D_800A45BE = (unsigned short)D_800A45BE + 1;
            } else {
                D_800A45BC = 7;
            }
        } else {
            D_800A45BC = 4;
        }
        break;
    case 4:
        for (i = 0x63; i >= 0; i--) {
        }
        q = GetHwCardEvent();
        if (q != 0) {
            if (q == 1) {
                D_800A45BC = 5;
            } else if (D_800A45BE >= 3) {
                D_800A45BC = 7;
            } else {
                D_800A45BC = 2;
                D_800A45BE = (unsigned short)D_800A45BE + 1;
            }
        }
        break;
    case 5:
        ret = 1;
        break;
    case 6:
    case 7:
    default:
        ret = -1;
        break;
    }
    return ret;
}

#undef PollCardStatus
#undef SaveBuffer

/* legacy typing of shared globals in this part of the unit */
#define PollCardStatus ((int (*)())PollCardStatus)
#define CardLoadStep ((int (*)())CardLoadStep)
#define D_800F3AF8 (*(short (*))&D_800F3AF8)

int LoadSaveFromCardStep(a0)
int a0;
{
    int ret;
    int i;
    int v;
    char *p;
    if (a0 == 0) {
        D_800A45C0 = 0;
    }
    ret = 0;
    switch (D_800A45C0) {
    case 0:
        CardSlotState[1] = 0x5A;
        CardSlotState[0] = 0x5A;
        PollCardStatus(0, 0, 1);
        i = 0x17;
        p = CardTargetFileName + i;
        do {
            *p = 0;
            i--;
            p--;
        } while (i >= 0);
        CopyStringClearTail(CardTargetFileName, CardSaveFileName);
        D_800F3AF8 = 0x30;
        D_800A45C0 = 1;
        break;
    case 1:
        PollCardStatus(0, 0, 0);
        if (CardSlotState[0] < 0x10U) {
            if (CardHasFile(CardFileCounts, CardTargetFileName) == 1) {
                CardLoadStep(2, 0);
                D_800A45C0 = 2;
            } else {
                goto set1;
            }
            break;
        }
        v = CardSlotState[0];
        switch (v) {
        case 0x14:
        case 0x1E:
        case 0x28:
        case 0x3C:
            D_800A45C0 = 3;
            goto set1;
        case 0x32:
        case 0x5A:
            ret = 0;
            break;
        }
        break;
    case 2:
        ret = CardLoadStep(0, 0);
        if (ret == 4) {
            goto set1;
        } else if (ret == 5) {
            goto set1;
        } else if (ret == 6) {
            goto set1;
        } else {
            ret = 0;
        }
        break;
    case 3:
set1:
        ret = 1;
        break;
    default:
        return ret;
    }
    return ret;
}

#undef PollCardStatus
#undef CardLoadStep
#undef D_800F3AF8

/* legacy typing of shared globals in this part of the unit */
#define PollCardStatus ((int (*)())PollCardStatus)
#define CardLoadStep ((int (*)())CardLoadStep)
#define D_800F3AF8 (*(short (*))&D_800F3AF8)

int func_8007CFC0(a0, a1)
int a0, a1;
{
    int ret;
    int v;
    if (a0 == 0) {
        D_800A45C4 = 0;
    }
    ret = 0;
    switch (D_800A45C4) {
    default:
    case 0:
        CardSlotState[1] = 0x5A;
        CardSlotState[0] = 0x5A;
        PollCardStatus(0, 0, 1);
        D_800A45C4 = 1;
        CopyStringClearTail(CardTargetFileName, CardTargetFileNames[a1]);
        D_800F3AF8 = 0x10;
        break;
    case 1:
        PollCardStatus(0, 0, 0);
        if (CardSlotState[0] < 0x10U) {
            if (CardHasFile(CardFileCounts, CardTargetFileName) == 1) {
                CardLoadStep(2, 0);
                D_800A45C4 = 2;
            } else {
                ret = -1;
            }
            break;
        }
        v = CardSlotState[0];
        switch (v) {
        case 0x14:
        case 0x1E:
        case 0x28:
        case 0x3C:
            D_800A45C4 = 3;
            ret = -2;
            break;
        case 0x32:
        case 0x5A:
            ret = 0;
            break;
        }
        break;
    case 2:
        ret = CardLoadStep(0, 0);
        if (ret == 4) {
            D_800A45C4 = ret;
            ret = 2;
        } else if (ret == 5) {
            ret = -1;
        } else if (ret == 6) {
            ret = -1;
        } else {
            ret = 0;
        }
        break;
    case 3:
        ret = -1;
        break;
    case 4:
        ret = 3;
        break;
    }
    return ret;
}

#undef PollCardStatus
#undef CardLoadStep
#undef D_800F3AF8

/* legacy typing of shared globals in this part of the unit */
#define DialogueState ((char *)DialogueState)
#define BadgeRows ((short *)BadgeRows)
#define D_800FFC18 (*(short (*))&D_800FFC18)

void func_8007D1B8(a0, a1, a2, a3, a4, a5) int a0; int a1; int a2; int a3; int a4; int a5; {
    unsigned char *v1;
    int s0 = a1;
    if (a2 != 0) {
        int c14 = 0x14, c16 = 0x16, c4 = 4, c1 = 1;
        v1 = D_800A4478;
        a3 = (int)v1 + c14;
L1:
        if (a5 != v1[1]) goto next;
        if (a5 == c14) goto done;
        if (a0 != c16 || s0 != c4 || a2 != c1) s0 = 0;
        a0 = v1[0];
        goto done;
next:
        v1 += 4;
        if ((int)v1 < a3) goto L1;
done:;
        if (a0 == 5) {
            if (D_801094A0 == 1) {
                a0 = 0x1A;
            } else if (D_800FFC18 == 2 && CardSlot == 1) {
                a0 = 0x21;
            }
        }
    }
    if (BadgeRows[8] == 3) {
        if (a0 == -1) {
            if (D_800FFC18 == 3 || D_800FFC18 == 6 || D_800FFC18 == 7) a0 = 0x19;
        } else if (a0 == -128) {
            a0 = (signed char)DialogueState[4];
        } else {
            if (a0 == 0x12 && D_801094A0 == 1 && D_8010B2DC[CardSlot] == 1) {
                a0 = 0x1C;
            } else if (a0 == 0x15 && CardSlot == 1) {
                a0 = 0x22;
            } else if (a0 == 0x25 && CardSlot == 1) {
                a0 = 0x26;
            } else if (a0 == 0x1F && D_80109490 == 0) {
                a0 = 0x20;
            }
        }
        if (a0 >= 0) {
            DialogueState[4] = a0;
            UpdateDialogue(BadgeRows[4] + 10, BadgeRows[5] + 0x28, DialogueState, a3);
        }
    }
    func_8007D448(a4, ((s0 & 2) > 0) & (*(short *)(DialogueState + 0x12) >= 6));
    func_8007D560(a4, (s0 & 4) > 0);
    if (s0 & 8) func_8007D65C(D_800F4D3C);
}

#undef DialogueState
#undef BadgeRows
#undef D_800FFC18
