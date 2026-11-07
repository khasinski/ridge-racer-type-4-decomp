/* RODATA 0x800120d8 0x14 */
/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "rr/channels.h"
#include "psyq/libpad/padenab.h"
#include "psyq/libpad/padopen.h"
#include "psyq/libpad/padpatch.h"
#include "psyq/libpad/padstat2.h"
#include "psyq/libpad/padctl.h"
#include "rr/channel_init.h"

struct E { unsigned char n; char pad[3]; unsigned char *p; };

struct S { char pad[8]; struct E *e; char pad2[0xde]; unsigned char c; };

void PadChkVsync();
void PadStopCom();
int PadChkMtap();
int PadInfoComb();
void PadSetMainMode(s32 selector, s32 first, s32 second);

void PadChkVsync() { _padChkVsync(); }

/* legacy typing of shared globals in this part of the unit */
#define _padStartCom ((int (*)())_padStartCom)

void PadStartCom() { _padStartCom(); }

#undef _padStartCom

void PadStopCom() { _padStopCom(); }

/* legacy typing of shared globals in this part of the unit */
#define PadChannelRecords ((char *)PadChannelRecords)

int PadChkMtap(a0) int a0; {
    if (PadMultitapEnabled) {
        int i = a0 >> 4;
        return PadChannelRecords[i * 0xF0 + 0xE8] == 8;
    }
    return 0;
}

#undef PadChannelRecords

/* legacy typing of shared globals in this part of the unit */
#define PadPortToChannelPtr (*(char *(*(*))())&PadPortToChannelPtr)

int PadGetState() {
  char *p = PadPortToChannelPtr();
  int a;
  if (p[0x37] == 0 && p[0x38] == 0) {
    if (p == *(char **)(p + 0x10) || p[0x39] == 0) {
      if (**(char **)(p + 0x30) == 0) return p[0x49];
    }
  }
  a = p[0x49];
  switch (a) {
  case 3: return 1;
  case 2: return 1;
  case 6: return 4;
  }
  return p[0x49];
}

#undef PadPortToChannelPtr

/* legacy typing of shared globals in this part of the unit */
#define PadPortToChannelPtr (*(char *(*(*))())&PadPortToChannelPtr)

int PadInfoMode(a0, a1, a2) int a0, a1, a2; {
  char *p = PadPortToChannelPtr();
  switch (a1) {
  case 1: return *(unsigned char *)(p + 0xE8);
  case 2: return *(unsigned short *)(p + 0xE6);
  case 3: return *(unsigned char *)(p + 0xE4);
  case 4:
    if (a2 < 0) return *(unsigned char *)(p + 0xE3);
    if (a2 < *(unsigned char *)(p + 0xE3)) return (*(unsigned short **)p)[a2];
    return 0;
  case 100: return *(int *)(p + 0x4C);
  }
  return 0;
}

#undef PadPortToChannelPtr

/* legacy typing of shared globals in this part of the unit */
#define PadPortToChannelPtr (*(unsigned char *(*(*))())&PadPortToChannelPtr)

int PadInfoAct(a0, a1, a2) int a0, a1, a2; {
  unsigned char *p = PadPortToChannelPtr();
  unsigned char *q;
  if (a1 < 0) return p[0xE9];
  if (a1 < p[0xE9]) {
    q = *(unsigned char **)(p + 4) + a1 * 5;
    switch (a2) {
    case 1: return q[0];
    case 2: return q[1];
    case 3: return q[2];
    case 4: return q[3];
    case 5: return q[4];
    }
  }
  return 0;
}

#undef PadPortToChannelPtr

/* legacy typing of shared globals in this part of the unit */
#define PadPortToChannelPtr (*(struct S *(*(*))())&PadPortToChannelPtr)

int PadInfoComb(a0, a1, a2) int a0, a1, a2; {
    struct S *s = PadPortToChannelPtr();
    struct E *e;
    if (a1 < 0) return s->c;
    if (a1 >= s->c) return 0;
    e = s->e + a1;
    if (a2 < 0) return e->n;
    if (a2 >= e->n) return 0;
    return e->p[a2];
}

#undef PadPortToChannelPtr

/* legacy typing of shared globals in this part of the unit */
#define PadPortToChannelPtr (*(int (*(*))())&PadPortToChannelPtr)

void PadSetActAlign(a0, a1) int a0, a1; { _padSetActAlign(PadPortToChannelPtr(), a1); }

#undef PadPortToChannelPtr

void PadSetMainMode(s32 selector, s32 first, s32 second) {
    _padSetMainMode(PadPortToChannelPtr(selector), first & 0xFF, second & 0xFF);
}

/* legacy typing of shared globals in this part of the unit */
#define PadPortToChannelPtr (*(int (*(*))())&PadPortToChannelPtr)

void PadSetAct(a0, a1, a2) int a0, a1, a2; { _padSetAct(PadPortToChannelPtr(), a1, a2); }

#undef PadPortToChannelPtr
