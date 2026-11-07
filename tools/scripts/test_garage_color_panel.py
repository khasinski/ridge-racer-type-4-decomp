"""RGB555 panel with actual tile/sprite bodies and observable channel boundaries."""
from pathlib import Path
import re
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

class GarageColorPanelTest(unittest.TestCase):
    def test_all_colors_full_pointers_and_captured_channels(self):
        root=Path(__file__).resolve().parents[2]
        bodies=[]
        for address in ['8011B034','8011B2D4','8011BF04']:
            body=(root/f'src/overlays/PAL/garage/func_{address}.c').read_text()
            body=re.sub(r' asm\("\$\d+"\)','',body)
            body=body.replace('    __asm__("" ::: "memory");\n','')
            bodies.append(body)
        blob=(root/'disc/PAL/R4.BIN').read_bytes()
        start=struct.unpack_from('<I',blob,4+662*4)[0]*2048+0x80126260-0x80114878
        rows=[struct.unpack_from('<iHHBBHiI',blob,start+20*i) for i in range(72)]
        original='static const RrGarageSpriteRecord original[72]={\n'+',\n'.join('{'+','.join(map(str,row))+'}' for row in rows)+'\n};\n'
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
#include "rr/overlay662_palette.h"
typedef union {u8 bytes[512];u32 words[128];struct {RrGpuTilePacket tile;RrSpritePacket sprites[2];u8 digits[456];} sequence;} Arena;
typedef struct {s32 channel,value;u8 *cursor;u32 *order;} Event;
static struct World {u32 guard[4];RrGpuScratchPointers scratch;RrGarageSelectionPrefix state;RrGaragePalette palettes[3];RrGarageSpriteRecord records[72];RrGarageSpriteRecord *lookup[72];u32 order[704];Arena arena;u32 calls;Event events[3];u32 tail[4];} world,expected;
#define RR_GPU_SCRATCH (&world.scratch)
#define garage_logo_state world.state
#define garage_sprite_records world.records
#define garage_logo_sprites world.lookup
''' + original + '\n'.join(bodies) + r'''
static u32 seed=0x1bf04;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 value){u16 raw=value;s16 result;memcpy(&result,&raw,2);return result;}
static void put16(void *base,unsigned offset,u32 value){u8 *p=base;p[offset]=(u8)value;p[offset+1]=(u8)(value>>8);}
static void put32(void *base,unsigned offset,u32 value){u8 *p=base;unsigned i;for(i=0;i<4;i++)p[offset+i]=(u8)(value>>(8*i));}
static u32 get32(const void *base,unsigned offset){const u8 *p=base;return (u32)p[offset]|((u32)p[offset+1]<<8)|((u32)p[offset+2]<<16)|((u32)p[offset+3]<<24);}
static RrGarageSpriteRecord *by_key(struct World *v,unsigned key){unsigned i;for(i=0;i<72;i++)if(v->records[i].key0C==(s32)key)return &v->records[i];assert(0);return 0;}
/* Unrecovered channel-renderer boundary: the original emits three20-byte
 * sprites. Record arguments and consume that footprint; palette mutations
 * independently verify the caller's captured channel values. */
static void channel_effect(struct World *v,unsigned offset,s32 channel,s32 value){
 unsigned i;Event *event;assert(v->calls<3 && channel==(s32)v->calls && value>=0 && value<=31);
 event=&v->events[v->calls++];event->channel=channel;event->value=value;
 event->cursor=v->scratch.cursor;event->order=v->scratch.order;
 for(i=0;i<60;i++)v->arena.bytes[offset+i]=(u8)(i*31U+(u32)channel*17U+(u32)value);
 v->scratch.cursor=world.arena.bytes+offset+60;
 v->state.active21^=1;v->state.config22=(v->state.config22+7)&15;
 v->palettes[channel].colors[(channel*7)&15].value^=0x7fff;
 v->state.palette48[v->state.active21]=world.palettes[(channel+1)%3].words;
}
void garage_draw_color_channel(s32 channel,s32 value){
 unsigned offset=56+world.calls*60;assert(world.scratch.cursor==world.arena.bytes+offset);
 channel_effect(&world,offset,channel,value);
}
static void ref_link(unsigned offset,unsigned head){
 u8 *p=expected.arena.bytes+offset;
 put32(p,0,(get32(p,0)&0xff000000U)|(expected.order[head]&0xffffffU));
 expected.order[head]=(expected.order[head]&0xff000000U)|((uintptr_t)(world.arena.bytes+offset)&0xffffffU);
}
static void ref_sprite(unsigned offset,unsigned key,s32 x,s32 y){
 u8 *p=expected.arena.bytes+offset;RrGarageSpriteRecord *r=by_key(&expected,key);
 p[3]=4;p[7]=(r->flags10&1)?0x67:0x65;put16(p,8,(u32)x);put16(p,10,(u32)y);
 put16(p,16,r->width04);put16(p,18,r->height06);p[12]=r->u08;p[13]=r->v09;put16(p,14,r->clut0A);
 ref_link(offset,128+r->depth00);expected.scratch.cursor=world.arena.bytes+offset+20;
}
static void reference(u16 color){
 u32 r=(color&31U)*8,g=((color>>5)&31U)*8,b=((color>>10)&31U)*8;
 u8 *p=expected.arena.bytes;
 p[3]=3;p[7]=0x60;put16(p,8,(u16)expected.state.halfC8+115U);put16(p,10,(u16)expected.state.halfCA+363U);
 put16(p,12,9);put16(p,14,32);p[4]=r;p[5]=g;p[6]=b;ref_link(0,139);
 expected.scratch.cursor=world.arena.bytes+16;
 ref_sprite(16,70,114,361);ref_sprite(36,46,125,356);
 channel_effect(&expected,56,0,r/8);channel_effect(&expected,116,1,g/8);channel_effect(&expected,176,2,b/8);
}
int main(void){unsigned k,i,active,index;u16 color;
 _Static_assert(sizeof(unsigned long)>=sizeof(void*),"full address width");
 _Static_assert(offsetof(Arena,sequence.sprites)==16,"real sprite owner");
 _Static_assert(offsetof(Arena,sequence.digits)==56,"real byte owner");
 for(k=0;k<131072;k++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  active=k>>16;index=(k>>8)&15;color=(u16)k;
  world.state.active21=active;world.state.config22=index;
  world.state.palette48[0]=world.palettes[0].words;world.state.palette48[1]=world.palettes[1].words;
  world.palettes[active].colors[index].value=color;
  world.state.halfC8=half(k*40503U+17U);world.state.halfCA=half(k*73U+11U);
  memcpy(world.records,original,sizeof(original));
  for(i=0;i<72;i++)world.lookup[world.records[i].key0C]=&world.records[i];
  for(i=0;i<2;i++){RrGarageSpriteRecord *r=by_key(&world,i?46:70);r->flags10=next();r->width04=(u16)next();r->height06=(u16)next();r->clut0A=(u16)next();r->u08=(u8)next();r->v09=(u8)next();r->depth00=4+(next()&7);}
  world.scratch.cursor=world.arena.bytes;world.scratch.order=world.order;world.calls=0;
  assert((uintptr_t)world.state.palette48[active]>UINT32_MAX && (uintptr_t)world.scratch.cursor>UINT32_MAX);
  memcpy(&expected,&world,sizeof(world));reference(color);
  garage_draw_color_panel();assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
