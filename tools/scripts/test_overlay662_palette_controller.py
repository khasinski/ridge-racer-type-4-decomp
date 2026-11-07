"""Native guarded state/service oracle for the palette controller; no hardware claim."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
body=Path('src/overlays/PAL/662/func_80119E00.c').read_text()
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
#include "rr/overlay662_palette_controller.h"
static struct State {
 u32 guard[4];s32 frame,tick,mode; s16 scene,state,selection,count,current,key,phase,fade,total;u16 enable;
 RrMenuItemState5C input[2];RrUiRow24 rows[2][4];RrBadgeRow1C badges[2][2];
 RrMenuGridPoint grid[4];u8 map[25];void *palette0,*palette1;unsigned root,badgeRoot;u32 tail[4];
} actual,reference,*w;
static u8 banks[3][25*0x840];
#define ModeFrame w->frame
#define ScreenFrame w->tick
#define D_800F4D8A w->state
#define ModeId w->scene
#define ModeGroup w->enable
#define MenuCursor w->selection
#define MenuItemCount w->count
#define PlayerControllers w->input
#define UiRows (w->rows[w->root])
#define BadgeRows (w->badges[w->badgeRoot])
#define PlayerCourseChoices w->grid
#define D_800F3B20 w->palette0
#define CarClutData w->palette1
#define ovl662_D_801272A0 w->map
#define ovl662_D_801272BE w->current
#define ovl662_D_801272D4 w->phase
#define ovl662_D_801272E2 w->key
#define ovl662_D_801272E8 w->mode
#define ovl662_D_80127398 w->fade
#define ovl662_D_8012739A w->total
'''+body+r'''
static struct Event {unsigned kind;s32 a[7];struct State before;} events[24];
static unsigned scenario,profile,serial,count,seen,coverage[11],action[4],exitRoute[2],mapped[26],readyOut[2];
static int referenceRun;
static s16 half(u16 x){s16 y;memcpy(&y,&x,2);return y;}
static s32 word(u32 x){s32 y;memcpy(&y,&x,4);return y;}
static void record(unsigned kind,s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g){
 s32 args[7]={a,b,c,d,e,f,g};assert(kind<11);
 if(referenceRun){assert(count<24);struct Event *v=&events[count++];v->kind=kind;memcpy(v->a,args,sizeof(args));v->before=*w;}
 else{coverage[kind]++;assert(seen<count);struct Event *v=&events[seen++];assert(v->kind==kind&&!memcmp(v->a,args,sizeof(args))&&!memcmp(&v->before,w,sizeof(*w)));}
}
static void change(unsigned kind){unsigned n=serial++;if(!(profile&1))return;
 u32 v=(scenario+17*n+11*kind)*0x9E3779B9U;
 w->frame=word((u32)w->frame+v);w->tick=word((u32)w->tick-v);w->root=(scenario+n)&1;w->badgeRoot=(scenario/2+n)&1;
 w->state=half(v);w->current=(scenario+n)%25;w->selection=(scenario/4+n)%3;w->count=(s32)((scenario/16+n)%4)-1;
 w->key=half(scenario+23*n);w->phase=half(scenario+3*n);w->mode=(scenario/32+n)%3-1;w->fade=half(scenario+13*n);w->total=half(~scenario);
 w->palette0=banks[(scenario+n)%3];w->palette1=banks[(scenario+n+1)%3];
 w->input[0].a=(scenario>>7)&1;w->input[0].b=(scenario>>8)&1;
 for(unsigned j=0;j<2;j++)for(unsigned i=0;i<4;i++){w->rows[j][i].cells[0]=(u8)v;w->rows[j][i].half20=half(v+i);w->rows[j][i].byte14=(u8)((scenario/512+n+i+j)%7);}
 for(unsigned j=0;j<2;j++)w->badges[j][0].state10=(s16)((scenario/4096+n+j)%12);
 for(unsigned i=0;i<25;i++)w->map[i]=(scenario+n+i)%26;
 // Keep independent gate decisions reachable after navigation services.
 if(kind==0||kind==1||kind==2){w->rows[w->root][0].byte14=((scenario>>9)&1)?2:5;w->badges[w->badgeRoot][0].state10=((scenario>>10)&1)?3:11;}
}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){record(0,a,b,c,0,0,0,0);change(0);}
void StepMenuValue(s32 a,s32 b,char *p,s32 d,s32 e){int target;
 if(p==(char*)&w->current)target=0;else{assert(p==(char*)&w->selection);target=1;}
 assert((uintptr_t)p>UINT32_MAX);record(target?2:1,a,b,target,d,e,0,0);change(target?2:1);
 // Navigation can select a different valid item, independently of later guards.
 if(target)w->selection=(scenario/2+serial)%3;else w->current=(scenario+serial)%25;
}
void UploadCarClut(void *p,s32 a,s32 b){unsigned bank;assert((uintptr_t)p>UINT32_MAX);
 for(bank=0;bank<3&&p!=banks[bank];bank++){}assert(bank<3&&a>=0&&a<25);
 if(!referenceRun){assert(w->key>=0&&w->key<=25);mapped[w->key]++;}
 record(3,bank,a,b,0,0,0,0);change(3);
}
void PlaySfx(s32 a){assert(a==1||a==2||a==4);record(4,a,0,0,0,0,0,0);change(4);}
s32 UpdateRect16Row(s32 a){record(5,a,0,0,0,0,0,0);change(5);return (scenario&1)?1:word(scenario*0x1020304U);}
void ovl662_func_80119898(s32 a){record(6,a,0,0,0,0,0,0);change(6);}
void DrawMenuStepArrows(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g){record(7,a,b,c,d,e,f,g);change(7);}
void DrawUiRow(s32 a){assert(a>=0&&a<4);record(8,a,0,0,0,0,0,0);change(8);}
void UpdateBadgeRowAnim(s32 a){record(9,a,0,0,0,0,0,0);change(9);}
void UpdateScreenFade(s32 a){record(10,a,0,0,0,0,0,0);change(10);}
void ovl662_func_80118ED8(void){exitRoute[0]++;record(10,-1,0,0,0,0,0,0);change(10);}
static void closeRows(void){for(unsigned i=0;i<2;i++)if(w->rows[w->root][i].byte14==2)w->rows[w->root][i].byte14=3;}
static void expected(void){unsigned state=(u16)w->state;int complete=0;
 w->frame=word((u32)w->frame+1);w->tick=word((u32)w->tick+1);
 if(state==0){SetMenuFrameOptions(1,1,w->mode==0);s16 prior=w->current;
  if(w->fade>=17)StepMenuValue(1,2,(char*)&w->current,25,0);
  if(prior!=w->current){int key=(s8)w->map[w->current];w->key=key;void *p=key?w->palette1:w->palette0;UploadCarClut(p,key?key-1:0,0);w->fade=0;}
  w->rows[w->root][w->selection].cells[0]=0;StepMenuValue(0,2,(char*)&w->selection,w->count,0);
  int highlight=2;if(w->key>=25){w->rows[w->root][0].half20=1;if(w->selection==0)highlight=4;}else w->rows[w->root][0].half20=0;
  w->rows[w->root][w->selection].cells[0]=highlight;
  int rowPhase=w->rows[w->root][0].byte14,badgePhase=w->badges[w->badgeRoot][0].state10;
  if(rowPhase==2&&badgePhase==3){
   if(w->input[0].a){w->rows[w->root][w->selection].cells[0]=1;
    if(w->selection==0){if(w->key<25){action[0]++;PlaySfx(1);w->phase=rowPhase;w->fade=0;w->state=badgePhase;closeRows();w->badges[w->badgeRoot][0].state10=4;}else{action[1]++;PlaySfx(4);}}
    else if(w->selection==1){action[2]++;PlaySfx(1);w->fade=0;w->phase=rowPhase;w->badges[w->badgeRoot][0].state10=4;closeRows();w->state=4;}
   }else if(w->input[0].b&&w->mode==0){action[3]++;PlaySfx(2);w->phase=rowPhase;w->fade=0;w->badges[w->badgeRoot][0].state10=4;closeRows();w->state=4;}
  }
 }else if(state==3||state==4){SetMenuFrameOptions(0,1,w->mode==0);
  if(w->rows[w->root][0].byte14==5&&w->badges[w->badgeRoot][0].state10==11){
   if(state==3)w->grid[0].second=half((u16)w->key+1);
   if(w->mode==0)ovl662_func_80118ED8();else{exitRoute[1]++;w->enable=3;w->scene=1;w->state=0;}
   complete=1;
  }
 }
 if(!complete){if(w->phase==0){s32 v=UpdateRect16Row(0);readyOut[v==1]++;if(v==1)w->phase=1;}
  else{ovl662_func_80119898(w->phase-1);DrawMenuStepArrows(160,192,48,0,w->current,0,w->total);}
  for(int i=0;i<w->count;i++)DrawUiRow(i);UpdateBadgeRowAnim(0);
 }
 UpdateScreenFade(0x1004A);
}
int main(void){assert(sizeof(void*)==8);for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<8;profile++){
 memset(&actual,0xA6,sizeof(actual));w=&actual;actual.frame=word(scenario*0x1234567U);actual.tick=word(~(scenario*0x3456789U));actual.state=profile<4?0:profile==4?3:profile==5?4:half(scenario);
 actual.root=scenario&1;actual.badgeRoot=(scenario>>1)&1;actual.current=scenario%25;actual.selection=(scenario/2)%3;actual.count=(int)((scenario>>2)%5)-1;
 actual.key=half(scenario);actual.mode=(scenario/16)%3-1;actual.phase=((scenario>>6)&1)?0:half(scenario);actual.fade=half(scenario);actual.total=half(~scenario);
 actual.input[0].a=(scenario>>7)&1;actual.input[0].b=(scenario>>8)&1;
 actual.palette0=banks[scenario%3];actual.palette1=banks[(scenario+1)%3];for(unsigned i=0;i<25;i++)actual.map[i]=(scenario+i)%26;
 for(unsigned j=0;j<2;j++){actual.badges[j][0].state10=((scenario>>10)&1)?3:11;for(unsigned i=0;i<4;i++)actual.rows[j][i].byte14=((scenario>>9)&1)?2:5;}
 reference=actual;count=seen=serial=0;referenceRun=1;w=&reference;expected();serial=0;referenceRun=0;w=&actual;ovl662_func_80119E00();assert(seen==count&&!memcmp(&actual,&reference,sizeof(actual)));
 }
 for(unsigned i=0;i<11;i++)assert(coverage[i]);for(unsigned i=0;i<4;i++)assert(action[i]);for(unsigned i=0;i<26;i++)assert(mapped[i]);assert(exitRoute[0]&&exitRoute[1]&&readyOut[0]&&readyOut[1]);
 puts("524288 cases passed; guarded states,all palette mappings,accept/reject/cancel paths and fresh native roots");
}
'''
with TemporaryDirectory() as d:
    c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
    subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined','-fno-sanitize-recover=all','-Iinclude',str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
