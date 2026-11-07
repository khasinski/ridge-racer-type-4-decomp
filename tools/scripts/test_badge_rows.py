"""Five-row renderer with fresh metadata, arbitrary full native returns and real sprite ABI."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]

class BadgeRowsTest(unittest.TestCase):
    def test_native_renderer_and_sprite_return(self):
        def header(name):
            return re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',
                          (ROOT/'include/rr'/name).read_text(),flags=re.M)
        body=(ROOT/'tools/scripts/fixtures/views/func_80030BEC.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r' asm\("\$\d+"\)','',body)
        body=re.sub(r'__asm__\("".*?\);',';',body)
        assert body.count('(u8 *)0x800B0000U')==1 and body.count('(u8 *)0x800F0000U')==1
        # Real owners provide aligned native pointer slots at0x80 and0x3B00.
        # The shifted anchors preserve the target's signed displacements.
        body=body.replace('(u8 *)0x800B0000U','(actual->context_page.bytes+0x346C)')
        body=body.replace('(u8 *)0x800F0000U','(actual->row_page.bytes+4)')
        helper=(ROOT/'tools/scripts/fixtures/views/func_80020074.c').read_text()
        helper=re.sub(r'^#include[^\n]*\n','',helper,flags=re.M)
        helper=helper.replace('void *AddSpriteWithDrawMode(', 'void *native_sprite(')
        # Give the existing unprototyped helper boundaries pointer-safe native
        # declarations; their argument counts and ignored return values agree.
        old='extern struct P *AddDrawModePrim();'
        assert helper.count(old)==1
        helper=helper.replace(old,'extern struct P *AddDrawModePrim(u32 *, void *, s32);')
        old='extern int SetSprt(), SetShadeTex(), SetSemiTrans(), AddPrim();'
        assert helper.count(old)==1
        helper=helper.replace(old,'extern int SetSprt(void *), SetShadeTex(void *, s32), SetSemiTrans(void *, s32), AddPrim(u32 *, void *);')
        exe=(ROOT/'assets/PAL/main.exe').read_bytes()
        def rom(address,size):
            start=0x800+address-0x80010000
            return ','.join(str(x) for x in exe[start:start+size])
        code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
'''+''.join(header(n) for n in ['gpu_scratch_types.h','gpu.h','ui_bar.h','animated_sprite_grid.h','icon_sprites.h','badge_row_types.h','badge_rows.h'])+r'''
typedef union {u8 bytes[0x4000];struct {u8 pad[0x80];RrIconSpriteContext *value;} slot;} ContextPage;
typedef union {u8 bytes[0x4000];struct {u8 pad[0x3B00];RrBadgeRow1C *value;} slot;} RowPage;
typedef struct {int kind;u32 *head;void *cursor;s32 args[10];} Call;
typedef struct {
 ContextPage context_page;RowPage row_page;
 RrBadgeRow1C rows[2][12];RrIconSpriteContext contexts[2];
 s8 patterns[320];RrUiUvRect uv[260];
 u8 arena[2048] __attribute__((aligned(8)));
 unsigned calls;Call trace[14];u8 suffix[32];
} World;
static World *actual,*expected;static unsigned iteration;
#define BadgePatternTable (actual->patterns+16)
#define BadgeUvTable (actual->uv+128)
'''+body+r'''
static u32 mixed(u32 x){x^=x>>16;x*=0x7feb352dU;x^=x>>15;x*=0x846ca68bU;return x^(x>>16);}
static u8 *mapped(const void *pointer,size_t size) {
 uintptr_t offset=(uintptr_t)pointer-(uintptr_t)actual;
 assert(offset<=sizeof(*actual)-size);return (u8 *)expected+offset;
}
static void hook(World *w,unsigned call) {
 if((iteration&64) && call!=99)return;
 u32 key=mixed(iteration^call*0x9E3779B9U);
 w->context_page.slot.value=&actual->contexts[(key>>4)&1];
 w->row_page.slot.value=actual->rows[(key>>8)&1]+3;
 for(int bank=0;bank<2;bank++)for(int j=0;j<12;j++) {
  u8 *p=(u8 *)&w->rows[bank][j];
  for(int k=0;k<28;k++)p[k]=mixed(key+bank*1024+j*32+k);
 }
 for(int g=-2;g<=36;g++)for(int j=0;j<8;j++) {
  int k=16+g*8+j;
  w->patterns[k]=(s8)mixed(key+k);
 }
 for(int j=0;j<260;j++) {
  u8 *p=(u8 *)&w->uv[j];for(int k=0;k<4;k++)p[k]=mixed(key+j*4+k);
 }
 w->contexts[0].head70^=key;w->contexts[1].head70^=key>>3;
}
static void *emit(World *w,int kind,u32 *head,void *cursor,const s32 *args,int count) {
 unsigned call=w->calls++;assert(call<14);
 Call *trace=&w->trace[call];trace->kind=kind;trace->head=head;trace->cursor=cursor;
 for(int j=0;j<count;j++)trace->args[j]=args[j];
 hook(w,call);
 u32 *tag=w==actual?(u32 *)cursor:(u32 *)mapped(cursor,8);
 u32 *order=w==actual?head:(u32 *)mapped(head,4);
 tag[0]=(tag[0]&0xFF000000U)|(*order&0xFFFFFFU);
 tag[1]=mixed((u32)args[0]^((u32)args[1]<<16)^call);
 *order=(*order&0xFF000000U)|((u32)(uintptr_t)cursor&0xFFFFFFU);
 if((call+iteration)%3==0)return (u8 *)&actual->rows[call&1][4]+((call+iteration)%4)*4;
 return actual->arena+64+((call*37+iteration)%320)*4;
}
void *AddTilePrim(u32 *h,void *p,s32 x,s32 y,s32 w,s32 height,s32 r,s32 g,s32 b) {
 s32 a[]={x,y,w,height,r,g,b};return emit(actual,0,h,p,a,7);
}
void *AddSpriteWithDrawMode(u32 *h,void *p,s32 x,s32 y,s32 w,s32 height,s32 u,s32 v,u16 mode,u16 clut,u16 semi,u16 color) {
 s32 a[]={x,y,w,height,u,v,mode,clut,semi,color};return emit(actual,1,h,p,a,10);
}
void *AddDrawAreaPrim(u32 *h,void *p,s32 x,s32 y,s32 w,s32 height) {
 s32 a[]={x,y,w,height};return emit(actual,2,h,p,a,4);
}
static void *reference(void *cursor,s32 style,s32 index) {
 for(int i=0;i<5;i++) {
  s8 *q=expected->patterns+16+style*8+i;s32 shift=q[4],glyph=q[0];
  RrBadgeRow1C *p=(RrBadgeRow1C *)mapped(expected->row_page.slot.value+index+i,28);
  u32 *head=&expected->context_page.slot.value->head70;
  s32 a[]={p->x08,(s32)p->y0A+p->extent0E-2,248,2,8,8,8};
  cursor=emit(expected,0,head,cursor,a,7);
  if(i<4) {
   RrUiUvRect *uv=expected->uv+128+glyph;
   p=(RrBadgeRow1C *)mapped(expected->row_page.slot.value+index+i,28);
   head=&expected->context_page.slot.value->head70;
   s32 b[]={(s32)p->x08+shift,(s32)p->y06-2,uv->w,uv->h,uv->u,uv->v,74,0x2E28,1,128};
   cursor=emit(expected,1,head,cursor,b,10);
  }
  p=(RrBadgeRow1C *)mapped(expected->row_page.slot.value+index+i,28);
  head=&expected->context_page.slot.value->head70;
  s32 c[]={p->x14,p->y16,p->w18,p->h1A};
  cursor=emit(expected,2,head,cursor,c,4);
 }
 return cursor;
}
'''+helper+r'''
static const u8 rom_patterns[320]={'''+rom(0x8009F108-16,320)+r'''};
static const u8 rom_uv[1040]={'''+rom(0x8009F074-512,1040)+r'''};
static u8 sprite_arena[512] __attribute__((aligned(8)));
static u32 sprite_head;static int mode_calls;static u16 expected_mode;
struct P *AddDrawModePrim(u32 *head,void *cursor,s32 mode) {
 assert(head==&sprite_head);
 if(mode_calls++==0){assert(cursor==sprite_arena+32 && mode==5);return (struct P *)(sprite_arena+160);}
 assert(cursor==sprite_arena+180 && mode==expected_mode);return (struct P *)(sprite_arena+288);
}
int SetSprt(void *p){((u8 *)p)[3]=4;((u8 *)p)[7]=0x64;return 0;}
int SetShadeTex(void *p,s32 x){u8 *c=(u8 *)p+7;*c=x?*c|1:*c&0xFE;return 0;}
int SetSemiTrans(void *p,s32 x){u8 *c=(u8 *)p+7;*c=x?*c|2:*c&0xFD;return 0;}
int AddPrim(u32 *head,void *p) {
 u32 *tag=p;*tag=(*tag&0xFF000000U)|(*head&0xFFFFFFU);
 *head=(*head&0xFF000000U)|((u32)(uintptr_t)p&0xFFFFFFU);return 0;
}
static void store16(u8 *p,u32 value){p[0]=value;p[1]=value>>8;}
static void check_sprite_abi(void) {
 for(unsigned n=0;n<1024;n++) {
  memset(sprite_arena,(u8)(n*13),512);sprite_head=mixed(n);mode_calls=0;expected_mode=(u16)(n*137);
  u8 expected_packet[20];memcpy(expected_packet,sprite_arena+160,20);
  s32 x=(n&1)?INT32_MIN+(s32)n:INT32_MAX-(s32)n,y=(s32)n*65537-32768;
  s32 width=0x8000+(s32)n,height=-1-(s32)n,u=n*3,v=n*7;
  u16 clut=n*71,semi=n%3,color=n%4?128:(u16)n;
  expected_packet[3]=4;expected_packet[7]=0x64|(semi?2:0);
  if(color==128)expected_packet[7]|=1;
  else {expected_packet[7]&=0xFE;expected_packet[4]=expected_packet[5]=expected_packet[6]=color;}
  store16(expected_packet+8,(u32)x);store16(expected_packet+10,(u32)y);
  store16(expected_packet+16,(u32)width);store16(expected_packet+18,(u32)height);
  expected_packet[12]=u;expected_packet[13]=v;store16(expected_packet+14,clut);
  u32 tag;memcpy(&tag,expected_packet,4);tag=(tag&0xFF000000U)|(sprite_head&0xFFFFFFU);memcpy(expected_packet,&tag,4);
  u32 expected_head=(sprite_head&0xFF000000U)|((u32)(uintptr_t)(sprite_arena+160)&0xFFFFFFU);
  void *result=native_sprite(&sprite_head,sprite_arena+32,x,y,width,height,u,v,expected_mode,clut,semi,color);
  assert(result==sprite_arena+288 && mode_calls==2 && sprite_head==expected_head);
  assert(!memcmp(sprite_arena+160,expected_packet,20));
 }
}
int main(void) {
 assert(sizeof(RrBadgeRow1C)==28);
 assert(offsetof(ContextPage,slot.value)==0x80 && offsetof(RowPage,slot.value)==0x3B00);
 actual=malloc(sizeof(*actual));expected=malloc(sizeof(*expected));assert(actual&&expected);
 for(iteration=0;iteration<32768;iteration++) {
  memset(actual,(u8)iteration,sizeof(*actual));actual->calls=0;
  hook(actual,99);
  if(iteration&64){memcpy(actual->patterns,rom_patterns,320);memcpy(actual->uv,rom_uv,1040);}
  s32 style=(s32)(iteration%21)-1,index=(s32)((iteration/5)%5)-2;
  /* Explicit signed16 extrema reach the wide helper argument sums. */
  if((iteration&7)==0)for(int b=0;b<2;b++)for(int j=0;j<12;j++) {
   actual->rows[b][j].x08=iteration&8?-32768:32767;
   actual->rows[b][j].y0A=actual->rows[b][j].extent0E=iteration&16?-32768:32767;
  }
  void *start=iteration&1?(void *)(actual->arena+32):(void *)&actual->rows[0][2];
  memcpy(expected,actual,sizeof(*actual));
  void *want=reference(start,style,index),*got=func_80030BEC(start,style,index);
  assert(got==want && actual->calls==14);
  if(memcmp(actual,expected,sizeof(*actual))){fprintf(stderr,"row case %u failed\n",iteration);abort();}
 }
 check_sprite_abi();free(actual);free(expected);
 puts("32768 renderer cases and1024 real sprite-helper ABI cases passed with UBSan");return 0;
}
'''
        with TemporaryDirectory() as directory:
            source=Path(directory)/'rows.c';binary=Path(directory)/'rows';source.write_text(code)
            subprocess.run(['cc','-std=gnu99','-O2','-fsanitize=undefined',
                            '-fno-sanitize-recover=all','-I',str(ROOT/'include'),
                            str(source),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)

if __name__=='__main__': unittest.main()
