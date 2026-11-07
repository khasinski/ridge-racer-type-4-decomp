"""16-color palette ownership and real TILE/sprite emission closure."""
from pathlib import Path
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

class GaragePaletteTileTest(unittest.TestCase):
    def test_complete_color_domain_and_both_palette_banks(self):
        root=Path(__file__).resolve().parents[2]
        blob=(root/'disc/PAL/R4.BIN').read_bytes()
        sector=struct.unpack_from('<I',blob,4+662*4)[0]
        offset=sector*2048-0x80114878
        rows=[struct.unpack_from('<iHHBBHiI',blob,offset+0x80126260+20*i) for i in range(72)]
        self.assertEqual(sorted(r[6] for r in rows),list(range(72)))
        original='static const RrGarageSpriteRecord original[72]={\n'+',\n'.join('{'+','.join(map(str,r))+'}' for r in rows)+'\n};\n'
        sprite=(root/'src/overlays/PAL/garage/func_8011B2D4.c').read_text()
        hint='    __asm__("" ::: "memory");\n'
        self.assertEqual(sprite.count(hint),1)
        sprite=sprite.replace(hint,'') # Existing empty scheduling metadata only.
        tile=(root/'src/overlays/PAL/garage/func_8011B034.c').read_text()
        body=(root/'src/overlays/PAL/garage/func_8011BCC0.c').read_text()
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
#include "rr/garage_palette_tile.h"
typedef struct {RrGpuTilePacket tile;RrSpritePacket highlight;} Stream;
static struct World {u32 guard[4];RrGarageSelectionPrefix state;RrGpuScratchPointers scratch;RrPalette16 palettes[2];RrGarageRectangle rectangles[46];RrGarageRectangle *table[46];RrGarageSpriteRecord records[72];RrGarageSpriteRecord *lookup[72];u32 order[704];Stream stream;u32 tail[4];} world,expected;
#define garage_logo_state world.state
#define RR_GPU_SCRATCH (&world.scratch)
#define garage_logo_buttons world.table
#define garage_sprite_records world.records
#define garage_logo_sprites world.lookup
''' + original + tile + sprite + body + r'''
static u32 seed=0x1BCC0;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 raw){u16 bits=(u16)raw;s16 v;memcpy(&v,&bits,2);return v;}
static void put16(void *base,unsigned at,u16 bits){u8 *p=base;p[at]=(u8)bits;p[at+1]=(u8)(bits>>8);}
static void put32(void *base,u32 bits){u8 *p=base;unsigned i;for(i=0;i<4;i++)p[i]=(u8)(bits>>(8*i));}
static void reference(unsigned slot){
 u16 *rect=expected.rectangles[20+slot];u16 color=expected.palettes[expected.state.active21].colors[slot].value;u8 *p=(u8*)&expected.stream.tile;u32 head=expected.order[139];
 p[3]=3;p[7]=0x60;put16(p,8,(u16)((u16)expected.state.halfC8+rect[0]));put16(p,10,(u16)((u16)expected.state.halfCA+rect[1]));put16(p,12,rect[2]);put16(p,14,rect[3]);
 p[4]=(u8)((color%32)*8);p[5]=(u8)(((color/32)%32)*8);p[6]=(u8)(((color/1024)%32)*8);
 put32(p,0x03000000U|(head&0xFFFFFFU));expected.order[139]=(head&0xFF000000U)|((uintptr_t)&world.stream.tile&0xFFFFFFU);
 expected.scratch.cursor=(u8*)&world.stream.highlight;
 if(slot==expected.state.config22){
  unsigned record=(unsigned)(world.lookup[69]-world.records);RrGarageSpriteRecord *r=&expected.records[record];unsigned depth=(unsigned)(128+r->depth00);
  p=(u8*)&expected.stream.highlight;head=expected.order[depth];
  p[3]=4;p[7]=(r->flags10&1)?0x67:0x65;put16(p,8,(u16)(rect[0]-1));put16(p,10,(u16)(rect[1]-2));
  put16(p,16,r->width04);put16(p,18,r->height06);p[12]=r->u08;p[13]=r->v09;put16(p,14,r->clut0A);
  put32(p,0x04000000U|(head&0xFFFFFFU));expected.order[depth]=(head&0xFF000000U)|((uintptr_t)&world.stream.highlight&0xFFFFFFU);
  expected.scratch.cursor=(u8*)(&world.stream.highlight+1);
 }
}
int main(void){unsigned k,mode,i,j,slot,record;
 _Static_assert(sizeof(Stream)==36 && offsetof(Stream,highlight)==16,"genuine mixed packet stream");
 _Static_assert(sizeof(RrPaletteAddress)==sizeof(void*),"address arithmetic preserves native pointers");
 for(k=0;k<65536;k++)for(mode=0;mode<3;mode++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();slot=k%16;
  world.state.active21=(u8)((k+mode)&1);world.state.config22=(u8)(mode==0?slot:mode==1?(slot^1):(k>>8));
  world.state.halfC8=half(k);world.state.halfCA=half(k*40503U);
  world.scratch.cursor=(u8*)&world.stream.tile;world.scratch.order=world.order;
  for(i=0;i<2;i++){world.state.palette48[i]=world.palettes[i].words;world.state.image38[i]=world.order;world.state.image40[i]=world.order;}
  for(i=0;i<8;i++)world.state.options78.flat[i]=0;
  for(i=0;i<46;i++)world.table[i]=&world.rectangles[i];
  for(j=0;j<4;j++)world.rectangles[20+slot][j]=(u16)(k*(73U+2*j)+j);
  world.palettes[world.state.active21].colors[slot].value=(u16)k;
  memcpy(world.records,original,sizeof(original));for(i=0;i<72;i++)world.lookup[world.records[i].key0C]=&world.records[i];
  record=(unsigned)(world.lookup[69]-world.records);world.records[record].depth00=(k&1)?11:(s32)(4+k%8);world.records[record].flags10=next();world.records[record].width04=(u16)next();world.records[record].height06=(u16)next();
  assert((uintptr_t)world.state.palette48[0]>UINT32_MAX && (uintptr_t)world.scratch.cursor>UINT32_MAX);
  memcpy(&expected,&world,sizeof(world));reference(slot);garage_draw_palette_swatch((s32)(20+slot));assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
