"""Mode button: real16-byte owners, all signed timers, actual sprite closure."""
from pathlib import Path
from tempfile import TemporaryDirectory
import struct
import subprocess
import unittest

class Overlay662ModeButtonTest(unittest.TestCase):
    def test_modes_timer_pairs_and_real_button_owners(self):
        root=Path(__file__).resolve().parents[2]
        blob=(root/'disc/PAL/R4.BIN').read_bytes();sector=struct.unpack_from('<I',blob,4+662*4)[0];offset=sector*2048-0x80114878
        buttons=[struct.unpack_from('<8h',blob,offset+0x80126F88+16*i) for i in range(46)]
        sprites=[struct.unpack_from('<iHHBBHiI',blob,offset+0x80126260+20*i) for i in range(72)]
        self.assertEqual(sorted(r[6] for r in buttons),list(range(46)))
        self.assertEqual(sorted(r[6] for r in sprites),list(range(72)))
        data='static const RrGarageButton original_buttons[46]={\n'+',\n'.join('{{'+','.join(map(str,r[:4]))+'},'+','.join(map(str,r[4:]))+'}' for r in buttons)+'\n};\n'
        data+='static const RrGarageSpriteRecord original_sprites[72]={\n'+',\n'.join('{'+','.join(map(str,r))+'}' for r in sprites)+'\n};\n'
        child=(root/'src/overlays/PAL/garage/func_8011B2D4.c').read_text().replace('    __asm__("" ::: "memory");\n','')
        body=(root/'src/overlays/PAL/garage/func_8011BDC8.c').read_text()
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
#include "rr/overlay662_buttons.h"
static struct World {
 u32 guard[4];RrGpuScratchPointers scratch;RrGarageSelectionPrefix garage;
 RrGarageButton buttons[46];RrGarageRectangle *table[46];
 RrGarageSpriteRecord records[72];RrGarageSpriteRecord *lookup[72];
 u32 order[704];RrSpritePacket packets[5];u32 tail[4];
} world,expected;
#define RR_GPU_SCRATCH (&world.scratch)
#define garage_D_801276A8 world.garage
#define garage_D_801273B8 world.table
#define garage_D_80126260 world.records
#define garage_D_80127548 world.lookup
'''+data+child+body+r'''
static u32 rng=0x1BDC8;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s16 half(u32 bits){u16 h=(u16)bits;s16 value;memcpy(&value,&h,2);return value;}
static void put16(void *base,unsigned offset,u32 value){u8 *p=base;p[offset]=(u8)value;p[offset+1]=(u8)(value>>8);}
static void put32(void *base,u32 value){u8 *p=base;unsigned j;for(j=0;j<4;j++)p[j]=(u8)(value>>(j*8));}
static void sprite(unsigned slot,unsigned index,s32 x){
 unsigned at=(unsigned)(world.lookup[index]-world.records);RrGarageSpriteRecord *r=&expected.records[at];
 unsigned depth=(unsigned)(128+r->depth00);u32 head;u8 *p=(u8*)&expected.packets[slot];assert(depth<704);head=expected.order[depth];
 p[3]=4;p[7]=(r->flags10&1)?0x67:0x65;put16(p,8,(u32)x);put16(p,10,173);
 put16(p,16,r->width04);put16(p,18,r->height06);p[12]=r->u08;p[13]=r->v09;put16(p,14,r->clut0A);
 put32(p,0x04000000U|(head&0xFFFFFFU));
 expected.order[depth]=(head&0xFF000000U)|((uintptr_t)&world.packets[slot]&0xFFFFFFU);
 expected.scratch.cursor=(u8*)&world.packets[slot+1];
}
static void reference(unsigned index,unsigned slot){
 unsigned at=(unsigned)((RrGarageButton*)world.table[index]-world.buttons);
 RrGarageButton *button=&expected.buttons[at];unsigned active=button->timer0E!=0;
 if(button->timer0E>0)button->timer0E-=1;
 sprite(slot,16+active,19);sprite(slot+1,18+expected.garage.config1F*2+active,39);
}
int main(void){unsigned bits,mode,index,at,j,step;uint64_t calls=0;
 memset(&world,0,sizeof(world));memcpy(world.buttons,original_buttons,sizeof(original_buttons));memcpy(world.records,original_sprites,sizeof(original_sprites));
 for(j=0;j<46;j++)world.table[(unsigned)world.buttons[j].key0C]=&world.buttons[j].rectangle;
 world.scratch.order=world.order;assert(garage_func_8011B2D4(-1,0,0,0)==0);
 for(bits=0;bits<65536;bits++)for(mode=0;mode<3;mode++){
  index=bits%46;at=(unsigned)((RrGarageButton*)world.table[index]-world.buttons);
  world.buttons[at].timer0E=half(bits);world.garage.config1F=(u8)mode;
  for(j=0;j<704;j++)world.order[j]=next();
  for(j=0;j<sizeof(world.packets);j++)((u8*)world.packets)[j]=(u8)next();
  for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();}
  world.scratch.cursor=(u8*)world.packets;world.scratch.order=world.order;
  memcpy(&expected,&world,sizeof(world));
  for(step=0;step<2;step++){
   reference(index,step*2);garage_func_8011BDC8((s32)index);
   assert(memcmp(&world,&expected,sizeof(world))==0);calls++;
  }
  assert(world.scratch.cursor==(u8*)&world.packets[4]);assert((uintptr_t)world.scratch.cursor>UINT32_MAX);
 }
 assert(calls==393216ULL);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
