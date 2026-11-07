"""Actual grid mapper, original ROM grids, rectangle centers and distance body."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest

class Overlay662MapperTest(unittest.TestCase):
    def test_every_original_grid_cell_and_root_side_effects(self):
        root=Path(__file__).resolve().parents[2]
        paths=['8011D278','8011DC74','8011DBD8','8011E5C0']
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
#include "rr/overlay662_options.h"
#include "rr/overlay662_distance.h"
static struct World {u32 head[4];RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;RrGarageRectangle rects[46];RrGarageRectangle *table[46];u32 tail[4];}world,expected,checkpoint;
#define garage_D_801276A8 world.garage
#define garage_D_80127668 world.work
#define garage_D_801273B8 world.table
RrGarageOptionRecord garage_D_801268CC[5]={{{0,0,0,19,15,0,0,0,0,0}},{{1,2,3,19,15,0,0,0,0,0}},{{4,5,6,19,15,0,0,0,0,0}},{{7,7,8,19,15,0,0,0,0,0}},{{9,9,10,19,15,0,0,0,0,0}}};
RrGarageOptionRecord garage_D_80126994[2]={{{11,12,20,16,0,0,0,0,0,0}},{{13,14,20,16,0,0,0,0,0,0}}};
RrGarageOptionRecord garage_D_801269E4[2]={{{21,22,23,24,25,26,27,28,17,18}},{{29,30,31,32,33,34,35,35,17,18}}};
RrGarageOptionRecord garage_D_80126A34[2]={{{0,1,4,7,0,0,0,0,0,0}},{{0,2,5,7,0,0,0,0,0,0}}};
RrGarageOptionRecord garage_D_80126A84[3]={{{7,9,11,13,21,29,0,0,0,0}},{{7,9,11,13,22,30,0,0,0,0}},{{7,9,11,13,23,31,0,0,0,0}}};
RrGarageOptionRecord garage_D_80126AFC[5]={{{0,3,6,8,10,12,14,24,32,0}},{{0,3,6,8,10,12,14,25,33,0}},{{0,3,6,8,10,12,14,26,34,0}},{{0,3,6,8,10,12,14,27,35,0}},{{0,3,6,8,10,12,14,28,28,0}}};
RrGarageOptionRecord garage_D_80126BC4[1]={{{19,20,17,0,0,0,0,0,0,0}}};
RrGarageOptionRecord garage_D_80126BEC[1]={{{15,16,18,0,0,0,0,0,0,0}}};
static RrGarageOptionRecord *tables[8]={garage_D_801268CC,garage_D_80126994,garage_D_801269E4,garage_D_80126A34,garage_D_80126A84,garage_D_80126AFC,garage_D_80126BC4,garage_D_80126BEC};

'''+bodies+r'''
static u32 rng=0x1E5C0;static unsigned group,row,col,variant,calls;static s32 argument,root_result;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s16 half(u32 x){u16 n=(u16)x;s16 v;memcpy(&v,&n,2);return v;}
static s32 word(u32 x){s32 v;memcpy(&v,&x,4);return v;}
static void effect(struct World*w){w->work.points.first20=(RrGaragePoint){half(variant),half(variant*251U)};w->work.points.second24=(RrGaragePoint){half(variant*73U),half(variant*127U)};w->garage.current60=word(0x87654321U^variant);w->work.points.distance2C=-123;w->garage.selectorsB8[0]=-1;w->garage.options78.flat[0]=tables[7];}
s32 CordicSqrt(s32 value){assert(calls++==0&&value==argument);assert(memcmp(&world,&checkpoint,sizeof(world))==0);effect(&world);return root_result;}
static s32 floor_half(s32 n){return n>=0?n/2:-((-n+1)/2);}
static void reference(void){s32 target=tables[group][row].destinations[col];RrGarageRectangle *ptr=expected.table[target];unsigned n=(unsigned)(ptr-world.rects);s32 x,y,w,h;int64_t dx,dy,sum;
 assert(n<46);x=half(expected.rects[n][0]);y=half(expected.rects[n][1]);w=half(expected.rects[n][2]);h=half(expected.rects[n][3]);expected.work.points.second24.x=half((u32)(x+floor_half(w)));expected.work.points.second24.y=half((u32)(y+floor_half(h)));
 dx=(int64_t)expected.work.points.second24.x-expected.work.points.first20.x;dy=(int64_t)expected.work.points.second24.y-expected.work.points.first20.y;sum=dx*dx*4+dy*dy;argument=word((u32)(uint64_t)sum);
 checkpoint=expected;effect(&expected);expected.work.points.distance2C=4096/(root_result/4096+1);expected.garage.current60=target;
}
int main(void){unsigned j;uint64_t cases=0;static const unsigned rows[8]={5,2,2,2,3,5,1,1};static const s32 outputs[]={0,1,4095,4096,8192,INT32_MAX,0x7FFFF000,40960};
 for(group=0;group<8;group++)for(row=0;row<rows[group];row++)for(col=0;col<10;col++)for(variant=0;variant<256;variant++){
 s32 mode;memset(&world,0,sizeof(world));for(j=0;j<4;j++){world.head[j]=next();world.tail[j]=next();world.garage.selectorsB8[j]=word(next());}
 for(j=0;j<16;j++)world.work.words[j]=next();for(j=0;j<46;j++){unsigned k;for(k=0;k<4;k++)world.rects[j][k]=(u16)next();world.table[j]=&world.rects[(j+variant)%46];}
 garage_func_8011D278(-1);world.garage.optionIndices98.flat[group]=(s32)row;
 if(group<3){world.garage.selectorsB8[0]=(s32)group;world.garage.selectorsB8[2]=(s32)col;switch(variant&3){case 0:mode=1;break;case 1:mode=INT32_MIN;break;case 2:mode=INT32_MAX;break;default:mode=word(next()|1U);break;}}
 else{mode=0;world.garage.selectorsB8[1]=(s32)group-3;world.garage.selectorsB8[3]=(s32)col;}
 root_result=outputs[variant%8];expected=world;calls=0;reference();garage_func_8011E5C0(mode);assert(calls==1);assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.garage.options78.flat[group]>UINT32_MAX);cases++;
 }assert(cases==53760ULL);return 0;}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
