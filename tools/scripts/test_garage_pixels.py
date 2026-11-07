"""Packed nibble blending with forward overlaps and source rereads."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayPixelsTest(unittest.TestCase):
    def test_packed_nibbles_and_overlapping_buffers(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_80125F60.c').read_text()
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "rr/overlay662_pixels.h"
static struct World {u32 before[8],pixels[1024],after[8];} world,expected;
static u32 seed=0x25f60;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
''' + body + r'''
int main(void){unsigned k,i,n;
 for(k=0;k<8192;k++){
  unsigned src=k%513,dst=(k*17)%513;
  if(k<513){src=0;dst=k;}else if(k<1026){src=k-513;dst=0;}
  memset(&world,0,sizeof(world));
  for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
  for(i=0;i<1024;i++){
   u32 value=next(),pattern=(i+k)&255,mask=0;
   for(n=0;n<8;n++)if(pattern&(1U<<n))mask|=15U<<(4*n);
   world.pixels[i]=value&mask;
  }
  expected=world;
  for(i=0;i<512;i++){
   u32 original=expected.pixels[src+i];
   /* Clear exactly the destination digits replaced by nonzero source digits. */
   for(n=0;n<8;n++)if((original>>(4*n))&15)
    expected.pixels[dst+i]&=~(15U<<(4*n));
   /* The second read observes the preceding write, including src == dst. */
   expected.pixels[dst+i]|=expected.pixels[src+i];
  }
  assert((uintptr_t)&world.pixels[src]>UINT32_MAX);
  garage_blend_image_nibbles(&world.pixels[src],&world.pixels[dst]);
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
