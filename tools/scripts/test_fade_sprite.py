"""Actual fade-sprite body with full pointer ABI, helper effects and aliases."""
from pathlib import Path
import re,subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[2]
def header(name):
 s=(ROOT/'include/rr'/name).read_text()
 s=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',s,flags=re.M)
 return re.sub(r'extern[^;]*;','',s)
body=(ROOT/'tools/scripts/fixtures/views/func_8005C37C.c').read_text()
body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
body=re.sub(r',?\s*"\$\d+"','',body)
body, scratch_remaps = re.subn(r'\(RrGpuScratchPointers\s*\*\)0x1F800000U', '&actual.scratch', body)
assert scratch_remaps == 1, 'native fixture must replace exactly one physical scratch address'
code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + ''.join(header(n) for n in ['gpu_scratch_types.h','gpu.h','ui_bar.h','animated_sprite_grid.h','fade_sprite.h'])+r'''
typedef struct {
 u8 guard0[32];
 RrGpuScratchPointers scratch;
 RrFadeSpriteContext contexts[2];
 RrFadeSpriteContext *context;
 s32 phase,gain;
 RrUiUvRect uv[8];
 u32 modes[4];
 u32 packets[128];
 u8 guard1[32];
} World;
typedef struct {
 int count;
 u32 *ordering;
 RrSpritePacket *cursor;
 s32 x,y,flags;
 RrUiUvRect *uv;
 u16 clut;
 World snapshot;
} Trace;
static World actual,expected;
static Trace actual_log,expected_log;
static int mutate,return_offset,return_kind,original_selector;
#define CurrentDrawBuf actual.context
#define FadeSpritePhase actual.phase
#define FadeSpriteIntensityScale actual.gain
#define FadeSpriteRects actual.uv
#define FadeSpriteTpageModes actual.modes
'''+body+r'''
static s32 bits(u32 u){s32 s;memcpy(&s,&u,4);return s;}
static void *mapped(World *world,const void *p) {
 uintptr_t offset=(uintptr_t)p-(uintptr_t)&actual;
 assert(offset<sizeof(actual));return (u8*)world+offset;
}
static RrSpritePacket *helper(int model,u32 *ordering,RrSpritePacket *cursor,
 s32 x,s32 y,RrUiUvRect *uv,u16 clut,s32 flags) {
 World *w=model?&expected:&actual;Trace *t=model?&expected_log:&actual_log;
 memset(t,0,sizeof(*t));t->count=1;t->ordering=ordering;t->cursor=cursor;
 t->x=x;t->y=y;t->uv=uv;t->clut=clut;t->flags=flags;
 memcpy(&t->snapshot,w,sizeof(*w));
 assert((uintptr_t)ordering>UINT32_MAX&&(uintptr_t)cursor>UINT32_MAX);
 assert((uintptr_t)uv>UINT32_MAX);
 if(mutate) {
  w->context=&actual.contexts[1];
  w->scratch.cursor=(u8*)actual.packets+240;
  for(int i=0;i<4;i++)w->modes[i]^=0xFEED9ABCU;
  w->contexts[1].head98=0xAB123456U;
  w->phase^=0x12345678;w->gain^=0x45678912;
 }
 u8 *raw=(u8*)cursor+return_offset;
 if(return_kind==1)raw=(u8*)&actual.contexts[0].head98;
 if(return_kind==2)raw=(u8*)&actual.contexts[1].head98;
 if(return_kind==3)raw=(u8*)&actual.context-8;
 if(return_kind==4)raw=(u8*)&actual.modes[original_selector];
 if(!return_kind)memset(mapped(w,raw),0xCA,24);
 return (RrSpritePacket*)raw;
}
RrSpritePacket *AddSpritePrim(u32 *a,RrSpritePacket *b,s32 c,s32 d,
 RrUiUvRect *e,u16 f,s32 g){return helper(0,a,b,c,d,e,f,g);}
static void reference(s32 index,s32 x,s32 y,s32 intensity) {
 s32 phase=expected.phase;
 unsigned selector=(index==4)+(phase>=128?2:0);
 u32 weight=phase>=128?(u32)phase-127U:128U-(u32)phase;
 s32 first=bits(weight*(u32)intensity)/256;
 s32 scale=bits((u32)first*(u32)expected.gain)/256;
 if(scale<=0)return;
 RrFadeSpriteContext *raw_context=expected.context;
 RrSpritePacket *returned=helper(1,&raw_context->head98,
  (RrSpritePacket*)expected.scratch.cursor,x,y,&actual.uv[index],0x7F38,
  bits(((u32)scale*2U)|1U));
 u32 mode=expected.modes[selector];
 expected.scratch.cursor=(u8*)returned;
 u8 *p=mapped(&expected,returned);p[3]=1;
 u32 *words=(u32*)p;
 RrFadeSpriteContext *ctx=mapped(&expected,expected.context);
 words[1]=0xE1000200U|(mode&0x9FFU);
 words[0]=(words[0]&0xFF000000U)|(ctx->head98&0xFFFFFFU);
 ctx->head98=(ctx->head98&0xFF000000U)|((uintptr_t)returned&0xFFFFFFU);
 expected.scratch.cursor=(u8*)returned+8;
}
static void check(s32 index,s32 x,s32 y,s32 intensity,s32 phase,s32 gain,u32 seed,int change) {
 memset(&actual,(u8)seed,sizeof(actual));
 actual.context=&actual.contexts[0];
 actual.scratch.cursor=(u8*)actual.packets+16;
 actual.scratch.order=&actual.contexts[0].head98;
 actual.phase=phase;actual.gain=gain;
 for(int i=0;i<4;i++)actual.modes[i]=seed*2654435761U+i*1717U;
 memcpy(&expected,&actual,sizeof(actual));
 memset(&actual_log,0,sizeof(actual_log));memset(&expected_log,0,sizeof(expected_log));
 mutate=change;return_offset=28+(seed%3)*8;
 original_selector=(index==4)+(phase>=128?2:0);
 reference(index,x,y,intensity);
 EmitFadeSprite(index,x,y,intensity);
 assert(!memcmp(&actual_log,&expected_log,sizeof(Trace)));
 assert(!memcmp(&actual,&expected,sizeof(actual)));
}
int main(int argc,char **argv) {
 assert(argc==2);int group=atoi(argv[1]);
 assert(sizeof(void*)>4);
 assert(offsetof(RrFadeSpriteContext,head98)==0x98);
 const s32 edges[]={(-2147483647-1),-2147483647,-65536,-256,-1,0,1,127,128,129,255,256,65535,2147483647};
 if(group==0) {
 for(unsigned p=0;p<14;p++)for(unsigned a=0;a<14;a++)for(unsigned g=0;g<14;g++) {
  for(s32 i=0;i<8;i++)for(int m=0;m<2;m++)check(i,edges[a],edges[g],edges[a],edges[p],edges[g],p*1717+a*31+g,m);
 }
 } else if(group==1) {
 for(u32 i=0;i<65536;i++) {
  check(i%8,bits(i*2654435761U),bits(i*3571U),bits(i*1717U),bits(i*2891336453U),bits(i*747796405U),i,i&1);
  check(i%8,-17,2147483647,255,(s16)i,256,i,1);
 }
 } else {
  for(return_kind=1;return_kind<=4;return_kind++)for(u32 seed=0;seed<256;seed++) {
   for(int change=0;change<2;change++)for(int index=0;index<=4;index+=4) {
    check(index,-2147483647,2147483647,255,0,256,seed,change);
    check(index,17,-29,255,255,256,seed,change);
   }
  }
  // Zero output must not touch absent packet/context/table entries.
  memset(&actual,0,sizeof(actual));memcpy(&expected,&actual,sizeof(actual));
  memset(&actual_log,0,sizeof(actual_log));
  EmitFadeSprite((-2147483647-1),1,2,0);
  assert(!actual_log.count&&!memcmp(&actual,&expected,sizeof(actual)));
 }
 return 0;
}
''' 
class FadeSpriteTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary=tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        source=Path(cls.temporary.name)/'fade_sprite.c'
        cls.binary=source.with_suffix('')
        source.write_text(code)
        subprocess.run(['cc','-std=gnu99','-O2','-fno-strict-aliasing',
                        '-fsanitize=undefined','-fno-sanitize-recover=all',
                        '-I',str(ROOT/'include'),str(source),'-o',str(cls.binary)],check=True)

    def test_wrapping_products_and_signed_division_extremes(self):
        subprocess.run([str(self.binary),'0'],check=True)

    def test_raw_phase_gain_coordinates_and_helper_effects(self):
        subprocess.run([str(self.binary),'1'],check=True)

    def test_packet_head_context_slot_and_mode_table_aliases(self):
        subprocess.run([str(self.binary),'2'],check=True)


if __name__=='__main__':
    unittest.main()
