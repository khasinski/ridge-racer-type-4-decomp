"""Garage reset over actual shared state and signed-halfword/word union owner."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayResetTest(unittest.TestCase):
    def test_partial_and_full_reset(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_8011A788.c').read_text()
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
static struct World {u32 before[8];RrGarageSelectionPrefix state;RrGarageWorkingStorage work;u32 after[8];} world,expected;
#define garage_logo_state world.state
#define garage_logo_work world.work
''' + body + r'''
static u32 seed=0x1a788;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
int main(void){unsigned k,m,i;const s32 modes[]={0,1,-1,(-2147483647-1)};
 for(m=0;m<4;m++)for(k=0;k<65536;k++){
  u16 x=k,y=k*40503U;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  /* Pointer fields remain genuine pointers even though reset preserves them. */
  for(i=0;i<2;i++){
   world.state.image38[i]=world.work.words;
   world.state.image40[i]=world.work.words;
   world.state.palette48[i]=world.work.words;
  }
  for(i=0;i<8;i++)world.state.options78.flat[i]=0;
  memcpy(&world.work.halves[10],&x,2);memcpy(&world.work.halves[11],&y,2);
  expected=world;
  expected.state.fade00=-1;expected.state.word04=0;
  expected.state.flagsD1=0;expected.state.stateD2=0;
  expected.work.words[0]=0;expected.work.words[1]=48;
  expected.work.words[2]=0;expected.work.words[3]=0;expected.work.words[4]=0;
  for(i=14;i<24;i++)expected.work.halves[i]=0;
  expected.work.halves[24]=13;expected.work.halves[25]=63;
  expected.work.halves[26]=294;expected.work.halves[27]=346;
  memcpy(&expected.work.halves[12],&x,2);memcpy(&expected.work.halves[13],&y,2);
  if(m){
   memset(expected.state.reset08,0,sizeof(expected.state.reset08));
   expected.state.config1C=2;expected.state.config1D=1;
   expected.state.config1E=0;expected.state.config1F=0;expected.state.config20=0;
   expected.state.active21=0;expected.state.config22=1;
   expected.state.halfC8=0;expected.state.halfCA=0;
   expected.state.phaseCC=0;expected.state.shadeD0=0;
   expected.work.halves[10]=67;expected.work.halves[11]=149;
   expected.work.halves[12]=67;expected.work.halves[13]=149;
   expected.work.words[14]=0;
  }
  garage_reset_editor_state(modes[m]);
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
