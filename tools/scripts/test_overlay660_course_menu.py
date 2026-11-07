from overlay_units import read_c_source
"""Independent guarded native oracle for the overlay660 course menu."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re,subprocess
root=next(p for p in Path(__file__).resolve().parents if (p/'tools/scripts/cc.sh').is_file())
stage=root/'build/goal_50/parallel660'
public=root/'src/overlays/PAL/660/func_80114D38.c'
use_public=True
body=read_c_source(public);body=body[body.index('void ovl660_func_80114D38'):]
body=re.sub(r' __asm__\("\$\d+"\)','',body)
source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "controller.h"
#include "rr/tables.h"
_Static_assert(sizeof(RrUiRow24)==36,"UI stride");
_Static_assert(sizeof(RrMenuItemState5C)==92,"input stride");
_Static_assert(offsetof(RrUiRow24,cells)==12,"cells");
_Static_assert(offsetof(RrUiRow24,byte16)==22,"highlight");
typedef struct {
 u32 guard[4];s32 frame,tick,available,rowCount; s16 timer,course,selection,limit,style,mapped,scene,secondary,settings;
 u16 phase,enabled;RrMenuItemState5C input[2];RrUiRow24 rows[2][16];RrFrameOrderingPrefix frames[2];u8 modes[2][8],map[8];
 unsigned rowRoot,frameRoot,modeRoot;u32 tail[4];
} State;
static State actual,reference,*w;
#define ModeFrame w->frame
#define ScreenFrame w->tick
#define MenuFadeTimer w->timer
#define CourseMenuSelection w->course
#define MenuCursor w->selection
#define MenuItemCount w->limit
#define ReverseCourse w->style
#define CourseIndex w->mapped
#define D_800F3A1E w->secondary
#define ModeId w->scene
#define D_800FFC18 w->settings
#define D_800F4D8A w->phase
#define ModeGroup w->enabled
#define MenuConfirmPressed w->input[0].a
#define D_800F3A64 w->input[0].b
#define ImageRows w->modes[w->modeRoot]
#define UiRowCount w->rowCount
#define UiRows w->rows[w->rowRoot]
#define CurrentDrawBuf (&w->frames[w->frameRoot])
#define ovl660_available_course_count w->available
''' + body + r'''
typedef struct {unsigned kind;s32 args[12];State before;} Event;
static Event events[24];
static unsigned scenario,profile,serial,count,seen,coverage[16],states[8],choices[4],exits[3],readiness[2],courseChange[2];
static int referenceRun;
static s32 word(u32 a){s32 b;memcpy(&b,&a,4);return b;}
static s16 half(u16 a){s16 b;memcpy(&b,&a,2);return b;}
static void record(unsigned kind,const s32 *args){assert(kind<16);
 if(referenceRun){assert(count<24);Event *e=&events[count++];e->kind=kind;memcpy(e->args,args,48);e->before=*w;}
 else{coverage[kind]++;assert(seen<count);Event *e=&events[seen++];assert(e->kind==kind&&!memcmp(args,e->args,48)&&!memcmp(w,&e->before,sizeof(*w)));}
}
static void effect(unsigned kind){unsigned n=serial++;if(!(profile&1))return;
 u32 v=(scenario+17*kind+31*n)*0x9E3779B9U;
 w->frame=word(v);w->tick=word(~v);w->selection=(v>>3)%3;w->limit=3;
 if(kind!=0||(scenario&4096))w->phase=(u16)v;
 w->enabled=(u16)v;w->scene=half(v>>7);w->timer=half(scenario+31*n);w->style=half(v);
 w->available=1+(scenario+n)%16;w->rowCount=(scenario+n)%17;
 w->rowRoot=(scenario+n)&1;w->frameRoot=(scenario+n+1)&1;w->modeRoot=(scenario+n)&1;
 w->input[0].a=(scenario>>6)&1;w->input[0].b=(scenario>>7)&1;
 for(unsigned i=0;i<16;i++)for(unsigned j=0;j<2;j++)w->rows[j][i].byte14=(scenario/256+n+i+j)%8;
 for(unsigned j=0;j<2;j++)w->rows[j][3].byte02=(scenario+n+j)%9;
}
static void service(unsigned kind){const s32 args[12]={0};record(kind,args);effect(kind);}
s32 LoadTrackNodesStep(void){static const u32 returns[]={0,1,2,0xFFFFFFFFU,0x10001,0xFFFF0001,0x80000000,0x80000001};service(0);return profile<16?1:word(returns[scenario%8]);}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){const s32 args[12]={a,b,c};record(1,args);effect(1);}
void StepMenuValue(s32 a,s32 b,char *p,s32 d,s32 e){unsigned kind=p==(char*)&w->course?0:1;assert(p==(char*)&w->course||p==(char*)&w->selection);assert((uintptr_t)p>UINT32_MAX);const s32 args[12]={a,b,kind,d,e};record(2,args);effect(2);
 if(kind==0){s16 value=(scenario&32)?half((u16)w->course+1):w->course;memcpy(p,&value,2);}else{ s16 value=(scenario+serial)%3;memcpy(p,&value,2);}
}
void ResetScreenCounters(void){service(3);}
void DrawCourseMapPreview(s32 a,s32 b){const s32 args[12]={a,b};record(4,args);effect(4);}
s32 DecodeCourseIndex(s32 a){const s32 args[12]={a};record(5,args);w->style=a>=8;return w->map[a&7];}
void PlaySfx(s32 a){const s32 args[12]={a};record(6,args);effect(6);}
void SeqFadeVolume(s32 a,s32 b){const s32 args[12]={a,b};record(7,args);effect(7);}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){va_list ap;va_start(ap,j);s32 x=va_arg(ap,s32),y=va_arg(ap,s32);va_end(ap);const s32 args[12]={a,b,c,d,e,f,g,h,i,j,x,y};record(8,args);effect(8);}
void ovl660_func_801163C4(void){service(9);}
void AddScreenFadeTiles(u32 *p,s32 b,u32 c){unsigned k;for(k=0;k<2&&p!=w->frames[k].ordering70;k++){}assert(k<2&&(uintptr_t)p>UINT32_MAX);const s32 args[12]={k,b,(s32)c};record(10,args);effect(10);}
void SeqMuteVolume(void){service(11);}
void DrawCourseInfoPanel(s32 a,s32 b){const s32 args[12]={a,b};record(12,args);effect(12);}
void DrawUiRow(s32 a){const s32 args[12]={a};record(13,args);effect(13);}
void DrawMenuStepArrows(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g){const s32 args[12]={a,b,c,d,e,f,g};record(14,args);effect(14);}
void UpdateScreenFade(s32 a){const s32 args[12]={a};record(15,args);effect(15);}
static void closeRows(int count){for(int i=0;i<count;i++)if(w->rows[w->rowRoot][i].byte14==2)w->rows[w->rowRoot][i].byte14=3;}
static void closeLiveRows(void){for(int i=0;i<w->rowCount;i++)if(w->rows[w->rowRoot][i].byte14==2)w->rows[w->rowRoot][i].byte14=3;}
static void expected(void){w->frame=word((u32)w->frame+1);w->tick=word((u32)w->tick+1);int ready=LoadTrackNodesStep();readiness[ready==1]++;int course=1;
 if(ready==1){unsigned phase=w->phase;states[phase==0?0:phase==2?1:phase==4?2:phase==5?3:phase==3?4:phase==8?5:phase==10?6:7]++;
  if(phase==0){SetMenuFrameOptions(1,1,1);s16 old=w->course;StepMenuValue(3,2,(char*)&w->course,w->available,0);courseChange[old!=w->course]++;if(old!=w->course)ResetScreenCounters();else DrawCourseMapPreview(old,0);
   w->mapped=DecodeCourseIndex(w->course);w->rows[w->rowRoot][w->selection].cells[0]=0;StepMenuValue(0,2,(char*)&w->selection,w->limit,0);w->rows[w->rowRoot][w->selection].cells[0]=2;
   for(int i=0;i<w->rows[w->rowRoot][3].byte02;i++){if(i==(w->course&7)){w->rows[w->rowRoot][3].cells[i]=(u8)((u8)w->style+2);w->rows[w->rowRoot][3].byte16=(u8)i;}
    else w->rows[w->rowRoot][3].cells[i]=i>=w->available?1:(u8)w->style;}
   int back=1;if(w->input[0].a&&w->rows[w->rowRoot][0].byte14==2){back=0;PlaySfx(1);SetMenuFrameOptions(0,1,1);int selected=w->selection;
    if(selected==0){choices[0]++;w->rows[w->rowRoot][0].cells[0]=1;closeRows(4);w->phase=3;}
    else if(selected==1){choices[1]++;closeLiveRows();w->timer=64;w->phase=10;}
    else if(selected==2){choices[2]++;closeLiveRows();w->timer=64;w->phase=8;SeqFadeVolume(0,60);}
   }
   if(back&&w->input[0].b&&w->rows[w->rowRoot][0].byte14==2){choices[3]++;PlaySfx(2);SetMenuFrameOptions(0,1,1);closeLiveRows();w->timer=64;w->phase=8;}
  }else if(phase==2){if(w->input[0].a&&w->rows[w->rowRoot][0].byte14==5){w->phase=4;InitUiRow(3,120,220,68,32,0,0,1,0,0,0,0);InitUiRow(4,120,260,68,32,0,0,1,0,0,0,0);}
  }else if(phase==4||phase==5){if(phase==4&&w->input[0].a&&w->rows[w->rowRoot][3].byte14==2){w->phase=5;w->modes[w->modeRoot][1]=3;w->rows[w->rowRoot][3].byte14=3;w->rows[w->rowRoot][4].byte14=3;}
   if(w->rows[w->rowRoot][3].byte14==5){for(unsigned i=0;i<3;i++)w->rows[w->rowRoot][i].byte14=0;w->phase=0;}
  }else if(phase==3){course=0;if(w->rows[w->rowRoot][0].byte14==5){exits[0]++;ovl660_func_801163C4();w->enabled=1;w->scene=51;w->phase=0;}
  }else if(phase==8||phase==10){course=0;s16 timer=half((u16)w->timer-1);w->timer=timer;AddScreenFadeTiles(w->frames[w->frameRoot].ordering70,(64-(s32)timer)*4,69);
   if(w->timer<=0){exits[phase==8?1:2]++;if(phase==8){SeqMuteVolume();w->enabled=1;w->scene=3;}else{w->settings=1;w->secondary=306;w->enabled=1;w->scene=71;}w->phase=0;}
  }
 }
 if(course)DrawCourseInfoPanel(w->course,0);for(unsigned i=0;i<4;i++)DrawUiRow(i);
 if(w->rows[w->rowRoot][0].byte14==2)DrawMenuStepArrows(206,208,94,0,w->course,-1,word((u32)w->available+1));UpdateScreenFade(0x10026);
}
int main(void){assert(sizeof(void*)==8);u16 endian=1;assert(*(u8*)&endian==1);
 for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<18;profile++){
 memset(&actual,0xA6,sizeof(actual));w=&actual;static const u16 phases[]={0,2,4,5,3,8,10};w->phase=profile<14?phases[profile/2]:profile<16?(u16)scenario:0;
 w->frame=word(scenario*0x1234567U);w->tick=word(~(scenario*0x2345678U));w->timer=half(scenario);w->course=half(scenario);w->selection=scenario%3;w->limit=3;w->style=half(~scenario);
 w->available=1+scenario%16;w->rowCount=scenario%17;w->rowRoot=scenario&1;w->frameRoot=(scenario>>1)&1;w->modeRoot=(scenario>>2)&1;
 w->input[0].a=(scenario>>6)&1;w->input[0].b=(scenario>>7)&1;
 for(unsigned i=0;i<16;i++)for(unsigned j=0;j<2;j++)w->rows[j][i].byte14=(scenario/256+i+j)%8;
 for(unsigned j=0;j<2;j++)w->rows[j][3].byte02=(scenario+j)%9;for(unsigned i=0;i<8;i++)w->map[i]=(u8)(scenario+31*i);
 reference=actual;serial=count=seen=0;referenceRun=1;w=&reference;expected();serial=0;referenceRun=0;w=&actual;ovl660_func_80114D38();assert(count==seen&&!memcmp(&actual,&reference,sizeof(actual)));
 }
 for(unsigned i=0;i<16;i++)assert(coverage[i]);for(unsigned i=0;i<8;i++)assert(states[i]);for(unsigned i=0;i<4;i++)assert(choices[i]);for(unsigned i=0;i<3;i++)assert(exits[i]);assert(readiness[0]&&readiness[1]&&courseChange[0]&&courseChange[1]);
 puts("1179648 cases passed; complete phase/timer/course sweeps, four choices, three exits, full-word readiness, fresh roots/counts and exact pre-service state");
}
'''
if use_public:source=source.replace('#include \"controller.h\"','#include \"rr/overlay660_menu_controller.h\"')
with TemporaryDirectory() as d:
 c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
 subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined,bounds','-fno-sanitize-recover=all',*(['-I'+str(stage/'include'),'-I'+str(stage)] if not use_public else []),'-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
