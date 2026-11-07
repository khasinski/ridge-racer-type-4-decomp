"""Decoder startup ordering with full native callback and queue pointers.

Device reset, callback installation, queue setup and playback are typed simulated
boundaries; this test does not execute the hardware driver implementations.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest


class MoviePlaybackStartTest(unittest.TestCase):
    def test_fresh_stream_root_callback_and_service_order(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/movie_playback/func_80114EE8.c').read_text()
        source = r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/movie_playback_decode.h"
static RrStrStreamPrefix streams[3];
static RrLibQueueRecord32 queues[3][4];
RrStrStreamPrefix *StrStreamState;
static unsigned scenario, step;
static RrLibInterruptCallback requested;
static void callback0(void) { assert(0); }
static void callback1(void) { assert(0); }
static void callback2(void) { assert(0); }
static RrLibInterruptCallback callbacks[4]={0,callback0,callback1,callback2};
static struct {
 u32 guard[4]; RrLibInterruptCallback installed;
 RrLibQueueRecord32 *queue; s32 count,args[5]; unsigned events[4]; u32 tail[4];
} world, expected;
void movie_playback_dec_dct_reset(s32 mode) {
 assert(step==0 && mode==0); world.events[step++]=1;
 /* Both preceding boundaries can replace the active root. */
 StrStreamState=&streams[(scenario+1)%3];
 streams[(scenario+2)%3].queue20=&queues[(scenario+1)%3][scenario%4];
}
RrLibInterruptCallback movie_playback_dec_dct_out_callback(RrLibInterruptCallback callback) {
 RrLibInterruptCallback previous=world.installed;
 assert(step==1 && callback==requested); world.events[step++]=2;
 world.installed=callback;
 StrStreamState=&streams[(scenario+2)%3];
 streams[(scenario+2)%3].queue20=&queues[(scenario+2)%3][(scenario+1)%4];
 return previous;
}
void StSetRing(RrLibQueueRecord32 *queue,s32 count) {
 assert(step==2 && count==32);
 assert(queue==&queues[(scenario+2)%3][(scenario+1)%4]);
 world.events[step++]=3; world.queue=queue; world.count=count;
 StrStreamState=&streams[scenario%3];
}
void StSetStream(s32 a,s32 b,s32 c,s32 d,s32 e) {
 assert(step==3); world.events[step++]=4;
 world.args[0]=a;world.args[1]=b;world.args[2]=c;world.args[3]=d;world.args[4]=e;
}
'''+body+r'''
int main(void) {
 unsigned i,j;
 _Static_assert(sizeof(void*)==8,"full native pointers required");
 _Static_assert(offsetof(RrStrStreamPrefix,queue20)==48,"native queue offset");
 for(scenario=0;scenario<65536;scenario++) {
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)(scenario*29+i);
  for(i=0;i<sizeof(queues);i++)((u8*)queues)[i]=(u8)(scenario+i*17);
  for(i=0;i<3;i++) {
   memset(&streams[i],0xA5,sizeof(streams[i]));
   for(j=0;j<2;j++)streams[i].buffers18[j]=&queues[i][j];
   streams[i].queue20=&queues[i][scenario%4];
  }
  world.installed=callbacks[(scenario>>2)%4]; requested=callbacks[scenario%4];
  expected=world; expected.installed=requested;
  expected.queue=&queues[(scenario+2)%3][(scenario+1)%4];expected.count=32;
  expected.args[0]=1;expected.args[1]=1;expected.args[2]=-1;
  expected.args[3]=0;expected.args[4]=0;
  for(i=0;i<4;i++)expected.events[i]=i+1;
  StrStreamState=&streams[scenario%3]; step=0;
  movie_playback_stream_init(requested);
  assert(step==4 && StrStreamState==&streams[scenario%3]);
  assert(memcmp(&world,&expected,sizeof(world))==0);
  for(i=0;i<sizeof(queues);i++)assert(((u8*)queues)[i]==(u8)(scenario+i*17));
  for(i=0;i<3;i++)for(j=0;j<2;j++)assert(streams[i].buffers18[j]==&queues[i][j]);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            c = Path(temp) / 'start.c'
            exe = Path(temp) / 'start'
            c.write_text(source)
            subprocess.run(['cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror','-Wno-error=deprecated-non-prototype',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            '-I', str(root / 'include'), str(c), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
