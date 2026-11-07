"""Run the ramp and real channel getter against a wrapped-integer state oracle.

Only the unconverted setter is simulated. Its callback may replace a table root.
The old getter's PAL page is mapped into a complete native pointer-slot owner.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import re, subprocess, unittest
ROOT=Path(__file__).resolve().parents[2]

class ChannelVolumeRampTest(unittest.TestCase):
 def test_wrapping_ramps_and_mutable_table_roots(self):
  body=(ROOT/'tools/scripts/fixtures/views/func_80082FC4.c').read_text()
  body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
  body=re.sub(r' __asm__\("\$\d+"\)','',body)
  body=re.sub(r'__asm__\(""[^;]*;', '',body)
  getter=(ROOT/'src/main/PAL/psyq/libsnd/vmchval.c').read_text().split('s32 _SsVmGetSeqLVol')[0]
  getter=re.sub(r'^#include[^\n]*\n','',getter,flags=re.M)
  getter=re.sub(r' __asm__\("\$\d+"\)','',getter)
  getter=re.sub(r'__asm__(?: volatile)?\(""[^;]*;', '',getter)
  getter=getter.replace('0x80110000U','(actual.slots+998)').replace('_SsVmGetSeqVol','real_getter')
  code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/channels.h"
#define BANKS 4
#define ROWS 8
#define SLOTS (998+BANKS)
typedef struct {u32 guard[4]; RrChannelValueRecordB0 records[2][BANKS][ROWS];
 u32 tail[4];RrChannelValueRecordB0 *slots[SLOTS];s16 last;} World;
static World actual,expected;
typedef struct {World state;s32 id;u16 left,right;unsigned kind;} Event;
static Event events[3];static unsigned eventCount,eventIndex,mutate;
static u32 seed=0x82fc4;
static u32 next(void){u32 x=seed;x^=x<<13;x^=x>>17;x^=x<<5;return seed=x;}
static s32 signed32(u32 x){s32 y;memcpy(&y,&x,4);return y;}
static s16 signed16(u16 x){s16 y;memcpy(&y,&x,2);return y;}
static s32 wrap(int64_t x){return signed32((u32)x);}
static void check(const World *a,const World *e){unsigned i;
 assert(!memcmp(a->guard,e->guard,sizeof(a->guard)));
 assert(!memcmp(a->records,e->records,sizeof(a->records)));
 assert(!memcmp(a->tail,e->tail,sizeof(a->tail)));assert(a->last==e->last);
 for(i=0;i<SLOTS;i++)assert((uintptr_t)a->slots[i]-(uintptr_t)&actual == (uintptr_t)e->slots[i]-(uintptr_t)&expected);
}
#define _ss_score (actual.slots)
#define SvmLastChannel (actual.last)
'''+getter+r'''
static void snapshot(unsigned kind,s32 id,u16 left,u16 right){Event *e;assert(eventCount<3);
 e=&events[eventCount++];memcpy(&e->state,&expected,sizeof(expected));e->kind=kind;e->id=id;e->left=left;e->right=right;}
static void enter(unsigned kind,s32 id,u16 left,u16 right){Event *e;assert(eventIndex<eventCount);
 e=&events[eventIndex++];assert(e->kind==kind&&e->id==id);if(kind)assert(e->left==left&&e->right==right);check(&actual,&e->state);}
static void set(World *w,s32 id,u16 left,u16 right){unsigned bank=(u32)id&255,row=((u32)id>>8)&255;
 RrChannelValueRecordB0 *r=&w->slots[bank][row];r->value58=left;r->value5A=right;
 if(mutate){w->slots[bank]=w->records[1][bank];w->records[1][bank][row].mode98=wrap((int64_t)w->records[1][bank][row].mode98 ^ 0x12345678);}}
s32 _SsVmGetSeqVol(s32 id,u16 *left,u16 *right){enter(0,id,0,0);return real_getter(id,left,right);}
void _SsVmSetSeqVol(s32 id,u16 left,u16 right,s32 mode){assert(mode==1);enter(1,id,left,right);set(&actual,id,left,right);}
'''+body+r'''
static void get_model(s32 id,u16 *left,u16 *right){unsigned b=(u32)id&255,r=((u32)id>>8)&255;
 snapshot(0,id,0,0);expected.last=signed16((u16)id);*left=expected.slots[b][r].value58;*right=expected.slots[b][r].value5A;}
static unsigned paths[6];
static void model(unsigned bank,unsigned row){RrChannelValueRecordB0 *original=&expected.slots[bank][row];
 s32 elapsed=wrap((int64_t)original->rampElapsedA0+1),delta;u16 left,right;s32 l,r;
 s32 id=signed16((u16)(bank+256*row));original->rampElapsedA0=elapsed;
 if(elapsed>original->rampDuration9C){expected.slots[bank][row].mode98&=~16;paths[0]++;}
 else{int64_t product=wrap((int64_t)original->rampDelta48*elapsed);
 assert(original->rampDuration9C!=0);assert(!(product==INT32_MIN&&original->rampDuration9C==-1));
 delta=wrap(product/original->rampDuration9C-original->rampApplied4A);
 if(!delta)paths[1]++;
 else{original->rampApplied4A=signed16((u16)((int64_t)original->rampApplied4A+delta));
 get_model(id,&left,&right);l=wrap((int64_t)left+delta);r=wrap((int64_t)right+delta);
 l=l<0?0:l>127?127:l;r=r<0?0:r>127?127:r;
 snapshot(1,id,(u16)l,(u16)r);set(&expected,id,(u16)l,(u16)r);
 if((l==0&&r==0)||(l==127&&r==127)){expected.slots[bank][row].mode98&=~16;paths[l==0?2:3]++;}
 else paths[4]++;if(mutate)paths[5]++;
 }}
 get_model(id,&original->volume5C,&original->volume5E);
}
int main(void){unsigned n,i,b,row,bank;uint64_t cases=0;
 _Static_assert(sizeof(RrChannelValueRecordB0)==176,"record owner");
 _Static_assert(offsetof(RrChannelValueRecordB0,rampDelta48)==72,"delta");
 _Static_assert(offsetof(RrChannelValueRecordB0,rampApplied4A)==74,"applied");
 _Static_assert(offsetof(RrChannelValueRecordB0,volume5C)==92,"output left");
 _Static_assert(offsetof(RrChannelValueRecordB0,volume5E)==94,"output right");
 _Static_assert(offsetof(RrChannelValueRecordB0,rampDuration9C)==156,"duration");
 _Static_assert(offsetof(RrChannelValueRecordB0,rampElapsedA0)==160,"elapsed");
 assert((uintptr_t)actual.slots>UINT32_MAX);
 for(n=0;n<262144;n++){
  unsigned char *raw=(unsigned char*)&actual;
  for(i=0;i<sizeof(actual);i++)raw[i]=(u8)next();
  for(i=0;i<SLOTS;i++)actual.slots[i]=actual.records[(i>>2)&1][i%BANKS];
  for(b=0;b<BANKS;b++)actual.slots[b]=actual.records[0][b];
  bank=next()%BANKS;row=next()%ROWS;mutate=(n>>3)&1;
  {RrChannelValueRecordB0 *r=&actual.records[0][bank][row];
   switch(n&7){
   case 0:r->rampDuration9C=0;r->rampElapsedA0=(s32)(next()&INT32_MAX);if(r->rampElapsedA0==INT32_MAX)r->rampElapsedA0=0;break;
   case 1:r->rampDuration9C=1;r->rampElapsedA0=0;r->rampDelta48=0;r->rampApplied4A=0;break;
   case 2:r->rampDuration9C=1;r->rampElapsedA0=0;r->rampDelta48=32767;r->rampApplied4A=-32768;break;
   case 3:r->rampDuration9C=1;r->rampElapsedA0=0;r->rampDelta48=-32768;r->rampApplied4A=32767;r->value58=0;r->value5A=127;break;
   case 4:r->rampDuration9C=INT32_MAX;r->rampElapsedA0=INT32_MAX;break;
   case 5:r->rampDuration9C=32768;r->rampElapsedA0=16383;r->rampDelta48=1;r->rampApplied4A=-1;r->value58=35;r->value5A=64;break;
   default:break;
   }
   {s32 elapsed=wrap((int64_t)r->rampElapsedA0+1);s32 product=wrap((int64_t)r->rampDelta48*elapsed);
    if(elapsed<=r->rampDuration9C&&(r->rampDuration9C==0||(product==INT32_MIN&&r->rampDuration9C==-1)))r->rampDuration9C=1;}
  }
  memcpy(&expected,&actual,sizeof(actual));
  for(i=0;i<SLOTS;i++)expected.slots[i]=(RrChannelValueRecordB0 *)((unsigned char*)&expected+((unsigned char*)actual.slots[i]-(unsigned char*)&actual));
  eventCount=eventIndex=0;model(bank,row);
  _SsSndCrescendo(signed32((next()&0xffff0000U)|bank),signed32((next()&0xffff0000U)|row));
  assert(eventIndex==eventCount);check(&actual,&expected);cases++;
 }
 for(i=0;i<6;i++)assert(paths[i]);printf("%llu cases\n",(unsigned long long)cases);return 0;
}
'''
  with TemporaryDirectory() as d:
   p=Path(d);(p/'test.c').write_text(code)
   subprocess.run(['clang','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I'+str(ROOT/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True,capture_output=True,text=True)
   result=subprocess.run([str(p/'test')],check=True,capture_output=True,text=True)
   self.assertEqual(result.stdout.strip(),'262144 cases')

if __name__=='__main__':unittest.main()
