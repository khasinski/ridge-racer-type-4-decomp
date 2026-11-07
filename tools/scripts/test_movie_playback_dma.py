"""STR DMA setup with actual bounded waits, register aliases and native pointers."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayDmaTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/movie_playback/func_{a}.c').read_text()
                           for a in ('80116648', '801166D8', '80116764', '801167F8'))
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/overlay668_dma.h"
static struct World { u32 before[8],words[64],map[7],calls,after[8];
 const void *input_address;void *output_address;const char *message; } world,expected;
u32 *movie_playback_D_80116E58,*movie_playback_D_80116E24,*movie_playback_D_80116E28;
u32 *movie_playback_D_80116E30,*movie_playback_D_80116E50,*movie_playback_D_80116E34,*movie_playback_D_80116E54;
const void *volatile *movie_playback_D_80116E20;
void *volatile *movie_playback_D_80116E2C;
char movie_playback_D_801148DC[]="input timeout",movie_playback_D_801148EC[]="output timeout";
static void bind(void){
 movie_playback_D_80116E58=&world.words[world.map[0]];
 movie_playback_D_80116E24=&world.words[world.map[1]];
 movie_playback_D_80116E28=&world.words[world.map[2]];
 movie_playback_D_80116E30=&world.words[world.map[3]];
 movie_playback_D_80116E50=&world.words[world.map[4]];
 movie_playback_D_80116E34=&world.words[world.map[6]];
 movie_playback_D_80116E54=&world.words[63];
 movie_playback_D_80116E20=&world.input_address;
 movie_playback_D_80116E2C=&world.output_address;
}
static void diagnose(struct World *w,const char *message){
 w->calls++;w->message=message;
 w->words[w->map[5]]^=0x9e3779b9U;
 w->map[0]=(w->map[0]+1)&15;
}
s32 movie_playback_func_801168A4(const char *message){diagnose(&world,message);bind();return 0;}
''' + bodies + r'''
static u32 seed=0x16648;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void initialize(u32 k){unsigned i;memset(&world,0,sizeof(world));
 for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
 for(i=0;i<64;i++)world.words[i]=next();
 for(i=0;i<7;i++)world.map[i]=next()&15;
 /* Explicitly exercise all registers sharing the command word. */
 if((k&31)==0)for(i=0;i<7;i++)world.map[i]=k&15;
 world.words[63]&=~0x20000000U;
 world.words[world.map[6]]&=~0x01000000U;
 world.input_address=&world.words[32];world.output_address=&world.words[48];
 bind();assert((uintptr_t)&world.words[0]>UINT32_MAX);
}
static void cases(unsigned output){u32 k;for(k=0;k<65536;k++){
 u32 count=(next()&0xffff0000U)|k;void *buffer;
 initialize(k);buffer=&world.words[16+(next()&15)];
 if(k<8){if(output)world.words[world.map[6]]|=0x01000000U;else world.words[63]|=0x20000000U;}
 expected=world;
 if(k<8)diagnose(&expected,output?movie_playback_D_801148EC:movie_playback_D_801148DC);
 /* Independent register model: DPCR preserves every bit except enabling 0/1.
  * BCR has 32 words per block and the low16 bits of the block count. */
 expected.words[expected.map[0]]|=0x88;
 if(output){
  expected.words[expected.map[6]]=0;
  expected.output_address=buffer;
  expected.words[expected.map[3]]=((count/32)&65535U)*65536U+32;
  expected.words[expected.map[6]]=0x01000200;
  movie_playback_func_801166D8(buffer,count);
 }else{
  expected.input_address=(const u8 *)&world.words[world.map[5]]+4;
  expected.words[expected.map[1]]=((count/32)&65535U)*65536U+32;
  expected.words[expected.map[4]]=expected.words[expected.map[5]];
  expected.words[expected.map[2]]=0x01000201;
  movie_playback_func_80116648(&world.words[world.map[5]],count);
 }
 assert(memcmp(&world,&expected,sizeof(world))==0);
 assert(movie_playback_D_80116E58==&world.words[expected.map[0]]);
}}
int main(int argc,char **argv){assert(argc==2);cases(argv[1][0]=='o');return 0;}
'''
        path = Path(cls.temp.name)
        (path / 'test.c').write_text(source)
        cls.binary = path / 'test'
        subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                        '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                        str(path / 'test.c'), '-o', str(cls.binary)], check=True)

    def test_input_dma_registers_and_live_command(self):
        subprocess.run([str(self.binary), 'input'], check=True)

    def test_output_dma_registers_and_pointer(self):
        subprocess.run([str(self.binary), 'output'], check=True)


if __name__ == '__main__':
    unittest.main()
