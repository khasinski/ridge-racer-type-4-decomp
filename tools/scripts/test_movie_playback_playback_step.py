"""Playback retry/threshold flow with full native owners and mutable services."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest


class PlaybackStepTest(unittest.TestCase):
    def test_state_roots_retries_and_fresh_inputs(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/movie_playback/func_80114C3C.c').read_text()
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4||sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4||offsetof(t,f)==n,"PAL offset")
#include "rr/overlay668_playback_step.h"
typedef struct {u32 guard[4],rng;RrStrContextPrefix context;RrMenuItemState5C controllers[2];s32 timer;u32 tail[4];} State;
typedef struct {s32 kind,args[4];u32 hash;} Event;
typedef struct {State s;Event events[64];unsigned count,queries;} World;
static World actual,reference,initial,*w;
static RrStrPlaybackState players[3],expectedPlayers[3],initialPlayers[3];
static RrStrCommandHeader headers[6];static u32 pixels[6][64];
static unsigned scenario,mutation,seen[16],querySeen[8],thresholdSeen[4];
static const s32 timers[8]={3,INT32_MAX,INT32_MIN,-1,0,4,2,5};
static const s32 frames[8]={0,-1,INT32_MIN,1,17,INT32_MAX,-4096,4096};
const char movie_playback_str_next_timeout_fmt[]="time out in strNext() %d\n";
#define RaceContext w->s.context
#define ModeFrame w->s.timer
#define PlayerControllers w->s.controllers
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static u32 next(void){u32 n=w->s.rng;n^=n<<13;n^=n>>17;n^=n<<5;return w->s.rng=n;}
static unsigned playerIndex(const void *p){uintptr_t n=(uintptr_t)p,start=(uintptr_t)(w==&actual?players:expectedPlayers);assert(n>UINT32_MAX&&n>=start&&n<start+sizeof(players));assert((n-start)%sizeof(RrStrPlaybackState)==0);return (unsigned)((n-start)/sizeof(RrStrPlaybackState));}
static RrStrPlaybackState *active(void){uintptr_t n=(uintptr_t)RaceContext.activeB8,start=(uintptr_t)players;assert(n>=start&&n<start+sizeof(players)&&(n-start)%sizeof(RrStrPlaybackState)==0);return (w==&actual?players:expectedPlayers)+(n-start)/sizeof(RrStrPlaybackState);}
static unsigned displayIndex(const RrStrDisplayConfig *p){return playerIndex((const u8*)p - offsetof(RrStrPlaybackState,display24));}
static Event *event(s32 kind){unsigned i;u32 h=2166136261U;RrStrPlaybackState *p=w==&actual?players:expectedPlayers;Event *e;
 assert(w->count<64);e=&w->events[w->count++];memset(e,0,sizeof(*e));e->kind=kind;
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 for(i=0;i<sizeof(players);i++)h=(h^((u8*)p)[i])*16777619U;e->hash=h;return e;
}
static void change(void){unsigned i,j;RrStrPlaybackState *p=w==&actual?players:expectedPlayers;if(!mutation)return;
 RaceContext.activeB8=players+next()%3;RaceContext.thresholdC8=(scenario&1)?UINT32_MAX:next();RaceContext.stopD0=word(next()%4);RaceContext.phaseB4=word(next());
 w->s.timer=word(next());w->s.controllers[0].buttons08=(u16)next();
 for(i=0;i<4;i++){RaceContext.currentC0.c[i]=(u8)next();RaceContext.locationBC.c[i]=(u8)next();}
 for(i=0;i<3;i++){
  p[i].display24.word00=next()%2;p[i].display24.word08=next()%2;
  p[i].display24.ready0C=next();p[i].display24.rectangles[2].w=(s16)next();p[i].display24.rectangles[2].h=(s16)next();
  for(j=0;j<2;j++){p[i].commands10[j]=headers+next()%6;p[i].buffers18[j]=pixels[next()%6];}
 }
}
void SetDispMask(s32 mode){event(1)->args[0]=mode;change();}
void movie_playback_dec_dct_in(RrStrCommandHeader *p,u32 flags){Event *e=event(2);assert(p>=headers&&p<headers+6);e->args[0]=p-headers;e->args[1]=word(flags);change();}
void movie_playback_dec_dct_out(void *p,u32 words){uintptr_t n=(uintptr_t)p,start=(uintptr_t)pixels;Event *e=event(3);assert(n>UINT32_MAX&&n>=start&&n<start+sizeof(pixels)&&(n-start)%sizeof(pixels[0])==0);e->args[0]=(n-start)/sizeof(pixels[0]);e->args[1]=word(words);change();}
s32 movie_playback_next_vlc(RrStrDisplayConfig *p){Event *e=event(4);s32 result;unsigned limit=scenario%8;
 e->args[0]=displayIndex(p);w->queries++;result=w->queries<=limit?-1:(scenario&8?INT32_MIN:0);e->args[1]=result;change();return result;
}
s32 StGetBackloc(const RrCdLocation *p){Event *e=event(5);s32 frame=(scenario&32)?(s32)(w->queries%2):frames[(scenario+w->queries)%8];assert(p==&RaceContext.currentC0);memcpy(&e->args[0],p,4);e->args[1]=frame;change();return frame;}
s32 DebugPrintStub(const char *format,...){va_list args;u32 frame;Event *e=event(6);assert(format==movie_playback_str_next_timeout_fmt);va_start(args,format);frame=va_arg(args,u32);va_end(args);e->args[0]=word(frame);change();return 1;}
void movie_playback_kick_cd(u8 *p){Event *e=event(7);assert(p==RaceContext.currentC0.c);memcpy(&e->args[0],p,4);change();}
void movie_playback_sync(RrStrDisplayConfig *p,s32 mode){Event *e=event(8);e->args[0]=displayIndex(p);e->args[1]=mode;assert(mode==0);change();}
void DrawSync(s32 mode){event(9)->args[0]=mode;change();}
void movie_playback_dec_dct_reset(s32 mode){event(10)->args[0]=mode;change();}
void SsSetSerialVol(s8 a,s32 b,s32 c){Event *e=event(11);e->args[0]=a;e->args[1]=b;e->args[2]=c;change();}
''' + body + r'''
static void expected(void){RrStrPlaybackState *p;RrCdLocation *location=&RaceContext.currentC0;s32 result;u32 frame;
 w->s.timer=word((u32)w->s.timer+1U);if(w->s.timer==4){seen[0]++;SetDispMask(1);}else seen[1]++;
 p=active();movie_playback_dec_dct_in(p->commands10[p->display24.word00],3);
 p=active();movie_playback_dec_dct_out(p->buffers18[p->display24.word08],(u32)((int64_t)p->display24.rectangles[2].w*p->display24.rectangles[2].h/2));
 for(;;){p=active();result=movie_playback_next_vlc(&p->display24);if(result!=-1){seen[2]++;break;}
  frame=(u32)StGetBackloc(location);DebugPrintStub(movie_playback_str_next_timeout_fmt,frame);
  thresholdSeen[(frame>=RaceContext.thresholdC8)*2+(word(frame)<=0)]++;
  if(frame>=RaceContext.thresholdC8){RaceContext.stopD0=1;seen[3]++;break;}
  if(word(frame)<=0){memcpy(location->c,RaceContext.locationBC.c,4);seen[4]++;}else seen[5]++;
  movie_playback_kick_cd(location->c);
 }
 assert(w->queries>=1&&w->queries<=8);querySeen[w->queries-1]++;
 movie_playback_sync(&active()->display24,0);DrawSync(0);movie_playback_dec_dct_reset(1);
 if(RaceContext.stopD0==1){RaceContext.phaseB4=2;SsSetSerialVol(0,0,0);seen[6]++;}else seen[7]++;
 if(w->s.controllers[0].buttons08&2048){SsSetSerialVol(0,0,0);RaceContext.phaseB4=2;seen[8]++;}else seen[9]++;
}
int main(void){unsigned i,j;uint64_t cases=0;
 for(scenario=0;scenario<32768;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));memset(initialPlayers,0,sizeof(initialPlayers));w=&initial;w->s.rng=0x14C3C+scenario*73;
  for(i=0;i<4;i++){w->s.guard[i]=next();w->s.tail[i]=next();RaceContext.currentC0.c[i]=(u8)next();RaceContext.locationBC.c[i]=(u8)next();}
  RaceContext.activeB8=players+scenario%3;RaceContext.thresholdC8=(scenario&1)?UINT32_MAX:next();RaceContext.stopD0=scenario%4;RaceContext.phaseB4=word(next());
  w->s.timer=(scenario&16)?timers[(scenario>>5)%8]:word(next());w->s.controllers[0].buttons08=(u16)next();
  for(i=0;i<3;i++){
   initialPlayers[i].display24.word00=next()%2;initialPlayers[i].display24.word08=next()%2;
   initialPlayers[i].display24.rectangles[2].w=(s16)next();initialPlayers[i].display24.rectangles[2].h=(s16)next();
   for(j=0;j<2;j++){initialPlayers[i].commands10[j]=headers+i*2+j;initialPlayers[i].buffers18[j]=pixels[i*2+j];}
  }
  actual=reference=initial;memcpy(players,initialPlayers,sizeof(players));memcpy(expectedPlayers,initialPlayers,sizeof(players));
  w=&actual;movie_playback_step();w=&reference;expected();assert(!memcmp(&actual,&reference,sizeof(actual)));assert(!memcmp(players,expectedPlayers,sizeof(players)));cases++;
 }
 assert(cases==65536);for(i=0;i<10;i++)assert(seen[i]);for(i=0;i<8;i++)assert(querySeen[i]);for(i=0;i<4;i++)assert(thresholdSeen[i]);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p = Path(tmp)
            (p / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(p / 'test.c'), '-o', str(p / 'test')], check=True)
            subprocess.run([str(p / 'test')], check=True)
