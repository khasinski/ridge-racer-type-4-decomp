"""Distance scale: full signed16 point differences and PAL modular squares."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class GarageDistanceTest(unittest.TestCase):
    def test_modular_argument_and_nonnegative_root_contract(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/garage/func_8011DBD8.c').read_text()
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
#include "rr/garage_distance.h"
static struct World {u32 guard[4];RrGaragePoint p[2];unsigned calls;s32 argument;u32 tail[4];} world,before,expected;
static s32 expected_argument,root_result;static unsigned scenario;
static u32 rng=0x1DBD8;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s16 half(u32 bits){u16 h=(u16)bits;s16 v;memcpy(&v,&h,2);return v;}
static s32 word(u32 bits){s32 v;memcpy(&v,&bits,4);return v;}
s32 CordicSqrt(s32 squared){
 assert(world.calls++==0&&squared==expected_argument);
 assert(memcmp(world.p,before.p,sizeof(world.p))==0);world.argument=squared;
 world.p[0].x=half(scenario+7);world.p[0].y=half(scenario*73U+17);
 world.p[1].x=half(scenario+13);world.p[1].y=half(scenario*251U+19);
 return root_result;
}
'''+body+r'''
int main(void){unsigned i,mode,j;uint64_t cases=0;
 static const s32 outputs[]={0,1,63,64,4095,4096,8191,8192,INT32_MAX,0x7FFFF000,4097,40960};
 for(i=0;i<65536;i++)for(mode=0;mode<8;mode++){
  const RrGaragePoint *a,*b;int64_t dx,dy,sum,den;u32 bits;s32 result;
  memset(&world,0,sizeof(world));scenario=i*8+mode;
  for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();}
  world.p[0].x=half(i);world.p[0].y=half(i*73U+17);world.p[1].x=half(next());world.p[1].y=half(next());
  if(mode==0){world.p[0]=(RrGaragePoint){0,0};world.p[1]=(RrGaragePoint){half(i),0};}
  if(mode==1){world.p[0]=(RrGaragePoint){0,0};world.p[1]=(RrGaragePoint){0,half(i)};}
  if(mode==2)world.p[0]=(RrGaragePoint){INT16_MIN,INT16_MAX};
  if(mode==3)world.p[0]=(RrGaragePoint){INT16_MAX,INT16_MIN};
  if(mode==4)world.p[1]=(RrGaragePoint){half(i^0xFFFFU),half(i*251U+19)};
  a=&world.p[mode==6?1:0];b=mode==5?a:&world.p[mode==6?0:1];
  dx=(int64_t)b->x-a->x;dy=(int64_t)b->y-a->y;
  sum=dx*dx*4+dy*dy;bits=(u32)((uint64_t)sum&0xFFFFFFFFU);expected_argument=word(bits);
  root_result=mode==7?(s32)(next()&0x7FFFFFFFU):outputs[(i+mode)%12];
  den=(int64_t)root_result/4096+1;assert(den>=1&&den<=524288);
  memcpy(&before,&world,sizeof(world));memcpy(&expected,&world,sizeof(world));
  expected.calls=1;expected.argument=expected_argument;
  expected.p[0].x=half(scenario+7);expected.p[0].y=half(scenario*73U+17);
  expected.p[1].x=half(scenario+13);expected.p[1].y=half(scenario*251U+19);
  result=garage_calc_glide_step(a,b);assert(result==(s32)(4096/den));
  assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)a>UINT32_MAX);cases++;
 }
 assert(cases==524288ULL);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
