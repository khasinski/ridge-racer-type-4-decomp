"""Serial-menu reset thresholds and state publication across the setup hook."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlaySerialResetTest(unittest.TestCase):
    def test_thresholds_and_exact_flag(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/serial_link/func_801153F0.c').read_text()
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
#include "rr/overlay667_reset.h"
typedef struct {s32 word00;u8 pad04[3],flag07;s32 unused08,unused0C,word10,word14,word18,word1C;} Serial;
static struct World {
 u32 before[8];Serial serial;s32 frames;u16 screen,step;s16 counter;
 u32 calls;s32 post_word,post_frames;u16 post_screen,post_step;s16 post_counter;u32 after[8];
} world,expected;
#define serial_link_D_80118EE8 world.serial.word00
#define serial_link_D_80118EEF world.serial.flag07
#define serial_link_D_80118EF8 world.serial.word10
#define serial_link_D_80118EFC world.serial.word14
#define serial_link_D_80118F00 world.serial.word18
#define serial_link_D_80118F04 world.serial.word1C
#define ModeFrame world.frames
#define ModeGroup world.screen
#define ModeId world.step
#define ModeSubState world.counter
void serial_link_func_80114A40(void){
 assert(world.serial.word00==0 && world.serial.word10==0 && world.serial.word14==0 && world.serial.word18==0 && world.serial.word1C==0 && world.frames==0);
 world.calls++;world.serial.word1C=world.post_word;world.frames=world.post_frames;
 world.screen=world.post_screen;world.step=world.post_step;world.counter=world.post_counter;
}
''' + body + r'''
static u32 seed=0x153f0;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 word(u32 bits){s32 value;memcpy(&value,&bits,4);return value;}
int main(void){unsigned k,i;
 assert(offsetof(Serial,flag07)==7 && offsetof(Serial,word1C)==28);
 for(k=0;k<65536;k++){
  int reset;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  world.calls=0;world.serial.flag07=k>>8;
  world.serial.word1C=word(next());world.serial.word14=word(next());
  switch(k&7){
   case 0:world.serial.word1C=6;world.serial.word14=6;world.serial.flag07=1;break;
   case 1:world.serial.word1C=6;world.serial.word14=6;world.serial.flag07=2;break;
   case 2:world.serial.word1C=7;world.serial.word14=word(0x80000000U);break;
   case 3:world.serial.word1C=6;world.serial.word14=7;break;
   case 4:world.serial.word1C=word(0x80000000U);world.serial.word14=word(0x7fffffffU);break;
   case 5:world.serial.word1C=-1;world.serial.word14=-1;break;
   case 7:world.serial.word1C=0;world.serial.word14=0;break;
   default:break;
  }
  reset=world.serial.word1C>6 || world.serial.word14>6 || world.serial.flag07==1;
  memcpy(&expected,&world,sizeof(world));
  if(reset){
   expected.serial.word00=0;expected.serial.word10=0;expected.serial.word14=0;expected.serial.word18=0;
   expected.serial.word1C=world.post_word;expected.frames=world.post_frames;expected.calls=1;
   expected.screen=1;expected.step=44;expected.counter=0;
  }
  serial_link_func_801153F0();
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
