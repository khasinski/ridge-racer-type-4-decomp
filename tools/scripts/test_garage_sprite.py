"""Actual72-record sprite lookup and live descriptor/packet/order representations."""
from pathlib import Path
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

class GarageSpriteTest(unittest.TestCase):
    def test_lookup_and_live_packet_aliases(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/garage/func_8011B2D4.c').read_text()
        hint='    __asm__("" ::: "memory");\n'
        self.assertEqual(body.count(hint),1)
        body=body.replace(hint,'') # Only an empty compiler scheduling constraint.
        blob=(root/'disc/PAL/R4.BIN').read_bytes()
        sector=struct.unpack_from('<I',blob,4+662*4)[0]
        address=sector*2048+0x80126260-0x80114878
        rows=[struct.unpack_from('<iHHBBHiI',blob,address+20*i) for i in range(72)]
        self.assertEqual(sorted(row[6] for row in rows),list(range(72)))
        self.assertTrue(all(4<=row[0]<=11 for row in rows))
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
#include "rr/overlay662_sprites.h"
typedef union {u32 words[704];struct {u32 prefix[128];RrSpritePacket packets[64];} view;} Ordering;
typedef union {RrGarageSpriteRecord records[4];struct {u32 prefix;RrSpritePacket packets[3];} view;u32 words[20];} Aliased;
static struct World {u32 guard[4];RrGpuScratchPointers scratch;RrGarageSpriteRecord records[72];RrGarageSpriteRecord *lookup[72];Ordering order;Aliased alias;RrSpritePacket packets[4];u32 tail[4];} world,expected;
#define RR_GPU_SCRATCH (&world.scratch)
#define garage_D_80126260 world.records
#define garage_D_80127548 world.lookup
''' + original + body + r'''
static u32 seed=0x1b2d4;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 signed_word(u32 raw){s32 result;memcpy(&result,&raw,4);return result;}
static void put16(void *base,unsigned offset,u32 value){u8 *p=base;p[offset]=(u8)value;p[offset+1]=(u8)(value>>8);}
static void put32(void *base,unsigned offset,u32 value){u8 *p=base;unsigned i;for(i=0;i<4;i++)p[offset+i]=(u8)(value>>(8*i));}
static u32 get32(const void *base,unsigned offset){const u8 *p=base;return (u32)p[offset]|((u32)p[offset+1]<<8)|((u32)p[offset+2]<<16)|((u32)p[offset+3]<<24);}
static RrSpritePacket *packet(struct World *v,unsigned mode){
 if(mode==0)return &v->packets[0];if(mode==1)return &v->alias.view.packets[0];return &v->order.view.packets[0];
}
static RrGarageSpriteRecord *record(struct World *v,unsigned mode,unsigned index){return mode==1?&v->alias.records[0]:&v->records[index];}
static void reference(unsigned mode,unsigned index,s32 x,s32 y){
 u8 *p=(u8*)packet(&expected,mode);RrGarageSpriteRecord *r=record(&expected,mode,index);s32 depth;
 p[3]=4;p[7]=0x64;p[7]=(r->flags10&1)?0x66:0x64;
 put16(p,8,(u32)x);put16(p,10,(u32)y);p[7]|=1;
 put16(p,16,r->width04);put16(p,18,r->height06);
 p[12]=r->u08;p[13]=r->v09;put16(p,14,r->clut0A);
 depth=r->depth00;assert(depth>=-128 && depth<576);
 put32(p,0,(get32(p,0)&0xff000000U)|(expected.order.words[128+depth]&0xffffffU));
 depth=r->depth00;assert(depth>=-128 && depth<576);
 expected.order.words[128+depth]=(expected.order.words[128+depth]&0xff000000U)|((uintptr_t)packet(&world,mode)&0xffffffU);
}
static void randomize(void){unsigned i;for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();}
int main(void){unsigned k,i,index,mode;const u32 negatives[]={0x80000000U,0xffffffffU,0xfffffff0U,0x80000001U};
 _Static_assert(sizeof(RrGarageSpriteRecord)==20,"serialized record");
 _Static_assert(sizeof(RrSpritePacket)==20,"serialized packet");
 /* Initialization with original records and independently shuffled key maps. */
 for(k=0;k<1024;k++){
  u8 *returned;randomize();memcpy(world.records,original,sizeof(original));
  if(k){for(i=71;i>0;i--){unsigned j=next()%(i+1);s32 key=world.records[i].key0C;world.records[i].key0C=world.records[j].key0C;world.records[j].key0C=key;}}
  world.scratch.order=world.order.words;world.scratch.cursor=(u8*)world.packets;
  memcpy(&expected,&world,sizeof(world));
  for(i=0;i<72;i++)expected.lookup[world.records[i].key0C]=&world.records[i];
  returned=garage_func_8011B2D4(signed_word(negatives[k&3]),0,signed_word(next()),signed_word(next()));
  assert(returned==0 && memcmp(&world,&expected,sizeof(world))==0);
  for(i=0;i<72;i++)assert((uintptr_t)world.lookup[i]>UINT32_MAX);
 }
 /* Every low16 coordinate and record dimension, arbitrary high bits, all72
  * indices, full-word flags, and three genuine overlapping packet owners. */
 for(k=0;k<196608;k++){
  s32 x,y;u8 *returned;RrGarageSpriteRecord *r;
  randomize();memcpy(world.records,original,sizeof(original));index=k%72;mode=k/65536;
  world.scratch.order=world.order.words;world.scratch.cursor=(u8*)packet(&world,mode);
  for(i=0;i<72;i++)world.lookup[i]=&world.records[i];
  r=record(&world,mode,index);world.lookup[index]=r;
  r->depth00=mode==2?(s32)(k%5):(s32)(next()%704)-128;
  r->width04=(u16)k;r->height06=(u16)(k*40503U+17U);r->u08=(u8)k;r->v09=(u8)(k>>8);r->clut0A=(u16)(k*73U);r->flags10=next();
  x=signed_word((next()&0xffff0000U)|(k&0xffffU));y=signed_word((next()&0xffff0000U)|((k*40503U+17U)&0xffffU));
  memcpy(&expected,&world,sizeof(world));reference(mode,index,x,y);
  returned=garage_func_8011B2D4(index,(u8*)packet(&world,mode),x,y);
  assert(returned==(u8*)(packet(&world,mode)+1) && (uintptr_t)returned>UINT32_MAX);
  assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
