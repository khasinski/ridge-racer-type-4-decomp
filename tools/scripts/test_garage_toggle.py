"""Garage page toggle after the actual resident sound forwarding function."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayToggleTest(unittest.TestCase):
    def test_control_flag_and_post_sound_selector(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_801256D4.c').read_text()
        resident = (root / 'tools/scripts/fixtures/views/func_8004F24C.c').read_text()
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
#include "rr/overlay662_garage.h"
static struct World {u32 before[8];RrGarageSelectionPrefix state;
 RrGarageWorkingStorage work;u32 calls; s32 args[4];u8 next_selector;u32 after[8];} world,expected;
#define garage_D_801276A8 world.state
#define garage_D_80127668 world.work
int PlaySfxVolPan(s32 a,s32 b,s32 c,s32 d){
 world.calls++;world.args[0]=a;world.args[1]=b;world.args[2]=c;world.args[3]=d;
 world.state.active21=world.next_selector;world.work.words[1]^=2;
 return -1;
}
''' + resident + body + r'''
static u32 seed=0x256d4;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
int main(void){unsigned k,i;
 for(k=0;k<65536;k++){
  memset(&world,0,sizeof(world));
  for(i=0;i<sizeof(world.state);i++)((u8*)&world.state)[i]=(u8)next();
  for(i=0;i<16;i++)world.work.words[i]=next();
  for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
  world.work.words[1]=(next()&0xffff0000U)|k;
  world.state.active21=k;
  world.next_selector=k%4==0?0:k%4==1?1:next();
  expected=world;
  if(k&2){
   expected.calls=1;expected.args[0]=1;expected.args[1]=127;
   expected.args[2]=64;expected.args[3]=-1;
   expected.state.active21=world.next_selector==0?1:0;
   expected.work.words[1]^=2;
  }
  garage_func_801256D4();
  assert(memcmp(&world,&expected,sizeof(world))==0);
  assert((uintptr_t)&world>UINT32_MAX);
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
