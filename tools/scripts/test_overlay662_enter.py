"""Garage entry publishes state between initialization and screen dispatch."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayEnterTest(unittest.TestCase):
    def test_state_publication_and_hook_order(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/662/func_801161B0.c').read_text()
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
#include "rr/frame_loop.h"
static struct World {
 u32 before[8];u16 screen,step;s16 counter;
 u16 initial_screen,initial_step;s16 initial_counter;
 u16 changed_screen,changed_step;s16 changed_counter;
 u32 calls,command;u16 observed_screen,observed_step;s16 observed_counter;
 u32 after[8];
} world,expected;
#define ModeGroup world.screen
#define ModeId world.step
#define ModeSubState world.counter
void ovl662_func_80116148(void){
 assert(world.calls++==0);
 assert(world.screen==world.initial_screen && world.step==world.initial_step && world.counter==world.initial_counter);
 world.screen^=0xffff;world.step^=0xffff;world.counter=(s16)~world.counter;
}
void UpdateScreenFade(s32 command){
 assert(world.calls++==1);world.command=command;
 world.observed_screen=world.screen;world.observed_step=world.step;world.observed_counter=world.counter;
 world.screen=world.changed_screen;world.step=world.changed_step;world.counter=world.changed_counter;
}
''' + body + r'''
static u32 seed=0x161b0;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 signedhalf(u32 v){u16 bits=v;s16 result;memcpy(&result,&bits,2);return result;}
int main(void){unsigned k,i;
 for(k=0;k<65536;k++){
  memset(&world,0,sizeof(world));
  for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
  world.screen=k;world.step=k*40503U;world.counter=signedhalf(k*17U+19);
  world.initial_screen=world.screen;world.initial_step=world.step;world.initial_counter=world.counter;
  world.changed_screen=next();world.changed_step=next();world.changed_counter=signedhalf(next());
  memcpy(&expected,&world,sizeof(world));
  expected.calls=2;expected.command=0x10002;
  expected.observed_screen=1;expected.observed_step=66;expected.observed_counter=0;
  expected.screen=expected.changed_screen;expected.step=expected.changed_step;expected.counter=expected.changed_counter;
  ovl662_func_801161B0();
  assert(memcmp(&world,&expected,sizeof(world))==0);
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
