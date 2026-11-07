from overlay_units import read_c_source
"""Stable minimum selection, sentinel positions and unchanged shared standings."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayMinimumTest(unittest.TestCase):
    def test_minimum_ties_and_negative_sentinels(self):
        root = Path(__file__).resolve().parents[2]
        body = read_c_source(root / 'src/overlays/PAL/time_attack_course_select/func_801162D8.c')
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "rr/standings.h"
static struct World {u32 before[8];s32 values[10];u16 selected;u32 after[8];} world,expected;
#define LapTimes world.values
#define BestLapIndex world.selected
''' + body + r'''
static u32 seed=0x162d8;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 word(u32 raw){s32 value;memcpy(&value,&raw,4);return value;}
int main(void){unsigned k,i;
 const s32 alphabet[]={-1,0,1,2147483647};
 for(k=0;k<135168;k++){
  unsigned n=k%7,wanted=0;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  if(k<131072){
   for(i=0;i<n;i++){
    switch(k&7){
     case 0:world.values[i]=k;break;
     case 1:world.values[i]=2147483647-(s32)i;break;
     case 2:world.values[i]=i;break;
     case 3:world.values[i]=0;break;
     default:world.values[i]=next()&0x7fffffffU;break;
    }
   }
   if(n<6)world.values[n]=word(next()|0x80000000U);
  }else{
   unsigned pattern=k-131072;
   for(i=0;i<6;i++)world.values[i]=alphabet[(pattern>>(2*i))&3];
  }
  /* The first minimum in the active prefix is the stable-sort oracle. */
  for(i=0;i<6 && world.values[i]>=0;i++)
   if(world.values[i]<world.values[wanted])wanted=i;
  memcpy(&expected,&world,sizeof(world));expected.selected=wanted;
  time_attack_course_select_find_best_lap();
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
