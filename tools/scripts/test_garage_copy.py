"""Garage image/palette copy with genuine native pointers and live overlaps."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayCopyTest(unittest.TestCase):
    def test_forward_and_cross_stream_aliases(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_8011D120.c').read_text()
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
static struct World {u32 before[8];RrGarageSelectionPrefix state;u32 words[2048],after[8];} world,expected;
#define garage_logo_state world.state
''' + body + r'''
static u32 seed=0x1d120;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
int main(void){unsigned k,i,stream;
 assert(sizeof(void*)==8 && sizeof(RrGarageSelectionPrefix)==0x138 && offsetof(RrGarageSelectionPrefix,scratch34)==0x48 && offsetof(RrGarageSelectionPrefix,image38)==0x50 && offsetof(RrGarageSelectionPrefix,paletteBackup50)==0x80 && offsetof(RrGarageSelectionPrefix,paletteScratch58)==0x90 && offsetof(RrGarageSelectionPrefix,current60)==0xA0);
 for(k=0;k<8192;k++){
  unsigned selected=k&1,src[3],dst[3];
  memset(&world,0,sizeof(world));
  for(i=0;i<sizeof(world.state);i++)((u8*)&world.state)[i]=(u8)next();
  for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
  for(i=0;i<2048;i++)world.words[i]=next();
  for(i=0;i<3;i++){src[i]=next()%1537;dst[i]=next()%1537;}
  switch((k>>1)&7){
   case 0:src[0]=0;dst[0]=(k>>4)%513;break;
   case 1:src[0]=(k>>4)%513;dst[0]=0;break;
   case 2:src[1]=dst[0];break;
   case 3:dst[1]=src[0];break;
   case 4:src[2]=dst[1];dst[2]=dst[0];break;
   case 5:for(i=0;i<3;i++)src[i]=dst[i];break;
   case 6:src[1]=(dst[0]+1)%1537;dst[1]=(src[0]+1)%1537;break;
   default:break;
  }
  world.state.active21=selected;
  world.state.image38[selected]=&world.words[src[0]];
  world.state.image38[!selected]=&world.words[dst[0]];
  world.state.image40[selected]=&world.words[src[1]];
  world.state.image40[!selected]=&world.words[dst[1]];
  world.state.palette48[selected]=&world.words[src[2]];
  world.state.palette48[!selected]=&world.words[dst[2]];
  expected=world;expected.state.dirtyD3=1;
  for(stream=0;stream<3;stream++)for(i=0;i<(stream==2?8:512);i++)
   expected.words[dst[stream]+i]=expected.words[src[stream]+i];
  assert((uintptr_t)world.state.image38[selected]>UINT32_MAX);
  garage_save_undo_copy();
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
