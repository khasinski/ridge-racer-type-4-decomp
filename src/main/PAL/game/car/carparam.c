#include "scratchpad.h"
#include "rr_asm.h"

#define S(b, o) (*(short *)((b) + (o)))
#define U(b, o) (*(unsigned short *)((b) + (o)))
#define I(b, o) (*(int *)((b) + (o)))

typedef unsigned char u8;

typedef unsigned short u16;

typedef struct 
{
  short s0;
  char p2[4];
  short s6;
  char p8[2];
  unsigned short sA;
  short sC;
  short sE;
  char p10[2];
  short s12;
  short s14;
  char p16[2];
  short s18;
} R;

typedef struct 
{
  short s0;
  short s2;
  char p4[4];
  short s8[26];
} S;

typedef struct 
{
  char p0[0xC];
  int sC;
  char p10[8];
  S *s18;
  R *s1C;
  char p20[4];
  int s24;
  char p28[8];
  short s30;
  short s32;
  char p34[2];
  short s36;
  short s38;
  char p3A[2];
  short s3C;
  char p3E[2];
  short s40;
  char p42[2];
  short s44;
  char p46[0x40];
  short s86;
  int s88;
  int s8C;
  char p90[0x10];
  int sA0;
  int sA4;
  short sA8;
  short sAA;
  short sAC;
  short sAE;
} Q;

typedef struct 
{
  char p0[0x50];
  int s50;
  int s54;
  int s58;
  char p5C[0x116];
  short s172;
  char p174[0x44];
  short s1B8;
  char p1BA[0x1A];
  short s1D4;
  short s1D6;
  short s1D8;
  char p1DA[4];
  short s1DE;
  int s1E0;
  char p1E4[0x10];
  Q q;
} P;

extern int AngleDiffSigned();
extern int PadSetRumbleParams();
extern int rcos();
extern int PsyqRand();
extern char PlayerCars[];
extern int MirrorMode;
extern char *CarRecordPtrs[];
extern char PlayerControllers[];
extern int RaceState;
extern unsigned char CarInputs[][4];
extern short EngineRevDisplay[];
extern short EngineThrottleOn[];
extern int AngleDiffAbs();
extern void StepCarStateNormal();
extern void StepCarStateSlide();
extern void StepCarStateSpinOut();
extern int BuildRotMatrixY();
extern int BuildRotMatrixX();
extern int BuildRotMatrixZ();
extern int MulMatrix();

void UpdatePlayerSteering();
void UpdateCarEngine();
void IntegrateCarMotion(char *p);

/* legacy typing of shared globals in this part of the unit */
#define CarInputs (*(char (*)[])&CarInputs)

