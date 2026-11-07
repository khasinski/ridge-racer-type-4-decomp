#include "rr/overlay667_course_initializer.h"
typedef struct {s8 first,second;} Pair;
void ovl667_func_80116918(void){
 s32 i;s32 j;register s32 offset __asm__("$7");s32 pos;
 register u8 *repeat __asm__("$5");
 u8 *destBase;
 u8 *sourceBase;
 SetDispMask(0);
 SetupDrawDispEnvs(0,0,0,2);
 UpdateLinkHandshake(1);
 i=0;
 ClearScreenOverlayState();
 for(;i<2;i++) {CourseMenuPage[i]=0;CourseMenuCursor[i]=0;PlayerWinCounts[i]=0;}
 for(i=0;i<4;i++) {((RrMenuChoice4 *)((u8 *)&SelectionRows+i*4))->bytes[3]=0;PlayerWinCounts[i]=0;}
 i=0;
 destBase=(u8 *)&CourseMenuChoices;
 sourceBase=(u8 *)&CourseChoiceTable;
 __asm__("" : "+r"(destBase),"+r"(sourceBase));
 offset=0;
 CourseIndex=0;CourseMenuSelection=0;NumLaps=3;
 SelectionPlayer=(1-(s16)LinkRoleSlot)*2;
 for(;i<2;i++,offset+=36)for(j=0,repeat=sourceBase,pos=offset;j<9;repeat+=4,j++,pos+=4) {
  u8 *dest=((u8 *)((unsigned long)(long)pos+(unsigned long)destBase));
  __builtin_memcpy(dest,(u8 *)((unsigned long)(long)pos+(unsigned long)sourceBase),4);
  __builtin_memcpy(dest+72,repeat,4);
  __asm__("" : : "r"(j),"r"(pos),"r"(repeat));
 }
 for(i=0;i<2;i++)CopyStringClearTail(PlayerNames[SelectionPlayer+i],ovl667_D_80118E00[SelectionPlayer+i]);
 ovl667_func_801161A8((1-(s16)LinkRoleSlot)*2);
 UnpackChoiceGlyph((s8*)((u8 *)&SelectionRows+SelectionPlayer*4),CourseChoiceTable[0][0].glyph);
 UnpackChoiceGlyph((s8*)((u8 *)&SelectionRows+4+SelectionPlayer*4),CourseChoiceTable[0][0].glyph);
 AvailableCourseCount=8;SeqStartTrack(0);ModeGroup=1;ModeId=60;ModeSubState=0;D_8010C0B8=0;
 if(((Pair *)((s8 *)&LinkEntryState+SelectionPlayer))->second!=1)D_8010C0D4=0;else D_8010C0D4=2;
 SetMenuFrameOptions(0,1,1);
}
