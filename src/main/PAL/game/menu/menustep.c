/* RODATA 0x800115ec 0x20 */
#include "game/data/cardchk.h"
#include "game/menu/menutra21.h"
extern struct 
{
  unsigned short a;
  signed char b;
  unsigned char c;
  unsigned char d;
  unsigned char e;
  short f;
} MenuStateTable[];
extern void (*D_800A4548[])();
extern short MenuStateIndex;
extern unsigned char CardSlotState[];
extern unsigned short D_800F3AF8;
extern short D_800F4D3C;
extern short D_800F4D8A;
extern short CardSlot;
extern unsigned short D_800FFC18;
extern short D_80109478[];
extern short D_8010948E;
extern short MenuPlayerIndex;
extern struct { char pad[0x28]; int x28; int x2c; char pad2[0x2C]; } PlayerControllers[];

void func_8007B460();
int func_8007B654();

static inline int w4c7_nz(int arg0)
{
  return arg0 != 0;
}

void func_8007B460()
{
  int a;
  int t0;
  unsigned short v;
  int r;
  int s7;
  int s0;
  int s1;
  int s3;
  int new_var;
  int fp;
  r = 1;
  s0 = MenuStateTable[MenuStateIndex].e;
  s1 = MenuStateTable[MenuStateIndex].c;
  t0 = MenuStateTable[MenuStateIndex].a;
  s7 = s0 & 1;
  s3 = MenuStateTable[MenuStateIndex].b;
  fp = CardSlotState[CardSlot];
  if (w4c7_nz(s7))
  {
    PollCardStatus(CardSlot, 0, 0);
  }
  if (s0 & 2)
  {
    r = func_8007B654();
  }
  if (r == 1)
  {
    D_800A4548[s1]();
  }
  func_8007D1B8(s3, t0, s0, CardSlot, D_80109478[D_8010948E], CardSlotState[CardSlot]);
  if (w4c7_nz(s7))
  {
    if (fp == 0x1E)
    {
      if (CardSlotState[CardSlot] != fp)
      {
        D_800F4D8A = 0;
        D_800F4D3C = 0;
        v = D_800FFC18;
        D_800F3AF8 = v;
        a = (short) v;
        if (a == 0)
        {
          MenuStateIndex = 0;
          return;
        }
        if (((unsigned short) (v - 6)) < 2)
        {
          MenuStateIndex = 0x29;
          D_800F3AF8 = 0x10;
          return;
        }
        new_var = 1;
        if ((a == new_var) || (a == 5))
        {
          MenuStateIndex = 0x20;
        }
        else
          if (a == 2)
        {
          MenuStateIndex = 0x21;
        }
        else
          if (a == 3)
        {
          MenuStateIndex = 0x17;
        }
      }
    }
  }
}

/* legacy typing of shared globals in this part of the unit */
#define D_800FFC18 (*(short (*))&D_800FFC18)

int func_8007B654()
{
    int a1;
    int a2;
    int v;
    a2 = -1;
    switch (D_800FFC18) {
    case 0:
        a1 = 0;
        break;
    case 1:
    case 5:
        a1 = 0x20;
        break;
    case 2:
        a1 = 0x21;
        break;
    case 3:
        a1 = 0x17;
        break;
    case 6:
    case 7:
        a1 = 0x29;
        break;
    case 4:
        break;
    }
    v = CardSlotState[CardSlot];
    switch (v) {
    case 0x28:
    case 0x3C:
    case 0x1E:
        if (PlayerControllers[MenuPlayerIndex].x28 != 0 || PlayerControllers[MenuPlayerIndex].x2c != 0) {
            MenuStateIndex = a1;
        }
        break;
    case 0x14:
    case 0x32:
        a2 = 1;
        break;
    case 0x5A:
        break;
    default:
        a2 = 1;
        break;
    }
    return a2;
}

#undef D_800FFC18
