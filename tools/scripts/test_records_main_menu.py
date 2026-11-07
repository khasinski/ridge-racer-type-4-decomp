"""Guarded native state and service oracle for overlay663 main-menu handling."""
from overlay_units import read_c_source
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
body=read_c_source(Path('src/overlays/PAL/records/func_80114B98.c'));body=body[body.index('void records_update_menu'):]
source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/overlay663_menu_handler.h"
#include "rr/tables.h"
void SeqFadeVolume(s32,s32);void SeqMuteVolume(void);void records_init_preview(void);
static RrFrameStorage contexts[2];static u8 buffers[3][32];
static struct State{u32 guard[4];s32 frame,tick,mode,available,table[9];s16 menu,timer,scene;u16 state,enable;
 RrUiRow24 rows[2][4];RrMenuItemState5C input[2];RrFrameStorage *context;void *published,*buffer;unsigned root;u32 tail[4];}actual,reference,*w;
#define ModeFrame w->frame
#define ScreenFrame w->tick
#define ModeSubState w->state
#define ModeGroup w->enable
#define ModeId w->scene
#define MenuCursor w->menu
#define MenuFadeTimer w->timer
#define UiRows (w->rows[w->root])
#define PlayerControllers w->input
#define CurrentDrawBuf w->context
#define LoadCursor w->published
#define MenuArenaBase w->buffer
#define records_record_kind w->mode
#define records_available_course_count w->available
#define records_available_courses w->table
'''+body+r'''
static struct Event{unsigned kind;s32 a,b,c;struct State before;} events[24];
static unsigned scenario,profile,serial,count,seen,coverage[12],choices[5],closing[2],exits[3],completion[2];static int referenceRun;
static s16 half(u16 u){s16 v;memcpy(&v,&u,2);return v;}static s32 word(u32 u){s32 v;memcpy(&v,&u,4);return v;}
static void record(unsigned kind,s32 a,s32 b,s32 c){assert(kind<12);if(referenceRun){assert(count<24);events[count++]=(struct Event){kind,a,b,c,*w};}
 else{coverage[kind]++;assert(seen<count);struct Event *e=&events[seen++];assert(e->kind==kind&&e->a==a&&e->b==b&&e->c==c&&!memcmp(w,&e->before,sizeof(*w)));}}
