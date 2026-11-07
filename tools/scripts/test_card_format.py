"""Actual complete card formatter: every write failure, all memory/helper snapshots."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class CardFormatTest(unittest.TestCase):
    def test_actual_body_with_all_failure_locations(self):
        source=(ROOT/'tools/scripts/fixtures/views/func_80096198.c').read_text()
        source=re.sub(r'^#include.*\n','',source,flags=re.M)
        source=re.sub(r' asm\("\$\d+"\)','',source)
        source=re.sub(r'__asm__\(""[^;]*;',';',source)
        prefix=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,s)
#define RR_ASSERT_OFFSET(t,f,o)
#include "rr/card_format.h"

_Static_assert(sizeof(RrCardWireWord)==4,"wire word");
_Static_assert(_Alignof(RrCardWireWord)==1,"wire alignment");
_Static_assert(sizeof(RrCardDirectoryWire)==32,"directory wire");
_Static_assert(sizeof(RrCardDirectoryRecord)==32,"directory owner");
_Static_assert(offsetof(RrCardDirectoryRecord,fields.next)==8,"next link");
_Static_assert(sizeof(RrCardBrokenRecord)==4,"broken owner");
_Static_assert(sizeof(RrCardFormatSector)==128,"sector owner");
typedef struct {RrCardDirectoryRecord directory[15];RrCardBrokenRecord broken[20];RrCardFormatSector scratch;u8 canary[24];} World;
_Static_assert(offsetof(World,broken)==480,"broken region");
_Static_assert(offsetof(World,scratch)==560,"sector region");
typedef struct {int kind,offset,size,sector;s32 port;World snapshot;} Entry;
static World initial,actual,expected,*w;
static Entry entries[90],saved[90];
static unsigned seed=0x6910FCAB;static int count,writes,fail_at,mutation;static s32 failure;
#define CardFormatDirRecords (w->directory)
#define CardFormatBrokenRecords (w->broken)
#define CardFormatSectorBuf (w->scratch)
static unsigned random32(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void record(int kind,void *buffer,int size,int sector,s32 port){
 assert(count<90);Entry *p=&entries[count++];memset(p,0,sizeof(*p));
 p->kind=kind;p->offset=(int)((u8*)buffer-(u8*)w);p->size=size;p->sector=sector;p->port=port;p->snapshot=*w;
}
static void mutate_other(void *buffer,int size){
 if(!mutation)return;
 u8 *p=(u8*)w;size_t low=(size_t)((u8*)buffer-p),high=low+(unsigned)size;
 /* Preserve bzero's exact postcondition and mutate only the other real owners. */
 for(size_t i=0;i<560;i++)if(i<low||i>=high)p[i]^=(u8)(count*17U+(unsigned)i*131U);
}
void PsyqBzero(void *buffer,s32 size){
 assert(size==32||size==128);record(0,buffer,size,0,0);memset(buffer,0,(size_t)size);mutate_other(buffer,size);
}
s32 _card_sector_write(s32 port,s32 sector,u8 *buffer){
 assert(buffer==w->scratch.bytes);record(1,buffer,128,sector,port);
 unsigned checksum=0;for(int j=0;j<127;j++)checksum^=buffer[j];buffer[127]=(u8)checksum;
 mutate_other(buffer,128);
 return writes++==fail_at?failure:1;
}
static void reference_clear(unsigned offset,int size){
 void *buffer=(u8*)w+offset;record(0,buffer,size,0,0);memset(buffer,0,(unsigned)size);mutate_other(buffer,size);
}
static s32 reference_write(s32 port,int sector){
 record(1,w->scratch.bytes,128,sector,port);
 unsigned checksum=0;for(unsigned i=0;i<127;i++)checksum^=((u8*)w)[560+i];((u8*)w)[687]=(u8)checksum;
 mutate_other(w->scratch.bytes,128);
 return writes++==fail_at?failure:1;
}
static int reference(s32 port){
 u8 *bytes=(u8*)w;
 for(unsigned i=0;i<15;i++){
  reference_clear(560,128);reference_clear(i*32,32);
  u32 state=160,size=0;u16 next=65535;
  memcpy(bytes+i*32,&state,4);memcpy(bytes+i*32+4,&size,4);memcpy(bytes+i*32+8,&next,2);
  memcpy(bytes+560,bytes+i*32,32);
  if(reference_write(port,(int)i+1)!=1)return 0;
 }
 for(unsigned i=0;i<20;i++){
  s32 invalid=-1;memcpy(bytes+480+i*4,&invalid,4);
  reference_clear(560,128);memcpy(bytes+560,bytes+480+i*4,4);
  if(reference_write(port,(int)i+16)!=1)return 0;
 }
 reference_clear(560,128);bytes[560]=77;bytes[561]=67;
 return reference_write(port,0)==1;
}
s32 _card_format(s32);
int main(void){
 const s32 failures[]={0,-1,2,INT32_MIN,INT32_MAX};
 unsigned completed=0,stopped[36]={0};
 assert((uintptr_t)&actual>UINT32_MAX);
 for(unsigned k=0;k<8192;k++){
  u8 *bytes=(u8*)&initial;for(size_t j=0;j<sizeof(initial);j++)bytes[j]=(u8)random32();
  fail_at=(int)(k%37)-1;failure=failures[(k/37)%5];mutation=(k/185)&1;
  s32 port=(s32)random32();if(k%13==0)port=INT32_MIN;if(k%17==0)port=INT32_MAX;
  actual=expected=initial;w=&actual;count=writes=0;memset(entries,0,sizeof(entries));
  int got=_card_format(port),n=count,number=writes;memcpy(saved,entries,sizeof(saved));
  w=&expected;count=writes=0;memset(entries,0,sizeof(entries));int wanted=reference(port);
  assert(got==wanted);assert(n==count);assert(number==writes);
  assert(!memcmp(saved,entries,sizeof(saved)));assert(!memcmp(&actual,&expected,sizeof(actual)));
  if(fail_at<0){assert(got==1&&number==36);completed++;}
  else{assert(got==0&&number==fail_at+1);stopped[fail_at]++;}
 }
 assert(completed);for(int i=0;i<36;i++)assert(stopped[i]);
 puts("8192 native card-format cases passed");
}
'''
        # Only empty hints/register constraints are stripped; native word
        # addresses retain their full width and all objects have real owners.
        with TemporaryDirectory() as directory:
            c=Path(directory)/'format.c'
            exe=Path(directory)/'format'
            c.write_text(prefix+source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined',
                            '-fno-sanitize-recover=all','-I',str(ROOT/'include'),
                            str(c),'-o',str(exe)],check=True,capture_output=True,text=True)
            result=subprocess.run([str(exe)],check=True,capture_output=True,text=True)
            self.assertIn('8192 native card-format cases passed',result.stdout)
