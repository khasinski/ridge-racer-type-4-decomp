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

extern int ScaleMatrixRow1();

extern int BuildCameraMatrices();

extern short MenuCarCameraAngles[];

extern short MenuCarCameraOffsets[];

extern struct 
{
  char pad[0x178];
  int v;
  char pad2[0x320 - 0x17C];
} PlayerCars[];

extern int SelectionPlayer;

extern struct 
{
  short a;
  unsigned short b;
  char pad[0x58];
} PlayerControllers[];

void SetupMenuCarCamera(int a)
{
  int b1 = (int) MenuCarCameraOffsets;
  int o = a * 6;
  short *p = (short *) (o + b1);
  short *q;
  int i;
  /* ASM_BARRIER: pin scratchpad base to $a0 like the original */
  register int *g asm("$4");
  g = (int *) 0x1F800000;
  g[2] = p[0];
  g[3] = p[1];
  g[4] = p[2];
  q = (short *) (o + ((int) MenuCarCameraAngles));
  g[6] = q[0];
  g[7] = q[1];
  g[8] = q[2];
  BuildCameraMatrices(g, q);
  {
    int t;
    t = 0x1F800028;
    /* ASM_BARRIER: forces the 0x1F800028 li to be scheduled before the li a0,2 (original delay-slot layout) */
    asm volatile("" : "=r"(t) : "0"(t));
    ScaleMatrixRow1(2, t);
  }
  ;
  if (PlayerControllers[SelectionPlayer & 1].b & 4)
  {
    PlayerCars[SelectionPlayer & 1].v += 0xC0;
  }
  else
    if (PlayerControllers[SelectionPlayer & 1].b & 8)
  {
    PlayerCars[SelectionPlayer & 1].v -= 0xC0;
  }
  i = PlayerCars[SelectionPlayer & 1].v;
  if (i > 0xF00)
  {
    PlayerCars[SelectionPlayer & 1].v = 0xF00;
  }
  else
    if (i < (-0xF00))
  {
    PlayerCars[SelectionPlayer & 1].v = -0xF00;
  }
}
