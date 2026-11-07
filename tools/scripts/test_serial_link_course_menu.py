"""Independent guarded native oracle for overlay667 course/card menu."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess

root=Path(__file__).resolve().parents[2]
body=(root/'src/overlays/PAL/serial_link/func_80117FA8.c').read_text()
body=body[body.index('void serial_link_update_course_select'):]
mapper=(root/'tools/scripts/fixtures/views/func_80032E9C.c').read_text().replace('DecodeCourseIndex','actual_mapper')
source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/overlay667_course_menu.h"
_Static_assert(sizeof(RrUiRow24)==36,"UI stride");
_Static_assert(sizeof(RrMenuItemState5C)==92,"input stride");
_Static_assert(offsetof(RrUiRow24,cells)==12,"cells");
_Static_assert(offsetof(RrUiRow24,byte16)==22,"highlight");
static u8 resources[3][32];
typedef struct {
 u32 guard[4];s32 frame,tick,available,players,clear;
 s16 timer,course,selection,style,mapped,scene,split,card,waitFlag,loadStatus;
 u16 phase,enabled;RrMenuItemState5C input[2];RrUiRow24 rows[2][25];
 RrFrameOrderingPrefix frames[2];RrMenuArenas arenas;void *published;u8 map[8];
 unsigned rowRoot,frameRoot;u32 tail[4];
} State;
static State actual,reference,*w;
#define ModeFrame w->frame
#define ScreenFrame w->tick
#define MenuFadeTimer w->timer
#define CourseMenuSelection w->course
#define MenuCursor w->selection
#define ReverseCourse w->style
#define CourseIndex w->mapped
#define NumLaps w->players
#define ModeId w->scene
#define LinkRoleSlot w->split
#define LinkState w->card
#define CourseMenuPhase w->waitFlag
#define FinalCarRanks w->loadStatus
#define AvailableCourseCount w->available
#define MenuActivePlayerPlusOne w->clear
#define ModeSubState w->phase
#define ModeGroup w->enabled
#define UiRows w->rows[w->rowRoot]
#define PlayerControllers w->input
#define MenuConfirmPressed w->input[0].a
#define CurrentDrawBuf (&w->frames[w->frameRoot])
#define MenuArenas w->arenas
#define LoadCursor w->published
#define CourseIndexMap w->map
''' + body + mapper + r'''
typedef struct {unsigned kind;s32 args[7];State before;} Event;
static Event events[32];
static unsigned scenario,profile,serial,count,seen,coverage[19],states[6],choices[4],exits[2],blocked[2],readiness[2],courseChange[2],early[2],inputKinds[3];
static int referenceRun;
static s32 word(u32 a){s32 b;memcpy(&b,&a,4);return b;}
static s16 half(u16 a){s16 b;memcpy(&b,&a,2);return b;}
static const u32 words[]={0,1,2,0x10000,0x10001,0xFFFF0001U,0xFFFFFFFFU,0x80000001U};
static void record(unsigned kind,const s32 *args){assert(kind<19);
 if(referenceRun){assert(count<32);Event *e=&events[count++];e->kind=kind;memcpy(e->args,args,28);e->before=*w;}
 else{coverage[kind]++;assert(seen<count);Event *e=&events[seen++];assert(e->kind==kind&&!memcmp(args,e->args,28)&&!memcmp(w,&e->before,sizeof(*w)));}
}
static void effect(unsigned kind){unsigned n=serial++;if(!(profile&1))return;
 u32 v=(scenario+17*kind+31*n)*0x9E3779B9U;
 w->frame=word(v);w->tick=word(~v);w->course=half(v);w->selection=(v>>2)%3;w->players=word(v^0x12345678U);
 if(kind!=0&&kind!=2)w->phase=(u16)v;
 w->enabled=(u16)v;w->scene=half(v>>7);w->split=(v>>16)&1;w->timer=half(scenario+31*n);
 if(kind==7&&profile>=18&&(scenario&3)==0)w->timer=64;
 w->rowRoot=(scenario+n)&1;w->frameRoot=(scenario+n+1)&1;
 w->arenas.first=resources[(scenario+n)%3];w->arenas.second=resources[(scenario+n+1)%3];w->published=resources[(scenario+n+2)%3];
 w->available=kind==14&&(scenario&3)==0?0x7FFFFFFF:1+v%16;
 w->input[0].a=(scenario>>6)&1;w->input[0].b=(scenario>>7)&1;
 for(unsigned i=0;i<25;i++)for(unsigned j=0;j<2;j++)w->rows[j][i].byte14=(scenario/256+n+i+j)%8;
 for(unsigned j=0;j<2;j++)w->rows[j][21].byte02=(scenario+n+j)%9;
 for(unsigned i=0;i<8;i++)w->map[i]=(u8)(v+i*31);
}
static void service(unsigned kind){const s32 args[7]={0};record(kind,args);effect(kind);}
s32 ExitOnLinkLost(void){service(0);return profile==14||profile==15?word(words[scenario%8]):0;}
void SeqMuteVolume(void){service(1);}
s32 UpdateLinkHandshake(s32 a){const s32 args[7]={a};record(2,args);effect(2);return profile>=18?0:profile>=12&&profile<14?word(words[scenario%8]):1;}
s32 LoadTrackNodesStep(void){service(3);return profile==10||profile==11?word(words[scenario%8]):1;}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){const s32 args[7]={a,b,c};record(4,args);effect(4);}
void StepMenuValue(s32 a,s32 b,char *p,s32 d,s32 e){unsigned k;
 if(p==(char*)&w->course){k=0;assert(a==3&&b==2);}
 else if(p==(char*)&w->selection){k=1;assert(a==0&&b==2&&d==3);}
 else{assert(p==(char*)&w->players);k=2;assert(a==1&&b==4&&d==10&&e==2);}
 assert((uintptr_t)p>UINT32_MAX);inputKinds[k]++;const s32 args[7]={a,b,k,d,e};record(5,args);effect(5);
 if(k==2){s32 value=scenario%10;memcpy(p,&value,4);}
}
void ResetScreenCounters(void){service(6);}
void DrawCourseMapPreview(s32 a,s32 b){const s32 args[7]={a,b};record(7,args);effect(7);}
s32 DecodeCourseIndex(s32 a){const s32 args[7]={a};record(8,args);effect(8);return actual_mapper(a);}
void PlaySfx(s32 a){const s32 args[7]={a};record(9,args);effect(9);}
void SeqFadeVolume(s32 a,s32 b){const s32 args[7]={a,b};record(10,args);effect(10);}
void BeginLinkSync(void){service(11);}
void serial_link_init_car_select(s32 a){assert(a==0||a==2);const s32 args[7]={a};record(12,args);effect(12);}
void AddScreenFadeTiles(u32 *p,s32 b,u32 c){unsigned k;for(k=0;k<2&&p!=w->frames[k].ordering70;k++){}assert(k<2&&(uintptr_t)p>UINT32_MAX);const s32 args[7]={k,b,(s32)c};record(13,args);effect(13);}
void DrawUiRow(s32 a){const s32 args[7]={a};record(14,args);effect(14);}
void DrawCourseInfoPanel(s32 a,s32 b){const s32 args[7]={a,b};record(15,args);effect(15);}
void DrawMenuStepArrows(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g){const s32 args[7]={a,b,c,d,e,f,g};record(16,args);effect(16);}
s32 _sio_control(s32 a,s32 b,unsigned long c){assert(c<=UINT32_MAX);const s32 args[7]={a,b,(s32)c};record(17,args);effect(17);return word(scenario*0x1234567U);}
void UpdateScreenFade(s32 a){const s32 args[7]={a};record(18,args);effect(18);}
static void closeRows(unsigned n,u8 state){for(unsigned i=18;i<18+n;i++)if(w->rows[w->rowRoot][i].byte14==2)w->rows[w->rowRoot][i].byte14=state;}
static void expected(void){s32 quit=ExitOnLinkLost();early[quit!=0]++;if(quit){SeqMuteVolume();return;}
 s32 wait=UpdateLinkHandshake(2);w->frame=word((u32)w->frame+1);w->tick=word((u32)w->tick+1);
 int ready=1;if(w->phase!=3)ready=LoadTrackNodesStep()==1;readiness[ready]++;
 if(ready){unsigned phase=w->phase;states[phase==0?0:phase==3?1:phase==4?2:phase==8?3:4]++;
  if(phase==0){SetMenuFrameOptions(1,1,1);s16 old=w->course;StepMenuValue(3,2,(char*)&w->course,w->available,0);
   courseChange[old!=w->course]++;if(old!=w->course)ResetScreenCounters();else DrawCourseMapPreview(old,2);
   w->mapped=DecodeCourseIndex(w->course);w->rows[w->rowRoot][18+w->selection].cells[0]=0;StepMenuValue(0,2,(char*)&w->selection,3,0);w->rows[w->rowRoot][18+w->selection].cells[0]=2;
   unsigned cells=w->rows[w->rowRoot][21].byte02;for(unsigned i=0;i<cells;i++){if((s32)i==w->course){w->rows[w->rowRoot][21].cells[i]=2;w->rows[w->rowRoot][21].byte16=i;}else w->rows[w->rowRoot][21].cells[i]=0;}
   int state=w->rows[w->rowRoot][18].byte14;if(state==2){int back=0;if(w->input[0].a){PlaySfx(1);w->rows[w->rowRoot][18+w->selection].cells[0]=1;
     if(w->selection==0){choices[0]++;closeRows(4,7);ResetScreenCounters();w->timer=64;w->published=w->arenas.first;SeqFadeVolume(0,60);BeginLinkSync();w->phase=3;SetMenuFrameOptions(0,1,1);}
     else if(w->selection==1){choices[1]++;w->rows[w->rowRoot][23].byte14=0;w->phase=4;}else if(w->selection==2){choices[2]++;back=1;}
    }else if(w->input[0].b){choices[3]++;PlaySfx(2);back=1;}
    if(back){SetMenuFrameOptions(0,1,1);closeRows(5,3);w->phase=8;}}
  }else if(phase==3){DrawCourseMapPreview(w->course,2);int blockedNow=wait==0&&w->timer==64;blocked[blockedNow]++;
   if(blockedNow)DrawUiRow(24);else{w->waitFlag=1;if(w->timer>0)w->timer=half((u16)w->timer-1);AddScreenFadeTiles(w->frames[w->frameRoot].ordering70,(64-(s32)w->timer)*4,69);
    if(w->timer<=0){exits[0]++;w->loadStatus=-1;SeqMuteVolume();w->enabled=1;w->scene=41;w->phase=0;w->card=0;_sio_control(3,0,0);}}
  }else if(phase==4){int state=w->rows[w->rowRoot][23].byte14;
   if(state==2){SetMenuFrameOptions(1,1,0);w->rows[w->rowRoot][23].cells[0]=2;StepMenuValue(1,4,(char*)&w->players,10,2);
    if(w->input[0].a){PlaySfx(1);w->rows[w->rowRoot][23].cells[0]=1;w->rows[w->rowRoot][23].byte14=3;}}
   else if(state==5)w->phase=0;DrawCourseMapPreview(w->course,2);
  }else if(phase==8){if(w->rows[w->rowRoot][18].byte14==5){exits[1]++;serial_link_init_car_select((1-(s32)w->split)*2);w->enabled=1;w->scene=60;w->phase=0;}}
 }
 DrawCourseInfoPanel(w->course,2);for(unsigned i=18;i<22;i++)DrawUiRow(i);
 if(w->rows[w->rowRoot][18].byte14==2&&w->phase!=4)DrawMenuStepArrows(206,208,94,0,w->course,-1,word((u32)w->available+1));
 if(w->phase==4)DrawUiRow(23);w->clear=0;UpdateScreenFade(0x10026);
}
int main(void){assert(sizeof(void*)==8);u16 endian=1;assert(*(u8*)&endian==1);
 for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<20;profile++){
 memset(&actual,0xA6,sizeof(actual));w=&actual;static const u16 phases[]={0,3,4,8};w->phase=profile<8?phases[profile/2]:profile<10?(u16)scenario:profile>=18?3:0;
 static const u32 extremes[]={0,0xFFFFFFFFU,0x7FFFFFFF,0x80000000U};w->frame=word(scenario<4?extremes[scenario]:scenario*0x1234567U);w->tick=word(scenario<4?extremes[3-scenario]:~(scenario*0x2345678U));
 w->timer=half(profile>=18&&(scenario&3)==0?64:scenario);w->course=half(scenario);w->selection=scenario%3;w->split=scenario&1;w->available=1+scenario%16;w->players=word(scenario*0x3456789U);
 w->rowRoot=scenario&1;w->frameRoot=(scenario>>1)&1;w->arenas.first=resources[scenario%3];w->arenas.second=resources[(scenario+1)%3];w->published=resources[(scenario+2)%3];
 w->input[0].a=(scenario>>6)&1;w->input[0].b=(scenario>>7)&1;
 for(unsigned i=0;i<25;i++)for(unsigned j=0;j<2;j++)w->rows[j][i].byte14=(scenario/256+i+j)%8;
 for(unsigned j=0;j<2;j++)w->rows[j][21].byte02=(scenario+j)%9;for(unsigned i=0;i<8;i++)w->map[i]=(u8)(scenario+31*i);
 reference=actual;serial=count=seen=0;referenceRun=1;w=&reference;expected();serial=0;referenceRun=0;w=&actual;serial_link_update_course_select();assert(count==seen&&!memcmp(&actual,&reference,sizeof(actual)));
 }
 for(unsigned i=0;i<19;i++)assert(coverage[i]);for(unsigned i=0;i<5;i++)assert(states[i]);for(unsigned i=0;i<4;i++)assert(choices[i]);for(unsigned i=0;i<3;i++)assert(inputKinds[i]);
 assert(exits[0]&&exits[1]&&blocked[0]&&blocked[1]&&readiness[0]&&readiness[1]&&courseChange[0]&&courseChange[1]&&early[0]&&early[1]);
 puts("1310720 cases passed; full phase/timer/course sweeps, all choices, blocking/early exits, actual mapper, full-word results and guarded pre-service state");
}
'''
with TemporaryDirectory() as d:
    c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
    subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
