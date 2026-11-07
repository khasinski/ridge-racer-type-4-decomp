#include "game/camera/camview.h"
#include "game/camera/camfx.h"
#include "game/camera/camview.h"
#include "rr/menu_overlay_runtime.h"
#include "game/camera/camview.h"

extern char D_800A0D70[];
extern char CamFxHeightCurve[];
extern char CarIntroHeightCurve[];
extern char CamFxFallCurve[];
typedef struct Q
{
  char pad0[0x2A];
  short h2a;
  char pad1[0x42 - 0x2C];
  short h42;
  char pad2[0x4C - 0x44];
  int w4c;
  int w50;
  char pad3[0x64 - 0x54];
  int w64;
  int w68;
  char pad4[0x70 - 0x6C];
  short h70;
  char pad5[0xA0 - 0x72];
  int wa0;
  char pad6[0xAC - 0xA4];
  int wac;
  int wb0;
  char pad7[0xC4 - 0xB4];
  int wc4;
  int wc8;
  char pad8[0x100 - 0xCC];
  int w100;
} Q;
extern struct 
{
  char pad[0x24C];
  Q *p;
} RaceContext;
void EvalCameraCurvesAtTime(a0)
int a0;
{
  /* ASM_BARRIER: pin first call result to $a2 as in the original */
  register int v0 __asm__("$6");
  int s1;
  int a2;
  /* ASM_BARRIER: pin a1 copy of s1 to $a1 as in the original */
  register int a1 __asm__("$5");
  int v1;
  int a0b;
  /* ASM_BARRIER: shifted copy lives in $v0 in the original */
  register int a1b __asm__("$2");
  int *G;
  Q *pq;
  Q *pq2;
  int *new_var;
  char *q;
  v0 = InterpCurveEase();
  RaceContext.p->h2a = v0;
  RaceContext.p->h42 = -v0;
  q = D_800A0D70;
  RaceContext.p->h70 = InterpCurveLinear(q, FindCurveSegment(q, a0, v0), a0);
  q = CamFxHeightCurve;
  s1 = InterpCurveLinear(q, FindCurveSegment(q, a0), a0);
  q = CarIntroHeightCurve;
  a2 = InterpCurveLinear(q, FindCurveSegment(q, a0), a0);
  RaceContext.p->w68 = (s1 - (RaceContext.p->wa0 * 4)) - 0x10;
  RaceContext.p->w64 = a2 + 0xC;
  RaceContext.p->w50 = s1;
  RaceContext.p->w4c = a2;
  pq = RaceContext.p;
  a1 = s1;
  if (s1 < 0)
  {
    a1 = s1 + 3;
  }
  a1b = a1 >> 2;
  pq->wb0 = a1b - pq->w100;
  pq2 = RaceContext.p;
  if (a2 < 0)
  {
    a2 += 3;
  }
  pq2->wac = a2 >> 2;
  q = CamFxFallCurve;
  s1 = InterpCurveLinear(q, FindCurveSegment(q, a0, a2), a0);
  v1 = -(s1 << 5);
  new_var = (int *) PlayerCars;
  a0b = v1;
  RaceContext.p->wc8 = s1;
  if (v1 < 0)
  {
    a0b = v1 + 0xFFF;
  }
  G = new_var;
  G[0x17C / 4] = v1 - ((a0b >> 12) << 12);
  G[0x28 / 4] = RaceContext.p->wb0 + RaceContext.p->wc8;
  G[0x24 / 4] = RaceContext.p->wac + RaceContext.p->wc4;
}
