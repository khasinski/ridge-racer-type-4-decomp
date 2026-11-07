#include "types.h"
#include "scratchpad.h"
#include "rr_asm.h"
#include "rr/race.h"
#include "m2c_macros.h"
#include "game/car/carmove1.h"
#include "rr/menu_overlay_runtime.h"

typedef struct T16 {
    s32 a;
    s32 pa;
    s32 c;
    s32 pc;
} T16;

typedef struct SP {
    char p0[8];
    s32 idx;
    char p1[0x34 - 0xC];
    s32 w34;
    s32 cnt;
    s32 r3c;
    s32 r40;
    char p2[0x64 - 0x44];
    u16 h64, h66, h68, h6a, h6c, h6e, h70, h72, h74, h76;
    s32 w78;
    char p3[0x80 - 0x7C];
    s32 w80;
    char p4[0x110 - 0x84];
    T16 t110[4];
    s16 s150[16];
} SP;

struct E
{
  char pad[0x1C];
  short w;
  char pad2[0x3C - 0x1E];
};

typedef struct 
{
  int n;
  char pad[0x38 - 4];
  short a;
  short b;
} T;

typedef struct 
{
  char p0[0x18];
  T *t;
  char p1[0x32 - 0x1C];
  short s32;
  short s34;
  char p2[2];
  short s38;
  char p3[0x84 - 0x3A];
  short s84;
  short s86;
  char p4[0xA4 - 0x88];
  int sA4;
} Q;

typedef struct 
{
  char pad[0x1D8];
  short s1D8;
} O;

typedef struct 
{
  char pad[3];
  unsigned char b;
} K;

extern s32 TrackNodeCount;
extern struct E *TrackNodes;
extern K CarInputs[];
extern int FindTrackNode();

s32 FindTrackNodeUnbounded(s32 *pos, s32 startIdx);
void UpdateCarTrackNode(RrRaceVehicle *p);
void UpdateWheelSpin(void *arg0);
void UpdateCarGear();
void FindTrackNodeAtXZ(int a0, int a1);

/* legacy typing of shared globals in this part of the unit */
#define TrackNodes (*(s32 (*))&TrackNodes)

s32 FindTrackNodeUnbounded(s32 *pos, s32 startIdx) {
    SP *g = (SP *)SCRATCH_CAR_WORK;
    s32 a, n, t, x;
    s32 r;
    /* ASM_BARRIER: keep g->s150[0] load in $v0 across g->cnt increment */
    register s32 u __asm__("$2");
    char *p, *q;
    s32 junk[2];
    /* ASM_BARRIER: GTE operand temps live in $t2/$t3/$t4 in the original PsyQ-style sequence */
    register s32 r1 __asm__("$10");
    /* ASM_BARRIER: GTE operand temps live in $t2/$t3/$t4 in the original PsyQ-style sequence */
    register s32 r2 __asm__("$11");
    /* ASM_BARRIER: GTE operand temps live in $t2/$t3/$t4 in the original PsyQ-style sequence */
    register s32 r3 __asm__("$12");

    g->cnt = 0;
    g->t110[0].a = pos[0];
    g->t110[0].c = pos[2];
    g->s150[0] = startIdx;
    g->s150[1] = TrackNodeCount;
    for (;;) {
        g->w34 = (g->s150[0] + 1) % TrackNodeCount;
        a = g->w34;
        p = (char *)TrackNodes + g->s150[0] * 0x3C;
        /* ASM_BARRIER: boost p reference weight so local-alloc assigns $a2 to p and $a3 to q */
        __asm__("" : "+r"(p));
        g->w78 = *(s32 *)(p + 8);
        g->w80 = *(s32 *)(p + 12);
        q = (char *)TrackNodes + a * 0x3C;
        g->h74 = g->h70 = *(u16 *)(q + 8) - (u16)g->w78;
        g->h76 = g->h72 = *(u16 *)(q + 12) - (u16)g->w80;
        g->h64 = (u16)g->t110[0].a - (u16)g->w78;
        g->h66 = (u16)g->t110[0].c - (u16)g->w80;
        g->h70 += *(u16 *)(q + 0);
        g->h72 += *(u16 *)(q + 2);
        g->h74 += *(u16 *)(q + 4);
        g->h76 += *(u16 *)(q + 6);
        g->h68 = *(u16 *)(p + 0);
        /* ASM_BARRIER: keep r3 load from g->h64 after g->h68 store */
        __asm__ volatile("" : : : "memory");
        g->h6a = *(u16 *)(p + 2);
        g->h6c = *(u16 *)(p + 4);
        g->h6e = *(u16 *)(p + 6);
        r3 = *(s32 *)&g->h64;
        r1 = *(s32 *)&g->h68;
        r2 = *(s32 *)&g->h6c;
        RR_GTE_NCLIP3(r1, r2, r3, &g->r3c);
        r1 = *(s32 *)&g->h74;
        r2 = *(s32 *)&g->h70;
        r3 = *(s32 *)&g->h64;
        RR_GTE_NCLIP3(r1, r2, r3, &g->r40);
        if (g->r3c >= 0 && g->r40 >= 0) {
            break;
        }
        n = g->cnt + 1;
        u = *(u16 *)&g->s150[0];
        /* ASM_BARRIER: schedule g->cnt + 1 before sign-extending g->s150[0] */
        __asm__("" : "+r"(u) : "r"(n));
        x = (s16)u;
        g->cnt = n;
        x = (n & 1) ? x + n : x - n;
        t = g->s150[1];
        a = g->s150[0] = x;
        if (a >= 0) {
            g->s150[0] = a % t;
        } else {
            r = (t + a) % t;
            /* ASM_BARRIER: keep modulo result in $v0 before storing to g->s150[0] */
            __asm__("" : "+r"(r));
            g->s150[0] = r;
        }
    }
    return g->s150[0];
}

