"""Unsigned animation ticks and the shared halfword/word working owner."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayTickTest(unittest.TestCase):
    def test_counter_wrap_modes_and_shared_storage(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/garage/func_{a}.c').read_text()
                           for a in ('8011DDD8', '8011E7C0'))
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/overlay662_garage.h"
static struct World {u32 before[8];RrGarageWorkingStorage storage;u32 after[8];} world,expected;
#define garage_D_80127668 world.storage
''' + bodies + r'''
static u32 seed=0x1ddd8;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
int main(void){unsigned k,m,i;
 const u32 edges[]={0,1,38,39,40,44,45,46,47,48,0x7fffffffU,0x80000000U,0xfffffffeU,0xffffffffU};
 const s32 modes[]={0,1,-1,(-2147483647-1)};
 for(m=0;m<4;m++)for(k=0;k<65536;k++){
  unsigned slot=m?2:3;u32 raw,result;uint64_t widened;
  memset(&world,0,sizeof(world));
  for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
  for(i=0;i<16;i++)world.storage.words[i]=next();
  raw=k<64?edges[k%14]:(next()&0xffff0000U)|k;
  world.storage.words[slot]=raw;
  /* Exercise the existing halfword publisher against the same union owner. */
  expected=world;
  for(i=0;i<4;i++){
   u16 bits=(u16)(k+i*371U);memcpy(&expected.storage.halves[24+i],&bits,2);
  }
  garage_func_8011E7C0(k,k+371,k+742,k+1113);
  assert(memcmp(&world,&expected,sizeof(world))==0);
  widened=(uint64_t)raw+1;
  if(widened==UINT64_C(0x100000000))widened=0;
  if(widened>46)widened=40;
  expected.storage.words[slot]=(u32)widened;
  result=garage_func_8011DDD8(modes[m]);
  assert(result==(widened==1 || widened==40));
  assert(memcmp(&world,&expected,sizeof(world))==0);
  assert((uintptr_t)&world.storage>UINT32_MAX);
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