void UpdatePlayerSteering(arg0, arg1)
char *arg0;
int arg1;
{
  /* ASM_BARRIER: pin s1 to $17 */
  register char *s1 asm("$17");
  char *a2;
  char *q;
  int v6;
  int v7;
  short h;
  int sw;
  int p0;
  int p1;
  int d0;
  int d1;
  int d2;
  int r0;
  int r1;
  int r2;
  int a1;
  int v;
  int s2;
  int s0;
  int a0;
  int c;
  int d;
  int t;
  int t2;
  int v0_3;
  s2 = 0x1800;
  s1 = arg0 + 0x1F4;
  if (RaceState < 2)
  {
    *((int *) (((char *) s1) + 0xC)) = 0;
    *((int *) (((char *) arg0) + 0x178)) = 0;
  }
  else
    if (RaceState < 4)
  {
    q = CarInputs;
    a2 = (arg1 * 4) + q;
    a1 = *((u8 *) (((char *) a2) + 3));
    sw = a1 & 0xE0;
    if (sw == 0x20)
    {
      s2 = ((*((short *) (((char *) (*((char **) (((char *) s1) + 0x1C)))) + 8))) * 0x1400) / 1000;
      if (MirrorMode != 0)
      {
        p1 = a1 & 1;
        p0 = a1 & 2;
      }
      else
      {
        p0 = a1 & 1;
        p1 = a1 & 2;
      }
      if (p0 != 0)
      {
        t2 = *((int *) (((char *) s1) + 0xC));
        *((short *) (((char *) s1) + 0x44)) = 1;
        if (t2 <= 0)
        {
          if ((-s2) < t2)
          {
            *((int *) (((char *) s1) + 0xC)) = t2 - 0x500;
          }
        }
        else
        {
          *((int *) (((char *) s1) + 0xC)) = 0;
        }
      }
      else
        if (p1 != 0)
      {
        t2 = *((int *) (((char *) s1) + 0xC));
        *((short *) (((char *) s1) + 0x44)) = -1;
        if (t2 < 0)
        {
          *((int *) (((char *) s1) + 0xC)) = 0;
        }
        else
          if (t2 < s2)
        {
          *((int *) (((char *) s1) + 0xC)) = t2 + 0x500;
        }
      }
      else
      {
        *((short *) (((char *) s1) + 0x44)) = 0;
        *((int *) (((char *) s1) + 0xC)) = (*((int *) (((char *) s1) + 0xC))) / 2;
      }
      *((int *) (((char *) arg0) + 0x178)) = -(*((int *) (((char *) s1) + 0xC)));
    }
    else
      if (sw == 0x40)
    {
      s2 = ((*((short *) (((char *) (*((char **) (((char *) s1) + 0x1C)))) + 8))) * 0x1800) / 1000;
      a0 = (s2 * (*((signed char *) (((char *) a2) + 0)))) / 127;
      d0 = MirrorMode;
      *((int *) (((char *) s1) + 0xC)) = a0;
      if (d0 != 0)
      {
        *((int *) (((char *) s1) + 0xC)) = -a0;
      }
      r0 = *((int *) (((char *) s1) + 0xC));
      if (r0 < 0)
      {
        *((short *) (((char *) s1) + 0x44)) = 1;
      }
      else
        if (r0 > 0)
      {
        *((short *) (((char *) s1) + 0x44)) = -1;
      }
      else
      {
        *((short *) (((char *) s1) + 0x44)) = 0;
      }
      *((int *) (((char *) arg0) + 0x178)) = -(*((int *) (((char *) s1) + 0xC)));
    }
    else
      if (sw == 0x60)
    {
      s2 = ((*((short *) (((char *) (*((char **) (((char *) s1) + 0x1C)))) + 8))) * 0x1800) / 1000;
      a0 = (s2 * (*((signed char *) (((char *) a2) + 0)))) / 127;
      d1 = MirrorMode;
      *((int *) (((char *) s1) + 0xC)) = a0;
      if (d1 != 0)
      {
        *((int *) (((char *) s1) + 0xC)) = -a0;
      }
      r1 = *((int *) (((char *) s1) + 0xC));
      if (r1 < 0)
      {
        *((short *) (((char *) s1) + 0x44)) = 1;
      }
      else
        if (r1 > 0)
      {
        *((short *) (((char *) s1) + 0x44)) = -1;
      }
      else
      {
        *((short *) (((char *) s1) + 0x44)) = 0;
      }
      *((int *) (((char *) arg0) + 0x178)) = -(*((int *) (((char *) s1) + 0xC)));
    }
    else
      if (sw == 0x80)
    {
      s2 = ((*((short *) (((char *) (*((char **) (((char *) s1) + 0x1C)))) + 8))) * 0x1400) / 1000;
      a0 = (s2 * (*((signed char *) (((char *) a2) + 0)))) / 127;
      d2 = MirrorMode;
      *((int *) (((char *) s1) + 0xC)) = a0;
      if (d2 != 0)
      {
        *((int *) (((char *) s1) + 0xC)) = -a0;
      }
      r2 = *((int *) (((char *) s1) + 0xC));
      if (r2 < 0)
      {
        *((short *) (((char *) s1) + 0x44)) = 1;
      }
      else
        if (r2 > 0)
      {
        *((short *) (((char *) s1) + 0x44)) = -1;
      }
      else
      {
        *((short *) (((char *) s1) + 0x44)) = 0;
      }
      *((int *) (((char *) arg0) + 0x178)) = -(*((int *) (((char *) s1) + 0xC)));
    }
    else
    {
      *((int *) (((char *) s1) + 0xC)) = 0;
    }
  }
  else
  {
    int s0_else;
    /* ASM_BARRIER: pin t5 to a1 for first call argument */
    register int t5 asm("$5");
    t5 = (*((short *) (((char *) arg0) + 0x1EA))) << 0xB;
    t5 += 0x800;
    s0_else = AngleDiffSigned(*((int *) (((char *) arg0) + 0x54)), t5 - (*((short *) (((char *) arg0) + 0x1EC)))) << 8;
    a0 = 0x1000 - rcos(((*((int *) (((char *) arg0) + 0xF4))) << 8) / 96);
    if ((*((short *) (((char *) arg0) + 0x1D8))) < 0x320)
    {
      a0 = a0 * 8;
    }
    else
    {
      a0 = a0 * 6;
    }
    if ((*((short *) (((char *) arg0) + 0x1D8))) >= 0x51)
    {
      if ((*((short *) (((char *) arg0) + 0x1EA))) != 0)
      {
        if ((*((int *) (((char *) arg0) + 0xF4))) < 0)
        {
          a0 = -a0;
        }
      }
      else
        if ((*((int *) (((char *) arg0) + 0xF4))) > 0)
      {
        a0 = -a0;
      }
      a0 = a0 + s0_else;
    }
    else
    {
      *((int *) (((char *) s1) + 0xC)) = 0;
      a0 = 0;
    }
    if (a0 < (-0x1400))
    {
      a0 = -0x1400;
    }
    if (!(a0 < 0x1401))
    {
      a0 = 0x1400;
    }
    *((int *) (((char *) s1) + 0xC)) = a0;
    *((int *) (((char *) arg0) + 0x178)) = -a0;
  }
  if ((*((short *) (((char *) arg0) + 0x1B8))) == 0)
  {
    t = *((short *) (((char *) s1) + 0x28));
    if ((t > 0) && (t < (*((short *) (((char *) s1) + 0x2A)))))
    {
      a0 = 0xC8;
    }
    else
    {
      if (((*((short *) (((char *) arg0) + 0x19E))) == 1) && ((*((int *) (((char *) s1) + 0xA4))) == 0))
      {
        /* ASM_BARRIER: pin h_sh to v0 for sll 16 */
        register int h_sh asm("$2");
        h_sh = (((short) (*((u16 *) (((char *) s1) + 0x36)))) / 3) << 16;
        a0 = (h_sh >> 16) / 30;
      }
      else
      {
        /* ASM_BARRIER: block hoist of a0 = 0 into bne delay slot */
        __asm__ volatile("");
        a0 = 0;
      }
    }
    a1 = *((short *) (((char *) arg0) + 0x1D8));
    if ((a1 < 0x100) && ((*((int *) (((char *) s1) + 0xA4))) == 0))
    {
      d = ((((*((int *) (((char *) s1) + 0xC))) * ((*((short *) (((char *) s1) + 0x36))) + a0)) / 256) * a1) / 0x10000;
      *((int *) (((char *) s1) + 0x9C)) = (*((int *) (((char *) s1) + 0x9C))) + d;
    }
    else
      if ((a1 < 0x200) && ((*((int *) (((char *) s1) + 0xA4))) == 4))
    {
      d = ((((*((int *) (((char *) s1) + 0xC))) * (*((short *) (((char *) s1) + 0x36)))) / 256) * a1) / 0x20000;
      *((int *) (((char *) s1) + 0x9C)) = (*((int *) (((char *) s1) + 0x9C))) + d;
    }
    else
    {
      d = ((*((int *) (((char *) s1) + 0xC))) * ((*((short *) (((char *) s1) + 0x36))) + a0)) / 0x10000;
      *((int *) (((char *) s1) + 0x9C)) = (*((int *) (((char *) s1) + 0x9C))) + d;
    }
  }
  *((int *) (((char *) s1) + 0x9C)) = (*((int *) (((char *) s1) + 0x9C))) & 0xFFF;
  if ((arg0 == PlayerCars) || (arg0 == (PlayerCars + 0x320)))
  {
    /* ASM_BARRIER: pin p4 to a0 for PlayerControllers base */
    register char *p4 asm("$4");
    p4 = PlayerControllers;
    *((short *) ((p4 + ((arg1 & 1) * 0x5C)) + 0x50)) = 1;
    s0 = *((short *) (((char *) arg0) + 0x1D8));
    t = (*((short *) (((char *) CarRecordPtrs[arg1]) + 0x1F0))) * 8;
    if (t < s0)
    {
      s0 = t;
    }
    a0 = *((int *) (((char *) s1) + 0xC));
    if ((a0 >> 3) != 0)
    {
      s2 = (((s0 * 0x14) / t) * a0) / s2;
    }
    else
    {
      s2 = 0;
    }
    c = *((int *) (((char *) s1) + 0xA4));
    if (c == 0)
      goto L_not2;
    if (c != 2)
    {
    L_not2:
      if ((*((short *) (((char *) (*((char **) (((char *) s1) + 0x1C)))) + 0xA))) > 0)
      {
        v6 = *((short *) (((char *) arg0) + 0x1DE));
        if (v6 > 0)
        {
          s2 += ((int) (((-v6) >> 3) + (((unsigned) (-v6)) >> 31))) >> 1;
        }
      }
    }
    else
    {
      s0 = AngleDiffSigned(*((short *) (((char *) arg0) + 0x1D4)), *((int *) (((char *) arg0) + 0x54)));
      if (s0 >= 0x201)
      {
        s0 = 0x200;
      }
      else
        if (s0 < (-0x200))
      {
        s0 = -0x200;
      }
      s2 = (s2 * rcos(s0 * 2)) / 0x1000;
    }
    s0 = 2;
    if (s2 <= 0)
    {
      s0 = 1;
      s2 = -s2;
    }
    if ((*((short *) (((char *) arg0) + 0x1B8))) != 0)
    {
      s2 = 0;
    }
    if (!(s2 < 0xC))
    {
      s2 -= PsyqRand() & 3;
    }
    if (!(s2 < 0x10))
    {
      s2 = 0xF;
    }
    if ((*((u8 *) (((char *) arg0) + 0x1A5))) >= 0x10U)
    {
      if ((*((u8 *) (((char *) arg0) + 0x1A4))) != 1)
      {
        /* ASM_BARRIER: pin v7_reg to v0 for lh 0x1A2 */
        register int v7_reg asm("$2");
        v7_reg = *((short *) (((char *) arg0) + 0x1A2));
        if (v7_reg < 0)
        {
          v = v7_reg;
          /* ASM_BARRIER: force copy to a0 in delay slot before negu a0,a0 */
          __asm__("" : "+r"(v));
          v = -v;
        }
        else
        {
          v = v7_reg;
        }
        if (v >= 0x96)
        {
          s2 = 0xF;
        }
        else
        {
          s2 = v / 10;
        }
        s0 = 1;
        if (((*((u8 *) (((char *) arg0) + 0x1A5))) & 7) >= 4U)
        {
          s0 = 2;
        }
      }
    }
    else
      if ((*((u8 *) (((char *) arg0) + 0x1A7))) != 0)
    {
      s2 = 0xD;
      s0 = 1;
      if ((*((short *) (((char *) arg0) + 0x1B6))) > 0)
      {
        s0 = 2;
      }
    }
    if ((*((short *) (((char *) arg0) + 0x1D8))) == 0)
    {
      s0 = 0;
      s2 = 0xF;
    }
    if (RaceState < 4)
    {
      PadSetRumbleParams(arg1 & 1, s0, s2);
    }
  }
}

