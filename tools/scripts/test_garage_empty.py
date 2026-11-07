"""Selected image emptiness, full-width pointers and protected early exits."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayEmptyTest(unittest.TestCase):
    def test_selected_layers_and_early_return(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_8011D0C0.c').read_text()
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/garage_garage.h"
static struct World {u32 before[8];RrGarageSelectionPrefix state;u32 pixels[4][512],after[8];} world,initial;
#define garage_logo_state world.state
''' + body + r'''
static u32 seed=0x1d0c0;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void initialize(unsigned k){unsigned i,j;
 memset(&world,0,sizeof(world));
 for(i=0;i<sizeof(world.state);i++)((u8*)&world.state)[i]=(u8)next();
 for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
 for(i=0;i<4;i++)for(j=0;j<512;j++)world.pixels[i][j]=next()|1;
 for(i=0;i<2;i++){world.state.image38[i]=world.pixels[i];world.state.image40[i]=world.pixels[2+i];}
 world.state.active21=k&1;world.state.layer23=(k>>1)&255;
}
int main(void){unsigned k,i;long page=sysconf(_SC_PAGESIZE);u8 *mapping;
 for(k=0;k<65536;k++){
  unsigned selected,count=k%513;int answer;
  initialize(k);selected=world.state.active21+(world.state.layer23?2:0);
  memset(world.pixels[selected],0,sizeof(world.pixels[selected]));
  if(count<512)world.pixels[selected][count]=1U<<(k&31);
  /* Other layers remain nonzero: success requires selecting the correct one. */
  initial=world;answer=garage_is_layer_empty();
  assert(answer==(count==512));assert(memcmp(&world,&initial,sizeof(world))==0);
  assert((uintptr_t)world.state.image38[0]>UINT32_MAX);
 }
 assert(page>=4096);
 mapping=mmap(0,(size_t)page*2,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);
 assert(mapping!=MAP_FAILED);assert(mprotect(mapping+page,(size_t)page,PROT_NONE)==0);
 for(i=0;i<4;i++){
  u32 *last=(u32 *)(mapping+page-4);*last=0x80000000U;
  initialize(i);world.state.active21=i&1;world.state.layer23=i>>1;
  if(i>>1)world.state.image40[i&1]=last;else world.state.image38[i&1]=last;
  initial=world;assert(garage_is_layer_empty()==0);
  assert(*last==0x80000000U && memcmp(&world,&initial,sizeof(world))==0);
 }
 assert(munmap(mapping,(size_t)page*2)==0);return 0;
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
