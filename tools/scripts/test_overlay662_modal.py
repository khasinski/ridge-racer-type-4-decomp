from pathlib import Path
from tempfile import TemporaryDirectory
import re,subprocess
s=Path('src/overlays/PAL/662/func_8011E824.c').read_text();s=re.sub(r'^extern .*\n','',s,flags=re.M)
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
#include "rr/overlay662_modal.h"
#include "rr/overlay662_tool_action.h"
#include "rr/menu_text_state.h"
#include "rr/tables.h"
static struct World {u32 guard[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage working;RrMenuItemState5C input[2];RrUiRow24 rows[2][2];RrUiRow24 *row;RrBadgeRow1C badges[2];RrBadgeRow1C *badge;RrMenuTextState texts[2];RrMenuTextState *text;s32 accept,exit;s16 scene,enable,state;u32 tail[4];} world,expected;
#define ovl662_D_801276A8 world.garage
#define ovl662_D_80127668 world.working
#define PlayerControllers world.input
#define UiRows world.row
#define BadgeRows world.badge
#define DialogueState world.text
#define MenuConfirmPressed world.accept
#define ovl662_D_801272E8 world.exit
#define ModeId world.scene
#define ModeGroup world.enable
#define D_800F4D8A world.state
'''+s+r'''
static struct Event {unsigned kind;s32 a;struct World before;} events[4];
static unsigned count,seen,serial,scenario,profile,coverage[4];static int reference_run;
static void service(unsigned kind,s32 a){struct World *w=reference_run?&expected:&world;unsigned order=serial++;
 if(reference_run){assert(count<4);events[count++]=(struct Event){kind,a,*w};}
 else{coverage[kind]++;assert(seen<count);struct Event *e=&events[seen++];assert(e->kind==kind&&e->a==a&&memcmp(w,&e->before,sizeof(*w))==0);}
 unsigned bank=(scenario+order+1)&1;w->row=world.rows[bank];w->badge=&world.badges[bank];w->text=&world.texts[bank];
 w->garage.flagsD1^=0xA5;w->garage.stateD2+=(u8)(profile+order+1);w->garage.fade00^=0x12345678;
 w->working.words[0]^=0x17;w->working.words[1]^=3;w->working.words[4]^=0x89ABCDEF;w->accept^=1;w->input[0].buttons0A^=0x5000;
 w->texts[bank].b4^=5;w->texts[bank].st^=3;w->badges[bank].half02^=7;w->badges[bank].state10^=1;w->rows[bank][0].byte14^=5;w->rows[bank][1].cells[0]^=0xD7;
}
void ovl662_func_8011E5C0(s32 a){assert(a==0||a==1);service(0,a);}
s32 ovl662_func_8011E734(s32 a){assert(a==0);service(1,a);return profile&1;}
void ovl662_func_8011E694(void){service(2,0);}void PlaySfx(s32 a){assert(a==0||a==1);service(3,a);}
static RrUiRow24 *rows(struct World *w){return w->rows[w->row==world.rows[1]];}
static RrBadgeRow1C *badge(struct World *w){return &w->badges[w->badge==&world.badges[1]];}
static RrMenuTextState *text(struct World *w){return &w->texts[w->text==&world.texts[1]];}
static void reference(s32 mode){struct World *w=&expected;
 if(mode<0){w->garage.optionIndices98.flat[0]=0;return;}
 if(mode>0){
  if(mode==1){if(w->garage.selectorsB8[1]==0){w->garage.optionIndices98.flat[2]=1;w->garage.selectorsB8[0]=2;w->garage.selectorsB8[1]=1;w->garage.selectorsB8[3]=5;}
   else{w->garage.optionIndices98.flat[2]=w->garage.optionIndices98.flat[5]==4?0:1;w->garage.selectorsB8[0]=w->garage.selectorsB8[1]=2;w->garage.selectorsB8[3]=8;}service(0,0);}
  else if(mode==2){w->garage.optionIndices98.flat[0]=1;w->garage.selectorsB8[3]=1;service(0,0);}
  else if(mode==4||mode==8){w->garage.selectorsB8[1]=w->garage.selectorsB8[2]=mode==4?4:3;service(0,1);}return;
 }
 if(w->working.words[0]&8){unsigned phase=rows(w)[0].byte14;
  if(phase==2){if(badge(w)->state10!=3)return;w->garage.flagsD1|=4;if(text(w)->st!=4)return;
   if(w->input[0].type01!=18){
    if(w->input[0].buttons0A&0x1000){if(w->garage.stateD2!=0){w->garage.stateD2=0;service(3,0);}}
    else if(w->input[0].buttons0A&0x4000){if(w->garage.stateD2!=1){w->garage.stateD2=1;service(3,0);}}
    if(w->accept){w->garage.stateD2=(u8)(w->garage.stateD2+2);service(3,1);}
   }
   unsigned choice=w->garage.stateD2;
   if(choice<=3){rows(w)[0].cells[0]=choice==0?2:choice==2?1:0;rows(w)[1].cells[0]=choice==1?2:choice==3?1:0;
    if(choice==2){rows(w)[0].byte14=rows(w)[1].byte14=7;w->garage.fade00=64;}
    else if(choice==3){rows(w)[0].byte14=rows(w)[1].byte14=3;badge(w)->state10=4;w->garage.flagsD1&=251;}
   }
  }else if(phase==5||phase==6){if(badge(w)->state10==11){w->working.words[0]&=~9U;w->garage.flagsD1=0;}}
  else if(phase==9&&w->garage.fade00==0){w->exit=1;w->enable=1;w->scene=69;w->state=0;}return;
 }
 if(ovl662_func_8011E734(0)!=0||!(w->working.words[1]&1))return;
 if(w->working.words[0]&4)service(2,0);else{w->garage.optionIndices98.flat[0]=w->garage.optionIndices98.flat[3]=0;for(unsigned i=0;i<5;i++)w->garage.saved64[i]=0;}
 w->working.words[4]=0;w->working.words[0]|=8;service(3,1);w->working.words[0]|=1;
 text(w)->b4=3;text(w)->b5=-1;text(w)->st=0;w->garage.flagsD1=3;badge(w)->half02=1;badge(w)->state10=0;rows(w)[0].byte14=rows(w)[1].byte14=0;w->garage.stateD2=1;
}
int main(void){assert(sizeof(void*)==8);static const s32 modes[]={-1,(-2147483647-1),2147483647,1,2,4,8,3,0,16};
 for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<8;profile++){
  memset(&world,0xA6,sizeof(world));world.row=world.rows[0];world.badge=&world.badges[0];world.text=&world.texts[0];
  for(unsigned bank=0;bank<2;bank++){world.rows[bank][0].byte14=scenario%12;world.badges[bank].state10=profile&2?3:11;world.texts[bank].st=profile&1?4:-1;world.texts[bank].p=(u16*)world.rows[bank];}
  world.working.words[0]=scenario;world.working.words[1]=scenario>>4;world.input[0].type01=scenario&1?18:0;world.input[0].buttons0A=scenario;world.accept=(scenario>>3)&1;
  world.garage.stateD2=scenario>>8;world.garage.fade00=scenario&16?0:64;world.garage.selectorsB8[1]=scenario&1;world.garage.optionIndices98.flat[5]=scenario%6;
  s32 mode=profile<4?0:modes[scenario%10];expected=world;count=seen=serial=0;reference_run=1;reference(mode);serial=0;reference_run=0;ovl662_func_8011E824(mode);
  assert(count==seen&&memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.row>UINT32_MAX&&(uintptr_t)world.badge>UINT32_MAX&&(uintptr_t)world.text>UINT32_MAX);
 }
 for(unsigned i=0;i<4;i++)assert(coverage[i]);puts("524288 cases passed; full guarded owners and fresh callback roots across reset,navigation,entry and modal states");
}
'''
with TemporaryDirectory() as d:
 c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
 subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined','-fno-sanitize-recover=all','-Iinclude',str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
