typedef struct { char pad[0x49]; char f49; char pad2[8]; short f52; char pad3[8]; } T;

typedef struct { char pad[0x49]; char f49; char pad2[6]; short f50; short f52; char pad3[8]; } T_1;

typedef struct { char pad[0x49]; char f49; char pad2[6]; short f50; short f52; char pad3[8]; } T_2;

typedef struct { char p0; unsigned char f1; char p2[6]; unsigned short f8; char pa[0x38]; unsigned char f42; char p43; unsigned char f44; char p45; char f46; unsigned char f47; unsigned char f48; char f49; char p4a[6]; short f50; char p52[10]; } T_3;

typedef signed char s8;

typedef unsigned char u8;

typedef signed short s16;

typedef unsigned short u16;

typedef signed int s32;

typedef unsigned int u32;

typedef struct {
    u8 pad0[0x43];
    u8 f43;
    u8 f44;
    u8 f45;
    u8 f46;
    u8 f47;
    u8 f48;
    u8 f49;
    s16 f4A;
    s16 f4C;
    s16 f4E;
    s16 f50;
    s16 f52;
    u8 pad54[8];
} Pad4BC5C;

typedef struct {
    u8 pad0[0x28];
    struct {
        s8 f28;
        s8 f29;
        s8 f2A;
        s8 f2B;
    } w[2];
} Settings4BC5C;

typedef struct { char p0; char b1; char pad[0xE]; int w10; char pad2[8]; short h1C; short h1E; char pad3[0x20]; char c40; char pad4[0x1B]; } W10_T;

extern T PlayerControllers[];
extern Settings4BC5C Options;
extern s16 PadRangeLimits[];
extern s32 VSyncThreshold;
extern char D_8009FFDC[];
extern char PadActAlignBytes[];
extern int PadGetState();
extern int PadInfoMode();
extern int PadInfoAct();
extern int PadSetActAlign();
extern int PadSetAct();
extern int PsyqBzero();

void PadSetRumbleParams(int i, short a, char b);
void PadRumbleStart(int i);
void PadRumbleStop(int i);
void PadSetRumbleFlags(int i, int flags);
void PadUpdateRumble(void);
void PadPollPort();

void PadSetRumbleParams(int i, short a, char b) {
    T *t = &PlayerControllers[i];
    t->f52 = a;
    t->f49 = b;
}

/* legacy typing of shared globals in this part of the unit */
#define PlayerControllers (*(T_1 (*)[])&PlayerControllers)

void PadRumbleStart(int i) {
    T_1 *t = &PlayerControllers[i];
    t->f50 = 1;
    PadSetRumbleFlags(i, 2);
    t->f49 = 0xF;
}

#undef PlayerControllers

/* legacy typing of shared globals in this part of the unit */
#define PlayerControllers (*(T_2 (*)[])&PlayerControllers)

void PadRumbleStop(int i) {
    T_2 *t = &PlayerControllers[i];
    t->f50 = 0;
    t->f49 = 0;
    t->f52 = 0;
}

#undef PlayerControllers

/* legacy typing of shared globals in this part of the unit */
#define PlayerControllers (*(T_3 (*)[])&PlayerControllers)

void PadSetRumbleFlags(int i, int flags) {
    T_3 *t = &PlayerControllers[i];
    if (flags & 1) {
        if (!(t->f44 & 0x40)) {
            t->f48 |= 0x40;
        }
    }
    if (flags & 2) {
        if (!(t->f44 & 0x80)) {
            t->f48 |= 0x80;
        }
    }
    if ((flags & 3) == 3) {
        t->f46 = 0;
        t->f47 = 0;
        t->f50 = 0;
        t->f49 = 0;
    }
    if (flags == 4) {
        if (!(t->f48 & 4)) {
            t->f47 = t->f42;
            t->f48 |= 0x84;
        }
    }
}

#undef PlayerControllers

/* legacy typing of shared globals in this part of the unit */
#define PlayerControllers (*(Pad4BC5C (*)[])&PlayerControllers)

