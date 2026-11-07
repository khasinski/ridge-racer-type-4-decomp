from pathlib import Path
from tempfile import TemporaryDirectory
import re,subprocess
body=Path('src/overlays/PAL/garage/func_80118A30.c').read_text();body=re.sub(r'^extern .*\n','',body,flags=re.M)
copy=Path('tools/scripts/fixtures/views/func_8002B75C.c').read_text().replace('CopyStringClearTail','actual_tail_copy')
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
#include "rr/overlay662_name_controller.h"
#include "rr/menu_transition.h"
#include "rr/menu_choice.h"
#include "rr/save_progress.h"
#include "rr/string_tail_copy.h"
#include "rr/tables.h"
#include "rr/icon_sprites.h"
static struct World {u32 guard[4];RrSaveProgressRecord7C save;u8 name[12],glyphs[44];s8 index,column,row;s32 selected,frame,tick,accept;RrMenuItemState5C input[2];s16 timer,state,slot,scene;u16 enable;RrIconSpriteContext contexts[2],*context;u32 tail[4];} world,expected;
#define SaveProgress world.save
#define NameCharset world.glyphs
#define garage_name_buffer world.name
#define garage_name_cursor world.index
#define garage_name_key_column world.column
#define garage_name_key_row world.row
#define garage_name_key_index world.selected
#define ModeFrame world.frame
#define ScreenFrame world.tick
#define MenuConfirmPressed world.accept
#define PlayerControllers world.input
#define MenuFadeTimer world.timer
#define ModeSubState world.state
#define MenuResultCode world.slot
#define ModeId world.scene
#define ModeGroup world.enable
#define CurrentDrawBuf world.context
'''+body+copy+r'''
static struct Event {unsigned kind;s32 a,b,c;void *p;struct World before;} events[9];
static unsigned count,seen,serial,scenario,profile,oneCalls,coverage[6],choices[46],outcome[2];static int reference_run;
static s16 half(u16 x){s16 y;memcpy(&y,&x,2);return y;}
static s32 word(u32 x){s32 y;memcpy(&y,&x,4);return y;}
static void event(unsigned kind,s32 a,s32 b,s32 c,void *p){struct World *w=reference_run?&expected:&world;
 if(reference_run){assert(count<9);events[count++]=(struct Event){kind,a,b,c,p,*w};}
 else{coverage[kind]++;assert(seen<count);struct Event *e=&events[seen++];assert(e->kind==kind&&e->a==a&&e->b==b&&e->c==c&&e->p==p&&memcmp(w,&e->before,sizeof(*w))==0);}
}
static void effect(unsigned kind,s32 a){struct World *w=reference_run?&expected:&world;unsigned n=serial++;u32 v=0x9E3779B9U*(scenario+n+profile+1);
 w->frame=word((u32)w->frame+v);w->tick=word((u32)w->tick-v);w->context=&world.contexts[(scenario+n)&1];w->contexts[(scenario+n)&1].head70^=v;
 w->timer=half((u16)(scenario*23+n*17));w->state=half((u16)v);w->index=(scenario+n+profile)%9;w->row=(scenario+n)%5;w->column=(scenario/5+profile)%(w->row==4?3:10);
 w->input[0].buttons0A=(u16)scenario;w->input[0].b=(scenario>>6)&1;w->accept=(scenario>>7)&1;
 w->name[(scenario+n)%8]^=(u8)(v>>24);w->name[11]=0;
 if(kind==1&&a==1){oneCalls++;w->selected=(scenario+profile+n)%(oneCalls==1?46:43);}
 else w->selected=(scenario+profile+n)%43;
}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){event(0,a,b,c,0);effect(0,a);}
void PlaySfx(s32 a){assert(a>=0&&a<=2);event(1,a,0,0,0);effect(1,a);}
void CopyStringClearTail(char *d,char *s){assert(d==world.save.prefix.progress.prefix00.fields.name&&s==(char*)world.name);event(2,0,0,0,d);
 if(reference_run){unsigned i=0;while(expected.name[i]){assert(i<12);expected.save.prefix.progress.prefix00.fields.name[i]=expected.name[i];i++;}while(expected.save.prefix.progress.prefix00.fields.name[i]){assert(i<12);expected.save.prefix.progress.prefix00.fields.name[i++]=0;}}
 else actual_tail_copy(d,s);
}
void AddScreenFadeTiles(u32 *p,s32 a,u32 b){assert((uintptr_t)p>UINT32_MAX);event(3,a,b,0,p);effect(3,a);}
void UpdateBadgeRowAnim(s32 a){event(4,a,0,0,0);effect(4,a);}void UpdateScreenFade(s32 a){event(5,a,0,0,0);effect(5,a);}
static void reference(void){struct World *w=&expected;unsigned state=(u16)w->state;
 w->frame=word((u32)w->frame+1);w->tick=word((u32)w->tick+1);
 if(state==0){int cancel=0;SetMenuFrameOptions(1,1,1);
  if(w->input[0].b)cancel=1;
  else if(w->input[0].buttons0A&0x1000){PlaySfx(0);w->row=(w->row+4)%5;if(w->row==4&&w->column>=3)w->column=2;}
  else if(w->input[0].buttons0A&0x4000){PlaySfx(0);w->row=(w->row+1)%5;if(w->row==4&&w->column>=3)w->column=2;}
  else if(w->input[0].buttons0A&0x8000){PlaySfx(0);w->column--;}
  else if(w->input[0].buttons0A&0x2000){PlaySfx(0);w->column++;}
  int columns=w->row==4?3:10;w->column=(w->column+columns)%columns;w->selected=w->row*10+w->column;
  if(w->accept){PlaySfx(1);int selected=w->selected;if(selected>=0&&selected<46)choices[selected]++;
   if(selected<41){if(w->index<8){PlaySfx(1);assert(w->index>=0&&w->index<12&&w->selected>=0&&w->selected<=42);w->name[w->index]=w->glyphs[w->selected==40?0:w->selected+1];if(w->index<7)w->index++;}}
   else if(selected==41){if(w->index>0)w->index--;}
   else if(selected==42){w->timer=64;w->state=1;CopyStringClearTail(world.save.prefix.progress.prefix00.fields.name,(char*)world.name);}
  }else if(cancel){PlaySfx(2);w->name[w->index]=w->glyphs[0];if(w->index>0)w->index--;}
 }else if(state==1){SetMenuFrameOptions(0,1,1);s16 timer=half((u16)((u16)w->timer-1));w->timer=timer;AddScreenFadeTiles(&w->context->head70,(64-(s32)timer)*4,69);
  if(w->timer<=0){outcome[0]++;w->slot=5;w->enable=1;w->scene=71;w->state=0;}else outcome[1]++;
 }
 UpdateBadgeRowAnim(0);UpdateBadgeRowAnim(1);UpdateScreenFade(0x1008B);
}
int main(void){assert(sizeof(void*)==8);
 for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<8;profile++){
  memset(&world,0xA6,sizeof(world));world.context=&world.contexts[0];world.index=scenario%9;world.column=scenario%10;world.row=(scenario>>4)%5;if(world.row==4)world.column%=3;world.selected=world.row*10+world.column;
  for(unsigned i=0;i<12;i++){world.name[i]='A'+(scenario+i)%26;world.save.prefix.progress.prefix00.fields.name[i]='a'+(scenario+i)%26;}world.name[scenario%12]=0;world.name[11]=0;world.save.prefix.progress.prefix00.fields.name[(scenario>>4)%12]=0;world.save.prefix.progress.prefix00.fields.name[11]=0;
  memcpy(world.glyphs," 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!?&@",42);world.glyphs[42]=world.glyphs[43]=0;
  world.state=profile<4?0:profile<6?1:half(scenario);world.timer=half(scenario);world.frame=word(scenario*0x1234567U);world.tick=word(~(scenario*0x2345678U));
  expected=world;count=seen=serial=oneCalls=0;reference_run=1;reference();serial=oneCalls=0;reference_run=0;garage_update_name_entry();
  assert(count==seen&&memcmp(&world,&expected,sizeof(world))==0);
 }
 for(unsigned i=0;i<6;i++)assert(coverage[i]);for(unsigned i=0;i<46;i++)assert(choices[i]);assert(outcome[0]&&outcome[1]);
 puts("524288 cases passed; all character actions,actual tail-copy,full signed16 fade states and fresh native roots");
}
'''
with TemporaryDirectory() as d:
 c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
 subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined','-fno-sanitize-recover=all','-Iinclude',str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
