"""Serial ring selection and transmit retry state machines under UBSan.

Receive/send services are typed simulated boundaries that mutate the live state;
this does not execute BIOS serial I/O or the receiver's packet parser.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest


class Overlay667SerialStepTest(unittest.TestCase):
    def test_ring_branches_retry_capture_and_wrapping_counters(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/667/func_{a}.c').read_text()
                           for a in ('80115068', '801151E0'))
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
static struct World {
 u32 guard[4];
 s32 peer,local,halt,command,rxcount,timeout,txcount,failures,pending,phase,serial;
 u8 read,write,selected,received,sent;
 RrOverlay667SerialRing ring;
 RrOverlay667ChoicePair incoming,outgoing,payload;
 u32 tail[4]; unsigned calls;u32 checkpoint;unsigned service;
} actual,expected,initial,*w;
static unsigned scenario,mutation;
static s32 result;
#define ovl667_D_80118ED0 w->peer
#define RaceState w->local
#define ovl667_D_80118ED4 w->halt
#define ovl667_D_80118EC8 w->command
#define ovl667_D_80118EF8 w->rxcount
#define ovl667_D_80118EFC w->timeout
#define ovl667_D_80118F00 w->txcount
#define ovl667_D_80118F04 w->failures
#define ovl667_D_80118F0C w->pending
#define ovl667_D_80118EF4 w->serial
#define ovl667_D_80118EEC w->phase
#define ovl667_D_80118F08 w->read
#define ovl667_D_80118F09 w->write
#define ovl667_D_80118F8D w->selected
#define ovl667_D_80118EED w->received
#define ovl667_D_80118EEE w->sent
#define ovl667_D_80118F20 w->ring
#define LinkRemoteInputs w->incoming
#define D_800ACBF0 w->outgoing
#define LinkLocalInputs w->payload
static const u32 edges[]={0,1,2,3,4,7,0x7FFFFFFF,0x80000000U,0xFFFFFFFFU,0x80000001U};
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static s32 inc(s32 n){uint64_t b=(uint64_t)(u32)n+1;return word((u32)b);}
static s32 service(unsigned kind) {
 unsigned i;u32 hash=2166136261U;
 for(i=0;i<offsetof(struct World,calls);i++)hash=(hash^((u8*)w)[i])*16777619U;
 assert(w->calls++==0);w->checkpoint=hash;w->service=kind;
 if(mutation) {
  w->peer=word(edges[(scenario+2)%10]);w->local=word(edges[(scenario+3)%10]);
  w->halt=word(edges[(scenario+4)%10]);w->command=(s32)((scenario>>2)%5)-1;
  w->rxcount=word(edges[(scenario+5)%10]);w->timeout=word(edges[(scenario+6)%10]);
  w->txcount=word(edges[(scenario+7)%10]);w->failures=word(edges[(scenario+8)%10]);
  w->pending=(s32)((scenario>>4)%5)-1;w->phase=word(scenario*0x9E3779B9U);
  w->serial=word(edges[(scenario+9)%10]);
  w->read=(u8)((scenario>>3)%8);w->write=(u8)((scenario>>6)%8);w->selected=(u8)((scenario>>9)%8);
  w->received=(u8)scenario;w->sent=(u8)(scenario>>8);
  for(i=0;i<sizeof(w->ring);i++)((u8*)&w->ring)[i]=(u8)(scenario+i*53);
  for(i=0;i<sizeof(w->payload);i++)((u8*)&w->payload)[i]=(u8)(scenario+i*37);
  for(i=0;i<sizeof(w->incoming);i++)((u8*)&w->incoming)[i]=(u8)(scenario+i*29);
  for(i=0;i<sizeof(w->outgoing);i++)((u8*)&w->outgoing)[i]=(u8)(scenario+i*17);
 }
 return result;
}
s32 ovl667_func_80114E34(void){return service(1);}
s32 ovl667_func_80114D30(void){return service(2);}
'''+bodies+r'''
static void reference_receive(void) {
 unsigned chosen;
 if(w->halt>0){if(w->halt==1)w->halt=2;return;}
 w->pending=1;
 if(service(1)==1) {
  w->timeout=0;w->serial=0;w->rxcount=inc(w->rxcount);
 } else {
  unsigned a=w->read,b=w->write;
  w->timeout=inc(w->timeout);
  if(a==b)chosen=(a+7)%8;
  else if(a==b+1){chosen=b;w->command=3;}
  else if(b==a+2){chosen=a;w->write=(u8)((a+9)%8);}
  else chosen=a;
  w->selected=(u8)chosen;
 }
 chosen=w->selected;assert(chosen<8);
 memcpy(w->incoming.choices[0].bytes,w->ring.packets[chosen].first04.bytes,4);
 memcpy(w->incoming.choices[1].bytes,w->ring.packets[chosen].second08.bytes,4);
 w->received=w->ring.packets[chosen].phase03;
}
static void reference_send(void) {
 s32 captured;
 if(w->peer==w->local && w->peer>=4) {if(w->halt==0)w->halt=1;return;}
 if(w->halt>0)return;
 captured=w->pending;
 if(captured!=1 && captured!=2)return;
 if(service(2)==1) {
  unsigned i;
  w->failures=0;
  for(i=0;i<8;i++)((u8*)&w->outgoing)[i]=((u8*)&w->payload)[i];
  w->txcount=inc(w->txcount);w->write=(u8)(((unsigned)w->write+1)%8);
  w->sent=(u8)(u32)w->phase;
  if(w->command==1)w->command=0;
  w->pending=0;
 } else if(captured==1)w->pending=2;
 else {
  w->pending=0;w->failures=inc(w->failures);
  if(w->command==0)w->command=1;else if(w->command==1)w->command=2;
 }
}
int main(void) {
 unsigned i,kind,r;
 static const s32 results[]={0,1,2,-1,INT32_MIN,INT32_MAX};
 _Static_assert(sizeof(RrOverlay667SerialPacket)==12,"wire record");
 _Static_assert(_Alignof(RrOverlay667SerialPacket)==1,"unaligned wire data");
 for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++) {
  for(i=0;i<sizeof(initial);i++)((u8*)&initial)[i]=(u8)(scenario*13+i*29);
  initial.peer=word(edges[(scenario>>1)%10]);
  initial.local=(scenario&1)?initial.peer:word(edges[(scenario>>4)%10]);
  initial.halt=word(edges[(scenario>>3)%10]);
  initial.pending=(s32)((scenario>>7)%7)-1;
  initial.command=(s32)((scenario>>11)%6)-1;
  initial.rxcount=initial.timeout=initial.txcount=initial.failures=word(edges[scenario%10]);
  initial.read=(u8)(scenario%8);initial.write=(u8)((scenario>>3)%8);initial.selected=(u8)((scenario>>6)%8);
  initial.calls=0;initial.checkpoint=0;initial.service=0;
  for(kind=0;kind<2;kind++)for(r=0;r<6;r++) {
   result=results[r];actual=initial;expected=initial;
   w=&actual;if(kind)ovl667_func_801151E0();else ovl667_func_80115068();
   w=&expected;if(kind)reference_send();else reference_receive();
   assert(memcmp(&actual,&expected,sizeof(actual))==0);
  }
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            c = Path(temp) / 'serial_step.c'
            exe = Path(temp) / 'serial_step'
            c.write_text(source)
            subprocess.run(['cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror','-Wno-error=deprecated-non-prototype',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            '-I', str(root / 'include'), str(c), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
