"""Actual STR readiness polling and timeout buffer-coordinate selection."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlaySyncTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/movie_playback/func_8011535C.c').read_text()
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/movie_playback_display.h"
static struct World {u32 before[8];RrStrDisplayConfig config;u32 after[8];} world,expected;
static u32 seed=0x1535c;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
''' + body + r'''
static void initialize(u32 k){unsigned i;u32 raw;
 memset(&world,0,sizeof(world));
 for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
 world.config.word00=next();world.config.word08=next();
 world.config.alternate04=k%8==0?0:k%8==1?1:next();
 for(i=0;i<3;i++){
  raw=next();memcpy(&world.config.rectangles[i].x,&raw,4);
  raw=next();memcpy(&world.config.rectangles[i].w,&raw,4);
 }
 assert((uintptr_t)&world.config>UINT32_MAX);
}
static void ready(void){u32 k;for(k=0;k<65536;k++){
 initialize(k);world.config.ready0C=(next()&0xffff0000U)|k|0x10000U;
 expected=world;expected.config.ready0C=0;
 movie_playback_sync(&world.config,(s32)(k%4)-2);
 assert(memcmp(&world,&expected,sizeof(world))==0);
}}
static void timeout(void){u32 k;for(k=0;k<32;k++){
 unsigned selected;
 initialize(k);world.config.ready0C=0;expected=world;
 selected=world.config.alternate04==0?1:0;
 expected.config.alternate04=selected;
 expected.config.rectangles[2].x=world.config.rectangles[selected].x;
 expected.config.rectangles[2].y=world.config.rectangles[selected].y;
 movie_playback_sync(&world.config,(s32)(k%4)-2);
 assert(memcmp(&world,&expected,sizeof(world))==0);
}}
int main(int argc,char **argv){assert(argc==2);if(argv[1][0]=='r')ready();else timeout();return 0;}
'''
        path = Path(cls.temp.name)
        (path / 'test.c').write_text(source)
        cls.binary = path / 'test'
        subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                        '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                        str(path / 'test.c'), '-o', str(cls.binary)], check=True)

    def test_ready_consumption(self):
        subprocess.run([str(self.binary), 'ready'], check=True)

    def test_timeout_buffer_selection_and_coordinates(self):
        subprocess.run([str(self.binary), 'timeout'], check=True)


if __name__ == '__main__':
    unittest.main()
