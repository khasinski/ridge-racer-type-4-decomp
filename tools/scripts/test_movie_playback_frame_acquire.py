"""Frame acquisition retry limits, fresh playback roots and full frame/config publication."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class MoviePlaybackFrameAcquireTest(unittest.TestCase):
    def test_headers_retries_aliases_and_clear_callbacks(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/movie_playback/func_80115164.c').read_text()
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4||sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4||offsetof(t,f)==n,"PAL offset")
#include "rr/movie_playback_frame_acquire.h"
typedef struct {u32 guard[4];RrStrContextPrefix context;RrStrPlaybackState players[2];RrLibQueueRecord32 headers[2];RrStrDisplayConfig config;u32 tail[4];} State;
typedef struct {State s;RrFrameStorage frames[2];unsigned polls,clears;RrGpuRectangleFields clear;u32 beforeClear;u8 *result;} World;
static World world,initial,actual;
static u8 packets[3][128];static unsigned scenario,mutation,busy,seenLimits,seenClear,seenRoots;
static const unsigned delays[]={0,1,2,3,1998,1999,2000,2001};
static const u32 edges[]={0,1,255,256,257,65535,0x7FFFFFFFU,0x80000000U,0xFFFFFFFFU};
#define RaceContext world.s.context
#define FrameEnvs world.frames
static s32 word(u32 bits){s32 v;memcpy(&v,&bits,4);return v;}
static s16 half(u32 bits){u16 u=bits;s16 v;memcpy(&v,&u,2);return v;}
static u32 hash(void){unsigned i;u32 h=2166136261U;for(i=0;i<sizeof(world.s);i++)h=(h^((u8*)&world.s)[i])*16777619U;return h;}
static void effect(unsigned n,unsigned clear){unsigned p,i;
 world.s.context.activeB8=world.s.players+(scenario+n)%2;
 world.s.context.counterC4=word(edges[(scenario+n)%9]);world.s.context.budgetCC=edges[(scenario+n+2)%9];
 world.s.context.thresholdC8=edges[(scenario+n+4)%9];world.s.context.stopD0=word(scenario+n*119U);
 for(p=0;p<2;p++){
  RrStrPlaybackState *s=world.s.players+p;
  s->width00=word(edges[(scenario+n+p)%9]);s->height04=word(edges[(scenario+n+p+3)%9]);
  s->display24.rectangles[0].x=half(scenario+n+p*17);s->display24.rectangles[2].w=half(scenario+n+p*31);
  world.s.headers[p].value08=word(edges[(scenario+n+p+5)%9]);
  world.s.headers[p].width10=(u16)(scenario*271U+n*17+p*33);world.s.headers[p].height12=(u16)(scenario*313U+n*19+p*51);
  world.frames[p].environment.display.y=half(scenario*7U+n*23+p*21);
 }
 if(clear){world.s.context.activeB8=world.s.players+(scenario/8)%2;seenRoots|=1U<<((scenario/8)%2);}
 for(i=0;i<4;i++)world.s.tail[i]=scenario+n+i;
}
s32 StGetNext(u8 **data,RrLibQueueRecord32 **header){unsigned n=world.polls++;
 assert(data&&header);assert((uintptr_t)data>UINT32_MAX);assert((uintptr_t)header>UINT32_MAX);
 if(mutation)effect(n,0);
 *data=packets[(scenario+n)%3]+(scenario+n)%32;*header=world.s.headers+(scenario+n)%2;
 return n<busy?-1:0;
}
void ClearImage(const RrGpuRectangleFields *rect,u8 r,u8 g,u8 b){assert(!r&&!g&&!b);assert(world.clears++==0);world.clear=*rect;world.beforeClear=hash();if(mutation)effect(2017,1);}
''' + body + r'''
static u8 *expected(RrStrDisplayConfig *config){u8 *data;RrLibQueueRecord32 *header;unsigned i;RrStrPlaybackState *s;int found=0;RrGpuRectangleFields rect;
 for(i=0;i<2000;i++)if(!StGetNext(&data,&header)){found=1;break;}
 if(!found)return 0;
 if((u32)header->value08>=world.s.context.thresholdC8)world.s.context.stopD0=1;
 if((u32)world.s.context.counterC4>=world.s.context.budgetCC)world.s.context.stopD0=1;
 s=world.s.context.activeB8;
 if(s->width00!=header->width10||s->height04!=header->height12){
  s->width00=header->width10;
  ((RrStrPlaybackState*)world.s.context.activeB8)->height04=header->height12;
  s=world.s.context.activeB8;
  rect.x=rect.y=0;rect.w=half(word((u32)s->width00*3U)/2);rect.h=512;
  ClearImage(&rect,0,0,0);
 }
 s=world.s.context.activeB8;
 config->rectangles[0].y=half(world.frames[0].environment.display.y+word(256U-(u32)s->height04)/2);
 s=world.s.context.activeB8;
 config->rectangles[1].y=half(world.frames[1].environment.display.y+word(256U-(u32)s->height04)/2);
 s=world.s.context.activeB8;
 {s16 width=half(word((u32)s->width00*3U)/2);config->rectangles[1].w=width;config->rectangles[0].w=width;}
 s=world.s.context.activeB8;config->rectangles[1].h=half(s->height04);config->rectangles[0].h=config->rectangles[1].h;
 s=world.s.context.activeB8;config->rectangles[2].h=half(s->height04);
 s=world.s.context.activeB8;world.frames[1].environment.display.w=half(s->width00);world.frames[0].environment.display.w=world.frames[1].environment.display.w;
 world.frames[1].environment.display.h=half(s->height04);world.frames[0].environment.display.h=world.frames[1].environment.display.h;
 return data;
}
static RrStrDisplayConfig *output(void){return scenario&16?&world.s.players[1].display24:&world.s.config;}
int main(void){unsigned i;
 for(scenario=0;scenario<8192;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&world,0,sizeof(world));memset(world.frames,0xA5,sizeof(world.frames));
  effect(0,0);busy=delays[scenario%8];
  for(i=0;i<4;i++){world.s.guard[i]=scenario+i;world.s.tail[i]=~(scenario+i);}
  /* Exercise successful headers with unchanged dimensions as well as resize. */
  if(!mutation&&(scenario&32)&&busy<2000){RrLibQueueRecord32 *h=world.s.headers+(scenario+busy)%2;RrStrPlaybackState *p=world.s.context.activeB8;p->width00=h->width10;p->height04=h->height12;}
  initial=world;
  world.result=movie_playback_next_frame(output());actual=world;world=initial;world.result=expected(output());
  assert(!memcmp(&world,&actual,sizeof(world)));assert(world.polls==(busy<2000?busy+1:2000));
  if(busy>=2000)assert(world.result==0&&world.clears==0);else assert(world.result!=0);
  seenClear|=1U<<world.clears;seenLimits|=1U<<(scenario%8);
 }
 assert(seenLimits==255&&seenClear==3&&seenRoots==3);return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
