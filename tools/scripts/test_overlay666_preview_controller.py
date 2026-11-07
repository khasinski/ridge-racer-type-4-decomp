from overlay_units import read_c_source
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess,re,sys
import os
os.chdir(Path(__file__).resolve().parents[2])
p=Path('build/goal_50/parallel666_preview');body=read_c_source(Path(sys.argv[1] if len(sys.argv)>1 else Path('src/overlays/PAL/666/func_80115C5C.c')));body=body[body.index('void ovl666_update_preview'):];body=re.sub(r' __asm__\("\$\d+"\)', '', body)
encode=read_c_source(Path('tools/scripts/fixtures/views/func_8003708C.c')).replace('PackChoiceGlyph','actual_encode');decode=read_c_source(Path('tools/scripts/fixtures/views/func_80037040.c')).replace('UnpackChoiceGlyph','actual_decode')
src=r'''
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "rr/layout.h"
#include "rr/frame_storage_types.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/selection_types.h"
#include "rr/current_selection_types.h"
#include "rr/menu_selection_word.h"
#include "rr/course_choice_types.h"
#include "rr/menu_glyph.h"
#include "rr/menu_selection.h"
#include "rr/race.h"
#include "rr/menu_arenas.h"
#include "rr/icon_sprites.h"
#include "rr/preview_rgb.h"
#include "rr/overlay_menu.h"
#include "rr/tables.h"
void SetupMenuCarCamera(s32);void SetupViewport(s32);s32 LoadCarRangeStep(s32,s32);s32 PackChoiceGlyph(s8 *);
void SetCarBounds(RrRaceVehicle *);void SetModelTexturePage(s32);void AddScreenFadeTiles(u32 *,s32,u32);void DrawUiRow(s32);
void DrawSelectionIconGrid(s32);void PrepareCarDrawPose(RrRaceVehicle *);void DrawPreviewCar(RrRaceVehicle *,RrPreviewRgb3,s32,s32);
void UpdateBadgeRowAnim(s32);s32 UpdateImageRow(s32);void UpdateScreenFade(s32);
void SetMenuFrameOptions(s32,s32,s32);void StepMenuValue(s32,s32,char*,s32,s32);void UnpackChoiceGlyph(s8*,s32);void ResetScreenCounters(void);void PlaySfx(s32);void SeqMuteVolume(void);void SeqFadeVolume(s32,s32);void ovl666_init_menu(void);
static struct State {u32 guard[4];s32 frame,player,rowCount; u16 state; s16 scene,timer,menu,row,course,d44,limitAlias;u16 enable,be8; s16 highCourse;u8 courseMap[8]; s32 overlayMode,ready; RrMenuSelectionWord32 selected; s16 limits[4];u16 courseTable[4][4];
 RrRaceVehicle vehicles[2];RrMenuItemState5C input[2];RrUiRow24 rows[2][7];RrBadgeRow1C badges[2][2];RrFrameStorage contexts[2];
 RrCurrentSelection choice;RrMenuGridPoint grid[4];RrCourseChoice4 courses[4][9];RrMenuArenas arenas;void *published;
 u8 modes[2][3];unsigned root,badgeRoot,modeRoot,contextRoot;u32 tail[4];} actual,reference,*w;
static u8 buffers[3][32];
#define ReverseCourse w->highCourse
#define CourseIndexMap w->courseMap
#define ModeFrame w->frame
#define UiRowCount w->rowCount
#define ModeSubState w->state
#define ModeId w->scene
#define ModeGroup w->enable
#define MenuFadeTimer w->timer
#define MenuCursor w->menu
#define CourseListIndex w->row
#define ovl666_selected_course w->selected
#define ovl666_course_counts w->limits
#define ovl666_course_dispatch w->courseTable
#define ovl666_preview_phase w->overlayMode
#define ovl666_result_kind w->ready
#define ovl666_available_courses w->courses
#define CourseIndex w->course
#define CourseMenuSelection w->limitAlias
#define RaceType w->d44
#define MenuScriptId w->be8
#define ScreenFrame w->player
#define MenuArenaBase w->arenas.second
#define PlayerCars w->vehicles
#define PlayerControllers w->input
#define UiRows (w->rows[w->root])
#define BadgeRows (w->badges[w->badgeRoot])
#define CurrentDrawBuf (&w->contexts[w->contextRoot])
#define SelectionRows w->choice
#define PlayerCourseChoices w->grid
#define CourseChoiceTable w->courses
#define LoadCursor w->published
#define MenuArenas w->arenas
#define ImageRows (w->modes[w->modeRoot])
'''+body+encode+decode+read_c_source(Path('tools/scripts/fixtures/views/func_80032E9C.c')).replace('DecodeCourseIndex','actual_course')+r'''
static struct Event {unsigned kind;s32 a[8];struct State before;}events[48];
static unsigned scenario,profile,serial,count,seen,coverage[24],actions[7],colors[2],exits[4],changed[2],completed[2];static int referenceRun;
static s16 half(u16 v){s16 r;memcpy(&r,&v,2);return r;}static s32 word(u32 v){s32 r;memcpy(&r,&v,4);return r;}
static void record(unsigned k,s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h){s32 args[]={a,b,c,d,e,f,g,h};assert(k<24);
 if(referenceRun){assert(count<48);struct Event *v=&events[count++];v->kind=k;memcpy(v->a,args,sizeof(args));v->before=*w;}
 else{coverage[k]++;assert(seen<count);struct Event *v=&events[seen++];assert(v->kind==k&&!memcmp(v->a,args,sizeof(args))&&!memcmp(w,&v->before,sizeof(*w)));}
}
static u16 scenePhase(void){static const u16 phases[]={0,3,4,5,6,99};return profile<12?phases[profile/2]:(u16)scenario;}
static void effect(unsigned k){unsigned n=serial++;if(!(profile&1))return;u32 v=(scenario+13*n+17*k)*0x9E3779B9U;
 w->frame=word((u32)w->frame+v);w->player=word((u32)w->player-v);w->state=scenePhase();
 w->root=(scenario+n)&1;w->badgeRoot=(scenario/2+n)&1;w->contextRoot=(scenario/4+n)&1;w->modeRoot=(scenario/8+n)&1;
 w->selected.word=(scenario/16+n)%9;w->menu=(scenario/32+n)%2;w->row=(scenario/64+n)%4;w->timer=half(scenario+31*n);
 w->choice.selection.primary=(scenario/128+n)%4;w->choice.selection.variant=(scenario/256+n)%4;w->choice.selection.pad02[1]=(scenario/512+n)%2;
 w->input[0].a=(scenario>>8)&1;w->input[0].b=(scenario>>9)&1;w->arenas.first=buffers[(scenario+n)%3];w->arenas.second=buffers[(scenario+n+1)%3];w->published=buffers[(scenario+n+2)%3];
 for(unsigned j=0;j<2;j++){for(unsigned i=0;i<7;i++)w->rows[j][i].byte14=((scenario>>11)&1)?2:5;w->modes[j][1]=(scenario/4+n+j)%12;w->modes[j][2]=(u8)(scenario+7*n);}
}
void SetupMenuCarCamera(s32 a){record(0,a,0,0,0,0,0,0,0);effect(0);}
void SetupViewport(s32 a){record(1,a,0,0,0,0,0,0,0);effect(1);}
s32 LoadCarRangeStep(s32 a,s32 b){record(2,a,b,0,0,0,0,0,0);effect(2);static const u32 values[]={0,1,0x10001,1,0xFFFFFF01,1,0xFFFFFFFF,1};return word(values[scenario%8]);}
void SetCarBounds(RrRaceVehicle *p){assert(p==w->vehicles&&(uintptr_t)p>UINT32_MAX);record(3,0,0,0,0,0,0,0,0);effect(3);}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){record(4,a,b,c,0,0,0,0,0);effect(4);}
s32 PackChoiceGlyph(s8 *p){assert(p==(s8*)&w->choice);record(5,0,0,0,0,0,0,0,0);if(referenceRun)return w->choice.selection.primary*160+w->choice.selection.variant*40+w->choice.selection.pad02[0];return actual_encode(p);}
void StepMenuValue(s32 a,s32 b,char *p,s32 d,s32 e){unsigned target;if(p==(char*)&w->selected.halves[0])target=0;else if(p==(char*)&w->menu)target=1;else{assert(p==(char*)&w->choice.selection.pad02[1]);target=2;}assert((uintptr_t)p>UINT32_MAX);
 record(6,a,b,target,d,e,0,0,0);effect(6);if(target==0)w->selected.halves[0]=(scenario+serial)%9;else if(target==1)w->menu=(scenario/4+serial)%2;else w->choice.selection.pad02[1]=(scenario+serial)%2;
}
void UnpackChoiceGlyph(s8 *p,s32 a){assert(p==(s8*)&w->choice);record(7,a,0,0,0,0,0,0,0);if(referenceRun){w->choice.selection.primary=a/160;w->choice.selection.variant=(a%160)/40;w->choice.selection.pad02[0]=a%40;}else actual_decode(p,a);}
void ResetScreenCounters(void){record(8,0,0,0,0,0,0,0,0);effect(8);}
void SetModelTexturePage(s32 a){record(9,a,0,0,0,0,0,0,0);effect(9);}
void PlaySfx(s32 a){record(10,a,0,0,0,0,0,0,0);effect(10);}
void ovl666_init_menu(void){record(11,0,0,0,0,0,0,0,0);effect(11);}
void AddScreenFadeTiles(u32 *p,s32 a,u32 b){assert(p==&w->contexts[w->contextRoot].ordering.ordering70&&(uintptr_t)p>UINT32_MAX);record(12,w->contextRoot,a,b,0,0,0,0,0);effect(12);}
void DrawUiRow(s32 a){record(13,a,0,0,0,0,0,0,0);effect(13);}
void DrawSelectionIconGrid(s32 a){record(14,a,0,0,0,0,0,0,0);effect(14);}
void PrepareCarDrawPose(RrRaceVehicle *p){assert(p==w->vehicles&&(uintptr_t)p>UINT32_MAX);record(15,0,0,0,0,0,0,0,0);effect(15);}
void DrawPreviewCar(RrRaceVehicle *p,RrPreviewRgb3 c,s32 a,s32 b){assert(p==w->vehicles);record(16,c.r,c.g,c.b,a,b,0,0,0);effect(16);}
void UpdateBadgeRowAnim(s32 a){record(17,a,0,0,0,0,0,0,0);effect(17);}
s32 UpdateImageRow(s32 a){record(18,a,0,0,0,0,0,0,0);effect(18);return half(scenario);}
void DrawMenuStepArrows(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g){record(19,a,b,c,d,e,f,g,0);effect(19);}
void UpdateScreenFade(s32 a){record(19,a,0,-1,0,0,0,0,0);effect(19);}
s32 DecodeCourseIndex(s32 a){record(20,a,0,0,0,0,0,0,0);effect(20);if(referenceRun){w->highCourse=a>=8;return w->courseMap[a&7];}return actual_course(a);}
void SeqMuteVolume(void){record(21,0,0,0,0,0,0,0,0);effect(21);}
void SeqFadeVolume(s32 a,s32 b){record(22,a,b,0,0,0,0,0,0);effect(22);}
/* Independent control flow transcribed from original overlay666 MIPS.
 * All services record complete pre-call worlds before deterministic callbacks. */
static void closeTwo(void){for(int i=3;i<5;i++)if(w->rows[w->root][i].byte14==2)w->rows[w->root][i].byte14=3;}
static void expected(void){int ready=0;w->frame=word((u32)w->frame+1);w->player=word((u32)w->player+1);w->vehicles[0].rot54=((u32)w->vehicles[0].rot54+8)&4095;
 SetupMenuCarCamera(1);SetupViewport(6);
 if(w->state!=3){int completion=LoadCarRangeStep(0,1);completed[completion==1]++;
  if(completion!=1){if((s8)w->modes[w->modeRoot][1]==1)w->modes[w->modeRoot][1]=0;goto footer;}
  SetCarBounds(w->vehicles);if((s8)w->modes[w->modeRoot][1]==7)w->modes[w->modeRoot][1]=1;
 }
 ready=1;
 switch(w->state){
 case 0:{SetMenuFrameOptions(1,1,1);int old=PackChoiceGlyph((s8*)&w->choice);StepMenuValue(1,2,(char*)&w->selected.halves[0],w->limits[w->row],0);UnpackChoiceGlyph((s8*)&w->choice,w->courses[w->row][w->selected.word].glyph);int current=PackChoiceGlyph((s8*)&w->choice);changed[(u16)old!=(u16)current]++;
  if((u16)old!=(u16)current){ResetScreenCounters();w->published=w->arenas.second;UnpackChoiceGlyph((s8*)&w->choice,w->courses[w->row][w->selected.word].glyph);w->grid[0].first=w->courses[w->row][w->selected.word].first;w->grid[0].second=w->courses[w->row][w->selected.word].second;w->modes[w->modeRoot][1]=6;}
  w->rows[w->root][w->menu+3].cells[0]=0;StepMenuValue(0,2,(char*)&w->menu,2,0);w->rows[w->root][w->menu+3].cells[0]=2;int phase=w->rows[w->root][w->menu+3].byte14;
  if(phase==2){if(w->input[0].a){w->rows[w->root][w->menu+3].cells[0]=1;if(w->menu==0){actions[0]++;PlaySfx(1);w->overlayMode=2;w->state=4;}else if(w->menu==1){actions[1]++;PlaySfx(1);closeTwo();w->modes[w->modeRoot][1]=3;w->state=5;w->badges[w->badgeRoot][0].state10=4;}}
   else if(w->input[0].b){actions[2]++;PlaySfx(2);closeTwo();w->modes[w->modeRoot][1]=3;w->state=5;w->badges[w->badgeRoot][0].state10=4;}}
  break;}
 case 3:{SetMenuFrameOptions(0,1,1);if(w->timer>0)w->timer=half((u16)w->timer-1);SetModelTexturePage(-1);AddScreenFadeTiles(&w->contexts[w->contextRoot].ordering.ordering70,(64-w->timer)*4,69);int completion=LoadCarRangeStep(1,1);
  if(completion==1&&w->timer<=0){exits[0]++;w->d44=6;u16 course=w->courseTable[w->choice.selection.primary][w->choice.selection.variant];w->ready=completion;w->limitAlias=course;w->course=DecodeCourseIndex(half(course));SeqMuteVolume();w->be8=0;w->enable=completion;w->scene=35;w->state=0;w->arenas.first=w->published;}break;}
 case 4:{int phase=w->rows[w->root][5].byte14;if(phase==2){w->rows[w->root][w->choice.selection.pad02[1]+5].cells[0]=0;StepMenuValue(0,1,(char*)&w->choice.selection.pad02[1],2,0);w->rows[w->root][w->choice.selection.pad02[1]+5].cells[0]=phase;
   if(w->input[0].a){actions[3]++;PlaySfx(1);SetMenuFrameOptions(0,1,1);w->rows[w->root][w->choice.selection.pad02[1]+5].cells[0]=1;w->rows[w->root][5].byte14=w->rows[w->root][6].byte14=7;
    u8 bytes[4];memcpy(bytes,&w->choice,4);memcpy(&w->choice.selection.rows[0],bytes,4);w->choice.selection.rows[0].parameter=19;w->course=0;w->state=3;ResetScreenCounters();w->timer=64;SeqFadeVolume(0,60);w->arenas.first=w->published;
   }else if(w->input[0].b){actions[4]++;PlaySfx(2);w->rows[w->root][5].byte14=w->rows[w->root][6].byte14=3;w->state=6;}}
  break;}
 case 6:if(w->rows[w->root][5].byte14==5){actions[5]++;w->state=0;w->overlayMode=1;w->rows[w->root][5].byte14=w->rows[w->root][6].byte14=0;}break;
 case 5:SetMenuFrameOptions(0,1,1);if(w->rows[w->root][3].byte14==5&&(s8)w->modes[w->modeRoot][1]==5){exits[1]++;ResetScreenCounters();ovl666_init_menu();w->enable=1;w->scene=79;w->state=0;goto title;}break;
 }
footer:
 for(int i=3;i<5;i++)DrawUiRow(i);if(w->overlayMode==2){DrawUiRow(5);DrawUiRow(6);}
 if((u8)((s8)w->modes[w->modeRoot][1]-1)<2&&ready==1){DrawSelectionIconGrid(0);PrepareCarDrawPose(w->vehicles);RrPreviewRgb3 c;int scale,flags;
  if((s8)w->modes[w->modeRoot][1]==ready){colors[0]++;unsigned level=w->modes[w->modeRoot][2];c.r=c.g=c.b=(u8)~(level*255/30);scale=level*256/30;flags=258;}else{colors[1]++;c.r=c.g=c.b=128;scale=256;flags=1;}DrawPreviewCar(w->vehicles,c,scale,flags);}
 UpdateBadgeRowAnim(0);if(UpdateImageRow(0)>0&&w->limits[w->row]>=2)DrawMenuStepArrows(160,144,8,1,w->selected.word,0,w->limits[w->row]);
title:UpdateScreenFade(0x10036);
}
int main(void){assert(sizeof(void*)==8);for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<14;profile++){
 memset(&actual,0xA6,sizeof(actual));w=&actual;static const u32 edge[]={0,0xFFFFFFFF,0x7FFFFFFF,0x80000000};w->frame=word((scenario&4)?scenario*0x1234567U:edge[scenario%4]);w->player=word((scenario&4)?scenario*0x9876543U:edge[scenario%4]);w->state=scenePhase();w->timer=half(scenario);w->menu=(scenario/4)%2;w->row=scenario%4;w->selected.word=scenario%9;for(int i=0;i<8;i++)w->courseMap[i]=(u8)(scenario+i*29);w->overlayMode=(scenario/16)%3;
 w->root=scenario&1;w->badgeRoot=(scenario>>1)&1;w->contextRoot=(scenario>>2)&1;w->modeRoot=(scenario>>3)&1;w->input[0].a=(scenario>>8)&1;w->input[0].b=(scenario>>9)&1;
 w->arenas.first=buffers[scenario%3];w->arenas.second=buffers[(scenario+1)%3];w->published=buffers[(scenario+2)%3];w->vehicles[0].rot54=word(scenario*0x3456789U);w->vehicles[1].rot54=word(~(scenario*0x456789AU));
 for(unsigned j=0;j<2;j++){for(unsigned i=0;i<7;i++)w->rows[j][i].byte14=((scenario>>11)&1)?2:5;w->modes[j][1]=(u8)scenario;w->modes[j][2]=(u8)(scenario>>8);}
 for(unsigned g=0;g<4;g++){w->limits[g]=half(scenario+g);for(unsigned h=0;h<4;h++)w->courseTable[g][h]=(u16)(scenario*17+g*251+h*29);for(unsigned i=0;i<9;i++){w->courses[g][i].glyph=(scenario&64)?half(scenario+g*79+i*40):(scenario/2+g*79+i*40)%640;w->courses[g][i].first=(s8)(scenario+i);w->courses[g][i].second=(s8)(~scenario-i);}}
 unsigned glyph=(scenario&32)?(scenario*3)%640:(scenario/2+w->row*79+((scenario+6)%9)*40)%640;w->choice.selection.primary=glyph/160;w->choice.selection.variant=(glyph%160)/40;w->choice.selection.pad02[0]=glyph%40;w->choice.selection.pad02[1]=(scenario>>5)&1;
 reference=actual;count=seen=serial=0;referenceRun=1;w=&reference;expected();serial=0;referenceRun=0;w=&actual;ovl666_update_preview();assert(count==seen&&!memcmp(&actual,&reference,sizeof(actual)));
 }
 for(unsigned i=0;i<23;i++)assert(coverage[i]);for(unsigned i=0;i<6;i++)assert(actions[i]);for(unsigned i=0;i<2;i++)assert(exits[i]);assert(colors[0]&&colors[1]&&changed[0]&&changed[1]&&completed[0]&&completed[1]);
 puts("917504 cases passed: complete guarded worlds and helper arguments, 6 actions, 2 exits, both RGB paths, actual codec, full-width counters/readiness");
}

'''
(p/'native.c').write_text(src)
subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-Iinclude',str(p/'native.c'),'-o',str(p/'native')],check=True)
subprocess.run([str(p/'native')],check=True)
