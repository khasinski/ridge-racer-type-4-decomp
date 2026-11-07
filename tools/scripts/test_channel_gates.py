"""Actual gate selector, native byte aliases and wrapped budgets, UBSan."""
from pathlib import Path
import re,subprocess,tempfile,unittest
root=Path(__file__).resolve().parents[2]
header=(root/'include/rr/channels.h').read_text();header=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',header,flags=re.M)
body=(root/'tools/scripts/fixtures/views/func_8009A05C.c').read_text();body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M);body=re.sub(r' asm\("\$\d+"\)','',body);body=re.sub(r'__asm__\("".*?\);',';',body)
body,n=re.subn(r'\(s32 \*\)0x800B0000U','(s32 *)(native_budget_page.bytes+0x4350)',body);assert n==2
code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "types.h"
#define RR_FIELD_OFFSET(t,f) offsetof(t,f)
'''+header+r'''
typedef struct {u8 guard0[32];RrChannelRecordF0 record;u8 data[8];RrChannelGateEntry5 entries[256];u8 guard1[32];} World;
static World actual,expected;
static union {void *align;u8 bytes[0x4358];} native_budget_page;
#define D_800ABCB0 (*(s32 *)native_budget_page.bytes)
static s32 expected_budget;
static unsigned test_seed,calls;
static void hook(RrChannelRecordF0 *r) {
 if(test_seed&0x1000) {
  r->status.pending=(test_seed>>8)&1;r->status.length34=test_seed>>5;
  r->responseE8=(test_seed>>4)&7;
 }
 /* The original special-response path requires a valid two-byte input. */
 if(!r->status.pending && (r->responseE8==4||r->responseE8==5||r->responseE8==7) && r->status.length34>=2 && !r->status.data28)r->status.data28=actual.data;
}
void PsyqBzero(void *target,s32 length) {
 assert(target==actual.record.status.gates57 && length==6 && calls==0);
 calls++;memset(target,0,6);hook(&actual.record);
}
'''+body+r'''
static u8 *map(const void *pointer) {
 ptrdiff_t offset=(const u8 *)pointer-(const u8 *)&actual;
 assert(offset>=0 && offset<sizeof(actual));return (u8 *)&expected+offset;
}
static s32 wrap(u32 x) {s32 n;memcpy(&n,&x,4);return n;}
static void reference(void) {
 RrChannelRecordF0 *r=&expected.record;int i,j,n,found,mask;s32 total;u8 *data;
 memset(r->status.gates57,0,6);hook(r);
 if(r->status.pending && r->status.data28) {
  n=r->status.length34<6?r->status.length34:6;
  for(i=0;i<r->entry_countE9;i++) {
   const u8 *entry=map(r->status.entries04)+i*5;
   found=0;mask=entry[2]?255:1;data=map(r->status.data28);
   for(j=0;j<n;j++)if(r->status.payload5D[j]==i && (data[j]&mask)){found=1;break;}
   if(!found)continue;
   total=wrap((u32)expected_budget+entry[3]);
   if(total>=61)continue;
   expected_budget=total;
   for(j=0;j<n;j++)if(r->status.payload5D[j]==i)r->status.gates57[j]=1;
  }
  return;
 }
 if((r->responseE8==4||r->responseE8==5||r->responseE8==7) && !r->status.pending && r->status.length34>=2) {
  data=map(r->status.data28);
  if((data[0]&192)==64 && (data[1]&1) && wrap((u32)expected_budget+10)<61) {
   r->status.gates57[0]=r->status.gates57[1]=1;expected_budget=wrap((u32)expected_budget+10);
  }
  return;
 }
 if(r->responseE8==3){r->status.gates57[0]=1;return;}
 if(!r->status.pending)memset(r->status.gates57,1,6);
}
static u32 seed=0x7919b8a4;
static u32 next(void) {seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
int main(void) {
 unsigned n,j;u8 *raw=(u8 *)&actual;
 s32 budgets[]={INT32_MIN,INT32_MAX,-256,-1,0,50,51,60,61,100};
 memset(&native_budget_page,0xA5,sizeof(native_budget_page));
 for(n=0;n<32768;n++) {
  test_seed=n;calls=0;
  for(j=0;j<sizeof(actual);j++)raw[j]=next();
  actual.record.status.pending=n%3?(u16)next():0;
  actual.record.status.length34=n;
  actual.record.responseE8=n/4;
  actual.record.entry_countE9=n%17==0?255:(n%17);
  actual.record.status.entries04=actual.entries;
  switch((n>>2)%4) {
  case 0:actual.record.status.data28=actual.data;break;
  case 1:actual.record.status.data28=actual.record.status.payload5D;break;
  case 2:actual.record.status.data28=actual.record.status.gates57;break;
  default:actual.record.status.data28=0;break;
  }
  if((n&31)==0) {
   actual.record.entry_countE9=1;
   actual.record.status.entries04=(RrChannelGateEntry5 *)actual.record.status.gates57;
  }
  for(j=0;j<6;j++)actual.record.status.payload5D[j]=(n&1)?next():(n+j)%17;
  D_800ABCB0=expected_budget=budgets[n%10];
  memcpy(&expected,&actual,sizeof(actual));reference();_padAllocGates(&actual.record);
  assert(calls==1 && D_800ABCB0==expected_budget && !memcmp(&actual,&expected,sizeof(actual)));
  assert(native_budget_page.bytes[4]==0xA5 && native_budget_page.bytes[0x4357]==0xA5);
 }
 puts("32768 gate cases passed with UBSan");return 0;
}
'''
class ChannelGatesTests(unittest.TestCase):
    def test_actual_gate_selection_with_ubsan(self):
        with tempfile.TemporaryDirectory() as tmp:
            c=Path(tmp)/'gate.c';exe=Path(tmp)/'gate';c.write_text(code)
            subprocess.run(['cc','-std=gnu99','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(c),'-o',str(exe)],check=True,capture_output=True)
            subprocess.run([str(exe)],check=True,capture_output=True)
if __name__=='__main__':unittest.main()
