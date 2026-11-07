"""Option initialization with opaque object identities and full native pointers."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayOptionsTest(unittest.TestCase):
    def test_negative_modes_and_complete_owner(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_8011D278.c').read_text()
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
#include "rr/overlay662_options.h"
/* Fixture payloads are opaque to the implementation; only identity matters. */
/* Record schema is now shared with the real mapper. */
static struct World {u32 before[8];RrGarageSelectionPrefix state;
 RrGarageOptionRecord records[8][5];u32 buffer[512],after[8];} world,expected;
#define garage_D_801276A8 world.state
#define garage_D_801268CC world.records[0]
#define garage_D_80126994 world.records[1]
#define garage_D_801269E4 world.records[2]
#define garage_D_80126A34 world.records[3]
#define garage_D_80126A84 world.records[4]
#define garage_D_80126AFC world.records[5]
#define garage_D_80126BC4 world.records[6]
#define garage_D_80126BEC world.records[7]
''' + body + r'''
static u32 seed=0x1d278;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
int main(void){unsigned k,i;
 const u32 edges[]={0,1,0x7fffffffU,0x80000000U,0xffffffffU,0x80000001U,0xffff0000U,0x0000ffffU};
 const s32 defaults[]={0,1,0,0,1,0,0,0};
 assert(sizeof(void*)==8 && sizeof(RrGarageSelectionPrefix)==0x138 && offsetof(RrGarageSelectionPrefix,scratch34)==0x48 && offsetof(RrGarageSelectionPrefix,image38)==0x50 && offsetof(RrGarageSelectionPrefix,paletteBackup50)==0x80 && offsetof(RrGarageSelectionPrefix,paletteScratch58)==0x90 && offsetof(RrGarageSelectionPrefix,current60)==0xA0);
 for(k=0;k<65544;k++){
  u32 raw=k<65536?(next()&0xffff0000U)|k:edges[k-65536];s32 mode;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  for(i=0;i<2;i++){
   world.state.image38[i]=world.buffer;
   world.state.image40[i]=world.buffer;
   world.state.palette48[i]=world.buffer;
  }
  for(i=0;i<8;i++)world.state.options78.flat[i]=&world.records[next()&7][0];
  memcpy(&mode,&raw,4);expected=world;
  if(raw&0x80000000U){
   for(i=0;i<8;i++){
    expected.state.options78.flat[i]=&world.records[i][0];
    expected.state.optionIndices98.flat[i]=defaults[i];
   }
   expected.state.current60=0;
   memset(expected.state.saved64,0,sizeof(expected.state.saved64));
   memset(expected.state.selectorsB8,0,sizeof(expected.state.selectorsB8));
  }
  assert((uintptr_t)&world.records[0]>UINT32_MAX);
  garage_func_8011D278(mode);
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
