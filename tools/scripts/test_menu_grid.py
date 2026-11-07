"""Actual grid initializer with full pointer-slot ownership and overlapping packed rows."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]

class MenuGridTest(unittest.TestCase):
    def test_native_grid(self):
        header=(ROOT/'include/rr/menu_grid.h').read_text()
        header=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',header,flags=re.M)
        body=(ROOT/'tools/scripts/fixtures/views/func_80041A54.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r' asm\("\$\d+"\)','',body)
        body=re.sub(r'__asm__\("".*?\);',';',body)
        assert body.count('(u8 *)0x800F0000U')==1
        # A real page owner includes an aligned native pointer at offset3A28;
        # the function's address arithmetic stays inside this owner.
        body=body.replace('(u8 *)0x800F0000U','actual->page.bytes')
        exe=(ROOT/'assets/PAL/main.exe').read_bytes()
        start=0x800+0x8009FDA8-32-0x80010000
        table=','.join(str(x) for x in exe[start:start+256])
        code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
'''+header+r'''
typedef union {
 u8 bytes[0x4000];
 struct {u8 prefix[0x3A28];RrMenuGrid38 *records;} slots;
} Page;
typedef struct {
 Page page;
 RrMenuGrid38 records[12];
 u8 table[256];
 RrGridRow8 *source;
 u8 suffix[32];
} World;
static World *actual,*expected;
#define MenuGridLayoutRows actual->source
'''+body+r'''
static const u8 original_table[256]={'''+table+r'''};
static u32 seed=0x89b32617;
static u32 random32(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 signed32(u32 n){s32 result;memcpy(&result,&n,4);return result;}
static s16 signed16(u16 n){s16 result;memcpy(&result,&n,2);return result;}
static u8 *mapped(const void *pointer,size_t size) {
 uintptr_t offset=(uintptr_t)pointer-(uintptr_t)actual;
 assert(offset<=sizeof(*actual)-size);return (u8 *)expected+offset;
}
static u32 read_value(const void *pointer,int size) {
 u8 *p=mapped(pointer,size);u32 n=0;
 for(int i=0;i<size;i++)n|=(u32)p[i]<<(8*i);return n;
}
static void write_value(void *pointer,u32 n,int size) {
 u8 *p=mapped(pointer,size);for(int i=0;i<size;i++)p[i]=n>>(8*i);
}
static u8 *record(s32 index){return (u8 *)expected->page.slots.records+index*56;}
static void reference(s32 index,s32 layout,s32 count,s32 x,s32 y) {
 u8 *p=record(index);
 write_value(p+46,(u32)(6/count*3),1);
 write_value(record(index)+47,6,1);
 p=record(index);
 s32 sy=signed16((u16)y)/(s32)read_value(p+47,1);
 s32 denominator=signed32((u32)count*read_value(p+46,1));assert(denominator);
 s32 sx=signed16((u16)x)/denominator;
 write_value(p+44,(u32)count,2);write_value(p+4,64,2);write_value(p+6,8,2);
 write_value(p+10,(u32)sy,2);
 u8 *next=record(index);
 write_value(p+8,(u32)sx,2);
 u8 captured[8];const u8 *src=(const u8 *)expected->source+layout*8;
 for(int i=0;i<8;i++)captured[i]=read_value(src+i,1);
 for(int i=0;i<8;i++)write_value(p+48+i,captured[i],1);
 u32 divisor=read_value(next+46,1);assert(divisor);
 write_value(next+50,read_value(next+50,1)/divisor,1);
 next=record(index);divisor=read_value(next+47,1);assert(divisor);
 write_value(next+51,read_value(next+51,1)/divisor,1);
}
int main(void) {
 assert(sizeof(RrMenuGrid38)==56 && sizeof(RrGridRow8)==8);
 assert(offsetof(Page,slots.records)==0x3A28 && sizeof(Page)==0x4000);
 actual=malloc(sizeof(*actual));expected=malloc(sizeof(*expected));assert(actual&&expected);
 static const s32 counts[]={-6,-5,-4,-3,-2,-1,1,2,3,4,5,6};
 static const s32 extremes[]={INT32_MIN,INT32_MAX,-65537,-32769,-32768,-1,0,1,32767,32768,65535,65536};
 static const int overlaps[]={0,4,6,8,0x2C,0x2E,0x30,0x31,0x34};
 for(unsigned n=0;n<32768;n++) {
  u8 *bytes=(u8 *)actual;for(size_t i=0;i<sizeof(*actual);i++)bytes[i]=random32();
  if(n&1)memcpy(actual->table,original_table,256);
  actual->page.slots.records=actual->records+4;
  actual->source=(RrGridRow8 *)(actual->table+32+(n%4));
  s32 index=(s32)(n%7)-3,layout=(s32)((n/7)%7)-3,count=counts[(n/13)%12];
  s32 x=n&2?signed32(random32()):extremes[(n/3)%12];
  s32 y=n&4?signed32(random32()):extremes[(n/5)%12];
  if(n%4==0) {
   /* Source may overlap either half of the destination, even unaligned.
    * Capturing both words must precede the first destination store. */
   layout=0;
   actual->source=(RrGridRow8 *)((u8 *)(actual->page.slots.records+index)+overlaps[(n/4)%9]);
  }
  memcpy(expected,actual,sizeof(*actual));
  reference(index,layout,count,x,y);InitMenuGrid(index,layout,count,x,y);
  if(memcmp(actual,expected,sizeof(*actual))) {fprintf(stderr,"case %u failed\n",n);abort();}
 }
 free(actual);free(expected);puts("32768 native grid cases passed with UBSan");return 0;
}
'''
        with TemporaryDirectory() as directory:
            source=Path(directory)/'grid.c';binary=Path(directory)/'grid'
            source.write_text(code)
            subprocess.run(['cc','-std=gnu99','-O2','-fsanitize=undefined',
                            '-fno-sanitize-recover=all','-I',str(ROOT/'include'),
                            str(source),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)

if __name__=='__main__': unittest.main()
