"""Actual IRQ dispatcher vs an independent queued-event oracle, under UBSan.

The fixture models I_STAT write-zero-to-clear, callback effects, diagnostics,
register-root changes and a returning BIOS service. No physical BIOS/IRQ runs.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import re, subprocess, unittest
ROOT=Path(__file__).resolve().parents[2]

class InterruptDispatchTest(unittest.TestCase):
 def test_callback_reloads_ack_order_and_stall_counter(self):
  body=(ROOT/'tools/scripts/fixtures/legacy/func_8008B5D0.c').read_text()
  body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
  body=re.sub(r' __asm__\("\$\d+"\)','',body)
  body=re.sub(r'__asm__\(""[^;]*;', '',body)
  wrappers='\n'.join(f'static void callback{i}(void){{call_callback({i});}}' for i in range(22))
  code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#define RR_IRQ_ACK16(port,value) ack_actual((port),(u16)(value))
#include "rr/interrupt_dispatch.h"
typedef struct {u16 status,statusUpper,mask,maskUpper;u32 guard;} Registers;
typedef struct {u32 guard[4];RrLibSavedState saved;Registers regs[2];
 volatile u16 *statusRoot,*maskRoot;s32 stalled;
 u32 mode,acks,callbacks,prints,bios,injected,inactiveReturn;u32 tail[4];} World;
static World actual,expected;
const char UnexpectedInterruptFmt[]="inactive interrupt";
const char IntrTimeoutFmt[]="unhandled interrupt";
typedef struct {World state;unsigned kind,first,second,third;} Event;
static Event events[128];static unsigned count,cursor;
static u32 seed=0x8b5d0;static unsigned paths[12],pendingSeen[2048],modeSeen[256];
static void call_callback(unsigned id);
'''+wrappers+r'''
static RrLibInterruptCallback callbacks[22]={'''+','.join('callback'+str(i) for i in range(22))+r'''};
static u32 next(void){u32 x=seed;x^=x<<13;x^=x>>17;x^=x<<5;return seed=x;}
static s32 signed32(u32 x){s32 y;memcpy(&y,&x,4);return y;}
static s32 increment(s32 x){return signed32((u32)((int64_t)x+1));}
static unsigned port_id(const World *w,const volatile u16 *p){unsigned i;
 for(i=0;i<2;i++)if(p==&w->regs[i].status)return i;assert(0);return 0;}
static unsigned mask_id(const World *w,const volatile u16 *p){unsigned i;
 for(i=0;i<2;i++)if(p==&w->regs[i].mask)return i;assert(0);return 0;}
static unsigned callback_id(RrLibInterruptCallback p){unsigned i;for(i=0;i<22;i++)if(p==callbacks[i])return i;assert(0);return 0;}
static void compare(const World *a,const World *e){
 assert(!memcmp(a->guard,e->guard,sizeof(a->guard)));assert(!memcmp(&a->saved,&e->saved,sizeof(a->saved)));
 assert(!memcmp(a->regs,e->regs,sizeof(a->regs)));assert(!memcmp(a->tail,e->tail,sizeof(a->tail)));
 assert(port_id(&actual,a->statusRoot)==port_id(&expected,e->statusRoot));
 assert(mask_id(&actual,a->maskRoot)==mask_id(&expected,e->maskRoot));
 assert(a->stalled==e->stalled&&a->mode==e->mode&&a->acks==e->acks&&a->callbacks==e->callbacks);
 assert(a->prints==e->prints&&a->bios==e->bios&&a->injected==e->injected&&a->inactiveReturn==e->inactiveReturn);
}
static void emit(unsigned kind,unsigned first,unsigned second,unsigned third){Event *e;assert(count<128);e=&events[count++];
 memcpy(&e->state,&expected,sizeof(expected));e->kind=kind;e->first=first;e->second=second;e->third=third;}
static void enter(unsigned kind,unsigned first,unsigned second,unsigned third){Event *e;assert(cursor<count);e=&events[cursor++];
 assert(e->kind==kind&&e->first==first&&e->second==second&&e->third==third);compare(&actual,&e->state);}
static void acknowledge(World *w,unsigned bank,u16 value){unsigned bit=0;u16 cleared=(u16)~value;
 w->regs[bank].status&=value;w->acks++;
 if(cleared&&!(cleared&(u16)(cleared-1))){while(((u32)cleared>>bit)!=1)bit++;assert(bit<11);
  if(w->acks==1){
   if(w->mode&1)w->saved.callbacks[bit]=callbacks[bit+11];
   if(w->mode&2)w->saved.callbacks[(bit+1)%11]=0;
  }
 }
}
static void callback_effect(World *w,unsigned id){unsigned bit=id%11;w->callbacks++;
 assert(w->callbacks<64);
 if(w->callbacks==1){
  if(w->mode&8)w->statusRoot=&w->regs[1].status;
  if(w->mode&16)w->maskRoot=&w->regs[1].mask;
  if(w->mode&32)w->saved.enabled_mask=0;
  if(w->mode&64){w->saved.dispatching02=-17;w->saved.register32=signed32((u32)w->saved.register32^0xa5a5a5a5U);}
 }
 if((w->mode&4)&&!w->injected){*w->statusRoot|=(u16)(1U<<(bit?bit-1:10));w->injected=1;}
}
static void print_effect(World *w,unsigned message){w->prints++;
 if(!message)w->inactiveReturn=1;
 else if(w->mode&128)w->statusRoot=&w->regs[1-port_id(w,w->statusRoot)].status;
}
static void bios_effect(World *w){w->bios++;
 if(w->inactiveReturn){w->inactiveReturn=0;
  if(w->mode&128){w->saved.active=0x8001;w->statusRoot=&w->regs[1].status;
   w->saved.enabled_mask=(u16)(((w->saved.enabled_mask<<1)|(w->saved.enabled_mask>>10))&2047);}
 }
}
#define IntrEnv (actual.saved)
#define I_STAT (actual.statusRoot)
#define I_MASK (actual.maskRoot)
#define IntrStuckCount (actual.stalled)
static void ack_actual(volatile u16 *p,u16 value){unsigned bank=port_id(&actual,p);enter(0,bank,value,0);acknowledge(&actual,bank,value);}
static void call_callback(unsigned id){enter(1,id,0,0);callback_effect(&actual,id);}
s32 DebugPrintStub(const char *format,...){va_list ap;unsigned message,first,second=0;
 assert(format==UnexpectedInterruptFmt||format==IntrTimeoutFmt);message=format==IntrTimeoutFmt;
 va_start(ap,format);first=(unsigned)va_arg(ap,int);if(message)second=(unsigned)va_arg(ap,int);va_end(ap);
 enter(2,message,first,second);print_effect(&actual,message);return 1;}
void ReturnFromException(void){enter(3,0,0,0);bios_effect(&actual);}
'''+body+r'''
static void model(void){unsigned batch,repeat=0,i,bank;u16 pending;s32 old;
 if(!expected.saved.active){paths[0]++;emit(2,0,*expected.statusRoot,0);print_effect(&expected,0);
  emit(3,0,0,0);bios_effect(&expected);}
 expected.saved.dispatching02=1;
 batch=*expected.statusRoot & *expected.maskRoot & expected.saved.enabled_mask;
 assert(batch<2048);pendingSeen[batch]++;
 while(batch){assert(++repeat<16);pending=(u16)batch;
  for(i=0;i<11;i++)if(pending&(1U<<i)){
   RrLibInterruptCallback cb;bank=port_id(&expected,expected.statusRoot);
   emit(0,bank,(u16)(65535U-(1U<<i)),0);acknowledge(&expected,bank,(u16)(65535U-(1U<<i)));
   cb=expected.saved.callbacks[i];if(cb){unsigned id=callback_id(cb);emit(1,id,0,0);callback_effect(&expected,id);paths[1]++;}
   else paths[2]++;
  }
  batch=*expected.statusRoot & *expected.maskRoot & expected.saved.enabled_mask;assert(batch<2048);
 }
 if(repeat>1)paths[3]++;
 if(*expected.statusRoot & *expected.maskRoot){
  old=expected.stalled;expected.stalled=increment(old);
  if(old>2048){paths[4]++;emit(2,1,*expected.statusRoot,*expected.maskRoot);print_effect(&expected,1);
   expected.stalled=0;bank=port_id(&expected,expected.statusRoot);emit(0,bank,0,0);acknowledge(&expected,bank,0);
  }else paths[5]++;
 }else{paths[6]++;expected.stalled=0;}
 expected.saved.dispatching02=0;emit(3,0,0,0);bios_effect(&expected);
 if(expected.callbacks&&expected.mode&8)paths[7]++;
 if(expected.callbacks&&expected.mode&16)paths[8]++;
 if(expected.injected)paths[9]++;
 if(expected.callbacks&&expected.mode&32)paths[10]++;
 if(expected.mode&128&&expected.prints)paths[11]++;
}
int main(void){unsigned n,i,pattern;uint64_t cases=0;
 static const s32 counters[10]={-1,0,1,2047,2048,2049,2050,INT32_MIN,INT32_MAX,0x7ffff800};
 _Static_assert(offsetof(Registers,mask)==4,"hardware register spacing");
 _Static_assert(offsetof(RrLibSavedState,callbacks)==8,"native callback alignment");
 _Static_assert(sizeof(((RrLibSavedState*)0)->callbacks)/sizeof(RrLibInterruptCallback)==11,"full callback owner");
 _Static_assert(offsetof(RrLibSavedState,enabled_mask)==8+11*sizeof(RrLibInterruptCallback),"native mask");
 _Static_assert(sizeof(RrLibSavedState)==112,"complete native saved state");
 assert((uintptr_t)&actual>UINT32_MAX);assert((uintptr_t)callback0>UINT32_MAX);
 for(n=0;n<524288;n++){
  unsigned char *p=(unsigned char*)&actual;for(i=0;i<sizeof(actual);i++)p[i]=(u8)next();
  pattern=(n>>8)&2047;actual.mode=n&255;modeSeen[actual.mode]++;
  actual.acks=actual.callbacks=actual.prints=actual.bios=actual.injected=actual.inactiveReturn=0;
  actual.saved.active=(next()&7)?(u16)(next()|1):0;
  actual.saved.enabled_mask=(n&1)?2047:(u16)(next()&2047);
  actual.regs[0].status=(u16)(pattern|(next()&0xf800));actual.regs[0].mask=65535;
  actual.regs[1].status=(u16)(((pattern<<1)|(pattern>>10))&2047);actual.regs[1].status|=(u16)(next()&0xf800);
  actual.regs[1].mask=(n&2)?65535:(u16)next();
  actual.statusRoot=&actual.regs[0].status;actual.maskRoot=&actual.regs[0].mask;
  actual.stalled=(n&4)?signed32(next()):counters[(n>>3)%10];
  for(i=0;i<11;i++)actual.saved.callbacks[i]=(next()&3)?callbacks[(n+i)%22]:0;
  memcpy(&expected,&actual,sizeof(actual));expected.statusRoot=&expected.regs[0].status;expected.maskRoot=&expected.regs[0].mask;
  count=cursor=0;model();trapIntr();assert(cursor==count);compare(&actual,&expected);cases++;
 }
 for(i=0;i<2048;i++)assert(pendingSeen[i]);for(i=0;i<256;i++)assert(modeSeen[i]);for(i=0;i<12;i++)assert(paths[i]);
 printf("%llu cases\n",(unsigned long long)cases);return 0;
}
'''
  with TemporaryDirectory() as d:
   p=Path(d);(p/'test.c').write_text(code)
   r=subprocess.run(['clang','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I'+str(ROOT/'include'),str(p/'test.c'),'-o',str(p/'test')],capture_output=True,text=True)
   self.assertEqual(r.returncode,0,r.stderr)
   r=subprocess.run([str(p/'test')],capture_output=True,text=True)
   self.assertEqual(r.returncode,0,r.stderr)
   self.assertEqual(r.stdout.strip(),'524288 cases')

if __name__=='__main__':unittest.main()
