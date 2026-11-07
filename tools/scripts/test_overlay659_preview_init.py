"""Guarded native oracle for the three-state overlay659 preview initializer."""
from overlay_units import read_c_source
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess

root = Path(__file__).resolve().parents[2]
body = read_c_source(root/'src/overlays/PAL/659/func_801159E8.c')
body = body[body.index('void ovl659_init_preview'):]
source = r'''
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
#include "rr/overlay659_preview_init.h"
_Static_assert(sizeof(RrRaceVehicle)==0x320,"vehicle layout");
_Static_assert(sizeof(RrMenuSettings2C)==0x2C,"settings layout");
_Static_assert(sizeof(RrUiRow24)==0x24,"UI layout");
_Static_assert(offsetof(RrUiRow24,cells)==12,"cell offset");
_Static_assert(offsetof(RrMenuSettings2C,phase17)==23,"selection offset");
_Static_assert(offsetof(RrRaceVehicle,pos20)==32,"position offset");
_Static_assert(offsetof(RrRaceVehicle,rot54)==84,"rotation offset");
static u8 resources[3][32];
typedef struct {
 u32 guard[4];s32 frame,tick;s16 selection,mode,scene;u16 phase,enable;
 RrMenuSettings2C settings;RrUiRow24 rows[2][7];RrRaceVehicle vehicles[2];
 s32 scratch[16];void *published;u8 *resource;unsigned root;u32 tail[4];
} State;
static State actual,reference,*w;
#define ModeFrame w->frame
#define ScreenFrame w->tick
#define MenuCursor w->selection
#define MenuItemCount w->mode
#define ModeId w->scene
#define D_800F4D8A w->phase
#define ModeGroup w->enable
#define SelectionState w->settings
#define UiRows (w->rows[w->root])
#define PlayerCars w->vehicles
#define LoadCursor w->published
#define LoadCursorBase w->resource
#undef RR_MENU_PREVIEW_SCRATCH_WORDS
#define RR_MENU_PREVIEW_SCRATCH_WORDS w->scratch
''' + body + r'''
typedef struct {unsigned kind;s32 args[10];State before;} Event;
static Event events[16];
static unsigned scenario,profile,serial,count,seen,coverage[10],completion[2],paths[4];
static int referenceRun;
static s32 word(u32 a){s32 b;memcpy(&b,&a,4);return b;}
static s8 byte(u8 a){s8 b;memcpy(&b,&a,1);return b;}
static s16 half(u16 a){s16 b;memcpy(&b,&a,2);return b;}
static void record(unsigned kind,const s32 *args){assert(kind<10);
 if(referenceRun){assert(count<16);Event *e=&events[count++];e->kind=kind;memcpy(e->args,args,40);e->before=*w;}
 else{coverage[kind]++;assert(seen<count);Event *e=&events[seen++];assert(e->kind==kind&&!memcmp(args,e->args,40)&&!memcmp(w,&e->before,sizeof(*w)));}
}
static void effect(unsigned kind){unsigned n=serial++;if(!(profile&1))return;
 u32 v=(scenario+17*kind+31*n)*0x9E3779B9U;
 w->frame=word(v);w->tick=word(~v);w->phase=(u16)v;w->selection=half(v>>7);w->mode=half(v);w->enable=(u16)(v>>3);w->scene=half(v>>16);
 w->root=(scenario+n)&1;w->resource=resources[(scenario+n)%3];w->published=resources[(scenario+n+1)%3];w->settings.phase17=byte(scenario+13*n);
 for(unsigned i=0;i<2;i++)for(unsigned j=0;j<3;j++)w->vehicles[i].pos20[j]=word(v+j+i*13);
 for(unsigned i=0;i<2;i++)w->vehicles[i].rot54=word(v+i);
 for(unsigned i=0;i<2;i++)for(unsigned j=0;j<8;j++)w->rows[i][3].cells[j]=(u8)(v+i+j);
 for(unsigned j=0;j<16;j++)w->scratch[j]=word(v+j);
}
static void service(unsigned kind){const s32 args[10]={0};record(kind,args);effect(kind);}
void SeqMuteVolume(void){service(0);}
void ResetScreenCounters(void){service(1);}
void ClearScreenOverlayState(void){service(2);}
void InitMenuArenas(void){service(3);}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){
 const s32 args[10]={a,b,c,d,e,f,g,h,i,j};record(4,args);effect(4);
}
void ResetCarState(RrRaceVehicle *p){assert(p==w->vehicles&&(uintptr_t)p>UINT32_MAX);service(5);}
s32 LoadTrackNodesStep(void){static const u32 returns[]={0,1,2,0xFFFFFFFFU,0x10001,0xFFFF0001,0x80000000,0x80000001};service(6);return word(returns[scenario%8]);}
void CdAudioInit(void){service(7);}
void CdAudioSelectTrack(s32 a,s32 b){const s32 args[10]={a,b};record(8,args);effect(8);}
void UpdateScreenFade(s32 a){const s32 args[10]={a};record(9,args);effect(9);}
static void expected(void){unsigned phase=w->phase;paths[phase<3?phase:3]++;
 if(phase==0){w->frame=0;w->tick=0;w->selection=0;w->mode=3;
  for(unsigned kind=0;kind<4;kind++)service(kind);
  w->published=w->resource;
  static const s32 rows[7][10]={
   {0,12,64,68,32,0,0,1,0,84},{1,12,104,68,32,0,0,1,1,12},
   {2,12,144,68,32,0,0,1,0,29},{3,12,368,208,32,0,0,8,4,0},
   {4,116,88,28,32,0,0,1,0,55},{5,116,128,28,32,0,0,1,0,56},
   {6,24,88,68,32,0,0,1,0,83}};
  for(unsigned i=0;i<7;i++){s32 args[10];memcpy(args,rows[i],40);if(i==3)args[9]=w->settings.phase17;record(4,args);effect(4);}
  memset(w->rows[w->root][3].cells,0,8);int selected=w->settings.phase17;
  if(selected>=0&&selected<8)w->rows[w->root][3].cells[selected]=2;
  w->scratch[6]=512;w->scratch[7]=0;w->scratch[8]=0;w->scratch[2]=0;w->scratch[3]=-416;w->scratch[4]=0;
  memset(w->vehicles[1].pos20,0,sizeof(w->vehicles[1].pos20));service(5);
  w->vehicles[0].pos20[1]=10;w->vehicles[0].pos20[2]=308;w->vehicles[0].rot54=2560;w->phase=1;
 }else if(phase==1){s32 result=LoadTrackNodesStep();completion[result==1]++;if(result==1)w->phase=2;
 }else if(phase==2){service(7);const s32 args[10]={0,2};record(8,args);effect(8);w->enable=1;w->scene=18;w->phase=0;}
 const s32 args[10]={0x1002E};record(9,args);effect(9);
}
int main(void){assert(sizeof(void*)==8);for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<8;profile++){
 memset(&actual,0xA6,sizeof(actual));w=&actual;w->phase=profile<6?profile/2:(u16)scenario;
 w->frame=word(scenario*0x1234567U);w->tick=word(~(scenario*0x2345678U));w->settings.phase17=byte(scenario);
 w->root=scenario&1;w->resource=resources[scenario%3];w->published=resources[(scenario+1)%3];
 reference=actual;serial=count=seen=0;referenceRun=1;w=&reference;expected();serial=0;referenceRun=0;w=&actual;ovl659_init_preview();
 assert(count==seen&&!memcmp(&actual,&reference,sizeof(actual)));
 }
 for(unsigned i=0;i<10;i++)assert(coverage[i]);for(unsigned i=0;i<4;i++)assert(paths[i]);assert(completion[0]&&completion[1]);
 puts("524288 cases passed; complete phase/selection sweeps, exact-word completion, fresh UI/resource roots and guarded pre-service states");
}
'''
with TemporaryDirectory() as d:
    c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
    subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
