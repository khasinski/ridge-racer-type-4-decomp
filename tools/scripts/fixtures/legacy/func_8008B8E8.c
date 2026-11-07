/* CC1_FLAGS: -mno-split-addresses */
#include "rr/library.h"
extern RrLibSavedState IntrEnv;
extern unsigned short *I_MASK, *I_STAT; extern int *DPCR_Intr;
extern void EnterCriticalSection(), ResetEntryInt();
RrLibSavedState *stopIntr(void) {
  RrLibSavedState *p = &IntrEnv;
  unsigned short *a, *c; int *b; int t;
  if (p->active == 0) return 0;
  EnterCriticalSection();
  a = I_MASK;
  b = DPCR_Intr;
  p->register16 = *a;
  t = *b;
  c = I_STAT;
  p->register32 = t;
  *a = 0;
  *c = *(volatile unsigned short *)a;
  *DPCR_Intr = *DPCR_Intr & 0x77777777;
  ResetEntryInt();
  p->active = 0;
  return p; }