static void effect(unsigned kind){unsigned n=serial++;if(!(profile&1))return;u32 v=(scenario+31*n+17*kind)*0x9E3779B9U;
 w->frame=word((u32)w->frame+v);w->tick=word((u32)w->tick-v);w->mode=word(v);w->available=(s32)((scenario/16+n)%11)-1;w->menu=(scenario/4+n)%4;
 w->timer=half(scenario+31*n);w->state=(u16)v;w->root=(scenario+n)&1;w->context=&contexts[(scenario+n)&1];w->buffer=buffers[(scenario+n)%3];w->published=buffers[(scenario+n+1)%3];
 w->input[0].a=(scenario>>6)&1;w->input[0].b=(scenario>>7)&1;
 for(unsigned j=0;j<2;j++)for(unsigned i=0;i<4;i++)w->rows[j][i].byte14=((scenario/256+n+i+j)&1)?2:5;
 for(unsigned i=0;i<9;i++)w->table[i]=word(v+i*0x1234567U);
}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){record(0,a,b,c);effect(0);}
void StepMenuValue(s32 a,s32 b,char *p,s32 d,s32 e){assert(p==(char*)&w->menu&&(uintptr_t)p>UINT32_MAX&&d==4&&e==0);record(1,a,b,d);effect(1);w->menu=(scenario/2+serial)%4;}
void PlaySfx(s32 a){record(2,a,0,0);effect(2);}
void ResetScreenCounters(void){record(3,0,0,0);effect(3);}
void SeqFadeVolume(s32 a,s32 b){record(4,a,b,0);effect(4);}
void AddScreenFadeTiles(u32 *p,s32 a,u32 b){unsigned k;for(k=0;k<2&&p!=contexts[k].ordering.ordering70;k++){}assert(k<2&&(uintptr_t)p>UINT32_MAX);record(5,k,a,b);effect(5);}
s32 LoadCoursePreviewStep(s32 a){record(6,a,0,0);effect(6);return ((scenario>>5)&1)?1:word(scenario*0x1234567U);}
void records_init_preview(void){record(7,0,0,0);effect(7);}
void records_init_selection(s32 a){record(8,a,0,0);effect(8);}
void SeqMuteVolume(void){record(9,0,0,0);effect(9);}
void DrawUiRow(s32 a){record(10,a,0,0);effect(10);}
void UpdateScreenFade(s32 a){record(11,a,0,0);effect(11);}
static void expected(void){unsigned phase=w->state;w->frame=word((u32)w->frame+1);w->tick=word((u32)w->tick+1);
 if(phase==0&&w->rows[w->root][0].byte14==2){SetMenuFrameOptions(1,1,1);w->rows[w->root][w->menu].cells[0]=0;StepMenuValue(0,2,(char*)&w->menu,4,0);
  w->rows[w->root][w->menu].cells[0]=(w->available==0&&w->menu==0)?4:2;int close=0;
  if(w->input[0].a){w->rows[w->root][w->menu].cells[0]=1;int selected=w->menu;
   if(selected==0){if(w->available>0){choices[0]++;PlaySfx(1);close=2;ResetScreenCounters();w->state=3;w->published=w->buffer;}else{choices[1]++;PlaySfx(4);}}
   else if(selected==1||selected==2){choices[selected+1]++;PlaySfx(1);close=2;w->mode=selected;w->state=selected+3;}
   else if(selected==3){choices[4]++;close=1;PlaySfx(1);w->state=8;}
  }else if(w->input[0].b){choices[4]++;close=1;PlaySfx(2);w->state=8;}
  if(close){closing[close-1]++;SetMenuFrameOptions(0,1,1);w->timer=64;
   for(unsigned i=0;i<4;i++)if(w->rows[w->root][i].byte14==2){w->rows[w->root][i].byte14=close==2?7:3;if(close==1)SeqFadeVolume(0,half((u16)w->timer-4));}
  }
 }else if(phase==3){if(w->timer>0)w->timer=half((u16)w->timer-1);AddScreenFadeTiles(w->context->ordering.ordering70,(64-(s32)w->timer)*4,69);
  if(w->timer<=0){s32 v=LoadCoursePreviewStep(w->table[0]);completion[v==1]++;if(v==1){exits[0]++;records_init_preview();w->enable=1;w->scene=76;w->state=0;return;}}
 }else if(phase==4||phase==5||phase==8){s16 timer=half((u16)w->timer-1);w->timer=timer;AddScreenFadeTiles(w->context->ordering.ordering70,(64-(s32)timer)*4,69);
  if(w->timer<=0){if(phase==8){exits[2]++;SeqMuteVolume();w->enable=1;w->scene=3;w->state=0;}
   else{exits[1]++;records_init_selection(w->mode);w->enable=1;w->scene=75;w->state=0;}}
 }
 for(unsigned i=0;i<4;i++)DrawUiRow(i);UpdateScreenFade(0x10057);
}
int main(void){assert(sizeof(void*)==8);for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<12;profile++){
 memset(&actual,0xA6,sizeof(actual));w=&actual;static const u16 phases[]={0,3,4,5,8};w->state=profile<10?phases[profile/2]:(u16)scenario;
 w->frame=word(scenario*0x1234567U);w->tick=word(~(scenario*0x2345678U));w->timer=half(scenario);w->menu=scenario%4;w->mode=word(scenario*0x3456789U);w->available=(s32)((scenario>>4)%11)-1;
 w->root=scenario&1;w->context=&contexts[(scenario>>1)&1];w->buffer=buffers[scenario%3];w->published=buffers[(scenario+1)%3];w->input[0].a=(scenario>>6)&1;w->input[0].b=(scenario>>7)&1;
 for(unsigned j=0;j<2;j++)for(unsigned i=0;i<4;i++)w->rows[j][i].byte14=((scenario/256+i+j)&1)?2:5;for(unsigned i=0;i<9;i++)w->table[i]=word(scenario*0x456789AU+i);
 reference=actual;count=seen=serial=0;referenceRun=1;w=&reference;expected();serial=0;referenceRun=0;w=&actual;records_update_menu();assert(count==seen&&!memcmp(&actual,&reference,sizeof(actual)));
 }
 for(unsigned i=0;i<12;i++)assert(coverage[i]);for(unsigned i=0;i<5;i++)assert(choices[i]);assert(closing[0]&&closing[1]&&completion[0]&&completion[1]);for(unsigned i=0;i<3;i++)assert(exits[i]);
 puts("786432 cases passed; all menu choices,both closing modes,three exits and fresh UI/context roots");
}
'''
with TemporaryDirectory() as d:
 c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
 subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-Iinclude',str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
