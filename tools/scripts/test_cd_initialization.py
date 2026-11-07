"""Actual CD initialization with typed interrupt/BIOS and MMIO simulations."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class CdInitializationTest(unittest.TestCase):
    def test_native_reset_retries_irq_and_command_failures(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'tools/scripts/fixtures/views/func_8008D86C.c').read_text()
        body = re.sub(r' __asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r'__asm__(?:\s+volatile)?\(""[^;]*;','',body)
        source = r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
static u8 read8(volatile u8 *port);
static void write8(volatile u8 *port,u8 value);
static void write32(volatile u32 *port,u32 value);
#define RR_CD_READ8(port) read8(port)
#define RR_CD_WRITE8(port,value) write8(port,value)
#define RR_CD_WRITE32(port,value) write32(port,value)
#include "rr/cd_init.h"
const char CD_str_init_prefix[]="CD_init:",StrAddrFmt[]="addr=%08x\n";
static u8 metadata_buffer[3];u8 *CdDebugPtrTable=metadata_buffer;
static struct Ports {volatile u8 index,irq,enable;volatile u32 control;} ports[3];
struct State {
 u32 guard[4];u8 active,command,response[3];RrCdDriveStatus status;s32 mode;
 RrLibDataCallback callback1,callback2;
 volatile u8 *indexPort,*irqPort,*enablePort;volatile u32 *controlPort;
 struct {u8 index,irq,enable;u32 control;} device[3];unsigned pending;
 u32 tail[4];
};
static struct World {struct State s;unsigned calls;struct {u32 hash;int kind,id;u32 value;} events[64];} actual,expected,initial,*w;
static unsigned scenario,mutation;
#define D_800A64A4 w->s.active
#define D_800A64A5 w->s.command
#define CdIntrStatusBytes w->s.response
#define CD_status w->s.status
#define CD_status1 w->s.mode
#define CD_cbsync w->s.callback1
#define CD_cbready w->s.callback2
#define CD_reg0 w->s.indexPort
#define CD_reg3 w->s.irqPort
#define CD_REG2 w->s.enablePort
#define COM_DELAY w->s.controlPort
static void callback1(s32 code,u8 *data){(void)code;(void)data;}
static void callback2(s32 code,u8 *data){(void)code;(void)data;}
void CdCommandCallback(void){}
static void effect(unsigned n){
 unsigned i,b=(scenario+n)%3;u32 value=scenario*0x9e3779b9U+n*47;
 w->s.active=(u8)value;w->s.command=(u8)(value+1);w->s.mode=(s32)value;w->s.status.word=value;
 w->s.callback1=callback1;w->s.callback2=callback2;
 for(i=0;i<3;i++){
  w->s.response[i]=(u8)(value+i*53);w->s.device[i].index=(u8)value;
  w->s.device[i].irq=w->s.pending?1:0;w->s.device[i].enable=(u8)(value+3);w->s.device[i].control=value+i;
 }
 w->s.indexPort=&ports[b].index;w->s.irqPort=&ports[b].irq;
 w->s.enablePort=&ports[(b+1)%3].enable;w->s.controlPort=&ports[(b+2)%3].control;
}
static void event(int kind,int id,u32 value){
 unsigned i,n=w->calls;u32 hash=2166136261U;assert(n<64);
 for(i=0;i<sizeof(w->s);i++)hash=(hash^((u8*)&w->s)[i])*16777619U;
 w->events[n].hash=hash;w->events[n].kind=kind;w->events[n].id=id;w->events[n].value=value;w->calls++;
}
static void service(int kind,int id,u32 value){unsigned n=w->calls;event(kind,id,value);if(mutation)effect(n);}
static unsigned port8(volatile u8 *p,unsigned *field){
 unsigned b;assert((uintptr_t)p>UINT32_MAX);
 for(b=0;b<3;b++){
  if(p==&ports[b].index){*field=0;return b;}
  if(p==&ports[b].irq){*field=1;return b;}
  if(p==&ports[b].enable){*field=2;return b;}
 }
 assert(0);return 0;
}
static u8 read8(volatile u8 *p){unsigned f,b=port8(p,&f);u8 v;assert(f==1);v=w->s.device[b].irq;event(10,b*3+f,v);return v;}
static void write8(volatile u8 *p,u8 value){
 unsigned f,b=port8(p,&f);event(11,b*3+f,value);
 if(f==0)w->s.device[b].index=value;
 else if(f==2)w->s.device[b].enable=value;
 else{
  if(value==7 && w->s.pending)w->s.pending--;
  w->s.device[b].irq=(value==7 && w->s.pending)?1:0;
  if(value==7 && mutation){
   unsigned next=(b+1)%3;w->s.irqPort=&ports[next].irq;w->s.indexPort=&ports[next].index;
   w->s.device[next].irq=w->s.pending?1:0;
  }
 }
}
static void write32(volatile u32 *p,u32 value){unsigned b;for(b=0;b<3;b++)if(p==&ports[b].control)break;assert(b<3);event(12,b,value);w->s.device[b].control=value;}
s32 PsyqPuts(const char *message){assert(message==CD_str_init_prefix);service(1,0,0);return -1;}
s32 DebugPrintStub(const char *format,...){
 va_list args;u32 address;assert(format==StrAddrFmt);va_start(args,format);address=va_arg(args,u32);va_end(args);
 assert(address==(u32)(uintptr_t)&CdDebugPtrTable);service(2,0,address);return 1;
}
void ResetCallback(void){
 assert(w->s.active==0 && w->s.command==0 && w->s.callback1==0 && w->s.callback2==0 && w->s.status.word==0 && w->s.mode==0);
 service(3,0,0);
}
RrLibInterruptCallback InterruptCallback(s32 channel,RrLibInterruptCallback CdCommandCallback){assert(channel==2 && CdCommandCallback==CdCommandCallback);service(4,channel,0);return 0;}
s32 CD_cw(s32 command,const u8 *params,u8 *result,s32 mode){
 unsigned plan=(scenario>>1)%4;assert(params==0 && result==0 && mode==0);service(5,command,0);
 if(command==1){w->s.status.word=(scenario*0x1234567U&~16U)|((scenario&1)?16U:0U);return -1;}
 if(command==10)return plan==0?-1:0;
 assert(command==12);return plan==1?1:0;
}
s32 CD_sync(s32 mode,u8 *status){assert(mode==0 && status==0);service(6,0,0);return ((scenario>>1)%4)==2?2:3;}
''' + body + r'''
static s32 reference(void){
 PsyqPuts(CD_str_init_prefix);DebugPrintStub(StrAddrFmt,(u32)(uintptr_t)&CdDebugPtrTable);
 w->s.command=0;w->s.active=0;w->s.callback2=0;w->s.callback1=0;w->s.mode=0;w->s.status.word=0;
 ResetCallback();InterruptCallback(2,CdCommandCallback);
 write8(w->s.indexPort,1);
 while(read8(w->s.irqPort)&7){write8(w->s.indexPort,1);write8(w->s.irqPort,7);write8(w->s.enablePort,7);}
 w->s.response[2]=0;w->s.response[1]=0;w->s.response[0]=2;
 write8(w->s.indexPort,0);write8(w->s.irqPort,0);write32(w->s.controlPort,0x1325);
 (void)CD_cw(1,0,0,0);
 if(w->s.status.word&16)(void)CD_cw(1,0,0,0);
 if(CD_cw(10,0,0,0)!=0)return -1;
 if(CD_cw(12,0,0,0)!=0)return -1;
 return CD_sync(0,0)==2?0:-1;
}
int main(void){unsigned i;s32 got,want;
 for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.pending=(scenario>>4)%4;effect(9);
  for(i=0;i<4;i++){w->s.guard[i]=0x12345678U+i;w->s.tail[i]=0x87654321U-i;}
  actual=initial;expected=initial;w=&actual;got=CD_init();w=&expected;want=reference();
  assert(got==want && memcmp(&actual,&expected,sizeof(actual))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as directory:
            p = Path(directory)
            (p / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(p / 'test.c'), '-o', str(p / 'test')], check=True)
            subprocess.run([str(p / 'test')], check=True)


if __name__ == '__main__':
    unittest.main()
