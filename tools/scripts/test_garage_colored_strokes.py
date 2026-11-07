"""Stroke wrapper with actual hit-test/clip/backup and typed rasterizer boundary."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class GarageColoredStrokesTest(unittest.TestCase):
    def test_points_byte_arguments_backup_and_fresh_postcall_anchor(self):
        root=Path(__file__).resolve().parents[2]
        paths=['8011E6C8','8011E7C0','8011D120','80124ABC','80124C94']
        bodies='\n'.join((root/f'src/overlays/PAL/garage/func_{a}.c').read_text() for a in paths)
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
#include "rr/garage_image_stroke.h"
static struct World {
 u32 guard[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;RrGaragePoint anchor,point;
 u32 images[5][512],palettes[2][8];u32 tail[4];
} world,expected,checkpoint;
static unsigned family,scenario,line_calls,expected_calls,selected_slot;static s32 args[4];static u8 size_arg,color_arg;
#define garage_logo_state world.garage
#define garage_logo_work world.work
#define garage_pen_prev_point world.anchor
#define garage_line_start_point world.anchor
#define garage_pen_point world.point
'''+bodies+r'''
static u32 rng=0x25500;static uint64_t cases;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s16 half(u32 n){u16 bits=(u16)n;s16 value;memcpy(&value,&bits,2);return value;}
static unsigned image_slot(u32 *p){unsigned n=0;while(p!=world.images[n]){n++;assert(n<5);}return n;}
static unsigned palette_slot(u32 *p){unsigned n=0;while(p!=world.palettes[n]){n++;assert(n<2);}return n;}
static void clip(s16 x,s16 y,s16 w,s16 h){expected.work.halves[24]=x;expected.work.halves[25]=y;expected.work.halves[26]=w;expected.work.halves[27]=h;}
static void backup(void){unsigned active=expected.garage.active21,other=!active,j,s,d;
 expected.garage.dirtyD3=1;
 s=image_slot(expected.garage.image38[active]);d=image_slot(expected.garage.image38[other]);for(j=0;j<512;j++)expected.images[d][j]=expected.images[s][j];
 s=image_slot(expected.garage.image40[active]);d=image_slot(expected.garage.image40[other]);for(j=0;j<512;j++)expected.images[d][j]=expected.images[s][j];
 s=palette_slot(expected.garage.palette48[active]);d=palette_slot(expected.garage.palette48[other]);for(j=0;j<8;j++)expected.palettes[d][j]=expected.palettes[s][j];
}
static void effect(struct World *w){unsigned at=(unsigned)args[3]*8+(unsigned)args[2]/8;
 w->images[selected_slot][at]^=0x9E3779B9U;
 w->point.x=half(scenario);w->point.y=half(scenario*73U+17);
 w->anchor.x=half(scenario+3);w->anchor.y=half(scenario+4);
 w->garage.config20=(u8)(scenario+7);w->garage.active21=255;w->garage.image38[0]=0;
 w->work.words[0]^=0x12345678U;
}
void garage_draw_brush_line(u32 *image,s32 x0,s32 y0,s32 x1,s32 y1,u8 color,u8 size){
 assert(line_calls++==0&&expected_calls==1);assert(memcmp(&world,&checkpoint,sizeof(world))==0);
 assert(image==world.images[selected_slot]);assert((uintptr_t)image>UINT32_MAX);
 assert(x0==args[0]&&y0==args[1]&&x1==args[2]&&y1==args[3]&&color==color_arg&&size==size_arg);effect(&world);
}
static void reference(unsigned kind){unsigned active;
 if(kind==2){expected.garage.config20=0;expected.work.words[0]&=~8U;clip(13,63,294,346);if(family==0)return;}
 if(kind>=3)return;
 if(kind==0){expected.garage.config20=1;expected.work.words[4]=52;expected.work.words[0]|=8;
  clip(82,71,128,256);backup();
  expected.anchor.x=(s16)(expected.garage.reset08[0]+expected.garage.reset08[2]);
  expected.anchor.y=(s16)(expected.garage.reset08[1]+expected.garage.reset08[3]);
 }
 active=expected.garage.active21;selected_slot=(family==1&&kind!=2)?image_slot(expected.garage.scratch34):image_slot(expected.garage.layer23==0?expected.garage.image38[active]:expected.garage.image40[active]);
 if(family==0){expected.point.x=(s16)(expected.garage.reset08[0]+expected.garage.reset08[2]);expected.point.y=(s16)(expected.garage.reset08[1]+expected.garage.reset08[3]);}
 args[0]=expected.anchor.x;args[1]=expected.anchor.y;args[2]=family==0?expected.point.x:expected.garage.reset08[0]+expected.garage.reset08[2];args[3]=family==0?expected.point.y:expected.garage.reset08[1]+expected.garage.reset08[3];size_arg=expected.garage.config1D;color_arg=expected.garage.config22;
 memcpy(&checkpoint,&expected,sizeof(expected));expected_calls=1;effect(&expected);
 if(family==0)expected.anchor=expected.point;
}
static void run(unsigned kind,unsigned point,unsigned previous,unsigned active,unsigned layer){unsigned j,k;
 memset(&world,0,sizeof(world));scenario=(unsigned)cases;
 for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();}for(j=0;j<5;j++)for(k=0;k<512;k++)world.images[j][k]=next();
 for(j=0;j<2;j++){for(k=0;k<8;k++)world.palettes[j][k]=next();world.garage.image38[j]=world.images[j];world.garage.image40[j]=world.images[2+j];world.garage.palette48[j]=world.palettes[j];}
 if(cases%11==0){world.garage.image38[1]=world.garage.image38[0];world.garage.image40[1]=world.garage.image40[0];world.garage.palette48[1]=world.garage.palette48[0];}
 world.garage.scratch34=world.images[4];if(cases%13==0)world.garage.scratch34=world.images[active];world.garage.config22=(u8)(scenario>>3);world.garage.active21=(u8)active;world.garage.layer23=(u8)layer;world.garage.config20=(u8)next();world.garage.config1D=(u8)scenario;
 world.garage.reset08[0]=(s16)((int)(next()%32768)-16384);world.garage.reset08[2]=(s16)((int)(point%64)-world.garage.reset08[0]);
 world.garage.reset08[1]=(s16)((int)(next()%32768)-16384);world.garage.reset08[3]=(s16)((int)(point/64)-world.garage.reset08[1]);
 world.anchor.x=(s16)(previous%64);world.anchor.y=(s16)(previous/64);world.point.x=half(next());world.point.y=half(next());
 for(j=0;j<16;j++)world.work.words[j]=next();
 world.work.words[0]=0xA1230000U|((kind==1||kind==2)?8U:0U);world.work.words[1]=0xFEDC0000U|(kind==2?4U:kind==4?0U:1U);
 world.work.halves[10]=kind==3?81:kind==1?INT16_MIN:100;world.work.halves[11]=100;
 if(kind>=3){world.garage.active21=255;for(j=0;j<2;j++){world.garage.image38[j]=0;world.garage.image40[j]=0;world.garage.palette48[j]=0;}}
 expected_calls=line_calls=0;memcpy(&expected,&world,sizeof(world));reference(kind);if(family==0)garage_pencil_stroke();else garage_line_stroke();
 assert(line_calls==expected_calls);assert(memcmp(&world,&expected,sizeof(world))==0);cases++;
}
int main(void){unsigned i,active,layer,kind;
 for(family=0;family<2;family++){for(i=0;i<65536;i++)for(active=0;active<2;active++)for(layer=0;layer<2;layer++)run(1,(i*73U+17)%4096,i%4096,active,layer?((i&1)?1:255):0);
 for(i=0;i<4096;i++)for(active=0;active<2;active++)for(layer=0;layer<2;layer++)run(0,i,4095,active,layer);
 for(kind=2;kind<5;kind++)for(i=0;i<256;i++)run(kind,0,0,0,0);
 }assert(cases==558592ULL);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
