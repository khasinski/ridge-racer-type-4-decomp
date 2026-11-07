/* RODATA 0x80010774 0x4c */
/* CC1_FLAGS: -fno-strength-reduce */
#include "types.h"
#include "scratchpad.h"
#include "rr/selection.h"
#include "m2c_macros.h"
#include "rr/menu_transition.h"
#include "game/gfx/gfxmtx.h"
#include "game/ui/hudmap2.h"
#include "game/ui/uidraw.h"
#include "game/ui/uitext.h"
#include "game/ui/uiwin.h"
#include "game/menu/mnrows.h"
#include "game/ui/uidraw.h"
#include "rr/audio_cd_controller.h"
#include "rr/ui.h"
#include "rr/menu_text_rows.h"
#include "rr/frame_environment.h"
#include "game/data/screenframe.h"

extern int CdDirectoryBuffer;
extern char D_8001075C[];
extern int LoadCursor;
extern int MenuArenas[];
extern int StopRaceAudioAndRumble();
extern int ClearScreenOverlayState();
extern int VSyncThreshold;
extern int MenuArenaBase;
extern short CourseListIndex;

void ResetScreenCounters(void);
void InitMenuArenas();
void InitMenuScript();

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
    weights=D_8009F81C;
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
  if(group==1) {word=D_800AD4EC;group=word+1;}
  {
   register u8 *middle_page=(u8 *)0x800B0000U;
   register RrSelectionStatePrefix *middle;
   /* ASM_BARRIER: repeat the state address after selecting the threshold group. */
   __asm__("" : "+r"(middle_page));
   middle=(RrSelectionStatePrefix *)(middle_page-0x2B28);
   i=0;
   if(middle->phase<4) {
    s8 *thresholds=D_8009F82C;
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
   if(check==6) {func_8003989C();i=1;}}
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

void ResetScreenCounters(void) {
    LoadState = 0;
    MenuScriptStep = 0;
    ScreenFrame = 0;
}

void InitMenuArenas()
{
  /* ASM_BARRIER: pin the 0x2D000 constant to $a2 so the addu and the call share one lui/ori like the original */
  register int c asm("$6") = 0x2D000;
  BadgeRowCount = 10;
  Rect16RowCount = 4;
  GlyphRowCount = 4;
  UiRowCount = 0x28;
  ImageRowCount = 1;
  MenuTextRowCount = 0x37;
  MenuArenas[1] = CdDirectoryBuffer + c;
  LoadCursor = MenuArenas[1];
  DebugPrintStub(D_8001075C, LoadCursor, 0x2D000);
  LoadCursor = LoadCursor + AllocBadgeRows(LoadCursor);
  LoadCursor = LoadCursor + AllocRect16Rows(LoadCursor);
  LoadCursor = LoadCursor + AllocGlyphRows(LoadCursor);
  LoadCursor = LoadCursor + AllocImageRows(LoadCursor);
  LoadCursor = LoadCursor + AllocUiRows(LoadCursor);
  D_800ACC1C = LoadCursor;
  MenuArenas[1] = LoadCursor + 0x19000;
  LoadCursor = LoadCursor + 0x19000;
}

/* legacy typing of shared globals in this part of the unit */
#define SetDispMask ((int (*)())SetDispMask)
#define IntroScriptStep ((int (*)())IntroScriptStep)
#define MenuArenas (*(int (*))&MenuArenas)
#define ModeGroup (*(short (*))&ModeGroup)

void InitMenuScript() {
    int a0, a1, a3;
    ModeFrame = 0;
    VSyncThreshold = 0x80;
    LoadCursor = MenuArenaBase;
    a3 = 0;
    switch (MenuScriptId) {
    case 0:
        SetupDrawDispEnvs(0, 0, 0, 1);
        D_800FFC10 = 1;
        LoadCursor = MenuArenas;
        goto l428;
    case 10:
        if (RaceType == 5) goto five;
        SetupDrawDispEnvs(0, 0, 0, 1);
        D_800FFC10 = 1;
        LoadCursor = CdDirectoryBuffer;
        goto l428;
    five:
        SetupDrawDispEnvs(0, 0, 0, 2);
        D_800FFC10 = 0;
        goto l428;
    case 6:
        SetDispMask(0);
        StopRaceAudioAndRumble();
        a3 = 1;
    case 1:
    case 4:
        SetupDrawDispEnvs(8, 8, 8, a3 | 2);
        D_800FFC10 = 2;
        goto l418;
    default:
        SetupDrawDispEnvs(0, 0, 0, 2);
        D_800FFC10 = 0;
    }
l418:
    ClearScreenOverlayState();
    InitMenuArenas();
l428:
    ResetScreenCounters();
    if (D_800FFC10 == 1) {
        SetDispMask(0);
        a1 = 0;
        switch (RaceType) {
        case 0: a0 = 8; break;
        case 1:
            a0 = SelectionState[0x17];
            a1 = 3 - SelectionState[a0 + 8];
            break;
        case 2:
        case 3: a0 = 9; break;
        case 4: a0 = 10; break;
        case 6: a0 = CourseListIndex + 0xB; break;
        default:
            a0 = 0;
            a1 = a0;
            D_800FFC10 = 3;
            break;
        }
        if (D_800FFC10 == 1) func_80059314(a0, a1);
    } else if (D_800FFC10 == 0) {
        IntroScriptStep();
    }
    ModeGroup = 1;
    ModeId = 0x24;
    D_800F4D8A = 0;
}

#undef SetDispMask
#undef IntroScriptStep
#undef MenuArenas
#undef ModeGroup

void UpdateMenuScript(void) {
    s32 next_frame;
    s32 mode;
    s32 status;
    s8 next_step;
    s8 opcode;
    s32 step;

    next_frame = (s32)((u32)ModeFrame + 1U);
    ModeFrame = next_frame;
    if (next_frame == 2) {
        SetDispMask(1);
    }
    if (RaceType == 4) {
        UpdateLinkHandshake(1);
    }
    mode=D_800FFC10;
    if (mode == 1) {
        status = func_80059E40(ModeFrame);
    } else {
        if (mode == 0) {
            status = IntroScriptStep();
        } else {
            status = 1;
        }
    }
    step = MenuScriptStep;
    if (step >= 0) {
        if (MenuScriptOps[MenuScripts[MenuScriptId][step]]() == 1) {
            LoadState = 0;
            next_step = (u8) MenuScriptStep + 1;
            MenuScriptStep = next_step;
            opcode = MenuScripts[MenuScriptId][next_step];
            if (opcode < 0) {
                MenuScriptId = (u16) -opcode;
                MenuScriptStep = -1;
                if (D_800FFC10 == 0) {
                    func_8005CA14();
                }
            }
        }
    } else {
        if((status==1 && D_800FFC10==status) || (D_800FFC10==0 && status==2) || (D_800FFC10>=2 && status==1)) {
            MenuSceneEntries[MenuScriptId]();
            D_800F4D8A = 0;
            ModeGroup = (u16) ((u16) D_800AD50A >> 8);
            ModeId = D_800AD50A & 0xFF;
        }
    }
    if (ModeGroup != 1) {
        *(void **)SCRATCHPAD = AddTileGrid2x2((u32 *)((u8 *)CurrentDrawBuf + 0xB6C), *(void **)SCRATCHPAD, 0, 0, 0x140, 0x200, 0, 0, 0);
    }
}