#undef TrackNodes

void UpdateCarTrackNode(RrRaceVehicle *p)
{
  int s = 1;
  int t;
  int cur;
  int back;
  int fwd;
  int k;
  int j;
  int x;
  struct E *new_var;
  int n;
  struct E *base;
  int bk;
  t = FindTrackNodeUnbounded(p->pos20, p->trackNodeF0);
  if (t < 0)
  {
    *(short *)&p->flags1E4 = -1;
    return;
  }
  if (ReverseCourse == 0)
  {
    if (t != p->trackNodeF0)
    {
      cur = p->trackNodeF0;
      n = TrackNodeCount;
      base = TrackNodes;
      while (1)
      {
        x = cur - s;
        back = x;
        new_var = base;
        if (x < 0)
        {
          back = n + x;
        }
        fwd = (cur + s) % n;
        if (t == back)
        {
          for (k = 0; k < s; k++)
          {
            x = p->trackNodeF0 - k;
            j = x;
            if (x < 0)
            {
              j = n + x;
            }
            p->first += new_var[j].w;
          }

          break;
        }
        if (t == fwd)
        {
          for (k = 1; s >= k; k++)
          {
            p->first -= new_var[(p->trackNodeF0 + k) % n].w;
          }

          break;
        }
        s++;
        if (t == cur)
        {
          break;
        }
      }

    }
  }
  else
    if (t != p->trackNodeF0)
  {
    cur = p->trackNodeF0;
    n = TrackNodeCount;
    base = TrackNodes;
    while (1)
    {
      x = cur - s;
      back = x;
      if (x < 0)
      {
        back = n + x;
      }
      fwd = (cur + s) % n;
      if (t == fwd)
      {
        for (k = 0; k < s; k++)
        {
          p->first += base[(p->trackNodeF0 + k) % n].w;
        }

        break;
      }
      /* ASM_BARRIER: legacy, reason not recorded (forces reg copy/CSE/scheduling); retire & retest with fd.py */
      __asm__("" : "=r"(bk) : "0"(back));
      if (t == bk)
      {
        for (k = 1; s >= k; k++)
        {
          x = p->trackNodeF0 - k;
          j = x;
          if (x < 0)
          {
            j = n + x;
          }
          p->first -= base[j].w;
        }

        if (n)
        {
          break;
        }
        else
        {
          break;
        }
      }
      s++;
      x = p->trackNodeF0 - k;
      if (t == cur)
      {
        break;
      }
    }

  }
  p->trackNodeF0 = t;
}

void UpdateWheelSpin(void *arg0) {
    s32 temp_v1;
    s32 var_v1;

    var_v1 = M2C_FIELD(arg0, s16 *, 0x1D8) * 3;
    if (var_v1 >= 0xC1) {
        var_v1 = 0xC0;
    }
    temp_v1 = M2C_FIELD(arg0, s32 *, 0x178);
    M2C_FIELD(arg0, s32 *, 0x17C) = (s32) ((M2C_FIELD(arg0, s32 *, 0x17C) - var_v1) & 0xFFF);
    if (temp_v1 >= 0x1000) {
        M2C_FIELD(arg0, s32 *, 0x178) = 0x1000;
        return;
    }
    if (temp_v1 < -0xFFF) {
        M2C_FIELD(arg0, s32 *, 0x178) = -0x1000;
    }
}

void UpdateCarGear(a0, a1)
O *a0;
int a1;
{
  Q *q = (Q *) (((char *) a0) + 0x1F4);
  T *t;
  q->s34 = q->s86;
  t = q->t;
  if (q->s84 != 0)
  {
    int v = CarInputs[a1].b;
    if (v & 4)
    {
      if (q->s86 < t->n)
      {
        if (q->s38 == 0)
        {
          q->s86 = q->s86 + 1;
          q->s38 = 10;
          q->s32 = 1;
          return;
        }
      }
    }
    else
      if (v & 8)
    {
      if (q->s86 >= 2)
      {
        q->s86 = q->s86 - 1;
        q->s38 = 10;
      }
    }
  }
  else
  {
    int a;
    unsigned short u, u2;
    T *e1, *e2;
    if (u = q->s86, a0->s1D8 < (e1 = &t[q->s86])->b)
    {
      if (q->s86 >= 2)
      {
        q->s86 = u - 1;
        q->s38 = 10;
        goto L;
      }
    }
    if (u2 = q->s86, a0->s1D8 >= (e2 = &t[q->s86])->a)
    {
      if (q->s38 == 0)
      {
        if (q->s86 < t->n)
        {
          q->s86 = u2 + 1;
          q->s38 = 10;
          q->s32 = 1;
        }
      }
    }
    L:
    if (a0->s1D8 == 0)
    {
      if (q->s86 >= 2)
      {
        if (q->sA4 != 4)
        {
          q->s86 = 1;
          q->s38 = 0;
        }
      }
    }

  }
}

void FindTrackNodeAtXZ(int a0, int a1)
{
    int sp[4];
    register int y asm("$5");
    register int v asm("$2");
    a0 >>= 2;
    y = a1 >> 2;
    sp[0] = a0;
    sp[2] = y;
    if (a0 < 0) a0 += 0x7FF;
    v = y;
    a0 >>= 11;
    if (v < 0) v += 0x7FF;
    a0 = a0 + ((v >> 11) << 5);
    if (a0 < 0) a0 = 0;
    else if (a0 >= 0x400) a0 = 0x3FF;
    FindTrackNode(sp, TrackNodeGrid[a0], 6);
}
