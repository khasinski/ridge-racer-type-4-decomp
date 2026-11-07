from pathlib import Path
from tempfile import TemporaryDirectory
import re,subprocess
s=Path('src/overlays/PAL/garage/func_80120B68.c').read_text();s=re.sub(r'^extern .*\n','',s,flags=re.M)
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
#include "rr/overlay662_save_dialog.h"
#include "rr/overlay662_modal.h"
#include "rr/overlay662_tool_action.h"
#include "rr/overlay662_tim.h"
#include "rr/menu_text_state.h"
#include "rr/tables.h"
static struct Assets {RrGarageTimBytes tim[2];u32 image[2][512],palette[2][8];} assets;
static struct World {u32 guard[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage working;RrMenuItemState5C input[2];RrUiRow24 rows[2][2];RrUiRow24 *row;RrBadgeRow1C badges[2];RrBadgeRow1C *badge;RrMenuTextState texts[2];RrMenuTextState *text;s32 accept,exit;s16 slot,scene,enable,state;u32 tail[4];} world,expected;
#define garage_logo_state world.garage
#define garage_logo_work world.working
#define PlayerControllers world.input
#define UiRows world.row
#define BadgeRows world.badge
#define DialogueState world.text
#define MenuConfirmPressed world.accept
#define garage_returned_from_logo_editor world.exit
#define MenuResultCode world.slot
#define ModeId world.scene
#define ModeGroup world.enable
#define ModeSubState world.state
'''+s+r'''
static struct Event {unsigned kind;s32 a;struct World before;} events[4];
static unsigned count,seen,serial,scenario,profile,coverage[5],choices[5],phases[12];static int reference_run;
static void service(unsigned kind,s32 a){struct World *w=reference_run?&expected:&world;unsigned order=serial++;
 if(reference_run){assert(count<4);events[count++]=(struct Event){kind,a,*w};}
 else{coverage[kind]++;assert(seen<count);struct Event *e=&events[seen++];assert(e->kind==kind&&e->a==a&&memcmp(w,&e->before,sizeof(*w))==0);}
 unsigned bank=(scenario+order+1)&1;w->row=world.rows[bank];w->badge=&world.badges[bank];w->text=&world.texts[bank];
 w->garage.flagsD1^=0xA5;w->garage.stateD2+=(u8)(profile+order+1);w->garage.fade00^=0x12345678;
 w->working.words[0]^=0x17;w->working.words[1]^=3;w->working.words[4]^=0x89ABCDEF;w->accept^=1;w->input[0].buttons0A^=0x5000;
 w->texts[bank].b4^=5;w->texts[bank].st^=3;w->badges[bank].half02^=7;w->badges[bank].state10^=1;w->rows[bank][0].byte14^=5;w->rows[bank][1].cells[0]^=0xD7;
}
void garage_navigate_to_option(s32 a){assert(a==0||a==1);service(0,a);}
s32 garage_cursor_outside_button(s32 a){assert(a==17);service(1,a);return profile&1;}
void garage_save_cursor_selection(void){service(2,0);}void PlaySfx(s32 a){assert(a==0||a==1);service(3,a);}
void garage_build_save_tim(void){struct World *w=reference_run?&expected:&world;unsigned bank=w->garage.active21;assert(bank<2);assert(w->garage.tim2C[bank]==&assets.tim[bank]&&w->garage.image38[bank]==assets.image[bank]&&w->garage.palette48[bank]==assets.palette[bank]);service(4,0);}
static RrUiRow24 *rows(struct World *w){return w->rows[w->row==world.rows[1]];}
static RrBadgeRow1C *badge(struct World *w){return &w->badges[w->badge==&world.badges[1]];}
static RrMenuTextState *text(struct World *w){return &w->texts[w->text==&world.texts[1]];}
static void reference(s32 mode){struct World *w=&expected;
 if(mode<0)return;
 if(mode>0){
  if(mode==1||mode==2){w->garage.selectorsB8[0]=w->garage.selectorsB8[3]=mode==1?1:0;service(0,0);}
  else if(mode==4){w->garage.optionIndices98.flat[5]=w->garage.optionIndices98.flat[2]==0?4:3;w->garage.selectorsB8[1]=2;w->garage.selectorsB8[2]=7;service(0,1);}
  else if(mode==8){w->garage.selectorsB8[1]=4;w->garage.selectorsB8[2]=9;service(0,1);}return;
 }
 if(w->working.words[0]&8){unsigned phase=rows(w)[0].byte14;if(phase<12)phases[phase]++;
  if(phase==2){if(badge(w)->state10!=3)return;w->garage.flagsD1|=4;if(text(w)->st!=4)return;
   if(w->input[0].type01!=18){
    if(w->garage.stateD2<2){
     if(w->input[0].buttons0A&0x1000){if(w->garage.stateD2!=0){w->garage.stateD2=0;service(3,0);}}
     else if(w->input[0].buttons0A&0x4000){if(w->garage.stateD2!=1){w->garage.stateD2=1;service(3,0);}}
    }
    if(w->accept){w->garage.stateD2=(u8)(w->garage.stateD2+2);service(3,1);}
   }
   unsigned choice=w->garage.stateD2;if(choice<5)choices[choice]++;
   if(choice<=3){rows(w)[0].cells[0]=choice==0?2:choice==2?1:0;rows(w)[1].cells[0]=choice==1?2:choice==3?1:0;
    if(choice==2)text(w)->b4=6;
    else if(choice==3){rows(w)[0].byte14=rows(w)[1].byte14=3;badge(w)->state10=4;w->garage.flagsD1&=251;}
   }else if(choice==4&&text(w)->st==4){garage_build_save_tim();rows(w)[0].byte14=rows(w)[1].byte14=7;w->garage.fade00=64;}
  }else if(phase==5||phase==6){if(badge(w)->state10==11){w->working.words[0]&=~9U;w->garage.flagsD1=0;}}
  else if(phase==9&&w->garage.fade00==0){w->slot=7;w->enable=1;w->scene=71;w->state=0;}return;
 }
 if(garage_cursor_outside_button(17)!=0||!(w->working.words[1]&1))return;
 if(w->working.words[0]&4)service(2,0);else{static const s32 saved[5]={17,2,3,8,2};for(unsigned i=0;i<5;i++)w->garage.saved64[i]=saved[i];w->garage.optionIndices98.flat[2]=w->garage.optionIndices98.flat[6]=0;}
 w->working.words[4]=17;w->working.words[0]|=8;service(3,1);w->working.words[0]|=1;
 text(w)->b4=5;text(w)->b5=-1;text(w)->st=0;w->garage.flagsD1=3;badge(w)->half02=1;badge(w)->state10=0;rows(w)[0].byte14=rows(w)[1].byte14=0;w->garage.stateD2=1;
}
int main(void){assert(sizeof(void*)==8);memset(&assets,0xB7,sizeof(assets));static const s32 modes[]={-1,(-2147483647-1),2147483647,1,2,4,8,3,0,16};
 for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<8;profile++){
  memset(&world,0xA6,sizeof(world));world.row=world.rows[0];world.badge=&world.badges[0];world.text=&world.texts[0];
  for(unsigned bank=0;bank<2;bank++){world.rows[bank][0].byte14=scenario%12;world.badges[bank].state10=profile&2?3:11;world.texts[bank].st=profile&1?4:-1;world.texts[bank].p=(u16*)world.rows[bank];}
  world.working.words[0]=scenario;world.working.words[1]=scenario>>4;world.input[0].type01=(scenario>>5)&1?18:0;world.input[0].buttons0A=scenario;world.accept=(scenario>>6)&1;
  world.garage.active21=profile&1;for(unsigned bank=0;bank<2;bank++){world.garage.tim2C[bank]=&assets.tim[bank];world.garage.image38[bank]=assets.image[bank];world.garage.palette48[bank]=assets.palette[bank];}world.garage.stateD2=scenario>>8;world.garage.fade00=scenario&16?0:64;world.garage.selectorsB8[1]=scenario&1;world.garage.optionIndices98.flat[5]=scenario%6;
  s32 mode=profile<4?0:modes[scenario%10];expected=world;count=seen=serial=0;reference_run=1;reference(mode);serial=0;reference_run=0;garage_save_dialog(mode);
  assert(count==seen&&memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.row>UINT32_MAX&&(uintptr_t)world.badge>UINT32_MAX&&(uintptr_t)world.text>UINT32_MAX);
 }
 for(unsigned i=0;i<5;i++)assert(coverage[i]&&choices[i]);assert(phases[5]&&phases[6]&&phases[9]);for(unsigned i=0;i<sizeof(assets);i++)assert(((u8*)&assets)[i]==0xB7);puts("524288 cases passed; full guarded owners and fresh callback roots across reset,navigation,entry and modal states");
}
'''
with TemporaryDirectory() as d:
 c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
 subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined','-fno-sanitize-recover=all','-Iinclude',str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
