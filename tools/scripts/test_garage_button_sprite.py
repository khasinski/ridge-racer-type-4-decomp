"""Real16-byte button owners and the actual72-record sprite emitter closure."""
from pathlib import Path
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

class Overlay662ButtonSpriteTest(unittest.TestCase):
    def test_signed_timers_and_original_button_records(self):
        root=Path(__file__).resolve().parents[2]
        blob=(root/'disc/PAL/R4.BIN').read_bytes()
        sector=struct.unpack_from('<I',blob,4+662*4)[0]
        offset=sector*2048-0x80114878
        buttons=[struct.unpack_from('<8h',blob,offset+0x80126F88+16*i) for i in range(46)]
        sprites=[struct.unpack_from('<iHHBBHiI',blob,offset+0x80126260+20*i) for i in range(72)]
        self.assertEqual(sorted(row[6] for row in buttons),list(range(46)))
        self.assertEqual(sorted(row[6] for row in sprites),list(range(72)))
        self.assertTrue(all(0<=row[4]<71 for row in buttons))
        button_data='static const RrGarageButton original_buttons[46]={\n'+',\n'.join('{{'+','.join(map(str,r[:4]))+'},'+','.join(map(str,r[4:]))+'}' for r in buttons)+'\n};\n'
        sprite_data='static const RrGarageSpriteRecord original_sprites[72]={\n'+',\n'.join('{'+','.join(map(str,r))+'}' for r in sprites)+'\n};\n'
        child=(root/'src/overlays/PAL/garage/func_8011B2D4.c').read_text()
        hint='    __asm__("" ::: "memory");\n'
        self.assertEqual(child.count(hint),1)
        child=child.replace(hint,'') # Only the existing empty scheduling metadata.
        body=(root/'src/overlays/PAL/garage/func_8011B940.c').read_text()
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
#include "rr/overlay662_buttons.h"
static struct World {u32 guard[4];RrGpuScratchPointers scratch;RrGarageButton buttons[46];RrGarageRectangle *table[46];RrGarageSpriteRecord records[72];RrGarageSpriteRecord *lookup[72];u32 order[704];RrSpritePacket packets[4];u32 tail[4];} world,expected;
#define RR_GPU_SCRATCH (&world.scratch)
#define garage_logo_buttons world.table
#define garage_sprite_records world.records
#define garage_logo_sprites world.lookup
''' + button_data + sprite_data + child + body + r'''
static u32 seed=0x1B940;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 raw){u16 bits=(u16)raw;s16 v;memcpy(&v,&bits,2);return v;}
static void put16(void *base,unsigned at,u32 bits){u8 *p=base;p[at]=(u8)bits;p[at+1]=(u8)(bits>>8);}
static void put32(void *base,u32 bits){u8 *p=base;unsigned i;for(i=0;i<4;i++)p[i]=(u8)(bits>>(i*8));}
static void reference(unsigned index,unsigned packetIndex){
 RrGarageButton *button=&expected.buttons[index];s32 sprite=button->sprite08;RrGarageSpriteRecord *r;unsigned slot,depth;u32 head;u8 *p=(u8*)&expected.packets[packetIndex];
 if(button->timer0E!=0){sprite++;if(button->timer0E>0)button->timer0E-=1;}
 assert(sprite>=0 && sprite<72);slot=(unsigned)(world.lookup[sprite]-world.records);r=&expected.records[slot];depth=(unsigned)(128+r->depth00);assert(depth<704);head=expected.order[depth];
 p[3]=4;p[7]=(r->flags10&1)?0x67:0x65;put16(p,8,button->rectangle[0]);put16(p,10,button->rectangle[1]);
 put16(p,16,r->width04);put16(p,18,r->height06);p[12]=r->u08;p[13]=r->v09;put16(p,14,r->clut0A);
 put32(p,0x04000000U|(head&0xFFFFFFU));expected.order[depth]=(head&0xFF000000U)|((uintptr_t)&world.packets[packetIndex]&0xFFFFFFU);
 expected.scratch.cursor=(u8*)&world.packets[packetIndex+1];
}
int main(void){unsigned k,i,index,packetIndex;static const u16 boundaries[8]={0,1,2,0x7FFF,0x8000,0x8001,0xFFFE,0xFFFF};
 _Static_assert(sizeof(RrGarageButton)==16,"real button owner");
 for(k=0;k<65904;k++){
  u16 timerBits;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();memcpy(world.buttons,original_buttons,sizeof(original_buttons));memcpy(world.records,original_sprites,sizeof(original_sprites));
  index=k<65536?k%46:(k-65536)/8;timerBits=k<65536?(u16)k:boundaries[(k-65536)%8];packetIndex=k%3;
  world.scratch.order=world.order;world.scratch.cursor=(u8*)&world.packets[packetIndex];
  for(i=0;i<46;i++)world.table[i]=&world.buttons[i].rectangle;
  for(i=0;i<72;i++){world.lookup[world.records[i].key0C]=&world.records[i];world.records[i].flags10=next();world.records[i].width04=(u16)next();world.records[i].height06=(u16)next();}
  world.buttons[index].timer0E=half(timerBits);
  if(k<65536){world.buttons[index].rectangle[0]=(u16)k;world.buttons[index].rectangle[1]=(u16)(k*40503U);}
  assert((uintptr_t)world.table[index]>UINT32_MAX && (uintptr_t)world.scratch.cursor>UINT32_MAX);
  memcpy(&expected,&world,sizeof(world));reference(index,packetIndex);garage_draw_button((s32)index);assert(memcmp(&world,&expected,sizeof(world))==0);
  reference(index,packetIndex+1);garage_draw_button((s32)index);assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