#undef CarInputs

void UpdateCarEngine(a0)
P *a0;
{
  Q *q;
  R *r;
  S *s;
  int t0;
  int a2;
  int a1;
  int v0;
  int v1;
  int s1;
  int s3;
  int s4;
  short v;
  short *pf;
  int m1;
  int m2;
  int m3;
  int m4;
  int m5;
  int m6;
  int m7;
  int m8;
  int m9;
  int n1;
  int n2;
  int n3;
  int n4;
  int n5;
  int d1;
  int a;
  int b;
  int off;
  int x;
  int lo;
  int y;
  int k1;
  int k2;
  int k3;
  int k4;
  int k5;
  int k6;
  int fl;
  short t;
  int pad[8];
  int ten;
  q = &a0->q;
  r = q->s1C;
  off = (q->s86 * 0x3C) + 4;
  s = (S *) (((int) q->s18) + off);
  if (RaceState < 4)
  {
    q->sAC = CarInputs[q->s24][1];
    q->sAE = CarInputs[q->s24][2];
    if (q->sAC >= 0xFF)
    {
      q->sAC = 0x100;
    }
    if (q->sAE >= 0xFF)
    {
      q->sAE = 0x100;
    }
  }
  else
  {
    q->sAC = (0x400 - a0->s172) / 4;
    q->sAE = a0->s172 / 8;
  }
  if (q->sA8 == 0)
  {
    if (q->sAC >= 0x85)
    {
      q->sA8 = 1;
    }
  }
  else
    if (q->sA8 == 1)
  {
    q->sA8 = 2;
  }
  else
    if (q->sAC < 0x7C)
  {
    q->sA8 = 0;
  }
  if (q->sAA == 0)
  {
    if (q->sAE >= 0x85)
    {
      q->sAA = 1;
    }
  }
  else
    if (q->sAA == 1)
  {
    q->sAA = 2;
  }
  else
    if (q->sAE < 0x7C)
  {
    q->sAA = 0;
  }
  t0 = r->s0;
  if (((short) r->sA) > 0)
  {
    if (a0->s1D8 >= 0x321)
    {
      a2 = a0->s1DE + 0x12C;
    }
    else
    {
      a2 = 0x12C;
    }
    if (a0->s1DE > 0)
    {
      t0 += (a0->s1DE * r->s18) >> 2;
    }
  }
  else
  {
    m1 = q->sAC * 0x64;
    m2 = m1 >> 8;
    if (m1 < 0)
    {
      m1 += 0xFF;
      m2 = m1 >> 8;
    }
    a2 = 0x17C - m2;
    if (q->s3C > 0)
    {
      a2 += q->s3C * 3;
    }
    a2 += ((q->sAE * (0x64 + (((short) r->sA) / 2))) * r->sC) / 256000;
  }
  if (q->sA4 == 1)
  {
    q->s36 = (q->s36 + a2) / 2;
  }
  else
  {
    n2 = q->s44 * a0->s58;
    if (n2 > 0)
    {
      if (r->s12 < n2)
      {
        n2 = r->s12;
      }
      a2 = a2 + ((n2 * r->s14) / 1000);
    }
    m4 = a2 * t0;
    q->s36 = q->s36 + ((((m4 / 1000) - q->s36) * 0x1C) / 32);
  }
  ten = 10;
  s3 = s->s0;
  if (q->s38 == ten)
  {
    a2 = ((q->s88 * s3) - q->sA0) / s3;
    q->s8C = a2 / 10;
    q->s40 = (q->s88 - ((a0->s1D8 * 10000) / ((s->s2 * 0x490) / 160))) / (q->s3C + 1);
  }
  if (q->s38 > 0)
  {
    s1 = q->s8C;
    q->s38 = q->s38 - 1;
    if (q->s32 == 1)
    {
      s3 = ((s->s0 * (ten - q->s38)) + (s[-1].s0 * q->s38)) / 10;
    }
  }
  else
  {
    m5 = (q->s88 * s->s0) - q->sA0;
    q->s32 = 0;
    s1 = m5 / s3;
  }
  if (q->s3C > 0)
  {
    s1 += q->s40;
  }
  k1 = q->s88;
  if (k1 >= 10000)
  {
    a1 = 0x26AC - k1;
  }
  else
  {
    n2 = k1 / 500;
    m7 = k1 - (n2 * 500);
    if ((q->s86 == 1) && (n2 < 8))
    {
      k1 = s->s8[n2] * 3;
    }
    else
    {
      k1 = s->s8[n2];
    }
    a1 = ((k1 * (500 - m7)) + (s->s8[n2 + 1] * m7)) / 500;
  }
  s4 = ((a1 * q->sAC) >> 8) * ((0x100 - q->s30) >> 8);
  if (a0->s1B8 == 0)
  {
    k2 = q->s88;
    n2 = k2 / 256;
    n2 += (((k2 * q->sAE) / 256) * q->s1C->sC) / 32000;
    if (((unsigned) (q->sA4 - 1)) < 2)
    {
      n2 = 0;
    }
    s1 += n2;
  }
  if (a1 > 0)
  {
    if (q->sAC < 0x7F)
    {
      s1 += a1 / 2;
    }
  }
  else
  {
    s1 -= a1 / 2;
  }
  k3 = AngleDiffAbs(a0->s54, a0->s1D4);
  if (k3 > 0x400)
  {
    k3 = 0x800 - k3;
  }
  k6 = k3;
  if (k3 < 0)
  {
    k6 = k3 + 0xFF;
  }
  s1 += k6 >> 8;
  if (q->sA4 == 0)
  {
    k4 = q->sC;
    if ((CarInputs[q->s24][3] & 0xE0) != 0x20)
    {
      k4 = k4 >> 1;
    }
    if (q->sC >= 0)
    {
      s1 += k4 / r->s6;
    }
    else
    {
      s1 -= k4 / r->s6;
    }
  }
  k1 = a0->s50;
  if (k1 < (-200))
  {
    k1 = -200;
  }
  else
    if (k1 > 100)
  {
    k1 = 100;
  }
  s4 = (s4 * (k1 + 1000)) / 1000;
  n3 = q->s88 + (s4 - s1);
  q->s88 = n3;
  if (n3 < 0)
  {
    q->s88 = 0;
  }
  else
    if (n3 >= 0x3A99)
  {
    q->s88 = 0x3A98;
  }
  lo = q->s88 * s3;
  q->sA0 = lo;
  if (a0->s1B8 != 0)
  {
    a0->s1E0 = 0;
    t = a0->s1B8 | 0x100;
    a0->s1B8 = t;
    a0->s1D8 = (a0->s1D8 * 999) / 1000;
    if (t == 0x100)
    {
      q->s3C = 30;
      q->s40 = (q->s88 - ((a0->s1D8 * 10000) / ((s->s2 * 0x490) / 160))) / 30;
      a0->s1B8 = 0;
    }
  }
  else
    if (((unsigned) (q->sA4 - 1)) < 2)
  {
    n2 = 0x1000 - rcos(AngleDiffAbs(a0->s1D4, a0->s54));
    x = (n2 * 100) >> 12;
    k3 = r->sE;
    k3 = k3 + 9000;
    a0->s1D8 = (a0->s1D8 * (k3 - x)) / 10000;
    a0->s1E0 = 0;
  }
  else
    if (q->s3C > 0)
  {
    x = lo;
    if (x < 0)
    {
      x += 0x1FFFF;
    }
    a0->s1E0 = ((x >> 17) * (30 - q->s3C)) / 30;
    k2 = (((a0->s1D8 * 6) / 100) * (30 - q->s3C)) / 30;
    a0->s1D8 = a0->s1D8 - k2;
  }
  else
  {
    n5 = lo;
    if (n5 < 0)
    {
      n5 += 0x1FFFF;
    }
    a0->s1E0 = n5 >> 17;
    k2 = (a0->s1D8 * 6) / 100;
    a0->s1D8 = a0->s1D8 - k2;
  }
  if (a0->s1D8 < 8)
  {
    a0->s1D4 = a0->s54;
  }
  if (RaceState >= 2)
  {
    switch (q->sA4)
    {
      case 0:
        StepCarStateNormal(a0);
        break;

      case 2:
        StepCarStateSlide(a0);
        break;

      case 4:
        StepCarStateSpinOut(a0);
        break;

    }

  }
  else
  {
    a0->s1D8 = 0;
  }
  if (q->sAC == 0)
  {
    if (a0->s1D8 < 0x20)
    {
      a0->s1D8 = a0->s1D8 >> 1;
    }
  }
  if ((((char *) a0) == PlayerCars) || (((char *) a0) == (PlayerCars + 0x320)))
  {
    d1 = EngineRevDisplay[q->s24 & 1];
    k2 = a0->s1DE;
    if (k2 > 0)
    {
      k2 = 0x64;
    }
    else
    {
      k2 += 0x50;
    }
    if (k2 <= 0)
    {
      k2 = 1;
    }
    EngineRevDisplay[q->s24 & 1] = d1 + (((q->s88 - d1) * 0x14) / k2);
    if (EngineRevDisplay[q->s24 & 1] < 0x1F5)
    {
      EngineRevDisplay[q->s24 & 1] += 0x1F4 - ((PsyqRand() * 0x64) / 32767);
    }
    EngineThrottleOn[q->s24 & 1] = (q->sAC >= 0x41) && (q->s38 == 0);
  }
}

