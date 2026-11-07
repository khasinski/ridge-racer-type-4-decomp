"""Pixel-reference tests for the packed64x64 toroidal image shift."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

class Overlay662ImageScrollTest(unittest.TestCase):
    def test_all_coordinate_differences_and_overlapping_images(self):
        root=Path(__file__).resolve().parents[2]
        # Preserve the executable C; remove only MIPS register-placement metadata.
        body=re.sub(r' asm\("[^"]*"\)','',(root/'src/overlays/PAL/garage/func_80125DE0.c').read_text())
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "rr/overlay662_image_scroll.h"
static struct {u32 head[8];u32 words[2048];u32 tail[8];} world,expected;
''' + body + r'''
static u32 seed=0x25DE0;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void reference(unsigned from,unsigned to,s32 x,s32 y){
 u32 image[512],output[512]={0};unsigned row,col;
 memcpy(image,&world.words[from],sizeof(image));
 for(row=0;row<64;row++)for(col=0;col<64;col++){
  unsigned sy=(row+64-y)&63U,sx=(col+64-x)&63U;
  u32 pixel=(image[sy*8+sx/8]>>(4*(sx%8)))&15U;
  output[row*8+col/8]|=pixel<<(4*(col%8));
 }
 memcpy(&expected.words[to],output,sizeof(output));
}
int main(void){
 s32 x,y;unsigned mode,i,scenario=0;static const s32 offsets[5]={-511,-1,0,1,511};
 for(y=-63;y<=63;y++)for(x=-63;x<=63;x++)for(mode=0;mode<5;mode++){
  unsigned from=768,to=(unsigned)((s32)from+offsets[mode]);
  for(i=0;i<2048;i++)world.words[i]=next();
  for(i=0;i<8;i++){world.head[i]=next();world.tail[i]=next();}
  if(scenario%257<3)for(i=0;i<512;i++)world.words[from+i]=scenario%257==0?0:scenario%257==1?0xFFFFFFFFU:0x76543210U^(i*0x01010101U);
  memcpy(&expected,&world,sizeof(world));reference(from,to,x,y);
  assert((uintptr_t)&world.words[from]>UINT32_MAX && (uintptr_t)&world.words[to]>UINT32_MAX);
  garage_func_80125DE0(&world.words[from],&world.words[to],x,y);
  assert(memcmp(&world,&expected,sizeof(world))==0);scenario++;
 }
 assert(scenario==80645);return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
