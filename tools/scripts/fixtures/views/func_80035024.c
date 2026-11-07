#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define PlayerCars (*(S2 (*)[])&PlayerCars)

#define PlayerControllers (*(S1 (*)[])&PlayerControllers)

#undef PlayerCars

#undef PlayerControllers

/* legacy typing of shared globals in this part of the unit */
#define _sio_control ((unsigned int (*)())_sio_control)

#define PlayerControllers (*(T38 (*))&PlayerControllers)

#define LinkState (*(unsigned short (*))&LinkState)

#undef _sio_control

#undef PlayerControllers

#undef LinkState

/* legacy typing of shared globals in this part of the unit */
#define _sio_control ((unsigned (*)())_sio_control)

#undef _sio_control

typedef struct 
{
  char pad[0x84];
  int a84;
  int a88;
} T38;

extern struct 
{
  short a;
  unsigned short b;
  char pad[0x58];
} PlayerControllers[];

extern int PlaySfx();

extern int _sio_control();

extern s16 LinkState;

extern int LinkHandshakeCountdown;

extern signed char LinkEntryState[];

extern signed char D_800ABEBE;

extern int RaceTimeRemaining;

extern short LinkRoleSlot;

extern int D_800F3A64;

int UpdateLinkHandshake(int a0)
{
  int r = 0;
  long new_var2;
  int st;
  int mm;
  signed char *t;
  signed char *bp;
  short z;
  int xx;
  int q3;
  T38 *e;
  signed char *new_var;
  unsigned int k = (_sio_control(0, 0, 0) >> 7) & 3;
  short new_var3;
  st = (short) (LinkState + 1);
  switch (st)
  {
    case 1:
      if (_sio_control(3, 1, 0) != 0)
    {
      _sio_control(1, 1, 2);
      if (a0 == 0)
      {
        LinkRoleSlot = 0;
        D_800ABEBE = 1;
        LinkHandshakeCountdown = 3;
        LinkState = 10;
      }
      else
      {
        goto s2;
      }
      break;
    }
      _sio_control(1, 1, 2);
      if (a0 == 0)
    {
      LinkRoleSlot = 1;
      LinkEntryState[0] = 1;
      LinkState = 1;
    }
    else
      if (k != 1)
    {
      LinkState = 1;
    }
    else
    {
      goto s2;
    }
      break;

    case 11:
      if (LinkHandshakeCountdown > 0)
    {
      _sio_control(1, 1, 2);
      LinkHandshakeCountdown -= 1;
      if (k == 1)
      {
        LinkState = 2;
        LinkHandshakeCountdown = 0;
      }
    }
    else
    {
      goto s2;
    }
      break;
      s2:
    LinkState = 2;

      break;

    case 2:
      if ((k == 0) || ((k == 3) && (a0 == 0)))
    {
      goto m1;
    }
    else
      if (k == 2)
    {
      LinkState = k;
      LinkHandshakeCountdown = 4;
    }
      break;

    case 3:
      _sio_control(1, 1, 1);
      if (LinkHandshakeCountdown > 0)
    {
      if (a0 == 0)
      {
        LinkHandshakeCountdown -= 1;
        break;
      }
    }
    else
      if (a0 == 0)
    {
      new_var = LinkEntryState;
      xx = LinkRoleSlot;
      z = 3;
      new_var2 = xx * 2;
      bp = new_var;
      t = &bp[z - new_var2];
      if ((*t) == 0)
      {
        e = &PlayerControllers;
        new_var3 = z;
        if (e->a84 != 0)
        {
          PlaySfx(1);
          bp[new_var3 - (LinkRoleSlot * 2)] = 1;
        }
        else
          if (e->a88 != 0)
        {
          *t = -1;
        }
        if (D_800F3A64 != 0)
        {
          LinkEntryState[1 - (q3 = LinkRoleSlot)] = -1;
          _sio_control(1, 1, 0, q3);
        }
      }
    }
      if ((k == 0) || ((k == 3) && (a0 == 0)))
    {
      goto m1;
    }
    else
      if (a0 == 0)
    {
      int v = RaceTimeRemaining - 1;
      RaceTimeRemaining = v;
      if (v <= 0)
      {
        LinkState = 4;
        _sio_control(1, 1, 2);
      }
      else
        if (LinkEntryState[3 - (LinkRoleSlot * 2)] != 0)
      {
        LinkState = 3;
        _sio_control(1, 1, 2);
      }
    }
      break;

    case 4:
      if (a0 == 0)
    {
      if (RaceTimeRemaining > 0)
      {
        RaceTimeRemaining -= 1;
      }
    }
      if ((k == 0) || ((k == 3) && (a0 == 0)))
    {
      goto m1;
    }
    else
      if (k != 1)
    {
      goto c5;
    }
      break;

    case 5:
      c5:
    if (a0 != 2)
    {
      LinkState = 0;
    }

      r = 1;
      break;

    case 0:
      r = -1;
      break;
      m1:
    *((short *) (&LinkState)) = -1;

      break;

  }

  return r;
}
