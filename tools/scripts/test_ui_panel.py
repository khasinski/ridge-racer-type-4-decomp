"""Panel renderer: fresh rows, signed callback indices and full returned cursors."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class UiPanelTest(unittest.TestCase):
    def test_actual_body_callbacks_and_complete_owners(self):
        s=(ROOT/'tools/scripts/fixtures/legacy/func_8003D474.c').read_text()
        s=re.sub(r'^#include.*\n','',s,flags=re.M)
        s=re.sub(r' asm\("\$\d+"\)','',s)
        s=re.sub(r'__asm__\(""[^;]*;',';',s)
        self.assertEqual(s.count('(RrGpuScratchPointers *)0x1F800000U'),1)
        s=s.replace('(RrGpuScratchPointers *)0x1F800000U','(&w->scratch)')
        prefix=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rr/layout.h"
/* The shared scratch slots intentionally widen on native hosts. Reassert
 * every record/context field used here; numeric PAL record layouts stay fixed. */
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,s)
#define RR_ASSERT_OFFSET(t,f,o)
#include "rr/ui_panel.h"
_Static_assert(sizeof(RrBadgeRow1C)==28,"row stride");
_Static_assert(offsetof(RrBadgeRow1C,status00)==0,"status");
_Static_assert(offsetof(RrBadgeRow1C,x04)==4,"x");
_Static_assert(offsetof(RrBadgeRow1C,y06)==6,"y");
_Static_assert(offsetof(RrBadgeRow1C,y0A)==10,"lower edge");
_Static_assert(offsetof(RrBadgeRow1C,width0C)==12,"width");
_Static_assert(offsetof(RrBadgeRow1C,extent0E)==14,"extent");
_Static_assert(offsetof(RrBadgeRow1C,x14)==20,"viewport x");
_Static_assert(offsetof(RrBadgeRow1C,y16)==22,"viewport y");
_Static_assert(offsetof(RrBadgeRow1C,w18)==24,"viewport width");
_Static_assert(offsetof(RrBadgeRow1C,h1A)==26,"viewport height");
_Static_assert(sizeof(RrPanelContext)==120,"context");
_Static_assert(offsetof(RrPanelContext,head74)==116,"head");
_Static_assert(sizeof(void*)==sizeof(unsigned long),"full pointers");
_Static_assert(sizeof(RrGpuScratchPointers)==2*sizeof(void*),"native scratch");
_Static_assert(offsetof(RrGpuScratchPointers,order)==sizeof(void*),"native order slot");
typedef struct {RrBadgeRow1C rows[2][6];RrPanelContext contexts[3];RrGpuScratchPointers scratch;u8 canary[32];} World;
typedef struct {int kind,context;void *cursor;s32 args[7];} Entry;
static World actual,expected,*w;
static Entry entries[32],actual_entries[32];
static int entry_count,row_bank,context_index,selected_index,mutate,skip_case;
static u32 seed;
static union {uint64_t align;u8 bytes[128];} arena[32];
static RrPanelDraw callbacks[256];
#define BadgeRows (w->rows[row_bank]+1)
#define CurrentDrawBuf (skip_case?NULL:&w->contexts[context_index].prefix)
#define PanelDrawCallbacks (callbacks+129)
'''
        reference=r'''
static void *emit(int kind,u32 *order,void *cursor,s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g){
 assert(entry_count<32);
 int context=-1;
 for(int i=0;i<3;i++)if(order==&w->contexts[i].head74)context=i;
 assert(context>=0);
 Entry *out=&entries[entry_count];memset(out,0,sizeof(*out));
 out->kind=kind;out->context=context;out->cursor=cursor;
 out->args[0]=a;out->args[1]=b;out->args[2]=c;out->args[3]=d;out->args[4]=e;out->args[5]=f;out->args[6]=g;
 int sequence=entry_count++;
 if(mutate){
  for(int bank=0;bank<2;bank++){
   RrBadgeRow1C *r=&w->rows[bank][selected_index+1];
   r->status00^=0xFFFF;
   r->x04=(s16)((u16)r->x04+101U);r->y06=(s16)((u16)r->y06+103U);
   r->width0C=(s16)((u16)r->width0C+107U);r->extent0E=(s16)((u16)r->extent0E+109U);
   r->y0A=(s16)((u16)r->y0A+113U);
   r->x14=(s16)((u16)r->x14+127U);r->y16=(s16)((u16)r->y16+131U);
   r->w18=(s16)((u16)r->w18+137U);r->h1A=(s16)((u16)r->h1A+139U);
  }
  row_bank^=1;context_index=(context_index+1)%3;
  w->scratch.cursor=arena[(sequence+13)%32].bytes;
  *order^=seed+(u32)sequence;
 }
 return arena[(seed+(u32)sequence*7U+(u32)kind)%32].bytes;
}
void *AddDrawAreaPrim(u32 *o,void *p,s32 x,s32 y,s32 width,s32 height){return emit(0,o,p,x,y,width,height,0,0,0);}
void *AddDrawModePrim(u32 *o,void *p,u16 mode){return emit(1,o,p,mode,0,0,0,0,0,0);}
RrSpritePacket *DrawBadgeTitleBar(u32 *o,RrSpritePacket *p,s32 x,s32 y,s32 style){return emit(2,o,p,x,y,style,0,0,0,0);}
void *AddTilePrim(u32 *o,void *p,s32 x,s32 y,s32 width,s32 height,s32 r,s32 g,s32 b){return emit(3,o,p,x,y,width,height,r,g,b);}
static void *cb0(u32 *o,void *p,s32 index){return emit(100,o,p,index,0,0,0,0,0,0);}
static void *cb1(u32 *o,void *p,s32 index){return emit(101,o,p,index,0,0,0,0,0,0);}
static u16 read16(const u8 *p){u16 v;memcpy(&v,p,2);return v;}
static s32 signed16(const u8 *p){u16 v=read16(p);return v<32768?v:(s32)v-65536;}
static u8 *record(int index){return (u8 *)(w->rows[row_bank]+1)+(ptrdiff_t)index*28;}
static void reference(int index){
 u8 *r=record(index);u16 status=read16(r);if(status==65535)return;
 int callback=status>>8;if(callback>=128)callback-=256;
 s32 x=signed16(r+4),y=signed16(r+6),width=signed16(r+12),height=signed16(r+14)-22;
 u32 *head=&w->contexts[context_index].head74;
 void *cursor=emit(0,head,w->scratch.cursor,0,0,320,512,0,0,0);
 cursor=emit(1,head,cursor,37,0,0,0,0,0,0);
 if(callback)cursor=emit(100+((unsigned)callback&1),head,cursor,index,0,0,0,0,0,0);
 cursor=emit(2,head,cursor,x,y,status&255,0,0,0,0);
 r=record(index);
 cursor=emit(3,head,cursor,x,signed16(r+10)+height+20,width,2,255,255,255);
 y+=22;
 cursor=emit(3,head,cursor,x,y,width,2,255,255,255);
 cursor=emit(3,head,cursor,x,y,1,height,255,255,255);
 cursor=emit(3,head,cursor,x+width-1,y,1,height,255,255,255);
 cursor=emit(3,head,cursor,x,y,width,height,74,74,74);
 cursor=emit(1,head,cursor,37,0,0,0,0,0,0);
 r=record(index);
 w->scratch.cursor=emit(0,head,cursor,signed16(r+20),signed16(r+22),signed16(r+24),signed16(r+26),0,0,0);
}
static u32 state=0x66AB1281;
static u32 next(void){state^=state<<13;state^=state>>17;state^=state<<5;return state;}
static void bind(World *world){w=world;entry_count=0;row_bank=0;context_index=0;memset(entries,0,sizeof(entries));}
int main(void){
 assert(sizeof(void*)>4 && (uintptr_t)arena>UINT32_MAX);
 for(int i=0;i<256;i++)callbacks[i]=(i&1)?cb1:cb0;
 for(unsigned k=0;k<131072;k++){
  skip_case=(u16)k==65535;seed=k*2654435761U;selected_index=(int)(k%5)-1;mutate=k>=65536;
  for(size_t i=0;i<sizeof(actual);i++)((u8 *)&actual)[i]=(u8)next();
  actual.rows[0][selected_index+1].status00=(u16)k;
  actual.scratch.cursor=skip_case?NULL:arena[k%32].bytes;actual.scratch.order=(u32*)arena[(k+1)%32].bytes;
  expected=actual;
  bind(&actual);DrawBadgeRow(selected_index);
  int count=entry_count,bank=row_bank,ctx=context_index;memcpy(actual_entries,entries,sizeof(entries));
  bind(&expected);reference(selected_index);
  if(entry_count!=count||row_bank!=bank||context_index!=ctx||memcmp(actual_entries,entries,sizeof(entries))||memcmp(&actual,&expected,sizeof(actual))){fprintf(stderr,"case %u status%04X index%d mutation%d failed actual entries%d expected%d\n",k,k&65535,selected_index,mutate,count,entry_count);return 1;}
 }
 puts("131072 native panel cases: OK (UBSan)");return 0;
}
'''
        with TemporaryDirectory() as temporary:
            p=Path(temporary)
            source=p/'panel.c'
            source.write_text(prefix+s+reference)
            executable=p/'panel'
            subprocess.run(['clang','-std=gnu11','-O2','-fsanitize=undefined',
                            '-fno-sanitize-recover=all','-I'+str(ROOT/'include'),
                            str(source),'-o',str(executable)],check=True,capture_output=True,text=True)
            result=subprocess.run([str(executable)],check=True,capture_output=True,text=True)
            self.assertIn('131072 native panel cases: OK (UBSan)',result.stdout)


if __name__=='__main__':
    unittest.main()
