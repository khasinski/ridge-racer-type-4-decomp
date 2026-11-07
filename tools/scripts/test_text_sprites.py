"""Actual text emitter, PAL ROM tables, aliasing and full native pointers under UBSan."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT = Path(__file__).resolve().parents[2]

class TextSpritesTest(unittest.TestCase):
    def test_native_emitter(self):
        def header(name):
            return re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '',
                          (ROOT/'include/rr'/name).read_text(), flags=re.M)
        body = (ROOT/'tools/scripts/fixtures/legacy/func_8004C5A4.c').read_text()
        body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
        body = re.sub(r' asm\("\$\d+"\)', '', body)
        body = re.sub(r'__asm__\("".*?\);', ';', body)
        assert body.count('(RrGpuScratchPointers *)0x1F800000U') == 2
        body = body.replace('(RrGpuScratchPointers *)0x1F800000U', '&actual->scratch')
        exe = (ROOT/'assets/PAL/main.exe').read_bytes()
        def table(address, size):
            start = 0x800 + address - 0x80010000
            return ','.join(str(x) for x in exe[start:start+size])
        code = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + ''.join(header(n) for n in ['gpu_scratch_types.h','gpu.h','animated_sprite_grid.h','icon_sprites.h','text_sprites.h']) + r'''
typedef struct {
 RrGpuScratchPointers scratch;
 RrIconSpriteContext *context;
 u8 *width_base;
 u8 guard[32];
 u8 arena[4096] __attribute__((aligned(8)));
 u8 mapping[288],widths[256],text[64],suffix[32];
} World;
static World *actual,*expected;
#define FontGlyphMapAsciiBase actual->mapping
#define FontGlyphWidths actual->width_base
#define CurrentDrawBuf actual->context
''' + body + r'''
static const u8 original_mapping[288]={''' + table(0x800A015C,288) + r'''};
static const u8 original_widths[256]={''' + table(0x800A01DC,256) + r'''};
static u32 seed=0x9546bc13;
static u32 random32(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 signed32(u32 n){s32 result;memcpy(&result,&n,4);return result;}
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
static void reference(s32 x,s32 y,const u8 *text,s32 color,s32 bank) {
 u8 *p=expected->scratch.cursor;u32 current=(u32)x;unsigned iterations=0;
 while(read_value(text,1)) {
  assert(++iterations<128);
  unsigned glyph=read_value(actual->mapping+read_value(text,1),1);text++;
  if(glyph==255){current+=4;continue;}
  write_value(p+3,4,1);write_value(p+7,0x64,1);
  unsigned gx=glyph%21*12,gy=glyph/21*12+188;
  if(color==255)write_value(p+7,0x65,1);
  else {write_value(p+7,0x66,1);for(int j=4;j<=6;j++)write_value(p+j,(u32)color,1);}
  const u8 *width=expected->width_base+glyph;
  write_value(p+8,current,2);write_value(p+10,(u32)y,2);
  unsigned captured_width=read_value(width,1);
  write_value(p+18,12,2);write_value(p+14,0x7F78,2);
  write_value(p+12,gx,1);write_value(p+13,gy,1);
  u8 *head=(u8 *)expected->context+0x70+bank*4;
  write_value(p+16,captured_width,2);
  write_value(p,(read_value(p,4)&0xFF000000U)|(read_value(head,4)&0xFFFFFFU),4);
  write_value(head,(read_value(head,4)&0xFF000000U)|((u32)(uintptr_t)p&0xFFFFFFU),4);
  p+=20;current+=read_value(width,1);
 }
 expected->scratch.cursor=p;
}
int main(void) {
 actual=malloc(sizeof(*actual));expected=malloc(sizeof(*expected));assert(actual&&expected);
 static const s32 extremes[]={INT32_MIN,INT32_MAX,-65537,-32768,-1,0,1,32767,65535,65536};
 static const s32 colors[]={255,-1,0,1,256,511,INT32_MIN,INT32_MAX};
 for(unsigned n=0;n<32768;n++) {
  u8 *bytes=(u8 *)actual;for(size_t i=0;i<sizeof(*actual);i++)bytes[i]=random32();
  memcpy(actual->mapping,original_mapping,288);memcpy(actual->widths,original_widths,256);
  actual->width_base=actual->widths;
  actual->scratch.cursor=actual->arena+512;actual->scratch.order=NULL;
  actual->context=(RrIconSpriteContext *)(actual->arena+128);
  s32 bank=(s32)(n%10)-2,color=colors[n%8];
  s32 x=n&1?signed32(random32()):extremes[(n/2)%10];
  s32 y=n&2?signed32(random32()):extremes[(n/3)%10];
  const u8 *text=actual->text;unsigned length=n%25;
  for(unsigned i=0;i<length;i++)actual->text[i]=(u8)(1+random32()%255);
  actual->text[length]=0;
  if(n&4) {
   for(unsigned i=0;i<256;i++)actual->mapping[i]=random32();
   for(unsigned i=0;i<256;i++)actual->widths[i]=random32();
  }
  /* Force every nonzero character through the complete u8 mapping domain. */
  if((n&31)==0) {actual->text[0]=1+(n/32)%255;actual->text[1]=0;}
  if((n&15)==1) {
   /* Selected head aliases an existing packet tag, color or UV word. */
   unsigned offset=((n/16)%4)*4;
   actual->context=(RrIconSpriteContext *)(actual->scratch.cursor+offset-0x70-bank*4);
  }
  if((n&15)==2) {
   /* Width loads see coordinate, CLUT or queue writes before the second read. */
   text=actual->text;actual->text[0]=1;actual->text[1]=1;actual->text[2]=0;
   unsigned glyph=(n/16)%255;actual->mapping[1]=glyph;
   unsigned offset=((n/128)%5)*4;
   actual->width_base=actual->scratch.cursor+offset-glyph;
  }
  if((n&15)==3) {
   /* The first output packet overwrites text, including its original NUL. */
   text=actual->scratch.cursor+1;
   ((u8 *)text)[0]=1;((u8 *)text)[1]=1;((u8 *)text)[2]=0;
   actual->scratch.cursor[16]=0;actual->scratch.cursor[17]=0;
   actual->mapping[1]=0;
  }
  memcpy(expected,actual,sizeof(*actual));
  reference(x,y,text,color,bank);DrawFontString(x,y,text,color,bank);
  if(memcmp(actual,expected,sizeof(*actual))) {fprintf(stderr,"case %u failed\n",n);abort();}
 }
 free(actual);free(expected);puts("32768 native text-emitter cases passed with UBSan");
 return 0;
}
'''
        with TemporaryDirectory() as directory:
            source=Path(directory)/'text.c';binary=Path(directory)/'text'
            source.write_text(code)
            subprocess.run(['cc','-std=gnu99','-O2','-fsanitize=undefined',
                            '-fno-sanitize-recover=all','-I',str(ROOT/'include'),
                            str(source),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)

if __name__=='__main__': unittest.main()
