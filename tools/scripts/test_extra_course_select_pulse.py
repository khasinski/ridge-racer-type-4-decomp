"""Overlay666 pulse with actual sine ROM, full pointer cursors and fresh context."""
from overlay_units import read_c_source
from pathlib import Path
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

class Overlay666PulseTest(unittest.TestCase):
    def test_real_sine_and_context_reload_after_first_emitter(self):
        root=Path(__file__).resolve().parents[2]
        body=read_c_source(root/'src/overlays/PAL/extra_course_select/func_801152B0.c')
        trig=read_c_source(root/'tools/scripts/fixtures/legacy/trig_sine.c')
        exe=(root/'assets/PAL/main.exe').read_bytes()
        table=struct.unpack_from('<4097h',exe,0x800+0x800A50C0-0x80010000)
        source=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/overlay666_pulse.h"
typedef union {u8 bytes[256];u32 words[64];} Arena;
typedef struct {u32 *ordering;void *cursor;RrFrameStorage *context;RrUiUvRect *region;s32 x,y,flags;u16 clut,page;u8 *published;} Event;
static struct World {u32 guard[4];RrGpuScratchPointers scratch;RrFrameStorage *context;RrFrameStorage frames[3];Arena arenas[3];s32 phase;RrUiUvRect region;u32 calls;Event events[2];u32 tail[4];} world,expected;
#define RR_GPU_SCRATCH (&world.scratch)
#define CurrentDrawBuf world.context
#define ScreenFrame world.phase
#define extra_course_select_selection_sprite world.region
static const s16 rom[4097]={''' + ','.join(map(str,table)) + r'''};
#include "rr/trig.h"
#define D_800A50C0 rom
#define D_800A58C0 (rom+0x400)
#define D_800A60C0 (rom+0x800)
#define SinTable (rom+0xc00)
''' + trig + body + r'''
static u32 seed=0x152b0;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 signed_word(u32 raw){s32 value;memcpy(&value,&raw,4);return value;}
static unsigned frame_id,arena_id,offset;static s32 arg_x,arg_y,expected_flags;static u16 expected_page;
static u8 *cursor(unsigned step){return world.arenas[arena_id].bytes+offset+step;}
static void root_effect(struct World *v,unsigned step){
 v->context=&world.frames[(frame_id+step+1)%3];
 v->scratch.cursor=world.arenas[(arena_id+step+1)%3].bytes+128;
 v->scratch.order=world.frames[(frame_id+step+2)%3].ordering.ordering70;
 v->phase=signed_word(0xF1234567U+step);v->region.u^=(u8)(17+step);v->region.h^=(u8)(29+step);
}
static void first_effect(struct World *v,s32 flags){
 unsigned i;Event *e=&v->events[v->calls++];
 e->ordering=world.frames[frame_id].ordering.ordering70;e->cursor=cursor(0);e->context=v->context;e->region=&world.region;
 e->x=arg_x;e->y=arg_y;e->clut=0x7FA4;e->flags=flags;e->published=v->scratch.cursor;
 for(i=0;i<20;i++)v->arenas[arena_id].bytes[offset+i]=(u8)(i*19U+(u32)flags+(u32)arg_x+(u32)arg_y);
 v->frames[frame_id].ordering.ordering70[0]^=0xA1234567;
 root_effect(v,0);
}
static void second_effect(struct World *v,u16 page){
 unsigned i;Event *e=&v->events[v->calls++];
 e->ordering=world.frames[(frame_id+1)%3].ordering.ordering70;e->cursor=cursor(20);e->context=v->context;e->page=page;e->published=v->scratch.cursor;
 for(i=0;i<12;i++)v->arenas[arena_id].bytes[offset+20+i]=(u8)(i*23U+page);
 v->frames[(frame_id+1)%3].ordering.ordering70[0]^=0xB1234567;
 root_effect(v,1);
}
/* ABI-confirmed resident emitter boundaries. Full pointer returns and the
 * captured/fresh context distinction are directly observable in the events. */
void *AddSpritePrim(u32 *ordering,void *packet,s32 x,s32 y,RrUiUvRect *region,u16 clut,s32 flags){
 assert(world.calls==0 && ordering==world.frames[frame_id].ordering.ordering70 && packet==cursor(0));
 assert(x==arg_x && y==arg_y && region==&world.region && clut==0x7FA4 && flags==expected_flags);
 first_effect(&world,flags);return cursor(20);
}
void *AddDrawModePrim(u32 *ordering,void *packet,u16 page){
 assert(world.calls==1 && ordering==world.frames[(frame_id+1)%3].ordering.ordering70 && packet==cursor(20) && page==expected_page);
 assert(world.scratch.cursor==world.arenas[(arena_id+1)%3].bytes+128);
 second_effect(&world,page);return cursor(32);
}
static s32 sine_model(u32 raw){
 unsigned phase=(raw&127U)*32U,q=phase/1024U,i=phase%1024U;
 if(q==0)return rom[3072+i];if(q==1)return rom[4096-i];if(q==2)return -rom[3072+i];return -rom[4096-i];
}
int main(void){unsigned k,i;const u32 edge[]={0,0x7fffffffU,0x80000000U,0xffffffffU,0xffffU,0x10000U,0x80008000U,0x7fff7fffU};
 for(k=0;k<131072;k++){
  u32 raw=(next()&0xffff0000U)|(k&0xffffU);s32 wave=sine_model(raw)/32;
  expected_page=wave<0?89:57;if(wave<0)wave=-wave;expected_flags=(wave+1)*2+1;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  frame_id=k%3;arena_id=(k>>2)%3;offset=((k>>4)&7)*4;
  arg_x=(k&8)?signed_word(next()):signed_word(edge[k&7]);arg_y=(k&16)?signed_word(next()):signed_word(edge[(k>>3)&7]);
  world.phase=signed_word(raw);world.context=&world.frames[frame_id];world.calls=0;
  world.scratch.cursor=cursor(0);world.scratch.order=world.frames[(frame_id+2)%3].ordering.ordering70;
  memcpy(&expected,&world,sizeof(world));first_effect(&expected,expected_flags);second_effect(&expected,expected_page);expected.scratch.cursor=cursor(32);
  assert((uintptr_t)cursor(0)>UINT32_MAX && (uintptr_t)world.context>UINT32_MAX);
  extra_course_select_draw_selection_pulse(arg_x,arg_y);assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
