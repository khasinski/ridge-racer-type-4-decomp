"""Native STR polling: idle paths, bounded timeouts and diagnostic arguments."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayWaitTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/movie_playback/func_{a}.c').read_text()
                           for a in ('80116764', '801167F8'))
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
#include "rr/overlay668_wait.h"
static struct World { u32 before[8],input,output,calls,after[8];
 const char *message; } world,expected;
u32 *movie_playback_mdec_ctrl_reg,*movie_playback_mdec_out_dma_chcr;
char movie_playback_mdec_in_sync_name[]="input timeout";
char movie_playback_mdec_out_sync_name[]="output timeout";
s32 movie_playback_mdec_timeout(const char *message){
 assert(message==movie_playback_mdec_in_sync_name || message==movie_playback_mdec_out_sync_name);
 world.calls++;world.message=message;
 world.input^=0x13579bdfU;world.output^=0x2468ace0U;
 /* The wait must return the timeout result even if diagnosis clears busy. */
 if(message==movie_playback_mdec_in_sync_name)world.input&=~0x20000000U;
 else world.output&=~0x01000000U;
 return 0;
}
''' + bodies + r'''
static void initialize(u32 k){unsigned i;memset(&world,0,sizeof(world));
 for(i=0;i<8;i++){world.before[i]=k+i;world.after[i]=~(k+i);}
 world.input=k*0x9e3779b9U;world.output=~world.input;
 movie_playback_mdec_ctrl_reg=&world.input;movie_playback_mdec_out_dma_chcr=&world.output;
 assert((uintptr_t)movie_playback_mdec_ctrl_reg>UINT32_MAX);
}
static void idle(void){u32 k;for(k=0;k<65536;k++){
 initialize(k);world.input&=~0x20000000U;world.output&=~0x01000000U;
 expected=world;assert(movie_playback_mdec_in_sync()==0);
 assert(memcmp(&world,&expected,sizeof(world))==0);
 assert(movie_playback_mdec_out_sync()==0);
 assert(memcmp(&world,&expected,sizeof(world))==0);
}}
static void timeout(void){u32 k;unsigned channel;for(k=0;k<8;k++)for(channel=0;channel<2;channel++){
 initialize(k*0x174935U);world.input|=0x20000000U;world.output|=0x01000000U;
 expected=world;expected.calls=1;
 expected.message=channel?movie_playback_mdec_out_sync_name:movie_playback_mdec_in_sync_name;
 expected.input^=0x13579bdfU;expected.output^=0x2468ace0U;
 if(channel)expected.output&=~0x01000000U;else expected.input&=~0x20000000U;
 assert((channel?movie_playback_mdec_out_sync():movie_playback_mdec_in_sync())==-1);
 assert(memcmp(&world,&expected,sizeof(world))==0);
}}
int main(int argc,char **argv){assert(argc==2);if(argv[1][0]=='i')idle();else timeout();return 0;}
'''
        path = Path(cls.temp.name)
        (path / 'test.c').write_text(source)
        cls.binary = path / 'test'
        subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                        '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                        str(path / 'test.c'), '-o', str(cls.binary)], check=True)

    def test_idle_status_words(self):
        subprocess.run([str(self.binary), 'idle'], check=True)

    def test_timeout_and_diagnostic_effects(self):
        subprocess.run([str(self.binary), 'timeout'], check=True)


if __name__ == '__main__':
    unittest.main()
