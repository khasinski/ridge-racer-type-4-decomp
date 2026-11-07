from overlay_units import read_c_source
"""Race-state gate over real vehicle views, wrapped negation and real fade body."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayRaceStateTest(unittest.TestCase):
    def test_state_gate_and_fade_publication(self):
        root = Path(__file__).resolve().parents[2]
        body = read_c_source(root / 'src/overlays/PAL/660/func_80117084.c')
        fade = (root / 'tools/scripts/fixtures/legacy/func_8004FC18.c').read_text()
        fade = re.sub(r'^extern[^\n]*;\n', '', fade, flags=re.M)
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/overlay_race_state.h"
typedef struct {char pad[0x18];u16 a;s16 b,c;u16 d,e;} Fade;
static struct World {
 u32 before[8];RrRaceVehicleViews car;s32 mode,limit;s16 tag,delay;
 Fade fade;u32 calls;s32 arg0,seen_mode;s16 arg1,seen_delay;
 s32 post_mode,post_first;s16 post_delay;u32 after[8];
} world,expected;
#define RaceState world.mode
#define TrackLength world.limit
#define ReverseCourse world.tag
#define MenuFadeTimer world.delay
#define SoundState world.fade
void SoundStopAll(void){assert(0);}
void CdAudioFade(int a,short b){
 assert(world.mode==5 && world.delay==75);
 assert(world.fade.b==71 && world.fade.c==0 && world.fade.d==0 && world.fade.e==world.fade.a);
 world.calls++;world.arg0=a;world.arg1=b;world.seen_mode=world.mode;world.seen_delay=world.delay;
 world.mode=world.post_mode;world.delay=world.post_delay;world.car.vehicle.first=world.post_first;
}
''' + fade + body + r'''
static u32 seed=0x17084;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 word(u32 bits){s32 value;memcpy(&value,&bits,4);return value;}
static s16 half(u32 bits){u16 raw=bits;s16 value;memcpy(&value,&raw,2);return value;}
int main(void){unsigned k,m,i;
 const u32 edges[]={0,1,0xffffffffU,0x80000000U,0x7fffffffU,2,3,4};
 for(m=0;m<4;m++)for(k=0;k<65536;k++){
  u32 mode=m<2?m+2:m==2?next():edges[k&7];
  u32 limit=k<8?edges[k]:next();int64_t neg;int allow=0;
  memset(&world,0,sizeof(world));
  for(i=0;i<sizeof(world.car);i++)((u8*)&world.car)[i]=(u8)next();
  for(i=0;i<sizeof(world.fade);i++)((u8*)&world.fade)[i]=(u8)next();
  for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
  world.mode=word(mode);world.limit=word(limit);world.delay=half(next());
  world.car.vehicle.half1EA=half(k);world.tag=(k&1)?half(k):half(k^0x5555);
  world.car.parts.control.steering94.count22=half(k*40503U);
  neg=-(int64_t)world.limit;
  if(neg>INT32_MAX)neg-=INT64_C(4294967296);
  world.car.vehicle.first=(k&7)<3?word((u32)neg+(u32)((s32)(k&7)-1)):word(next());
  world.post_mode=word(next());world.post_delay=half(next());world.post_first=word(next());
  memcpy(&expected,&world,sizeof(world));
  if(mode==2 || mode==3){
   if(world.car.parts.control.steering94.count22<=0)allow=world.car.vehicle.half1EA!=world.tag;
   else allow=(int64_t)world.car.vehicle.first<=neg;
  }
  if(allow){
   expected.fade.b=71;expected.fade.c=0;expected.fade.d=0;expected.fade.e=expected.fade.a;
   expected.calls=1;expected.arg0=0;expected.arg1=71;expected.seen_mode=5;expected.seen_delay=75;
   expected.mode=world.post_mode;expected.delay=world.post_delay;expected.car.vehicle.first=world.post_first;
  }
  ovl660_func_80117084(&world.car);
  assert(memcmp(&world,&expected,sizeof(world))==0);
  assert((uintptr_t)&world.car>UINT32_MAX);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            path = Path(temp)
            (path / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(path / 'test.c'), '-o', str(path / 'test')], check=True)
            subprocess.run([str(path / 'test')], check=True)


if __name__ == '__main__':
    unittest.main()
