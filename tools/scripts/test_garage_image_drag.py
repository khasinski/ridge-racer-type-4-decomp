"""Image drag executes actual hit-test, clip, backup and toroidal shift bodies."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest

class Overlay662ImageDragTest(unittest.TestCase):
    def test_start_continue_cancel_and_all_relative_shifts(self):
        root=Path(__file__).resolve().parents[2]
        paths=['8011E6C8','8011E7C0','8011D120','80125DE0','801251AC']
        bodies='\n'.join(re.sub(r' asm\("[^"\n]*"\)','',(root/f'src/overlays/PAL/garage/func_{a}.c').read_text()) for a in paths)
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
#include "rr/overlay662_image_drag.h"
static struct World {
 u32 guard[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;RrGaragePoint anchor;
 u32 images[4][512],palettes[2][8];u32 tail[4];
} world,expected;
#define garage_logo_state world.garage
#define garage_logo_work world.work
#define garage_shift_start_point world.anchor
'''+bodies+r'''
static u32 rng=0x251AC;static uint64_t cases;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static unsigned image_slot(u32 *p){unsigned n=0;while(p!=world.images[n]){n++;assert(n<4);}return n;}
static unsigned palette_slot(u32 *p){unsigned n=0;while(p!=world.palettes[n]){n++;assert(n<2);}return n;}
static void clip(s16 x,s16 y,s16 w,s16 h){expected.work.halves[24]=x;expected.work.halves[25]=y;expected.work.halves[26]=w;expected.work.halves[27]=h;}
static void backup(void){unsigned active=expected.garage.active21,other=!active,j,s,d;
 expected.garage.dirtyD3=1;
 s=image_slot(expected.garage.image38[active]);d=image_slot(expected.garage.image38[other]);for(j=0;j<512;j++)expected.images[d][j]=expected.images[s][j];
 s=image_slot(expected.garage.image40[active]);d=image_slot(expected.garage.image40[other]);for(j=0;j<512;j++)expected.images[d][j]=expected.images[s][j];
 s=palette_slot(expected.garage.palette48[active]);d=palette_slot(expected.garage.palette48[other]);for(j=0;j<8;j++)expected.palettes[d][j]=expected.palettes[s][j];
}
static void reference(unsigned kind){unsigned active,s,d,row,col;u32 input[512],output[512]={0};int dx,dy;
 if(kind==2){expected.garage.config20=0;expected.work.words[0]&=~8U;expected.work.words[14]=0;clip(13,63,294,346);return;}
 if(kind>=3)return;
 if(kind==0){expected.garage.config20=1;expected.work.words[4]=52;expected.work.words[14]=1;expected.work.words[0]|=8;
  clip(82,71,128,256);backup();
  expected.anchor.x=(s16)(expected.garage.reset08[0]+expected.garage.reset08[6]);
  expected.anchor.y=(s16)(expected.garage.reset08[1]+expected.garage.reset08[7]);
 }
 active=expected.garage.active21;
 if(expected.garage.layer23==0){s=image_slot(expected.garage.image38[!active]);d=image_slot(expected.garage.image38[active]);}
 else{s=image_slot(expected.garage.image40[!active]);d=image_slot(expected.garage.image40[active]);}
 dx=expected.garage.reset08[0]+expected.garage.reset08[6]-expected.anchor.x;
 dy=expected.garage.reset08[1]+expected.garage.reset08[7]-expected.anchor.y;
 assert(dx>=-63&&dx<=63&&dy>=-63&&dy<=63);memcpy(input,expected.images[s],sizeof(input));
 for(row=0;row<64;row++)for(col=0;col<64;col++){
  unsigned sx=(unsigned)((int)col+64-dx)%64,sy=(unsigned)((int)row+64-dy)%64;
  u32 pixel=(input[sy*8+sx/8]>>(4*(sx%8)))&15U;
  output[row*8+col/8]|=pixel<<(4*(col%8));
 }
 memcpy(expected.images[d],output,sizeof(output));
}
static void run(unsigned kind,unsigned x,unsigned y,unsigned ax,unsigned ay,unsigned active,unsigned layer){unsigned j,k;
 memset(&world,0,sizeof(world));
 for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();for(k=0;k<512;k++)world.images[j][k]=next();}
 for(j=0;j<2;j++){for(k=0;k<8;k++)world.palettes[j][k]=next();world.garage.image38[j]=world.images[j];world.garage.image40[j]=world.images[2+j];world.garage.palette48[j]=world.palettes[j];}
 if(cases%11==0){world.garage.image38[1]=world.garage.image38[0];world.garage.image40[1]=world.garage.image40[0];world.garage.palette48[1]=world.garage.palette48[0];}
 world.garage.active21=(u8)active;world.garage.layer23=(u8)layer;world.garage.config20=(u8)next();
 world.garage.reset08[0]=(s16)((int)(next()%32768)-16384);world.garage.reset08[6]=(s16)((int)x-world.garage.reset08[0]);
 world.garage.reset08[1]=(s16)((int)(next()%32768)-16384);world.garage.reset08[7]=(s16)((int)y-world.garage.reset08[1]);
 world.anchor.x=(s16)ax;world.anchor.y=(s16)ay;
 for(j=0;j<16;j++)world.work.words[j]=next();
 world.work.words[0]=0xA1230000U|((kind==1||kind==2)?8U:0U);
 world.work.words[1]=0xFEDC0000U|(kind==2?4U:kind==4?0U:1U);
 world.work.halves[10]=kind==3?81:kind==1?INT16_MIN:100;world.work.halves[11]=100;
 if(kind>=2){world.garage.active21=255;for(j=0;j<2;j++){world.garage.image38[j]=0;world.garage.image40[j]=0;world.garage.palette48[j]=0;}}
 memcpy(&expected,&world,sizeof(world));reference(kind);garage_move_layer_drag();
 assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.images>UINT32_MAX);cases++;
}
int main(void){int dx,dy;unsigned active,layer,i,kind;
 for(dx=-63;dx<=63;dx++)for(dy=-63;dy<=63;dy++)for(active=0;active<2;active++)for(layer=0;layer<2;layer++)
  run(1,dx>0?(unsigned)dx:0,dy>0?(unsigned)dy:0,dx<0?(unsigned)-dx:0,dy<0?(unsigned)-dy:0,active,layer?((dx+dy)&1?1:255):0);
 for(i=0;i<4096;i++)for(active=0;active<2;active++)for(layer=0;layer<2;layer++)run(0,i%64,i/64,63,17,active,layer);
 for(kind=2;kind<5;kind++)for(i=0;i<256;i++)run(kind,0,0,0,0,0,0);
 assert(cases==81668ULL);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
