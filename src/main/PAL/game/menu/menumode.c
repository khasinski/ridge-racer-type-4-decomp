#include "game/data/cardchk.h"
#include "game/menu/loadmenu.h"
#include "game/menu/savemenu.h"
#include "game/data/cardini.h"
#include "game/menu/menumode.h"
#include "game/data/menuvars.h"
struct E { char pad[4]; unsigned char a; char b; unsigned char c; signed char d; };

extern int StepMenuValue();
extern int CopyStringClearTail();
extern int PlaySfx();
extern short MenuResultCode;
extern short CardSlot;
extern short MenuPlayerIndex;
extern short MenuStateIndex;
extern short SaveSlotIndex;
extern short SaveSlotSelect;
extern char CardTargetFileName[];
extern char CardFileNameTemplate[];
extern char CardSaveFileName[];
extern struct { char a[4]; unsigned char b; char c[3]; } MenuStateTable[];
extern unsigned char D_800A448C[][4];
extern struct { char pad[0x28]; int x28; int x2c; char pad2[0x2C]; } PlayerControllers[];
extern unsigned char CardSlotState[];
extern char *UiRows;
extern short D_80109478[];
extern short D_8010948E;
extern unsigned char MenuCardStepRows[][4];
extern unsigned char CardFileCounts[];

void func_8007B774(void);
void func_8007B954();
void func_8007BB9C();

void func_8007B774(void) {
    /* ASM_BARRIER: pins value to s3 as in the original */
    register int s3 asm("$19");
    int i;
    i = MenuStateTable[MenuStateIndex].b;
    s3 = D_800A448C[i][1];
    switch (MenuResultCode) {
    case 3:
    case 6:
    case 7:
        StepMenuValue((MenuPlayerIndex & 1) << 2, 2, &SaveSlotIndex, 3, 0);
        if (PlayerControllers[MenuPlayerIndex].x28 != 0) {
            PlaySfx(1);
            if ((unsigned short)(MenuResultCode - 6) < 2) {
                SaveSlotSelect = 0x11;
                CopyStringClearTail(CardTargetFileName, CardTargetFileNames[SaveSlotIndex]);
            } else {
                CopyStringClearTail(CardTargetFileName, CardSaveFileName);
            }
        } else {
            if (PlayerControllers[MenuPlayerIndex].x2c != 0) {
                MenuStateIndex = 0;
            }
            return;
        }
        break;
    default:
        if (MenuResultCode == 0) {
            if (MenuStateIndex == 0x3A) {
                D_801094A0 = 1;
                SaveSlotSelect = 0x20;
                CopyStringClearTail(CardTargetFileName, CardFileNameTemplate);
            } else {
                CopyStringClearTail(CardTargetFileName, CardSaveFileName);
            }
        } else {
            CopyStringClearTail(CardTargetFileName, CardSaveFileName);
        }
        break;
    }
    MenuStateIndex = s3;
    PollCardStatus(CardSlot, 0, 1);
}

/* legacy typing of shared globals in this part of the unit */
#define MenuStateTable (*(struct E (*)[])&MenuStateTable)
#define PlayerControllers (*(char (*)[])&PlayerControllers)

void func_8007B954()
{
    int a, s4, s3;
    /* ASM_BARRIER: pin q to a0 ($4) to reproduce original register allocation */
    register unsigned char *q asm("$4");
    unsigned char *r, *r2;
    char *b3;
    unsigned char *b2;
    unsigned char (*tb)[4];
    int t, c;
    int ia;
    struct E *base;
    struct E *p;
    signed char sb;

    base = MenuStateTable;
    p = base + MenuStateIndex;
    q = (unsigned char *)(int)p->a;
    c = p->c;
    D_8010948E = c;
    if (D_800F4D8A == 0) {
        sb = p->d;
        if (sb >= 0) {
            D_80109478[c] = sb;
        }
        D_800F4D8A = 1;
    }
    tb = D_800A44A8;
    ia = (int)q * 4;
    b2 = CardSlotState;
    r = b2 + CardSlot;
    q = (unsigned char *)tb + ia;
    a = q[0];
    s4 = q[1];
    s3 = q[2];
    if (*r < 0x10 || a == *r) {
        if (UiRows[0x56C] == 2) {
            StepMenuValue(((unsigned short)MenuPlayerIndex & 1) << 2, 2, D_8010948E * 2 + (int)D_80109478, 2, 0);
            b3 = PlayerControllers;
        r2 = (unsigned char *)b3 + MenuPlayerIndex * 0x5C;
            if (*(int *)(r2 + 0x28) != 0) {
                PlaySfx(1);
                if (D_80109478[D_8010948E] == 0) {
                    MenuStateIndex = s4;
                } else {
                    MenuStateIndex = s3;
                }
                D_800F4D8A = 0;
            } else if (*(int *)(r2 + 0x2C) != 0) {
                PlaySfx(2);
                MenuStateIndex = s3;
                D_800F4D8A = 0;
            }
        }
    } else {
        if (*r == 0x32) return;
        if (*r != 0x14) return;
        t = MenuStateIndex;
        if (t == 2 || t == 0x1A || t == 0x2D || t == 0x3C) {
            D_80109458 = MenuStateIndex;
            MenuStateIndex = 8;
        } else if (t == 0x10 || t == 0x23) {
            MenuStateIndex = 0x12;
        } else if (t == 0x34) {
            MenuStateIndex = 0x36;
        }
        D_800F4D8A = 0;
    }
}

#undef MenuStateTable
#undef PlayerControllers

void func_8007BB9C()
{
    unsigned char *e;
    int s2, s4, s0, s3;
    int a0;
    unsigned char (*tab)[4];
    int i = MenuStateTable[MenuStateIndex].b;
    tab = MenuCardStepRows;
    e = tab[i];
    s2 = e[0];
    s4 = e[1];
    s0 = e[2];
    s3 = e[3];
    if (MenuResultCode != 0) goto l54;
    if (MenuStateIndex != 0x3D) goto l54;
    if (CardHasFile(CardFileCounts[CardSlot], CardSaveFileName) == 0) goto l48;
    D_80109490 = 1;
    goto l54;
l48:
    D_80109490 = 0;
l54:
    if (CardHasFile(CardFileCounts[CardSlot], CardTargetFileName) != 1) goto ca0;
    MenuStateIndex = s2;
    if (s0 == 1) { a0 = 2; goto cf0; }
    CardLoadStep(2, 0);
    return;
ca0:
    if (s0 != 1) goto ce0;
    if (CardSlotState[CardSlot] + s3 < 0x10) goto ce0;
    MenuStateIndex = 0x16;
    return;
ce0:
    MenuStateIndex = s4;
    if (s0 != 1) return;
    a0 = 1;
cf0:
    CardSaveStep(a0, 0);
}