void IntegrateCarMotion(char *p) {
    char *s5;
    char *s2;
    char *s1;
    char *s0;
    int s3;
    int v1, v0, a1, a0, m1, m2;

    s5 = (char *)SCRATCH_CAR_WORK;
    if ((U(p, 0x1E4) & 0x10) == 0) {
        m1 = S(p, 0x10E) * S(p, 0x1D8);
        I(p, 0x30) = I(p, 0x30) << 8;
        I(p, 0x38) = I(p, 0x38) << 8;
        S(s5, 0x68) = m1 / (I(p, 0x2EC) / 128 + 1);
    } else {
        m2 = S(p, 0x10E) * S(p, 0x1D8);
        v0 = I(p, 0x28C) + 1;
        S(s5, 0x68) = (m2 / v0) / 2;
        S(p, 0x1DA) = U(p, 0x1D8);
        S(p, 0x1D6) = U(p, 0x1D4);
    }
    s2 = s5 + 0x24;
    s3 = 0x800;
    a1 = s3 - I(p, 0x54);
    I(p, 0) = I(p, 0) + I(p, 0x30);
    I(p, 8) = I(p, 8) + I(p, 0x38);
    I(p, 0x10) = I(p, 0) >> 6;
    I(p, 0x18) = I(p, 8) >> 6;
    BuildRotMatrixY(s2, a1);
    s0 = s5 + 4;
    BuildRotMatrixX(s0, I(p, 0x50));
    MulMatrix(s2, s0);
    BuildRotMatrixZ(s0, I(p, 0x58));
    MulMatrix(s2, s0);
    s1 = s5 + 0x44;
    BuildRotMatrixY(s1, s3 - S(p, 0x82));
    BuildRotMatrixX(s0, S(p, 0x80));
    MulMatrix(s1, s0);
    BuildRotMatrixZ(s0, S(p, 0x84));
    MulMatrix(s1, s0);
    S(s5, 0x64) = 0;
    S(s5, 0x66) = -U(p, 0x108);
    RR_GTE_SETROT(s2);
    RR_GTE_MVMVA_ROT_V0_NOCV(s5 + 0x64);
    S(s5, 0x6C) = 0;
    v0 = S(p, 0xE8);
    v1 = S(p, 0xEA);
    S(s5, 0x70) = 0;
    S(s5, 0x6E) = (v0 + v1) >> 7;
    RR_GTE_STORE_IR(p + 0x40);
    RR_GTE_SETROT(s1);
    RR_GTE_MVMVA_ROT_V0_NOCV(s5 + 0x6C);
    S(s5, 0x74) = 0;
    v0 = S(p, 0xE0);
    v1 = S(p, 0xE2);
    S(s5, 0x78) = 0;
    S(s5, 0x76) = (v0 + v1) >> 3;
    RR_GTE_STORE_IR(p + 0x70);
    RR_GTE_MVMVA_ROT_V0_NOCV(s5 + 0x74);
    I(p, 0x10) = I(p, 0x10) + I(p, 0x40);
    I(p, 0x18) = I(p, 0x18) + I(p, 0x48);
    I(p, 0x20) = I(p, 0x10) >> 2;
    I(p, 0x28) = I(p, 0x18) >> 2;
    RR_GTE_STORE_IR(p + 0x60);
}
