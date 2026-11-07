from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess,sys
ROOT=next(p for p in Path(__file__).resolve().parents if (p/'tools/scripts/cc.sh').is_file());P=Path(__file__).resolve().parent
source=Path(sys.argv[1]) if len(sys.argv)>1 else ROOT/'tools/scripts/fixtures/views/func_80059700.c'
body=source.read_text();body=body[body.index('void DrawGlyphList'):]
fixture=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/glyph_list.h"
_Static_assert(sizeof(RrGlyphPacket28)==28,"complete output pair");
_Static_assert(sizeof(RrSpritePacketSignedClut)==20,"sprite layout");
_Static_assert(sizeof(RrGlyphCommand8)==8,"mode layout");
_Static_assert(sizeof(RrUiUvRect)==4,"descriptor layout");
RrUiUvRect HudGlyphUvRectsBank0[50],HudGlyphUvRects[40];
RrIconSpriteContext *CurrentDrawBuf;
static RrGpuScratchPointers scratch;
#undef RR_GLYPH_SCRATCH
#define RR_GLYPH_SCRATCH (&scratch)
''' + body + r'''
enum {FRAME_SIZE=0x22778,FRAME_BYTES=FRAME_SIZE*2,MAX_GLYPHS=9};
static u8 *frame,*reference;
static s16 script[40],script_before[40];static u32 order[16];
static s32 color,bank;static unsigned count,cursorOffset,contextOffset,which;
static s16 half(u16 u){s16 s;memcpy(&s,&u,2);return s;}
static s32 word(u32 u){s32 s;memcpy(&s,&u,4);return s;}
static u32 getword(const u8 *p){u32 u;memcpy(&u,p,4);return u;}
static void putword(u8 *p,u32 u){memcpy(p,&u,4);}
static void putshort(u8 *p,u16 u){memcpy(p,&u,2);}
void setup(unsigned n){which=n;if(!frame){frame=malloc(FRAME_BYTES);reference=malloc(FRAME_BYTES);assert(frame&&reference&&(uintptr_t)frame>UINT32_MAX);}
 memset(frame,0xA5,FRAME_BYTES);memset(script,0x5A,sizeof(script));memset(order,0x3C,sizeof(order));
 color=(n&32)?(s32[]){INT32_MIN,INT32_MAX,-1,0,1,127,128,255,256}[n%9]:word(n*0x9e3779b9U);bank=(s32[]){0,1,-1,INT32_MIN,INT32_MAX}[(n/17)%5];count=(n/5)%(MAX_GLYPHS+1);
 script[0]=half(n);script[1]=(n&64)?(s16[]){INT16_MIN,INT16_MAX,-1,0,1,127,128,255}[n%8]:half((n*137U)>>3);
 for(unsigned i=0;i<50;i++)HudGlyphUvRectsBank0[i]=(RrUiUvRect){(u8)(n+i),(u8)(n/256+i*7),(u8)(n+i*11),(u8)(n/7+i*17)};
 for(unsigned i=0;i<40;i++)HudGlyphUvRects[i]=(RrUiUvRect){(u8)(n+i*3),(u8)(n/256+i*11),(u8)(n+i*13),(u8)(n/7+i*19)};
 for(unsigned i=0;i<count;i++){unsigned high=(n+i)&1;script[2+i*2]=high?0x400+(n/11+i)%40:(n/13+i)%(bank?40:50);script[3+i*2]=((n+i)&4)?(s16[]){INT16_MIN,INT16_MAX,-1,0,1,127,128,255}[(n/8+i)%8]:half(n*31U+i*97);}
 script[2+count*2]=half(0x8000U|(n&0x7FFF));memcpy(script_before,script,sizeof(script));
 cursorOffset=((n/2)&1)*FRAME_SIZE+0x1670+(n%17)*4;contextOffset=(n&1)*FRAME_SIZE;
 CurrentDrawBuf=(RrIconSpriteContext*)(frame+contextOffset);scratch.cursor=frame+cursorOffset;scratch.order=order;
 u32 low=(u32)(uintptr_t)frame&0xFFFFFFU;u32 sentinel=0x123456;
 while(((sentinel-low)&0xFFFFFFU)<FRAME_BYTES)sentinel=(sentinel+0x100000U)&0xFFFFFFU;
 for(unsigned j=0;j<2;j++)putword(frame+j*FRAME_SIZE+0x70,(n<<24)|(sentinel&0xFFFFFFU));
 memcpy(reference,frame,FRAME_BYTES);
}
static void expected(void){u32 x=(u32)(s32)script[0];s32 y=script[1];u32 old=getword(reference+contextOffset+0x70);u32 clut=bank?0x7E38:0x7F78;
 for(unsigned i=0;i<count;i++){s32 glyph=script[2+2*i];const RrUiUvRect *uv=glyph>=0x400?&HudGlyphUvRects[glyph-0x400]:bank?&HudGlyphUvRects[glyph]:&HudGlyphUvRectsBank0[glyph];u8 *p=reference+cursorOffset+i*28;
 putword(p,0x04000000U|(old&0xFFFFFFU));p[4]=p[5]=p[6]=(u8)color;p[7]=0x66;putshort(p+8,(u16)x);putshort(p+10,(u16)y);p[12]=uv->u;p[13]=uv->v;putshort(p+14,clut);putshort(p+16,uv->w);putshort(p+18,uv->h);
 u32 sprite=(u32)(uintptr_t)(frame+cursorOffset+i*28)&0xFFFFFFU;putword(p+20,0x01000000U|sprite);putword(p+24,0xE1000200U|((bank||glyph>=0x400)?0x25:0x3E));
 old=(old&0xFF000000U)|((u32)(uintptr_t)(frame+cursorOffset+i*28+20)&0xFFFFFFU);
 x+=(u32)((s32)uv->w+script[3+2*i]);
 }
 putword(reference+contextOffset+0x70,old);
}
void run(void){DrawGlyphList(script,color,bank);assert(scratch.cursor==frame+cursorOffset+count*28&&scratch.order==order&&CurrentDrawBuf==(RrIconSpriteContext*)(frame+contextOffset));assert(!memcmp(script,script_before,sizeof(script)));}
void *get_frame(void){return frame;}void *get_script(void){return script;}void *get_order(void){return order;}void *get_primary(void){return HudGlyphUvRectsBank0;}void *get_secondary(void){return HudGlyphUvRects;}
u32 get_count(void){return count;}u32 get_cursor(void){return cursorOffset;}u32 get_context(void){return contextOffset;}s32 get_color(void){return color;}s32 get_bank(void){return bank;}
#ifndef RR_NATIVE_SHARED
int main(void){for(unsigned n=0;n<131072;n++){setup(n);RrUiUvRect primary[50],secondary[40];memcpy(primary,HudGlyphUvRectsBank0,sizeof(primary));memcpy(secondary,HudGlyphUvRects,sizeof(secondary));expected();run();assert(!memcmp(frame,reference,FRAME_BYTES));assert(!memcmp(primary,HudGlyphUvRectsBank0,sizeof(primary))&&!memcmp(secondary,HudGlyphUvRects,sizeof(secondary)));}puts("131072 native oracle cases; full frame/script/font guards passed");}
#endif
'''
if __name__=='__main__':
 with TemporaryDirectory() as td:
  d=Path(td);(d/'test.c').write_text(fixture);subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined,address','-fno-sanitize-recover=all','-I'+str(ROOT/'include'),'-I'+str(P),str(d/'test.c'),'-o',str(d/'test')],check=True);subprocess.run([str(d/'test')],check=True)
