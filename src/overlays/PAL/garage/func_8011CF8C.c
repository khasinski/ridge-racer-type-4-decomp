#include "rr/overlay662_update.h"

void garage_func_8011CF8C(void)
{
    s32 first,second;
    if(garage_D_801276A8.fade00>=0) {
        union {u32 bits;s32 value;} intensity;
        if(garage_D_801276A8.fade00!=0)garage_D_801276A8.fade00--;
        intensity.bits=(64U-(u32)garage_D_801276A8.fade00)<<2;
        AddScreenFadeTiles(((RrFrameOrderingPrefix *)CurrentDrawBuf)->ordering70,intensity.value,69);
    }
    if(garage_D_801276A8.flagsD1&1)UpdateBadgeRowAnim(0);
    if(garage_D_801276A8.flagsD1&2){DrawUiRow(0);DrawUiRow(1);}
    if(garage_D_801276A8.flagsD1&4){SkipDialoguePage(DialogueState);UpdateDialogue(22,320,DialogueState);}
    first=0;
    if(garage_D_801276A8.fade00>=0)goto no_second;
    if(!(garage_D_801276A8.flagsD1&3))goto first_only;
    if(!(garage_D_801276A8.flagsD1&4))goto no_second;
    first=1;second=first;goto publish;
first_only:
    first=1;
no_second:
    /* ASM_BARRIER: retain the common second-flag assignment after the branch. */
    __asm__ volatile("" : : : "memory");
    second=0;
publish:
    SetMenuFrameOptions(first,second,0);
    UpdateScreenFade(0x1004E);
}
