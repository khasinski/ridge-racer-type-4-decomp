"""Screen transition with a real containing frame prefix and live context pointers."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayScreenExitTest(unittest.TestCase):
    def test_setup_order_and_fresh_ordering_pointer(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/667/func_801155E0.c').read_text()
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/overlay_screen_exit.h"
typedef struct {RrGpuRectangle *context;u32 *ordering;u16 screen,step;s16 counter;s32 intensity;u32 mode;} Event;
static struct World {
 u32 before[8];RrFrameOrderingPrefix frames[2];RrGpuRectangle *active;
 u16 screen,step;s16 counter,post_counter;u16 post_screen,post_step;
 u32 selected,marker,calls;Event events[3];u32 after[8];
} world,expected;
#define CurrentDrawBuf world.active
#define ModeGroup world.screen
#define ModeId world.step
#define D_800F4D8A world.counter
static Event *capture(struct World *w){Event *e;assert(w->calls<3);e=&w->events[w->calls++];e->context=w->active;e->screen=w->screen;e->step=w->step;e->counter=w->counter;return e;}
static void first(struct World *w){capture(w);w->active=&world.frames[0].rectangle;w->screen^=0xffff;w->step^=0xffff;w->counter=(s16)~w->counter;}
static void second(struct World *w){capture(w);w->active=&world.frames[w->selected].rectangle;w->screen=0x1234;w->step=0x5678;w->counter=-123;}
void StopRaceAudioAndRumble(void){assert(world.calls==0);first(&world);}
void SetupRaceAndScene(void){assert(world.calls==1);second(&world);}
void AddScreenFadeTiles(u32 *ordering,s32 intensity,u32 mode){
 Event *e;assert(world.calls==2);e=capture(&world);e->ordering=ordering;e->intensity=intensity;e->mode=mode;
 assert(ordering==world.frames[world.selected].ordering70);
 *ordering=world.marker;world.active=&world.frames[!world.selected].rectangle;
 world.screen=world.post_screen;world.step=world.post_step;world.counter=world.post_counter;
}
''' + body + r'''
static u32 seed=0x155e0;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 v){u16 raw=v;s16 result;memcpy(&result,&raw,2);return result;}
int main(void){unsigned k,i;
 memset(&world,0,sizeof(world));
 for(i=0;i<sizeof(world.frames);i++)((u8*)world.frames)[i]=(u8)next();
 for(k=0;k<65536;k++){
  Event *e;
  world.calls=0;memset(world.events,0,sizeof(world.events));
  for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
  world.active=(k&7)?&world.frames[(k>>1)&1].rectangle:0;
  world.screen=k;world.step=k*40503U;world.counter=half(k*17U+3);
  world.post_screen=next();world.post_step=next();world.post_counter=half(next());
  world.selected=k&1;world.marker=next();
  memcpy(&expected,&world,sizeof(world));first(&expected);second(&expected);
  expected.screen=1;expected.step=26;expected.counter=0;e=capture(&expected);
  e->ordering=world.frames[world.selected].ordering70;e->intensity=255;e->mode=69;
  expected.frames[world.selected].ordering70[0]=world.marker;
  expected.active=&world.frames[!world.selected].rectangle;
  expected.screen=expected.post_screen;expected.step=expected.post_step;expected.counter=expected.post_counter;
  ovl667_func_801155E0();
  assert(memcmp(&world,&expected,sizeof(world))==0);
  assert((uintptr_t)world.frames[world.selected].ordering70>UINT32_MAX);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            path = Path(temp)
            (path / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(path / 'test.c'), '-o', str(path / 'test')], check=True)
            subprocess.run([str(path / 'test')], check=True)


if __name__ == '__main__':
    unittest.main()
