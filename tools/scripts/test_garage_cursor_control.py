"""Native cursor controller oracle: complete state, service order and fresh owners."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
body=Path('src/overlays/PAL/garage/func_8011E070.c').read_text()
center=Path('src/overlays/PAL/garage/func_8011DC74.c').read_text().replace('garage_get_button_center','actual_center')
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
#include "rr/overlay662_cursor_control.h"
static void action0(s32);static void action1(s32);static void action2(s32);static void action3(s32);
static RrGarageActionPrefix actionObjects[4]={{action0},{action1},{action2},{action3}};
static struct {u32 guard[4];RrGarageButton banks[2][46];u32 tail[4];} fixed;
static struct State {u32 guard[4];RrGarageWorkingStorage work;RrGarageSelectionPrefix selection;RrMenuItemState5C input[2];
 RrGarageRectangle *buttons[46];RrGarageActionPrefix *actions[4];u32 tail[4];} actual,reference,*w;
#define garage_logo_work w->work
#define garage_logo_state w->selection
#define PlayerControllers w->input
#define garage_logo_buttons w->buttons
#define garage_logo_actions w->actions
'''+body+center+r'''
static struct Event{unsigned kind;s32 a,b;struct State before;}events[16];
static unsigned scenario,profile,serial,count,seen,coverage[12],paths[8],callbacks[4];static int referenceRun;
static s16 half(u16 u){s16 v;memcpy(&v,&u,2);return v;}static s32 word(u32 u){s32 v;memcpy(&v,&u,4);return v;}
static void record(unsigned kind,s32 a,s32 b){assert(kind<12);
 if(referenceRun){assert(count<16);events[count++]=(struct Event){kind,a,b,*w};}
 else{coverage[kind]++;assert(seen<count);struct Event *e=&events[seen++];assert(e->kind==kind&&e->a==a&&e->b==b&&!memcmp(&e->before,w,sizeof(*w)));}
}
static void effect(unsigned kind){unsigned n=serial++;if(!(profile&1))return;u32 v=(scenario+31*n+17*kind)*0x9E3779B9U;
 w->work.words[0]=word(v);w->work.words[1]=~v;
 w->work.points.cursor14=(RrGaragePoint){half(v),half(v>>16)};
 w->work.points.cursor18=(RrGaragePoint){half(v+123),half(v>>12)};
 w->work.points.first20=(RrGaragePoint){half(~v),half(v>>8)};w->work.points.second24=(RrGaragePoint){half(v*3),half(v>>4)};
 if(profile&2){static const s32 progress[]={-1,4095,4096,4097};w->work.points.word28=(u32)progress[(scenario+n)&3];}
 else w->work.points.word28=(u32)(s32)half(scenario+13*n);
 w->work.points.distance2C=word(v);
 for(unsigned i=0;i<4;i++)w->work.halves[24+i]=half(scenario+i*0x1234U+n*71);
 for(unsigned i=0;i<5;i++)w->selection.saved64[i]=word(v+i*0x1234567U);w->selection.saved64[0]=(scenario+n)%46;
 w->selection.current60=word(~v);for(unsigned i=0;i<4;i++)w->selection.selectorsB8[i]=word(v*(i+1));
 w->input[0].type01=((scenario>>6)&1)?0x12:(u8)scenario;w->input[0].buttons08=(u16)(scenario+n*4);
 for(unsigned i=0;i<46;i++)w->buttons[i]=&fixed.banks[(scenario+n+i)&1][i].rectangle;
 for(unsigned i=0;i<4;i++)w->actions[i]=&actionObjects[(scenario+n+i+profile/4)%4];
}
void *PsyqMemset(void *p,s32 value,u32 size){assert(size==4&&value==0&&(uintptr_t)p>UINT32_MAX);record(0,value,size);return memset(p,value,size);}
void garage_update_cursor_glide(void){record(1,0,0);effect(1);}
static void movePoint(unsigned kind,RrGaragePoint *p){assert((uintptr_t)p>UINT32_MAX);record(kind,p->x,p->y);effect(kind);p->x=half(scenario*37U+serial*71U+kind);p->y=half(~(scenario*53U+serial*13U+kind));}
void garage_read_analog_motion(RrGaragePoint *p){movePoint(2,p);}void garage_read_mouse_motion(RrGaragePoint *p){movePoint(3,p);}void garage_read_dpad_motion(RrGaragePoint *p){movePoint(4,p);}
static void repeat(unsigned kind,u32 *p){assert((uintptr_t)p>UINT32_MAX);record(kind,*p,0);effect(kind);*p|=(scenario+serial*7U)&15;}
void garage_read_analog_repeat(u32 *p){repeat(5,p);}void garage_read_dpad_repeat(u32 *p){repeat(6,p);}
void garage_dispatch_direction_input(u32 a){record(7,a,0);effect(7);}
RrGaragePoint garage_get_button_center(s32 a){assert(a>=0&&a<46);record(8,a,0);RrGaragePoint result;
 if(referenceRun){RrGarageRectangle *r=w->buttons[a];int x=half((*r)[0]),y=half((*r)[1]),width=half((*r)[2]),height=half((*r)[3]);
  result.x=half(x+width/2-(width<0&&width%2));result.y=half(y+height/2-(height<0&&height%2));
 }else result=actual_center(a);
 effect(8);return result;
}
s32 garage_calc_glide_step(const RrGaragePoint *a,const RrGaragePoint *b){assert(a==&w->work.points.first20&&b==&w->work.points.second24&&(uintptr_t)a>UINT32_MAX);record(9,((u32)(u16)a->x<<16)|(u16)a->y,((u32)(u16)b->x<<16)|(u16)b->y);effect(9);return word(scenario*0x1234567U+serial);}
static void action(unsigned k,s32 a){callbacks[k]++;record(10,k,a);effect(10);}
static void action0(s32 a){action(0,a);}static void action1(s32 a){action(1,a);}static void action2(s32 a){action(2,a);}static void action3(s32 a){action(3,a);}
void PlaySfx(s32 a){record(11,a,0);effect(11);}
void garage_update_button_state(void){record(11,-1,0);effect(11);}
static void expected(void){RrGaragePoint movement={0,0};u32 events=0;PsyqMemset(&movement,0,4);
 if(w->work.words[0]&1){paths[0]++;return;}
 if(w->work.words[0]&2){garage_update_cursor_glide();if(word(w->work.points.word28)<4096){paths[1]++;return;}
  paths[2]++;w->work.points.cursor18=w->work.points.cursor14;w->work.words[0]&=~6U;
 }
 if(w->work.words[0]&4){if(!(w->work.words[0]&8)){garage_update_cursor_glide();if(word(w->work.points.word28)<4096){paths[3]++;return;}
   paths[4]++;garage_read_analog_repeat(&events);garage_read_dpad_repeat(&events);garage_dispatch_direction_input(events);
  }else paths[5]++;
 }else{garage_read_analog_motion(&movement);garage_read_mouse_motion(&movement);garage_read_dpad_motion(&movement);
  int x=half((u16)w->work.points.cursor18.x+(u16)movement.x),y=half((u16)w->work.points.cursor18.y+(u16)movement.y);w->work.points.cursor18=(RrGaragePoint){x,y};
  if(x<w->work.halves[24])w->work.points.cursor18.x=w->work.halves[24];int high=w->work.halves[24]+w->work.halves[26]-1;
  if(high<w->work.points.cursor18.x)w->work.points.cursor18.x=half(high);
  if(y<w->work.halves[25])w->work.points.cursor18.y=w->work.halves[25];high=w->work.halves[25]+w->work.halves[27]-1;
  if(high<w->work.points.cursor18.y)w->work.points.cursor18.y=half(high);w->work.points.cursor14=w->work.points.cursor18;
  if(!(w->work.words[0]&8)&&w->input[0].type01!=18&&(w->input[0].buttons08&4)){
   paths[6]++;w->work.words[0]|=4;w->work.words[1]=48;w->work.points.first20=w->work.points.cursor18;
   RrGaragePoint dest=garage_get_button_center(w->selection.saved64[0]);w->work.points.second24=dest;w->work.points.word28=0;
   s32 distance=garage_calc_glide_step(&w->work.points.first20,&w->work.points.second24);w->work.points.distance2C=distance;
   w->selection.current60=w->selection.saved64[0];for(unsigned i=0;i<4;i++)w->selection.selectorsB8[i]=w->selection.saved64[i+1];
   RrGarageButton *button=(RrGarageButton *)w->buttons[w->selection.saved64[0]];w->actions[button->action0A]->callback(-1);PlaySfx(3);return;
  }
 }
 paths[7]++;garage_update_button_state();
}
int main(void){assert(sizeof(void*)==8);for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<8;profile++){
 memset(&actual,0xA6,sizeof(actual));w=&actual;w->work.words[0]=scenario;w->work.points.word28=(u32)(s32)half(scenario);
 if(profile&2){static const s32 progress[]={-1,4095,4096,4097};w->work.points.word28=(u32)progress[(scenario>>4)&3];}
 w->work.points.cursor14=(RrGaragePoint){half(scenario),half(~scenario)};w->work.points.cursor18=(RrGaragePoint){half(scenario*71U),half(scenario*17U)};
 for(unsigned i=0;i<4;i++)w->work.halves[24+i]=half(scenario+i*0x2345U);
 for(unsigned i=0;i<5;i++)w->selection.saved64[i]=word(scenario*0x1234567U+i);w->selection.saved64[0]=scenario%46;
 w->input[0].type01=((scenario>>6)&1)?18:(u8)scenario;w->input[0].buttons08=(u16)(scenario>>1);
 for(unsigned g=0;g<2;g++)for(unsigned i=0;i<46;i++){for(unsigned j=0;j<4;j++)fixed.banks[g][i].rectangle[j]=(u16)(scenario+i*71+g*13+j*0x4567);fixed.banks[g][i].action0A=(scenario+i+g)%4;}
 for(unsigned i=0;i<46;i++)w->buttons[i]=&fixed.banks[(scenario+i)&1][i].rectangle;for(unsigned i=0;i<4;i++)w->actions[i]=&actionObjects[(scenario+i+profile/4)%4];
 reference=actual;count=seen=serial=0;referenceRun=1;w=&reference;expected();serial=0;referenceRun=0;w=&actual;garage_update_cursor();assert(count==seen&&!memcmp(&actual,&reference,sizeof(actual)));
 }
 for(unsigned i=0;i<12;i++)assert(coverage[i]);for(unsigned i=0;i<8;i++)assert(paths[i]);for(unsigned i=0;i<4;i++)assert(callbacks[i]);
 puts("524288 cases passed; all cursor paths,full signed16 clipping,signed progress and fresh button/action owners");
}
'''
with TemporaryDirectory() as d:
    c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
    subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-Iinclude',str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