void PadUpdateRumble(void) {
    s32 i;
    s32 j;
    u8 mask;
    s32 t1;
    u8 t0;
    u8 a0;
    Pad4BC5C *t;

    for (i = 0; i < 2; i++) {
        t = &PlayerControllers[i];
        for (j = 0; j < 2; j++) {
            mask = 1 << (j + 6);
            if (t->f48 & mask) {
                if (t->f44 & mask) {
                    t->f44 &= ~mask;
                } else {
                    t->f44 |= mask;
                    t->f48 &= ~mask;
                }
            } else if (t->f44 & mask) {
                s32 cond = (VSyncThreshold == 0x80) && !(t->f43 & mask);
                if (!cond) {
                    t->f44 &= ~mask;
                    t->f46 += t->f47;
                    t->f47 = 0;
                    t->f48 &= ~4;
                }
            }
        }
        t1 = t->f44 & 0xC0;
        t0 = t1;
        if (t->f49 >= 0x10) {
            t->f49 = 0xF;
        }
        a0 = (t->f49 * Options.w[i].f2A) / 2;
        if (t->f50 != 0) {
            if (t->f52 != 0) {
                if (__builtin_abs(t->f4C) < PadRangeLimits[Options.w[i].f29]) {
                    if (a0 != 0) {
                        t0 = t1 | (1 << (t->f52 + 3));
                    }
                    goto done;
                }
                t->f52 = 0;
            }
            if (a0 != 0) {
                t0 = t1 | 0x30;
            }
        } else {
            if (a0 != 0) {
                t0 = t1 | (1 << (t->f52 + 3));
            }
        }
    done:
        t->f44 = t0 | a0;
    }
}

#undef PlayerControllers

/* legacy typing of shared globals in this part of the unit */
#define PlayerControllers (*(W10_T (*)[])&PlayerControllers)

void PadPollPort(a0)
int a0;
{
  int w11_r;
  int s3 = a0;
  int s2 = a0 * 16;
  /* ASM_BARRIER: pin result var to $s0 (original assigns s0=result, s1=struct pointer) */
  register int s0 __asm__("$16");
  int v1;
  W10_T *s1 = &PlayerControllers[a0];
  s0 = PadGetState(s2);
  if (s0 == 0)
  {
    ((char *)s1)[1] = 0;
    s1->w10 = -1;
    return;
  }
  if (PadInfoMode(s2, 2, 0) == 0)
    s1->h1C = 0;
  else
    s1->h1C = 1;
  if (s0 == 1)
    ((unsigned char *)s1)[0x1B] = 0;
  if (s1->h1E != 0)
  {
    if (((unsigned char *)s1)[0x1B] != 0)
    {
      ((unsigned char *)s1)[0x1B] = 0;
      goto A;
    }
    goto B;
  }
A:
  if (((unsigned char *)s1)[0x1B] != 0)
    return;
B:
  if (s0 == 2)
  {
    s1->c40 = 0;
    ((unsigned char *)s1)[0x1B] = 1;
    s1->h1E = 0;
    return;
  }
  if (s0 != 6)
    return;
  { int t = PadInfoMode(s2, 1, 0); v1 = PadInfoMode(s2, 2, 0); s0 = t; }
  if (s0 == 0xE && v1 == 0x300)
  {
    PsyqBzero((char *)s1 + 0x40, 0x1C);
    s1->c40 = 1;
    s1->w10 = 2;
    PadSetAct(s2, (char *)s1 + 0x44, 1);
    if (PadSetActAlign(s2, PadActAlignBytes) != 0)
    {
      ((unsigned char *)s1)[0x1B] = 1;
      s1->h1E = 0;
      PadSetRumbleFlags(s3, 3);
    }
    return;
  }
  if (s0 != 2)
  {
    s1->c40 = 0;
    s1->w10 = 0;
    s0 = 0;
    s3 = PadInfoAct(s2, -1, s0);
    if (s3 == 1)
    {
      s0 = PadInfoAct(s2, 0, 1);
      v1 = PadInfoAct(s2, 0, 2);
      w11_r = s0;
      if (w11_r == 4 && v1 == 3)
        s1->c40 = s3;
    }
    PadSetAct(s2, (char *)s1 + 0x19, 2);
    if (PadSetActAlign(s2, D_8009FFDC) == 0)
      return;
    ((unsigned char *)s1)[0x1B] = 1;
    s1->h1E = 0;
  }
  else
  {
    s1->w10 = -1;
    s1->c40 = 0;
    ((unsigned char *)s1)[0x1B] = 1;
  }
}

#undef PlayerControllers
