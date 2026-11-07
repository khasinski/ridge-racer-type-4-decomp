"""Driver startup, retry loop and nonzero completion semantics."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayDriverTest(unittest.TestCase):
    def test_retry_counts_returns_and_order(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/serial_link/func_80115494.c').read_text()
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "rr/driver_control.h"
typedef struct {s32 kind,first,second;unsigned long argument;} Event;
static struct World {
 u32 before[8],planned,remaining,attempts,active,phase,calls;
 s32 final_result,ignored_reset,ignored_enable,ignored_flush,ignored_disable,ignored_wait;
 Event events[40];u32 after[8];
} world,expected;
static void record(struct World *w,s32 kind,s32 a,s32 b,unsigned long argument){
 Event *e;assert(w->calls<40);e=&w->events[w->calls++];e->kind=kind;e->first=a;e->second=b;e->argument=argument;
}
s32 ResetGraph(s32 mode){assert(world.phase==0);record(&world,1,mode,0,0);world.remaining=world.planned;world.phase=1;return world.ignored_reset;}
s32 _sio_control(s32 command,s32 operation,unsigned long argument){
 if(command==3 && operation==0 && argument==1){
  assert(world.phase==1);record(&world,2,command,operation,argument);world.phase=2;world.active=1;return world.ignored_enable;
 }
 if(command==3 && operation==1 && argument==0){
  assert(world.phase==2);record(&world,3,command,operation,argument);world.attempts++;
  if(world.remaining){world.remaining--;return 0;}
  world.phase=3;return world.final_result;
 }
 if(command==5 && operation==0 && argument==0){
  assert(world.phase==3);record(&world,4,command,operation,argument);world.phase=4;return world.ignored_flush;
 }
 assert(command==3 && operation==0 && argument==0 && world.phase==4);
 record(&world,5,command,operation,argument);world.phase=5;world.active=0;return world.ignored_disable;
}
s32 VSync(s32 mode){assert(world.phase==5 && world.active==0);record(&world,6,mode,0,0);world.phase=6;return world.ignored_wait;}
void SetDispMask(s32 mode){assert(world.phase==6);record(&world,7,mode,0,0);world.phase=7;}
''' + body + r'''
static u32 seed=0x15494;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 word(u32 raw){s32 value;memcpy(&value,&raw,4);return value;}
int main(void){unsigned k,i;const u32 terminals[]={1,0xffffffffU,0x80000000U,0x7fffffffU};
 for(k=0;k<65536;k++){
  memset(&world,0,sizeof(world));
  for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
  world.planned=k&31;world.remaining=next();world.active=next();
  world.final_result=word(terminals[(k>>5)&3]);
  world.ignored_reset=word(next());world.ignored_enable=(k&1)?0:-1;
  world.ignored_flush=word(next());world.ignored_disable=0;world.ignored_wait=word(next());
  memcpy(&expected,&world,sizeof(world));
  record(&expected,1,1,0,0);record(&expected,2,3,0,1);
  for(i=0;i<=world.planned;i++)record(&expected,3,3,1,0);
  record(&expected,4,5,0,0);record(&expected,5,3,0,0);
  record(&expected,6,0,0,0);record(&expected,7,1,0,0);
  expected.remaining=0;expected.attempts=world.planned+1;expected.active=0;expected.phase=7;
  serial_link_reset_graph_and_sio();
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
