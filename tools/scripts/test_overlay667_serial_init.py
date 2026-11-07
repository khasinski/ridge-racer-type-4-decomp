"""Serial initialization: owned word views, ready loops and live service state.

Device dispatch and timing are typed simulated boundaries. The registered status
callback is the actual recovered C helper; no BIOS/device hardware runs here.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest


class Overlay667SerialInitTest(unittest.TestCase):
    def test_word_clear_service_order_callback_width_and_loop_exits(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/667/func_80114AF8.c').read_text()
        callback = (root / 'src/overlays/PAL/667/func_8011492C.c').read_text()
        # Host build omits only the helper's target register/scheduling metadata.
        callback = callback.replace(' __asm__("$2")', '')
        callback = callback.replace('__asm__("");', '')
        callback = callback.replace('__asm__("" : "=r"(ret) : "0"(ret));', '')
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
#include "rr/overlay667_packet.h"
struct State {
 s32 command,peer,halt,pending,serial,rxcount,timeout,txcount,failures,event,phase;
 u8 error,read,write,selected,limit,received,sent;
 RrOverlay667SerialRing ring;
 RrOverlay667SerialStorage rx,tx;
 RrOverlay667ChoicePair payload,incoming,outgoing;
};
struct Event {s32 kind,command,operation;unsigned long argument;u32 hash;};
static struct World {
 u32 guard[4];struct State state;u32 tail[4];
 unsigned calls,probes,statuses;struct Event events[40];
} actual,expected,initial,*w;
static unsigned scenario,mutation;
#define ovl667_D_80118EC8 w->state.command
#define ovl667_D_80118ED0 w->state.peer
#define ovl667_D_80118ED4 w->state.halt
#define ovl667_D_80118F0C w->state.pending
#define ovl667_D_80118EF4 w->state.serial
#define ovl667_D_80118EF8 w->state.rxcount
#define ovl667_D_80118EFC w->state.timeout
#define ovl667_D_80118F00 w->state.txcount
#define ovl667_D_80118F04 w->state.failures
#define ovl667_D_80118EE8 w->state.event
#define ovl667_D_80118EEC w->state.phase
#define ovl667_D_80118EEF w->state.error
#define ovl667_D_80118F08 w->state.read
#define ovl667_D_80118F09 w->state.write
#define ovl667_D_80118F8D w->state.selected
#define ovl667_D_80118F8C w->state.limit
#define ovl667_D_80118EED w->state.received
#define ovl667_D_80118EEE w->state.sent
#define ovl667_D_80118F20 w->state.ring
#define ovl667_D_80118F10 w->state.rx
#define ovl667_D_80118F80 w->state.tx
#define LinkLocalInputs w->state.payload
#define LinkRemoteInputs w->state.incoming
#define D_800ACBF0 w->state.outgoing
static s32 word(u32 bits){s32 v;memcpy(&v,&bits,4);return v;}
static s32 boundary(s32 kind,s32 command,s32 operation,unsigned long argument) {
 unsigned i,n=w->calls;u32 hash=2166136261U;struct Event *e;
 assert(n<40);e=&w->events[w->calls++];
 for(i=0;i<sizeof(w->state);i++)hash=(hash^((u8*)&w->state)[i])*16777619U;
 e->kind=kind;e->command=command;e->operation=operation;e->argument=argument;e->hash=hash;
 if(mutation)for(i=0;i<sizeof(w->state);i++)((u8*)&w->state)[i]=(u8)(scenario*29+n*17+i*53);
 if(kind==1 && command==3 && operation==1) {
  unsigned probe=w->probes++,repeats=(scenario>>3)%8;
  if(probe==0)return (scenario&1)?word(0x80000000U|scenario):0;
  if(probe<=repeats)return 0;
  return (scenario&2)?17:0;
 }
 if(kind==1 && command==0) {
  unsigned nstatus=w->statuses++,repeats=(scenario>>3)%8;
  u32 status=nstatus<repeats ? 1U+((scenario+nstatus)&1U) : ((scenario&4)?3U:0U);
  return word((scenario*0x9E3779B9U&~0x180U)|(status<<7));
 }
 return word(0x80000000U|scenario);
}
void SetDispMask(s32 mode){assert(mode==0);(void)boundary(0,mode,0,0);}
s32 _sio_control(s32 command,s32 operation,unsigned long argument) {
 if(command==4) {
  s32 (*registered)(s32,u32)=(s32(*)(s32,u32))argument;
  assert(operation==0 && registered==ovl667_func_8011492C);
  assert(argument>UINT32_MAX);
  assert(registered(2,24999)==1 && registered(2,25000)==0);
  assert(registered(1,UINT32_MAX)==1);
 }
 return boundary(1,command,operation,argument);
}
s32 VSync(s32 mode){assert(mode==0);return boundary(2,mode,0,0);}
'''+callback+body+r'''
static void reference(unsigned long configuration) {
 unsigned i;u32 phase;
 (void)boundary(0,0,0,0);
 (void)boundary(1,1,3,configuration);
 (void)boundary(1,1,4,4);
 (void)boundary(1,4,0,(unsigned long)ovl667_func_8011492C);
 w->state.error=0;w->state.selected=0;w->state.limit=7;
 w->state.read=0;w->state.write=0;
 w->state.timeout=0;w->state.failures=0;w->state.rxcount=0;w->state.txcount=0;
 w->state.event=0;w->state.command=0;
 w->state.received=0;w->state.sent=0;
 phase=(u32)w->state.phase;w->state.phase=word(phase&0xFFFFFF00U);
 for(i=0;i<sizeof(w->state.ring);i++)((u8*)&w->state.ring)[i]=0;
 memset(&w->state.tx,0,sizeof(w->state.tx));memset(&w->state.rx,0,sizeof(w->state.rx));
 memset(&w->state.payload,0,sizeof(w->state.payload));
 memset(&w->state.outgoing,0,sizeof(w->state.outgoing));
 memset(&w->state.incoming,0,sizeof(w->state.incoming));
 w->state.peer=0;w->state.halt=0;
 if(boundary(1,3,1,0)!=0)(void)boundary(1,3,0,1);
 else {
  (void)boundary(1,3,0,1);
  for(;;) {
   u32 status;
   if(boundary(1,3,1,0)!=0)break;
   status=(u32)boundary(1,0,0,0);
   status=(status/128)%4;
   if(status==0 || status==3)break;
  }
 }
 (void)boundary(1,5,0,0);(void)boundary(2,0,0,0);(void)boundary(1,3,0,0);
}
int main(void) {
 unsigned i;u32 endian=1;
 assert(*(u8*)&endian==1);assert(sizeof(unsigned long)==sizeof(void*));
 _Static_assert(sizeof(RrOverlay667SerialRing)==96,"genuine ring span");
 for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++) {
  unsigned long configuration;
  memset(&initial,0,sizeof(initial));
  for(i=0;i<sizeof(initial.state);i++)((u8*)&initial.state)[i]=(u8)(scenario*19+i*73);
  for(i=0;i<4;i++){initial.guard[i]=0x12345678U+i;initial.tail[i]=0x87654321U-i;}
  configuration=(scenario&16)?(unsigned long)&initial:((unsigned long)scenario<<16)|scenario;
  actual=initial;expected=initial;
  w=&actual;ovl667_func_80114AF8(configuration);
  w=&expected;reference(configuration);
  assert(memcmp(&actual,&expected,sizeof(actual))==0);
  assert(actual.calls>=9 && actual.calls<40);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            c = Path(temp) / 'serial_init.c'
            exe = Path(temp) / 'serial_init'
            c.write_text(source)
            subprocess.run(['cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror','-Wno-error=deprecated-non-prototype',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            '-I', str(root / 'include'), str(c), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
