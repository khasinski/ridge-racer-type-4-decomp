"""STR live buffer state, captured rectangles and startup retry protocol."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class StreamTest(unittest.TestCase):
    def test_stream_and_start(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/movie_playback/func_{a}.c').read_text()
                           for a in ('80114F48', '80114B2C'))
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/overlay668_stream.h"
typedef struct {
 u32 guard[4],rng;RrStrPlaybackState players[3];s32 active,stream,flag,setting,frame;u8 volume;u32 tail[4];
} State;
typedef struct {s32 op,args[5];u32 hash;RrGpuRectangleFields rect;} Event;
typedef struct {State s;unsigned calls,queries;Event events[32];} World;
static World actual,reference,initial,*w;
static u8 pixels[3][2][64],resource[0x19B00];
static RrStrStreamPrefix streams[3];
RrStrContextPrefix RaceContext;
RrStrStreamPrefix *StrStreamState;
static unsigned scenario,mutation;
#define StCdIntrFlag w->s.flag
#define VSyncThreshold w->s.setting
#define ModeFrame w->s.frame
#define OptionsMusicVolume w->s.volume
''' + bodies + r'''
static u32 next(void){u32 x=w->s.rng;x^=x<<13;x^=x>>17;x^=x<<5;return w->s.rng=x;}
static s16 half(u32 v){u16 a=v;s16 b;memcpy(&b,&a,2);return b;}
static void roots(void){RaceContext.activeB8=&w->s.players[w->s.active];StrStreamState=&streams[w->s.stream];}
static void change(void){unsigned i,j;
 w->s.active=next()%3;w->s.stream=next()%3;w->s.flag=(s32)next();w->s.setting=(s32)next();w->s.frame=(s32)next();w->s.volume=next();
 RaceContext.phaseB4=(s32)next();RaceContext.counterC4=(s32)next();
 for(i=0;i<3;i++){
  RrStrPlaybackState *p=&w->s.players[i];p->width00=(s32)next();p->height04=(s32)next();
  p->display24.word08=next()%2;p->display24.alternate04=next()%2;p->display24.ready0C=next();
  for(j=0;j<3;j++){p->display24.rectangles[j].x=half(next());p->display24.rectangles[j].y=half(next());p->display24.rectangles[j].w=half(next());p->display24.rectangles[j].h=half(next());}
  for(j=0;j<2;j++)p->buffers18[j]=pixels[i][j];
 }
 roots();
}
static u32 hashbytes(const void *p,size_t n){size_t i;u32 h=2166136261U;for(i=0;i<n;i++)h=(h^((const u8*)p)[i])*16777619U;return h;}
static Event *event(s32 op,s32 a,s32 b,s32 c,s32 d,s32 e){
 Event *r;RrStrContextPrefix ctx=RaceContext;ctx.activeB8=0;
 assert(w->calls<32);r=&w->events[w->calls++];memset(r,0,sizeof(*r));r->op=op;r->args[0]=a;r->args[1]=b;r->args[2]=c;r->args[3]=d;r->args[4]=e;r->hash=hashbytes(&w->s,sizeof(w->s))^hashbytes(&ctx,sizeof(ctx));return r;
}
static void effect(void){if(mutation)change();}
void StCdInterrupt(void){event(1,0,0,0,0,0);effect();}
void movie_playback_func_80116478(void *output,u32 words){unsigned i,j;int found=-1;for(i=0;i<3;i++)for(j=0;j<2;j++)if(output==pixels[i][j])found=i*2+j;assert(found>=0);event(2,found,(s32)words,0,0,0);effect();}
void LoadImage(const RrGpuRectangleFields *rect,const void *image){unsigned i,j;int found=-1;Event *e;for(i=0;i<3;i++)for(j=0;j<2;j++)if(image==pixels[i][j])found=i*2+j;assert(found>=0);e=event(3,found,0,0,0,0);e->rect=*rect;effect();}
void SetDispMask(s32 mode){event(4,mode,0,0,0,0);effect();}
void SsSetSerialAttr(s8 a,s8 b,s8 c){event(5,a,b,c,0,0);effect();}
void SsSetSerialVol(s8 a,s32 l,s32 r){event(6,a,l,r,0,0);effect();}
void movie_playback_func_80114E48(u8 *base){assert(base==resource);event(7,0,0,0,0,0);effect();}
void movie_playback_func_80114E90(RrStrDisplayConfig *config,s32 a,s32 b,s32 c,s32 d){assert(config==&w->s.players[w->s.active].display24);event(8,a,b,c,d,0);effect();}
void movie_playback_func_80114EE8(RrLibInterruptCallback callback){assert(callback==movie_playback_func_80114F48);event(9,0,0,0,0,0);effect();}
s32 movie_playback_func_801150D8(RrStrDisplayConfig *config){s32 r=w->queries++<(scenario%5)?-1:(s32)(scenario%3);assert(config==&w->s.players[w->s.active].display24);event(10,r,0,0,0,0);effect();return r;}
void movie_playback_func_801153F0(u8 *location){assert(location==RaceContext.currentC0.c);event(11,0,0,0,0,0);RaceContext.currentC0.c[0]++;effect();}
s32 VSync(s32 mode){event(12,mode,0,0,0,0);effect();return -1;}
static void expected(unsigned kind){
 if(kind){s32 volume;SetDispMask(0);w->s.players[w->s.active].width00=320;w->s.players[w->s.active].height04=240;SsSetSerialAttr(0,0,1);volume=w->s.volume*127/16;SsSetSerialVol(0,volume,volume);w->s.setting=128;
  movie_playback_func_80114E48(resource);movie_playback_func_80114E90(&w->s.players[w->s.active].display24,0,24,0,264);movie_playback_func_80114EE8(movie_playback_func_80114F48);
  for(;;){if(movie_playback_func_801150D8(&w->s.players[w->s.active].display24)!=-1)break;movie_playback_func_801153F0(RaceContext.currentC0.c);}
  RaceContext.phaseB4=1;RaceContext.counterC4=0;w->s.frame=0;VSync(0);
 }else{
  RrStrPlaybackState *p;RrGpuRectangleFields old;unsigned previous;
  if(w->s.flag){StCdInterrupt();w->s.flag=0;}
  p=&w->s.players[w->s.active];previous=p->display24.word08;old=p->display24.rectangles[2];p->display24.word08=!previous;
  p->display24.rectangles[2].x=half((u32)((s32)old.x+old.w));
  if(p->display24.rectangles[2].x<(s32)p->display24.rectangles[p->display24.alternate04].x+p->display24.rectangles[p->display24.alternate04].w){
   movie_playback_func_80116478(p->buffers18[p->display24.word08],(u32)(((s32)old.w*old.h)/2));
  }else{unsigned other=!p->display24.alternate04;p->display24.ready0C=1;p->display24.alternate04=other;p->display24.rectangles[2].x=p->display24.rectangles[other].x;p->display24.rectangles[2].y=p->display24.rectangles[other].y;}
  LoadImage(&old,streams[w->s.stream].buffers18[previous]);
 }
}
int main(void){unsigned kind,i,j;RrStrContextPrefix initialCtx,aCtx,rCtx;
 for(i=0;i<3;i++)for(j=0;j<2;j++)streams[i].buffers18[j]=pixels[i][j];assert((uintptr_t)pixels>UINT32_MAX);
 for(scenario=0;scenario<65536;scenario++)for(kind=0;kind<2;kind++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));memset(&RaceContext,0,sizeof(RaceContext));w=&initial;w->s.rng=0x14f48+scenario*19+kind;change();w->s.flag=scenario%2;initialCtx=RaceContext;actual=reference=initial;
  w=&actual;RaceContext=initialCtx;roots();if(kind)movie_playback_func_80114B2C(resource);else movie_playback_func_80114F48();aCtx=RaceContext;aCtx.activeB8=0;
  w=&reference;RaceContext=initialCtx;roots();expected(kind);rCtx=RaceContext;rCtx.activeB8=0;
  assert(!memcmp(&actual,&reference,sizeof(actual)));assert(!memcmp(&aCtx,&rCtx,sizeof(aCtx)));
 }
 return 0;
}
'''
        with TemporaryDirectory() as d:
            p = Path(d)
            (p / 'test.c').write_text(source)
            run = subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                                  '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                                  str(p / 'test.c'), '-o', str(p / 'test')], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)
            subprocess.run([str(p / 'test')], check=True)
