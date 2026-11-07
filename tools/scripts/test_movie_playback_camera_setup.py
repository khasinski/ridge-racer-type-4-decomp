"""Replay camera setup with complete track tables, native roots and mutating services."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess, unittest
ROOT=Path(__file__).resolve().parents[2]
class ReplaySetupTest(unittest.TestCase):
 def test_sample320_initialization_and_service_reload_order(self):
  body=(ROOT/'src/overlays/PAL/movie_playback/func_80115DFC.c').read_text()
  body='\n'.join(line for line in body.splitlines() if not line.startswith('#include'))
  code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "rr/overlay668_camera.h"
typedef struct {u32 before[4];RrTrackSample3C samples[321];u32 after[4];} Track;
typedef struct {u32 before[4];RrReplayCameraContextPrefix camera;
 u32 cameraGuard[4];Track tracks[2];s32 lookup[20];RrTrackSample3C *root;
 s32 draw,clock,ready,delay,selected,bank;s16 row;u16 pad;u32 after[4];} Environment;
static Environment actual,expected,pre[27];
static unsigned eventIds[27],count,index_,profile,caseIndex;static u32 seed=0x66815dfc;
static unsigned roots[3],entries[20];
static u32 next(void){u32 x=seed;x^=x<<13;x^=x>>17;x^=x<<5;return seed=x;}
static s32 sign32(u32 x){s32 result;memcpy(&result,&x,4);return result;}
static s16 sign16(u16 x){s16 result;memcpy(&result,&x,2);return result;}
static unsigned root_index(RrTrackSample3C *root){
 if(!root)return 2;if(root==actual.tracks[0].samples)return 0;
 assert(root==actual.tracks[1].samples);return 1;
}
static void mutate(Environment *e,unsigned id,unsigned ordinal){
 unsigned which=(caseIndex+id+ordinal)&1;unsigned slot=(caseIndex+ordinal)%20;
 if(profile){
  e->camera.position450[0]=sign32((u32)e->camera.position450[0]+0x7fffffffU);
  e->camera.node498=sign32((u32)e->camera.node498+ordinal+1U);
  e->camera.offset488[3]=sign16((u16)(caseIndex+ordinal));
  e->tracks[which].samples[320].x08=sign32((u32)e->tracks[which].samples[320].x08+0x87654321U);
  e->tracks[which].samples[320].heading12=sign16((u16)(caseIndex+ordinal*32767U));
  e->draw=-7;e->clock=-8;e->ready=-9;e->delay=-10;e->selected=-11;
  e->lookup[slot]=sign32(0xabcdef00U+caseIndex+ordinal);
 }
 /* Before sample capture, the last setup callback guarantees a valid root.
  * Later callbacks can null it: the initializer no longer reads the table. */
 e->root=id>=5&&profile&&((ordinal+caseIndex)&1)?0:actual.tracks[which].samples;
 e->bank=sign32(((caseIndex&1)?0x40000000U:0U)+((caseIndex+ordinal)%5));
 e->row=(s16)((caseIndex+ordinal)%4);
 if((caseIndex&4)&&((caseIndex+ordinal)%5)>0){e->row-=4;}
}
static void expect_event(unsigned id){assert(count<27);pre[count]=expected;eventIds[count]=id;
 mutate(&expected,id,count);count++;}
static void service(unsigned id){assert(index_<count&&eventIds[index_]==id);
 assert(!memcmp(&actual,&pre[index_],sizeof(actual)));
 mutate(&actual,id,index_);index_++;}
static void call0(s32 mode){assert(mode==0);service(0);}
static void call1(s32 r,s32 g,s16 b,u32 flags){assert(r==0&&g==0&&b==0&&flags==1);service(1);}
static void call2(void){service(2);}
static void call3(s32 mode){assert(mode==5);service(3);}
static void call4(void){service(4);}
static void call5(s32 *position){assert(position==actual.camera.position450);service(5);}
static void call6(void){service(6);}
static void call7(void){service(7);}
static void call8(void){service(8);}
static void call9(void){service(9);}
static void call10(s32 mode,u8 r,u8 g,u8 b){assert(mode==0&&r==144&&g==144&&b==144);service(10);}
static void call11(s32 enabled){assert(enabled==1);service(11);}
#define RaceContext (actual.camera)
#define TrackNodes (actual.root)
#define VSyncThreshold (actual.draw)
#define ViewUsesCarTrackNode (actual.clock)
#define MaskBitSetting (actual.ready)
#define D_80127E90 (actual.delay)
#define D_80127E94 (actual.selected)
#define GameLanguage (actual.bank)
#define SelectedTeamIndex (actual.row)
#define movie_playback_D_80116CC0 (actual.lookup)
#define SetDispMask call0
#define SetupDrawDispEnvs call1
#define InitViewState call2
#define SetupRenderer call3
#define BuildCameraMatrices call4
#define movie_playback_func_80115460 call5
#define movie_playback_func_80115554 call6
#define movie_playback_func_801156FC call7
#define movie_playback_func_80115850 call8
#define ResetScreenCopyOverlay call9
#define SetSceneColorParams call10
#define SetSceneOverlayEnabled call11
'''+body+r'''
/* Oracle writes the specified storage offsets, independently of C field names. */
static void put32(unsigned offset,u32 value){memcpy((u8 *)&expected.camera+offset,&value,4);}
static void put16(unsigned offset,u16 value){memcpy((u8 *)&expected.camera+offset,&value,2);}
static void model(void){unsigned i,which,selected;RrTrackSample3C sample;
 expect_event(0);expect_event(1);expected.draw=384;expect_event(2);expect_event(3);expect_event(4);
 which=root_index(expected.root);assert(which<2);roots[which]++;
 sample=expected.tracks[which].samples[320];
 put32(0x444,0);put32(0x44c,0);put32(0x4a4,0);put32(0x4a8,1);put32(0x4ac,0);
 put32(0x450,(u32)((int64_t)sample.x08*4));put32(0x454,(u32)((int)sample.y10-120));
 put32(0x458,(u32)((int64_t)sample.z0C*4));put16(0x460,0);
 put16(0x462,(u16)(-(int)sample.heading12));put16(0x464,0);
 put16(0x488,0);put16(0x48a,0);put16(0x48c,0);put16(0x490,(u16)-96);
 expected.clock=0;put16(0x492,0);put16(0x494,0);put32(0x498,320);
 expect_event(5);for(i=0;i<16;i++)expect_event(6);
 expect_event(7);expect_event(8);expect_event(9);expect_event(10);expect_event(11);
 expected.ready=1;expected.delay=16000;
 selected=(u32)expected.bank*4U+(u32)(s32)expected.row;
 assert(selected<20);entries[selected]++;expected.selected=expected.lookup[selected];
}
int main(void){unsigned raw,i,b;uint64_t cases=0;
 /* All heading and height encodings are covered in each callback profile. */
 for(raw=0;raw<65536;raw++)for(profile=0;profile<2;profile++){
  unsigned char *bytes=(unsigned char *)&actual;
  caseIndex=raw;
  for(i=0;i<sizeof(actual);i++)bytes[i]=(unsigned char)(i*37U+raw+profile);
  for(b=0;b<2;b++){
   actual.tracks[b].samples[320].x08=sign32(next());
   actual.tracks[b].samples[320].z0C=sign32(next());
   actual.tracks[b].samples[320].y10=sign16((u16)raw);
   actual.tracks[b].samples[320].heading12=sign16((u16)raw);
  }
  for(i=0;i<20;i++)actual.lookup[i]=sign32(next());
  actual.root=0;actual.bank=0;actual.row=0;
  expected=actual;count=index_=0;model();
  movie_playback_func_80115DFC();assert(count==27&&index_==count);
  assert(!memcmp(&actual,&expected,sizeof(actual)));if(!actual.root)roots[2]++;cases++;
 }
 for(i=0;i<3;i++)assert(roots[i]);for(i=0;i<20;i++)assert(entries[i]);
 printf("%llu cases;27 ordered services, full native roots, camera and two321-sample owners\n",(unsigned long long)cases);
}
'''
  with TemporaryDirectory() as temp:
   path=Path(temp);(path/'test.c').write_text(code)
   subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-Wno-unused-function','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(path/'test.c'),'-o',str(path/'test')],check=True)
   subprocess.run([str(path/'test')],check=True)
if __name__=='__main__':unittest.main()
