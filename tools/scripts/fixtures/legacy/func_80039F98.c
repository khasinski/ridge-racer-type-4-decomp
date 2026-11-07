/* CC1_FLAGS: -fno-strength-reduce */
#include "rr/selection.h"
void UpdateSelectionRank(void) {
 register u8 *first_page=(u8 *)0x800B0000U;
 register RrSelectionStatePrefix *first;
 /* ASM_BARRIER: reuse V0 for word intermediates whose lifetimes end before publication. */
 register s32 word asm("$2");
 register s32 phase;
 register s32 level;
 register u8 *rows_page;
 register s32 group;
 /* ASM_BARRIER: reuse A0 for bucket and row indices, resetting it after the callback. */
 register s32 i asm("$4");
 register s32 sum;
 /* ASM_BARRIER: materialize the initial page before forming the state pointer. */
 __asm__("" : "+r"(first_page));
 first=(RrSelectionStatePrefix *)(first_page-0x2B28);
 word=first->phase;
 phase=word;
 if(word>0) {
  level=first->current.parameter;
  if(phase<4) {
   u8 *weights;
   register s8 *choices;
   sum=0;
   i=phase-1;
   if(i<=phase) {
    weights=SelectionRankWeights;
    choices=(s8 *)first;
    do {
     register s32 choice=*(s8 *)((unsigned long)i+(unsigned long)choices);
     register s32 scaled=i*4;
     sum+=*(u8 *)((unsigned long)(choice+scaled)+(unsigned long)weights);
     i++;
    }while(i<=phase);
   }
  }
  word=phase+((u32)phase>>31);
  group=word>>1;
  if(group==1) {word=SelectionRank;group=word+1;}
  {
   register u8 *middle_page=(u8 *)0x800B0000U;
   register RrSelectionStatePrefix *middle;
   /* ASM_BARRIER: repeat the state address after selecting the threshold group. */
   __asm__("" : "+r"(middle_page));
   middle=(RrSelectionStatePrefix *)(middle_page-0x2B28);
   i=0;
   if(middle->phase<4) {
    s8 *thresholds=SelectionRankThresholds;
    /* ASM_BARRIER: hold the configuration pointer separately from threshold values. */
    register RrSelectionStatePrefix *config asm("$9");

    register s32 next;
    /* ASM_BARRIER: retain the bucket adjustment base in T2. */
    register s32 four asm("$10");
    /* ASM_BARRIER: keep the bucket adjustment in V1 through the high-level branch. */
    register s32 delta asm("$3");
    group*=4;

    next=group+1;
    four=4;
    /* ASM_BARRIER: prepare the adjustment base before capturing the configuration pointer. */
    __asm__("" : "+r"(four) : : "$9");
    config=middle;
    do {
     s32 lower;
     if(i<3)lower=*(s8 *)((unsigned long)(i+next)+(unsigned long)thresholds);else lower=-1;
     if(*(s8 *)((unsigned long)(i+group)+(unsigned long)thresholds)>=sum && lower<sum) {
      delta=four-i;
      if(config->phase>=2)goto high_level;
      level+=delta;
      goto publish_state;
     }
     i++;
    }while(i<4);
    goto publish_state;
high_level:
    word=level+3;
    level=word+delta;
    goto publish_state;
   }else {level+=7;i=level<16;}
  }
publish_state:
  {
   /* ASM_BARRIER: materialize the configuration page in V0. */
   register u8 *publish_page asm("$2")=(u8 *)0x800B0000U;
   /* ASM_BARRIER: reuse the dead page temporary for the configuration pointer. */
   register RrSelectionStatePrefix *publish asm("$2");
   RrSelectionRow *target;
   register u32 index;
   /* ASM_BARRIER: form a fresh state pointer for publication. */
   __asm__("" : "+r"(publish_page));
   publish=(RrSelectionStatePrefix *)(publish_page-0x2B28);
   index=publish->index;
   /* ASM_BARRIER: read the history index before materializing the row page. */
   __asm__("" : "+r"(index) : : "$17");
   rows_page=(u8 *)0x800F0000U;
   publish->rank=i;
   /* ASM_BARRIER: publish rank before advancing the captured history index. */
   __asm__("" : "+r"(rows_page), "+r"(index) : "m"(publish->rank));
   target=(RrSelectionRow *)((unsigned long)(index*4)+(unsigned long)publish+0x20);
   __builtin_memcpy(target,&publish->current,sizeof(RrSelectionRow));
   publish->current.parameter=level;
   /* ASM_BARRIER: publish the new parameter before forming the displayed record address. */
   __asm__("" : "+r"(rows_page) : "m"(publish->current.parameter));
   /* ASM_BARRIER: pass the short-lived copy destination in T6. */
   {register RrSelectionRow *published_row asm("$14")=(RrSelectionRow *)(rows_page+0x4D98);
   /* ASM_BARRIER: consume the T6 copy address while preserving the original page for the callback. */
   __asm__("" : "+r"(rows_page) : "r"(published_row));
   __builtin_memcpy(published_row,&publish->current,sizeof(RrSelectionRow));}

   {register s32 check=publish->phase;
   i=1;
   if(check==6) {SetSelectionTableIndex();i=1;}}
  }

  {register RrSelectionRows *rows_base;
  RrSelectionRow *row;
  rows_base=(RrSelectionRows *)(rows_page+0x4D98);
  /* ASM_BARRIER: retain the row base before adding the first-row offset. */
  __asm__("" : "+r"(rows_base));
  row=rows_base->rows;
  do {row->parameter=level;i++;row++;}while(i<8);}
 }
}
