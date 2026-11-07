"""STR shutdown with actual callback dispatcher and resident audio stop body."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayShutdownTest(unittest.TestCase):
    def test_shutdown_order_callbacks_ports_and_state(self):
        root = Path(__file__).resolve().parents[2]
        paths = ['tools/scripts/fixtures/legacy/func_8008B3C4.c',
                 'src/overlays/PAL/668/func_80116534.c',
                 'tools/scripts/fixtures/legacy/func_8008F564.c',
                 'src/overlays/PAL/668/func_80114DF4.c']
        bodies = '\n'.join(re.sub(r'^extern[^\n]*;\n', '', (root / p).read_text(), flags=re.M)
                           for p in paths)
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
#include "rr/overlay668_decode.h"
#include "rr/cd.h"
#include "rr/frame_loop.h"
typedef struct {s32 kind,argument;RrLibInterruptCallback callback;} Event;
static struct World {
 u32 before[8];RrLibInterruptDriverPrefix driver,*active;
 RrLibInterruptCallback slot;s32 audio_mode,after_lock;
 s8 ports[4],*port0,*port1;u32 first,second,calls;
 u16 screen,step;s16 counter;Event events[8];u32 after[8];
} world,expected;
#define IntrDriverTablePtr world.active
#define StUseAltCallbacks world.audio_mode
#define StCdReg0 world.port0
#define CD_REG3_RING world.port1
#define ModeGroup world.screen
#define ModeId world.step
#define ModeSubState world.counter
static void callback(void) {assert(0);}
static void event(struct World *w,s32 kind,s32 a){Event *e;assert(w->calls<8);e=&w->events[w->calls++];e->kind=kind;e->argument=a;}
static void exchange_effect(struct World *w,s32 channel,RrLibInterruptCallback cb){
 event(w,1,channel);w->events[w->calls-1].callback=cb;w->slot=cb;
 w->audio_mode=99;w->screen=0x1234;w->counter=-17;
}
static RrLibInterruptCallback exchange(s32 channel,RrLibInterruptCallback cb){
 RrLibInterruptCallback old=world.slot;assert(channel==1 && cb==0);
 exchange_effect(&world,channel,cb);return old;
}
static void lock_effect(struct World *w){
 event(w,2,0);w->audio_mode=w->after_lock;
 w->port0=&world.ports[w->first];w->port1=&world.ports[w->second];
 w->step=0x8765;
}
static void channel_effect(struct World *w,s32 kind,s32 argument){
 event(w,kind,argument);w->audio_mode^=3;w->counter=-300;
}
static void unlock_effect(struct World *w){event(w,7,0);w->screen=42;w->counter=-42;}
static void cd_effect(struct World *w){event(w,8,9);w->screen=0xffff;w->step=0xffff;w->counter=-32768;}
s32 EnterCriticalSection(void){lock_effect(&world);return -1;}
s32 CdAltDataCallback(s32 a){channel_effect(&world,3,a);return 0;}
s32 CdAltReadyCallback(s32 a){channel_effect(&world,4,a);return 0;}
s32 CdDataCallback(s32 a){channel_effect(&world,5,a);return 0;}
s32 CdReadyCallback(s32 a){channel_effect(&world,6,a);return 0;}
s32 ExitCriticalSection(void){assert(*world.port0==0 && *world.port1==0);unlock_effect(&world);return 0;}
s32 CdControlB(s32 command,const u8 *parameters,u8 *result){
 assert(command==9 && parameters==0 && result==0);cd_effect(&world);return -1;
}
''' + bodies + r'''
static u32 seed=0x14df4;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
int main(void){unsigned k,i;
 for(k=0;k<16384;k++){
  memset(&world,0,sizeof(world));
  for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
  for(i=0;i<4;i++)world.ports[i]=next();
  world.driver.exchange=exchange;world.active=&world.driver;
  world.slot=(k&1)?callback:0;world.audio_mode=next();
  world.after_lock=k%4==0?1:k%4==1?0:k%4==2?-1:2;
  world.first=k&3;world.second=(k>>2)&3;
  world.port0=&world.ports[(k+1)&3];world.port1=&world.ports[(k+2)&3];
  world.screen=next();world.step=next();world.counter=next();
  memcpy(&expected,&world,sizeof(world));
  exchange_effect(&expected,1,0);lock_effect(&expected);
  if(world.after_lock==1){channel_effect(&expected,3,0);channel_effect(&expected,4,0);}
  else {channel_effect(&expected,5,0);channel_effect(&expected,6,0);}
  expected.ports[world.first]=0;expected.ports[world.second]=0;
  unlock_effect(&expected);cd_effect(&expected);
  expected.screen=1;expected.step=3;expected.counter=0;
  ovl668_func_80114DF4();
  assert(memcmp(&world,&expected,sizeof(world))==0);
  assert((uintptr_t)world.active>UINT32_MAX && (uintptr_t)world.port0>UINT32_MAX);
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
