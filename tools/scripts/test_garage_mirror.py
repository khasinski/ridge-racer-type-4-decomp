"""Packed64x64 horizontal image reflection, action dispatch and fresh callback state."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re,subprocess,unittest
class MirrorTest(unittest.TestCase):
 def test_pixels_and_service_order(self):
  root=Path(__file__).resolve().parents[2]
  body=(root/'src/overlays/PAL/garage/func_8011EFFC.c').read_text()
  body=re.sub(r'__asm__\("(?:v1|t9)"\)','',body)
  code=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/garage_buttons.h"
#include "rr/garage_options.h"
#include "rr/menu_choice.h"
static struct World {u32 guard[4];RrGarageSelectionPrefix g;RrGarageWorkingStorage w;RrGarageButton buttons[3];RrGarageRectangle *table[3];u32 images[4][512];u32 tail[4];} world,expected;
static struct Event {unsigned id;int arg;struct World before;} events[5];
static unsigned count,seen,scenario;static int reference_run;
#define garage_logo_state world.g
#define garage_logo_work world.w
#define garage_logo_buttons world.table
'''+body+r'''
static u32 seed=0x1effc;static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void effect(struct World *w,unsigned id){
 w->g.phaseCC+=id+17;w->w.words[15]^=0x12340000U+id;
 if(id==0){w->w.words[1]=(w->w.words[1]&~1U)|((scenario>>2)&1);w->w.words[0]=(w->w.words[0]&~4U)|((scenario>>1)&4);}
 if(id==1){w->g.saved64[2]=(s32)scenario;w->table[2]=&world.buttons[1].rectangle;}
 if(id==2){w->g.active21=(u8)((scenario>>3)&1);w->g.layer23=(u8)((scenario>>4)&1);w->table[2]=&world.buttons[scenario&1].rectangle;w->images[0][17]^=0x76543210U;}
 if(id==3){w->g.active21=(u8)((scenario>>5)&1);w->g.layer23=(u8)((scenario>>6)&1);w->images[3][21]^=0x13579BDFU;}
}
static void service(unsigned id,int arg){
 struct World *w=reference_run?&expected:&world;
 if(reference_run){assert(count<5);events[count].id=id;events[count].arg=arg;events[count++].before=*w;}
 else {assert(seen<count);assert(events[seen].id==id&&events[seen].arg==arg);assert(memcmp(w,&events[seen].before,sizeof(*w))==0);seen++;}
 effect(w,id);
}
s32 garage_cursor_outside_button(s32 n){assert(n==2);service(0,n);return (scenario>>1)&1;}
void garage_save_cursor_selection(void){service(1,0);}
void garage_save_undo_copy(void){service(2,0);}
void PlaySfx(s32 n){service(3,n);}
void garage_navigate_to_option(s32 n){service(4,n);}
static unsigned slot(u32 *p){for(unsigned i=0;i<4;i++)if(p==world.images[i])return i;assert(0);return 0;}
static void reference(s32 command){
 RrGarageSelectionPrefix *g=&expected.g;
 if(command<0){g->optionIndices98.flat[0]=1;g->optionIndices98.flat[3]=1;return;}
 if(command>0){
 switch(command){case 1:g->optionIndices98.flat[0]=0;g->selectorsB8[3]=0;service(4,0);break;
 case 2:g->optionIndices98.flat[0]=2;g->selectorsB8[3]=2;service(4,0);break;
 case 4:g->optionIndices98.flat[3]=0;g->selectorsB8[2]=0;service(4,1);break;
 case 8:g->selectorsB8[1]=2;g->selectorsB8[2]=2;service(4,1);break;}return;}
 service(0,2);if(((scenario>>1)&1)||!(expected.w.words[1]&1))return;
 if(expected.w.words[0]&4)service(1,0);
 else {g->optionIndices98.flat[0]=0;g->optionIndices98.flat[3]=1;g->saved64[0]=2;g->saved64[1]=0;g->saved64[2]=0;g->saved64[3]=1;g->saved64[4]=1;}
 service(2,0);unsigned button=(expected.table[2]==&world.buttons[1].rectangle);expected.buttons[button].timer0E=7;service(3,1);
 u32 **images=g->layer23?g->image40:g->image38;unsigned src=slot(images[!g->active21]),dst=slot(images[g->active21]);assert(src!=dst);
 /* Independent pixel coordinates: output(x,y) = input(63-x,y). */
 for(unsigned y=0;y<64;y++)for(unsigned x=0;x<64;x++){
  unsigned sx=63-x,sy=y;u32 pixel=(expected.images[src][sy*8+sx/8]>>(4*(sx%8)))&15U;
  unsigned at=y*8+x/8,shift=4*(x%8);expected.images[dst][at]=(expected.images[dst][at]&~(15U<<shift))|(pixel<<shift);
 }
}
int main(void){
 static const s32 commands[]={0,1,2,4,8,-1,INT32_MIN,INT32_MAX,3,5,6,7,9,16,-32768,32767};
 assert(sizeof(void*)==8&&(uintptr_t)world.images>UINT32_MAX);
 for(scenario=0;scenario<131072;scenario++){
  memset(&world,0xA6,sizeof(world));world.g.active21=scenario&1;world.g.layer23=(scenario>>1)&1;
  for(unsigned i=0;i<2;i++){world.g.image38[i]=world.images[i];world.g.image40[i]=world.images[2+i];world.table[i]=&world.buttons[i].rectangle;}world.table[2]=&world.buttons[2].rectangle;
  world.w.words[0]=next();world.w.words[1]=next();
  for(unsigned i=0;i<4;i++)for(unsigned j=0;j<512;j++)world.images[i][j]=next();
  /* Additional sparse inputs distinguish packed-nibble permutations. */
  if(scenario&256){memset(world.images,0,sizeof(world.images));unsigned pixel=(scenario>>1)&4095;for(unsigned i=0;i<4;i++)world.images[i][pixel/8]=((scenario>>13)&15U)<<(4*(pixel%8));}
  s32 command=commands[(scenario>>7)&15];expected=world;count=seen=0;reference_run=1;reference(command);reference_run=0;
  garage_mirror_tool_action(command);assert(seen==count);assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 scenario=4;
 for(unsigned pixel=0;pixel<4096;pixel++)for(unsigned value=1;value<16;value++){
  memset(world.images,0,sizeof(world.images));for(unsigned i=0;i<4;i++)world.images[i][pixel/8]=value<<(4*(pixel%8));
  expected=world;count=seen=0;reference_run=1;reference(0);reference_run=0;garage_mirror_tool_action(0);
  assert(seen==count&&memcmp(&world,&expected,sizeof(world))==0);
 }
 puts("192512 guarded cases; packed pixel oracle, full native pointers and pre-service snapshots");return 0;
}
'''
  with TemporaryDirectory() as d:
   c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(code)
   subprocess.run(['cc','-std=gnu11','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-fsanitize=undefined','-fno-sanitize-recover=all','-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
   subprocess.run([str(exe)],check=True)
if __name__=='__main__':unittest.main()
