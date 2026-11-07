"""Actual garage C, scalar snapshots, rectangle boundaries and live aliases."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayGarageTest(unittest.TestCase):
    def test_native_garage_helpers(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/garage/func_{name}.c').read_text()
                           .replace('#include "rr/overlay662_garage.h"', '')
                           for name in ('8011E694', '8011E6C8', '8011E734', '8011E7DC'))
        source = r'''
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/overlay662_garage.h"
/* Each overlapping source is a genuine four-halfword array in a union view. */
typedef union {
 RrGarageWorkingStorage storage;
 s16 halves[32];
 RrGarageRectangle blocks[8];
 struct {u16 before[23];RrGarageRectangle rect;u16 after[5];} from23;
 struct {u16 before[25];RrGarageRectangle rect;u16 after[3];} from25;
 struct {u16 before[26];RrGarageRectangle rect;u16 after[2];} from26;
 struct {u16 before[22];RrGarageRectangle rect;u16 after[6];} from22;
} ClipViews;
static struct World {
 u32 guard0[8];
 RrGarageSelectionPrefix state;
 ClipViews clip;
 RrGarageRectangle rectangles[16];
 RrGarageRectangle *table[16];
 u32 guard1[8];
} world,initial,expected;
#define garage_D_801276A8 world.state
#define garage_D_80127668 world.clip.storage
#define garage_D_801273B8 world.table
''' + bodies + r'''
static u32 seed=0xe694;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 low16(u32 v){s32 n=v&65535U;return n<32768?n:n-65536;}
static s32 signed32(u32 v){s32 n;memcpy(&n,&v,4);return n;}
static int outside(s32 x,s32 y,s32 w,s32 h){
 s32 px=world.clip.halves[10],py=world.clip.halves[11];
 return !(px>=x && py>=y && (int64_t)px<(int64_t)x+w && (int64_t)py<(int64_t)y+h);
}
static void check(unsigned iteration,unsigned function){
 if(memcmp(&world,&expected,sizeof(world))){
  fprintf(stderr,"garage mismatch case=%u function=%u\n",iteration,function);
  assert(0);
 }
}
int main(void){
 u32 k,i,index;
 assert(sizeof(void*)==8 && (uintptr_t)&world>UINT32_MAX);
 assert(sizeof(ClipViews)==64 && sizeof(RrGarageRectangle)==8);
 assert(sizeof(RrGarageSelectionPrefix)==0x138 && offsetof(RrGarageSelectionPrefix,scratch34)==0x48 && offsetof(RrGarageSelectionPrefix,image38)==0x50 && offsetof(RrGarageSelectionPrefix,paletteBackup50)==0x80 && offsetof(RrGarageSelectionPrefix,paletteScratch58)==0x90 && offsetof(RrGarageSelectionPrefix,current60)==0xA0);
 for(k=0;k<65536;k++){
  s32 args[4];int answer;
  RrGarageRectangle *selected;
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  for(i=0;i<16;i++)world.table[i]=&world.rectangles[i];
  index=k%16;
  switch(k%7){
   case 0:break;
   case 1:world.table[index]=&world.clip.blocks[6];break;
   case 2:world.table[index]=&world.clip.from23.rect;break;
   case 3:world.table[index]=&world.clip.from25.rect;break;
   case 4:world.table[index]=&world.clip.from26.rect;break;
   case 5:world.table[index]=&world.clip.from22.rect;break;
   case 6:world.table[index]=&world.clip.blocks[5];break;
  }
  for(i=0;i<4;i++)args[i]=signed32(next());
  if(k&1){
   static const s32 delta[]={0,-1,1,-128};
   static const s32 span[]={0,1,2,128,-1,-32768,32767,255};
   world.clip.halves[10]=(s16)(k%201-100);
   world.clip.halves[11]=(s16)((k/201)%201-100);
   args[0]=signed32((next()&0xffff0000U)|(u16)(world.clip.halves[10]+delta[k%4]));
   args[1]=signed32((next()&0xffff0000U)|(u16)(world.clip.halves[11]+delta[(k/4)%4]));
   args[2]=signed32((next()&0xffff0000U)|(u16)span[(k/16)%8]);
   args[3]=signed32((next()&0xffff0000U)|(u16)span[(k/128)%8]);
   for(i=0;i<4;i++)(*world.table[index])[i]=(u16)args[i];
  }
  memcpy(&initial,&world,sizeof(world));
  world.state.saved64[0]=world.state.current60;
  for(i=0;i<4;i++)world.state.saved64[i+1]=world.state.selectorsB8[i];
  memcpy(&expected,&world,sizeof(world));memcpy(&world,&initial,sizeof(world));
  garage_func_8011E694();check(k,0);

  memcpy(&world,&initial,sizeof(world));
  answer=outside(low16(args[0]),low16(args[1]),low16(args[2]),low16(args[3]));
  memcpy(&expected,&world,sizeof(world));
  assert(garage_func_8011E6C8(args[0],args[1],args[2],args[3])==answer);check(k,1);

  selected=world.table[index];
  answer=outside(low16((*selected)[0]),low16((*selected)[1]),
                 low16((*selected)[2]),low16((*selected)[3]));
  assert(garage_func_8011E734(index)==answer);check(k,2);

  for(i=0;i<4;i++){
   u16 bits=(*selected)[i];
   memcpy(&world.clip.halves[24+i],&bits,2);
  }
  memcpy(&expected,&world,sizeof(world));memcpy(&world,&initial,sizeof(world));
  garage_func_8011E7DC(index);check(k,3);
 }
 puts("262144 native garage cases with UBSan passed");return 0;
}
'''
        with TemporaryDirectory() as tmp:
            path = Path(tmp)
            (path / 'garage.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I' + str(root / 'include'),
                            str(path / 'garage.c'), '-o', str(path / 'garage')],
                           check=True, capture_output=True)
            result = subprocess.run([str(path / 'garage')], check=True,
                                    capture_output=True, text=True)
            self.assertIn('262144 native garage cases', result.stdout)


if __name__ == '__main__':
    unittest.main()
