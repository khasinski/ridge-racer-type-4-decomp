"""Rectangle-center struct return, signed extents and overlapping result owners."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayCenterTest(unittest.TestCase):
    def test_centers_and_overlapping_return_destinations(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_8011DC74.c').read_text()
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/garage_garage.h"
static struct World {
 u32 before[8];union {RrGarageRectangle rectangles[32];RrGaragePoint points[64];} arena;
 RrGarageRectangle *table[16];u32 after[8];
} world,expected,initial;
#define garage_logo_buttons world.table
''' + body + r'''
static u32 seed=0x1dc74;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static int signed16(unsigned raw){return raw<32768?raw:(int)raw-65536;}
static int floorhalf(int value){return value>=0?value/2:-((-value+1)/2);}
int main(void){unsigned k,i,j;
 for(k=0;k<65536;k++){
  unsigned index=k&15,selected=(k*7)&31,output=(k*11)&63;
  u16 ex,ey;RrGaragePoint result;
  memset(&world,0,sizeof(world));
  for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
  for(i=0;i<32;i++)for(j=0;j<4;j++)world.arena.rectangles[i][j]=next();
  for(i=0;i<16;i++)world.table[i]=&world.arena.rectangles[next()&31];
  world.table[index]=&world.arena.rectangles[selected];
  world.arena.rectangles[selected][0]=k;
  world.arena.rectangles[selected][2]=k*40503U;
  world.arena.rectangles[selected][3]=k*17U+19;
  ex=(u16)(signed16(world.arena.rectangles[selected][0])+floorhalf(signed16(world.arena.rectangles[selected][2])));
  ey=(u16)(signed16(world.arena.rectangles[selected][1])+floorhalf(signed16(world.arena.rectangles[selected][3])));
  if((k&3)<2)output=selected*2+(k&1);
  initial=world;expected=world;
  memcpy(&expected.arena.points[output].x,&ex,2);
  memcpy(&expected.arena.points[output].y,&ey,2);
  assert((uintptr_t)world.table[index]>UINT32_MAX);
  result=garage_get_button_center(index);
  assert(memcmp(&world,&initial,sizeof(world))==0);
  /* A caller can place the returned structure over the selected rectangle. */
  world.arena.points[output]=result;
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
