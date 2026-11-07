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
extern void (*CardMenuStepHandlers[])();
extern short MenuStateIndex;
extern unsigned char CardSlotState[];
extern unsigned short SaveSlotSelect;
extern short SaveSlotIndex;
extern short ModeSubState;
extern short CardSlot;
extern unsigned short MenuResultCode;
extern short MenuChoiceValues[];
extern short MenuChoiceIndex;
extern short MenuPlayerIndex;
extern struct { char pad[0x28]; int x28; int x2c; char pad2[0x2C]; } PlayerControllers[];

void DispatchCardMenuStep();
int CheckCardSlotReady();

static inline int w4c7_nz(int arg0)
{
  return arg0 != 0;
}

void DispatchCardMenuStep()
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
    r = CheckCardSlotReady();
  }
  if (r == 1)
  {
    CardMenuStepHandlers[s1]();
  }
  UpdateCardMenuDialogue(s3, t0, s0, CardSlot, MenuChoiceValues[MenuChoiceIndex], CardSlotState[CardSlot]);
  if (w4c7_nz(s7))
  {
    if (fp == 0x1E)
    {
      if (CardSlotState[CardSlot] != fp)
      {
        ModeSubState = 0;
        SaveSlotIndex = 0;
        v = MenuResultCode;
        SaveSlotSelect = v;
        a = (short) v;
        if (a == 0)
        {
          MenuStateIndex = 0;
          return;
        }
        if (((unsigned short) (v - 6)) < 2)
        {
          MenuStateIndex = 0x29;
          SaveSlotSelect = 0x10;
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
#define MenuResultCode (*(short (*))&MenuResultCode)

int CheckCardSlotReady()
{
    int a1;
    int a2;
    int v;
    a2 = -1;
    switch (MenuResultCode) {
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

#undef MenuResultCode
