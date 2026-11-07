"""Serial cleanup rereads event IDs and signed descriptors between BIOS hooks."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlaySerialHandlesTest(unittest.TestCase):
    def test_handle_order_reloads_and_ignored_failures(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/serial_link/func_80114A40.c').read_text()
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "rr/serial_handles.h"
typedef struct {u32 kind;s32 first,second;unsigned long argument;} Event;
static struct World {
 u32 before[8],event0,event1;s32 descriptor0,descriptor1;
 u32 calls;Event events[10];s32 results[10];u32 after[8];
} world,expected;
#define serial_link_D_80118ED8 world.event0
#define serial_link_D_80118EDC world.event1
#define LinkWriteHandle world.descriptor0
#define LinkReadHandle world.descriptor1
static s32 word(u32 raw){s32 value;memcpy(&value,&raw,4);return value;}
static s32 operation(struct World *w,u32 kind,s32 a,s32 b,unsigned long argument){
 Event *e;u32 index=w->calls;assert(index<10);e=&w->events[w->calls++];
 e->kind=kind;e->first=a;e->second=b;e->argument=argument;
 w->event0+=0x01020304U+kind;w->event1^=0xf00d0000U|kind;
 w->descriptor0=word((u32)w->descriptor0^0xa5a50000U);
 w->descriptor1=word((u32)w->descriptor1+kind);
 return w->results[index];
}
s32 _sio_control(s32 a,s32 b,unsigned long argument){return operation(&world,1,a,b,argument);}
s32 func_80096EC4(u32 event){return operation(&world,2,0,0,event);}
s32 FileClose(s32 descriptor){return operation(&world,3,descriptor,0,0);}
s32 CloseEvent(u32 event){return operation(&world,4,0,0,event);}
''' + body + r'''
static u32 seed=0x14a40;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
int main(void){unsigned k,i;
 for(k=0;k<65536;k++){
  memset(&world,0,sizeof(world));
  for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
  world.event0=0xf1000000U|k;world.event1=(next()&0xffff0000U)|(k^65535);
  world.descriptor0=(k&3)==0?-1:(k&3)==1?word(0x80000000U):word(next());
  world.descriptor1=word(next());
  for(i=0;i<10;i++)world.results[i]=(i&1)?0:word(next()|0x80000000U);
  memcpy(&expected,&world,sizeof(world));
  operation(&expected,1,2,3,0);operation(&expected,1,2,0,0);
  operation(&expected,2,0,0,expected.event0);operation(&expected,2,0,0,expected.event1);
  operation(&expected,3,expected.descriptor0,0,0);operation(&expected,3,expected.descriptor1,0,0);
  operation(&expected,4,0,0,expected.event0);operation(&expected,4,0,0,expected.event1);
  operation(&expected,1,4,0,0);operation(&expected,1,1,1,1);
  serial_link_func_80114A40();
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
