"""Actual serial receive/select and transmit/retry bodies with typed BIOS mocks.

Event-ready receive packets obey the original ring-index precondition0..7.
Control bits are not stripped by the target; out-of-range flags are outside this
memory-safe domain. BIOS event/read/write and device queries are simulated.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest


class Overlay667ReceiveTest(unittest.TestCase):
    def test_checksum_queue_retry_and_real_serial_state_machines(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/667/func_{a}.c').read_text()
                           for a in ('80114E34', '80115068', '80114D30', '801151E0'))
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
 u32 guard[4],handle;
 s32 halt,request,pending,serial,rxcount,timeouts,txcount,failures,command,peer,phase,local,reader,writer;
 u8 read,write,selected,error,remote,received,sent;
 RrOverlay667SerialStorage rx,tx;
 RrOverlay667SerialRing ring;
 RrOverlay667ChoicePair payload,incoming,outgoing;
 u32 tail[4];
};
struct Event {s32 kind,args[3];u32 hash;};
static struct World {struct State s;unsigned calls;struct Event events[3];} actual,expected,initial,*w;
static unsigned scenario,mutation;
static s32 eventResult,queryResult,writeResult;
#define ovl667_D_80118ED8 w->s.handle
#define ovl667_D_80118ED4 w->s.halt
#define ovl667_D_80118F0C w->s.request
#define ovl667_D_80118EE8 w->s.pending
#define ovl667_D_80118EF4 w->s.serial
#define ovl667_D_80118EF8 w->s.rxcount
#define ovl667_D_80118EFC w->s.timeouts
#define ovl667_D_80118F00 w->s.txcount
#define ovl667_D_80118F04 w->s.failures
#define ovl667_D_80118EC8 w->s.command
#define ovl667_D_80118ED0 w->s.peer
#define ovl667_D_80118EEC w->s.phase
#define RaceState w->s.local
#define LinkReadHandle w->s.reader
#define LinkWriteHandle w->s.writer
#define ovl667_D_80118F08 w->s.read
#define ovl667_D_80118F09 w->s.write
#define ovl667_D_80118F8D w->s.selected
#define ovl667_D_80118EEF w->s.error
#define ovl667_D_80118EF0 w->s.remote
#define ovl667_D_80118EED w->s.received
#define ovl667_D_80118EEE w->s.sent
#define ovl667_D_80118F10 w->s.rx
#define ovl667_D_80118F80 w->s.tx
#define ovl667_D_80118F20 w->s.ring
#define LinkLocalInputs w->s.payload
#define LinkRemoteInputs w->s.incoming
#define LinkSentLocalInputs w->s.outgoing
static const u32 edges[]={0,1,2,0xFFFFFFFFU,0x7FFFFFFFU,0x80000000U};
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static s32 inc(s32 n){return word((u32)((uint64_t)(u32)n+1));}
static u8 checksum(const RrOverlay667SerialPacket *p) {
 unsigned i;u8 sum=0;
 for(i=0;i<4;i++)sum^=p->first04.bytes[i]^p->second08.bytes[i];
 return sum;
}
static void effect(unsigned n) {
 unsigned i;u32 bits=scenario*0x9E3779B9U+n*29;
 w->s.handle=bits;w->s.halt=word(edges[(scenario+n)%6]);
 w->s.request=(s32)((scenario+n)%5)-1;w->s.pending=word(edges[(scenario+n+1)%6]);
 w->s.serial=word(bits);w->s.rxcount=word(edges[(scenario+n+2)%6]);
 w->s.timeouts=word(edges[(scenario+n+3)%6]);w->s.txcount=word(edges[(scenario+n+4)%6]);
 w->s.failures=word(edges[(scenario+n+5)%6]);w->s.command=(s32)((scenario+n)%5)-1;
 w->s.peer=word(bits^0xABCDEF01U);w->s.phase=word(bits^0x98765432U);
 w->s.local=word(bits^0x12345678U);w->s.reader=word(bits^0xF00DFACEU);w->s.writer=word(~bits);
 w->s.read=(u8)((scenario+n)%8);w->s.write=(u8)(scenario+n);w->s.selected=(u8)((scenario+n+3)%8);
 w->s.error=(u8)((scenario+n)%3);w->s.remote=(u8)(scenario+n*7);
 w->s.received=(u8)(scenario+n*11);w->s.sent=(u8)(scenario+n*17);
 for(i=0;i<sizeof(w->s.rx);i++)((u8*)&w->s.rx)[i]=(u8)(bits+i*37);
 for(i=0;i<sizeof(w->s.tx);i++)((u8*)&w->s.tx)[i]=(u8)(bits+i*41);
 for(i=0;i<sizeof(w->s.ring);i++)((u8*)&w->s.ring)[i]=(u8)(bits+i*43);
 for(i=0;i<sizeof(w->s.payload);i++)((u8*)&w->s.payload)[i]=(u8)(bits+i*47);
 for(i=0;i<sizeof(w->s.incoming);i++)((u8*)&w->s.incoming)[i]=(u8)(bits+i*53);
 for(i=0;i<sizeof(w->s.outgoing);i++)((u8*)&w->s.outgoing)[i]=(u8)(bits+i*59);
 w->s.rx.packet.flags00=(u8)((scenario+n)%8);
 w->s.rx.packet.checksum01=checksum(&w->s.rx.packet)^((scenario+n)&1?0:128);
}
static s32 event(s32 kind,s32 a,s32 b,s32 c) {
 unsigned i,n=w->calls;u32 hash=2166136261U;struct Event *e;
 assert(n<3);e=&w->events[w->calls++];
 for(i=0;i<sizeof(w->s);i++)hash=(hash^((u8*)&w->s)[i])*16777619U;
 e->kind=kind;e->args[0]=a;e->args[1]=b;e->args[2]=c;e->hash=hash;
 if(mutation)effect(n);
 return kind==0?eventResult:kind==3?writeResult:queryResult;
}
s32 TestEvent(RrBiosEventHandle handle) {
 assert(handle==w->s.handle);return event(0,word(handle),0,0);
}
s32 FileRead(s32 descriptor,void *buffer,s32 count) {
 assert(descriptor==w->s.reader && buffer==&w->s.rx.packet && count==12);
 assert((uintptr_t)buffer>UINT32_MAX);return event(1,descriptor,count,0);
}
s32 _sio_control(s32 command,s32 operation,unsigned long argument) {
 assert(argument==0 && ((command==2&&operation==0)||(command==3&&operation==1)));
 return event(2,command,operation,0);
}
s32 FileWrite(s32 descriptor,const void *buffer,s32 count) {
 assert(descriptor==w->s.writer && buffer==&w->s.tx && count==12);
 assert((uintptr_t)buffer>UINT32_MAX);return event(3,descriptor,count,0);
}
'''+bodies+r'''
static void choose(void) {
 unsigned a=w->s.read,b=w->s.write;
 if(a==b)w->s.selected=(u8)((a+7)%8);
 else if(a==b+1){w->s.selected=(u8)b;w->s.command=3;}
 else if(b==a+2){w->s.selected=(u8)a;w->s.write=(u8)((a+9)%8);}
 else w->s.selected=(u8)a;
}
static s32 reference_receive(void) {
 if(event(0,word(w->s.handle),0,0)==1) {
  w->s.serial=0;w->s.error=checksum(&w->s.rx.packet)!=w->s.rx.packet.checksum01;
  /* Valid received records use the full flags byte as an index0..7. */
  assert(w->s.rx.packet.flags00<8);
  w->s.read=w->s.rx.packet.flags00;choose();
  memcpy(&w->s.ring.packets[w->s.read],&w->s.rx.packet,12);
  (void)event(1,w->s.reader,12,0);return 1;
 }
 if(w->s.pending!=0)return -1;
 (void)event(2,2,0,0);(void)event(1,w->s.reader,12,0);
 w->s.pending=1;w->s.rxcount=inc(w->s.rxcount);return 0;
}
static void reference_select(void) {
 unsigned index;
 if(w->s.halt>0){if(w->s.halt==1)w->s.halt=2;return;}
 w->s.request=1;
 if(reference_receive()==1){w->s.timeouts=0;w->s.serial=0;w->s.rxcount=inc(w->s.rxcount);}
 else {w->s.timeouts=inc(w->s.timeouts);choose();}
 index=w->s.selected;assert(index<8);
 memcpy(w->s.incoming.choices[0].bytes,w->s.ring.packets[index].first04.bytes,4);
 memcpy(w->s.incoming.choices[1].bytes,w->s.ring.packets[index].second08.bytes,4);
 w->s.received=w->s.ring.packets[index].phase03;
}
static s32 reference_packet_send(void) {
 unsigned i;u8 sum=0;
 if(event(2,3,1,0)==0)return 0;
 w->s.tx.packet.flags00=w->s.write|(w->s.error==1?128:0);
 for(i=0;i<8;i++) {
  u8 value=((u8*)&w->s.payload)[i];((u8*)&w->s.tx.packet)[i+4]=value;sum^=value;
 }
 w->s.tx.packet.checksum01=sum;w->s.tx.packet.status02=(u8)w->s.local;
 w->s.tx.packet.phase03=(u8)w->s.phase;
 return event(3,w->s.writer,12,0)==12?1:2;
}
static void reference_transmit(void) {
 s32 captured;
 if(w->s.peer==w->s.local && w->s.peer>=4){if(w->s.halt==0)w->s.halt=1;return;}
 if(w->s.halt>0)return;
 captured=w->s.request;if(captured!=1&&captured!=2)return;
 if(reference_packet_send()==1) {
  w->s.failures=0;memcpy(&w->s.outgoing,&w->s.payload,8);
  w->s.txcount=inc(w->s.txcount);w->s.write=(u8)(((unsigned)w->s.write+1)%8);
  w->s.sent=(u8)w->s.phase;if(w->s.command==1)w->s.command=0;w->s.request=0;
 } else if(captured==1)w->s.request=2;
 else {
  w->s.request=0;w->s.failures=inc(w->s.failures);
  if(w->s.command==0)w->s.command=1;else if(w->s.command==1)w->s.command=2;
 }
}
int main(void) {
 unsigned i,kind;s32 got,want;
 static const s32 results[]={0,1,2,-1,INT32_MIN,INT32_MAX};
 static const s32 writes[]={12,0,-1,13,INT32_MIN,12};
 for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++) {
  memset(&initial,0,sizeof(initial));w=&initial;effect(7);
  for(i=0;i<4;i++){w->s.guard[i]=0x12345678U+i;w->s.tail[i]=0x87654321U-i;}
  w->s.halt=word(edges[(scenario>>2)%6]);w->s.pending=word(edges[(scenario>>5)%6]);
  w->s.request=(s32)((scenario>>8)%5)-1;
  w->s.local=(scenario&1)?w->s.peer:word((u32)w->s.peer^0xFFFFFFFFU);
  eventResult=results[(scenario>>1)%6];queryResult=results[(scenario>>3)%6];writeResult=writes[(scenario>>7)%6];
  for(kind=0;kind<4;kind++) {
   actual=initial;expected=initial;got=want=0;
   w=&actual;
   if(kind==0)got=ovl667_func_80114E34();else if(kind==1)ovl667_func_80115068();
   else if(kind==2)ovl667_func_801151E0();else got=ovl667_func_80114D30();
   w=&expected;
   if(kind==0)want=reference_receive();else if(kind==1)reference_select();
   else if(kind==2)reference_transmit();else want=reference_packet_send();
   assert(got==want && memcmp(&actual,&expected,sizeof(actual))==0);
  }
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            c = Path(temp) / 'receive.c'
            exe = Path(temp) / 'receive'
            c.write_text(source)
            subprocess.run(['cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror','-Wno-error=deprecated-non-prototype',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            '-I', str(root / 'include'), str(c), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
