"""Color-channel slider/digits with actual sprite emitter and ROM records."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import struct
import subprocess
import unittest

class GarageColorChannelTest(unittest.TestCase):
    def test_three_sprites_all_width_halves_and_color_values(self):
        root=Path(__file__).resolve().parents[2]
        blob=(root/'disc/PAL/R4.BIN').read_bytes();sector=struct.unpack_from('<I',blob,4+662*4)[0]
        rows=[struct.unpack_from('<iHHBBHiI',blob,sector*2048+0x80126260-0x80114878+20*i) for i in range(72)]
        self.assertEqual(sorted(row[6] for row in rows),list(range(72)))
        data='static const RrGarageSpriteRecord original[72]={\n'+',\n'.join('{'+','.join(map(str,r))+'}' for r in rows)+'\n};\n'
        child=(root/'src/overlays/PAL/garage/func_8011B2D4.c').read_text().replace('    __asm__("" ::: "memory");\n','')
        body=(root/'src/overlays/PAL/garage/func_8011C00C.c').read_text()
        body=re.sub(r' __asm__\("\$[0-9]+"\)','',body)
        body=body.replace('    __asm__("" : "+r"(tensSprite));\n','')
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/garage_palette.h"
_Static_assert(sizeof(RrSpritePacket)==20,"native packet size");
static struct World {
 u32 guard[4];RrGpuScratchPointers scratch;RrGarageSpriteRecord records[72];
 RrGarageSpriteRecord *lookup[72];u32 order[704];RrSpritePacket packets[4];u32 tail[4];
} world,expected;
#define RR_GPU_SCRATCH (&world.scratch)
#define garage_sprite_records world.records
#define garage_logo_sprites world.lookup
'''+data+child+body+r'''
static u32 rng=0x1C00C;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static void put16(void *base,unsigned offset,u32 value){u8 *p=base;p[offset]=(u8)value;p[offset+1]=(u8)(value>>8);}
static void put32(void *base,u32 value){u8 *p=base;unsigned j;for(j=0;j<4;j++)p[j]=(u8)(value>>(j*8));}
static void sprite(unsigned slot,unsigned index,s32 x,s32 y){
 unsigned at=(unsigned)(world.lookup[index]-world.records);RrGarageSpriteRecord *r=&expected.records[at];
 unsigned depth=(unsigned)(128+r->depth00);u32 head;u8 *p=(u8*)&expected.packets[slot];assert(depth<704);head=expected.order[depth];
 p[3]=4;p[7]=(r->flags10&1)?0x67:0x65;put16(p,8,(u32)x);put16(p,10,(u32)y);
 put16(p,16,r->width04);put16(p,18,r->height06);p[12]=r->u08;p[13]=r->v09;put16(p,14,r->clut0A);
 put32(p,0x04000000U|(head&0xFFFFFFU));
 expected.order[depth]=(head&0xFF000000U)|((uintptr_t)&world.packets[slot]&0xFFFFFFU);
 expected.scratch.cursor=(u8*)&world.packets[slot+1];
}
int main(void){unsigned bits,channel,j,value,tens;uint64_t cases=0;
 memset(&world,0,sizeof(world));memcpy(world.records,original,sizeof(original));world.scratch.order=world.order;
 assert(garage_add_ui_sprite(-1,0,0,0)==0);
 for(bits=0;bits<65536;bits++)for(channel=0;channel<3;channel++){
  value=bits%32;tens=value/10+59;world.lookup[tens]->width04=(u16)bits;
  for(j=0;j<704;j++)world.order[j]=next();
  for(j=0;j<sizeof(world.packets);j++)((u8*)world.packets)[j]=(u8)next();
  for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();}
  world.scratch.cursor=(u8*)world.packets;world.scratch.order=world.order;
  memcpy(&expected,&world,sizeof(world));
  sprite(0,71,(s32)(148+value*2),(s32)(353+channel*18));
  sprite(1,tens,133,(s32)(356+channel*18));
  /* Wire16 position wraps after adding the full unsigned16 descriptor width. */
  {u16 raw=(u16)(133+bits);s16 position;memcpy(&position,&raw,2);sprite(2,value%10+59,position,(s32)(356+channel*18));}
  garage_draw_color_channel((s32)channel,(s32)value);
  assert(memcmp(&world,&expected,sizeof(world))==0);
  assert(world.scratch.cursor==(u8*)&world.packets[3]);assert((uintptr_t)world.scratch.cursor>UINT32_MAX);cases++;
 }
 assert(cases==196608ULL);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
