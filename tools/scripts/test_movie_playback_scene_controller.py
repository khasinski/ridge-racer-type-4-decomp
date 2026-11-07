from pathlib import Path
import os,re,subprocess,sys
os.chdir(Path(__file__).resolve().parents[2]);p=Path('build/goal_50/parallel668')
source=Path(sys.argv[1]) if len(sys.argv)>1 else Path('src/overlays/PAL/movie_playback/func_80115F80.c')
if not source.exists():source=p/'ready.c'
s=source.read_text();body=s[s.index('void movie_playback_func_80115F80'):]
head=r'''
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/overlay668_camera.h"
#include "rr/icon_sprites.h"
#include "rr/tables.h"
typedef struct {s32 key,action;} RrReplayEvent;
void AddScreenFadeTiles(u32*,s32,u32);void CdAudioBeginPlay(s32);void CdAudioFade(u16,s32);void SetupPlayerView(s32);void DrawTrackSceneryForPlayer(s32);
void movie_playback_func_80115BD0(s32);void movie_playback_func_801159C0(s32,s32);
static struct State{u32 guard[4];RrReplayCameraContextPrefix camera;RrReplayEvent events[3];RrMenuItemState5C input[2];RrIconSpriteContext contexts[2];unsigned root;s32 textPosition,textDelta;s16 enable,scene,phase;u32 tail[4];}a,b,*w;
#define RaceContext w->camera
#define movie_playback_D_80116CA8 w->events
#define PlayerControllers w->input
#define CurrentDrawBuf (&w->contexts[w->root])
#define D_80127E90 w->textPosition
#define D_80127E94 w->textDelta
#define ModeGroup w->enable
#define ModeId w->scene
#define ModeSubState w->phase
'''
rest=r'''
static struct Event{struct State before;s32 args[4];}trace[90];static unsigned scenario,profile,serial,count,seen,coverage[13],stages[8],modes[3],exitCount,startCount,fadeCalls;static int ref;
static s32 word(u32 x){s32 y;memcpy(&y,&x,4);return y;}
static void hit(unsigned k,s32 x,s32 y,s32 z){s32 args[4]={k,x,y,z};if(ref){assert(count<90);trace[count].before=*w;memcpy(trace[count++].args,args,sizeof(args));}else{coverage[k]++;assert(seen<count);assert(!memcmp(w,&trace[seen].before,sizeof(*w)));assert(!memcmp(args,trace[seen++].args,sizeof(args)));}}
static void effect(unsigned k){unsigned n=serial++;if(!(profile&1))return;u32 v=(scenario*0x9E3779B9U+n*0x1234567U+k);w->camera.counter444=word(v);w->camera.state44C=word(v+1);w->camera.mode4A8=(scenario+n)%4;w->camera.phase4AC=(scenario/4+n)%3;w->camera.counter4A4=word(v);w->camera.position450[0]=word(v+3);w->camera.position450[2]=word(v-7);w->camera.delta478[0]=word(v*3);w->camera.delta478[2]=word(~v);w->root=(scenario+n)&1;w->textPosition=word(v*17);w->textDelta=word(v*29);w->input[0].buttons08=(u16)v;}
void movie_playback_func_80115DFC(void){hit(0,0,0,0);effect(0);}void SetDispMask(s32 x){hit(1,x,0,0);effect(1);}void CdAudioBeginPlay(s32 x){hit(2,x,0,0);effect(2);}void movie_playback_func_80115554(void){hit(3,0,0,0);effect(3);}void movie_playback_func_80115460(s32 *v){assert(v==w->camera.position450&&(uintptr_t)v>UINT32_MAX);hit(4,0,0,0);effect(4);}void movie_playback_func_801156FC(void){hit(5,0,0,0);effect(5);}void movie_playback_func_80115850(void){hit(6,0,0,0);effect(6);}void movie_playback_func_80115BD0(s32 x){hit(7,x,0,0);effect(7);}void CdAudioFade(u16 x,s32 y){hit(8,x,y,0);effect(8);}void AddScreenFadeTiles(u32*p,s32 x,u32 y){assert(p==&w->contexts[w->root].head70&&(uintptr_t)p>UINT32_MAX);hit(9,w->root,x,y);effect(9);}void movie_playback_func_801159C0(s32 x,s32 y){hit(10,x,y,0);effect(10);}void SetupPlayerView(s32 x){hit(11,x,0,0);effect(11);}void DrawTrackSceneryForPlayer(s32 x){hit(12,x,0,0);effect(12);}
/* Independent original-MIPS control flow, using explicit wrapping words. */
static void oracle(void){s32 stage=w->camera.counter444;
 if(stage==0){movie_playback_func_80115DFC();w->camera.counter444=word((u32)w->camera.counter444+1);return;}
 if(stage>=1&&stage<=4){if(stage==4)SetDispMask(1);w->camera.counter444=word((u32)w->camera.counter444+1);return;}
 if(stage!=5)return;
 int action=-1;for(int j=0;j<3&&w->events[j].key>=0;j++)if(w->events[j].key==w->camera.state44C){action=w->events[j].action;break;}
 if(action==0){w->camera.phase4AC=1;w->camera.counter4B4=0;CdAudioBeginPlay(0);}else if(action==1)w->camera.mode4A8=2;
 movie_playback_func_80115554();w->camera.position450[0]=word((u32)w->camera.position450[0]+(u32)(w->camera.delta478[0]/6));w->camera.position450[2]=word((u32)w->camera.position450[2]+(u32)(w->camera.delta478[2]/6));movie_playback_func_80115460(w->camera.position450);movie_playback_func_801156FC();movie_playback_func_80115850();movie_playback_func_80115BD0(2);
 int mode=w->camera.mode4A8;if(mode==1||mode==2){modes[mode]++;s32 value=word((u32)w->camera.counter4A4+(mode==1?4U:0xFFFFFFFCU));w->camera.counter4A4=value;
  if(mode==1&&value>=255){w->camera.counter4A4=255;w->camera.mode4A8=0;}else if(mode==2&&value<=0){exitCount++;w->camera.counter4A4=0;w->camera.mode4A8=0;CdAudioFade(0,0);w->enable=2;w->scene=3;w->camera.flagB0=1;w->phase=0;}
  fadeCalls++;AddScreenFadeTiles(&w->contexts[w->root].head70,word(255U-(u32)w->camera.counter4A4),69);
 }
 if(w->camera.phase4AC==1){w->textPosition=word((u32)w->textPosition-(u32)w->textDelta);for(int j=0;j<62;j++)movie_playback_func_801159C0(j,160);}
 if((w->input[0].buttons08&2048)&&w->camera.mode4A8==0){startCount++;w->camera.mode4A8=2;CdAudioFade(0,21);}
 SetupPlayerView(0);DrawTrackSceneryForPlayer(0);w->camera.state44C=word((u32)w->camera.state44C+1);movie_playback_func_80115BD0(1);movie_playback_func_80115BD0(0);
}
int main(void){assert(sizeof(void*)==8);static const s32 stageValues[]={0,1,2,3,4,5,-1,0x7FFFFFFF};static const u32 edge[]={0,1,2,3,4,251,254,255,256,0xFFFFFFFF,0x80000000,0x7FFFFFFF};for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<16;profile++){
 memset(&a,0xA6,sizeof(a));w=&a;w->camera.counter444=stageValues[profile/2];stages[profile/2]++;w->camera.state44C=(scenario%4==0?100:scenario%4==1?1800:word(scenario*0x9876543U));w->camera.counter4A4=word(edge[scenario%12]);w->camera.mode4A8=(scenario/4)%4;w->camera.phase4AC=(scenario/16)%3;w->camera.position450[0]=word(scenario*0x1234567U);w->camera.position450[2]=word(~(scenario*0x9876543U));w->camera.delta478[0]=word(edge[scenario%12]);w->camera.delta478[2]=word(scenario*0x3456789U);w->input[0].buttons08=(u16)scenario;w->root=scenario&1;w->textPosition=word(edge[(scenario+7)%12]);w->textDelta=word(scenario*0x8974563U);
 w->events[0].key=(scenario&32)?-1:100;w->events[0].action=0;w->events[1].key=(scenario&64)?-1:1800;w->events[1].action=1;w->events[2].key=-1;w->events[2].action=0;
 b=a;w=&b;serial=count=seen=0;ref=1;oracle();w=&a;serial=0;ref=0;movie_playback_func_80115F80();assert(seen==count&&!memcmp(&a,&b,sizeof(a)));}
 for(int i=0;i<13;i++)assert(coverage[i]);assert(modes[1]&&modes[2]&&exitCount&&startCount&&fadeCalls);puts("1048576 cases passed: complete worlds/arguments, all stages, sentinel outcomes, fade modes/exits, 62-call loop and callback root changes");}
'''
(p/'native.c').write_text(head+body+rest)
subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=address,undefined,bounds','-fno-sanitize-recover=all','-I'+str(p),'-Iinclude',str(p/'native.c'),'-o',str(p/'native')],check=True)
subprocess.run([str(p/'native')],check=True)
