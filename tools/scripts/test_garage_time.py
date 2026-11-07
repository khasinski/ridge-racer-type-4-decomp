"""Garage time text: signed input, raw byte conversion and complete buffer ownership."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayTimeTest(unittest.TestCase):
    def test_time_boundaries_and_full_word_inputs(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_8011883C.c').read_text()
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "rr/overlay662_time.h"
static struct World {u32 before[8];u8 text[6];u32 after[8];} world,expected;
#define garage_D_80126244 world.text
''' + body + r'''
static u32 seed=0x1883c;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 word(u32 raw){s32 value;memcpy(&value,&raw,4);return value;}
int main(void){unsigned k,i;const s32 edges[]={-1,(-2147483647-1),2147483647,2999,3000,179999,180000,3723000};
 for(k=0;k<131080;k++){
  s32 value;u8 *returned;
  if(k<65536)value=k;
  else if(k<131072)value=word((next()&0xffff0000U)|(k-65536));
  else value=edges[k-131072];
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  memcpy(&expected,&world,sizeof(world));
  if(value<0){expected.text[0]='-';expected.text[1]='-';expected.text[3]='-';expected.text[4]='-';}
  else{
   int64_t minutes=(int64_t)value/180000;
   int64_t seconds=((int64_t)value%180000)/3000;
   expected.text[0]=(u8)(48+minutes/10);expected.text[1]=(u8)(48+minutes%10);
   expected.text[3]=(u8)(48+seconds/10);expected.text[4]=(u8)(48+seconds%10);
  }
  returned=garage_func_8011883C(value);
  assert(returned==world.text && (uintptr_t)returned>UINT32_MAX);
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
