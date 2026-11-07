from pathlib import Path
from tempfile import TemporaryDirectory
import re,subprocess,sys
root=next(p for p in Path(__file__).resolve().parents if (p/'Makefile').exists() and (p/'tools/scripts').exists())
target=Path(sys.argv[1]) if len(sys.argv)>1 else root/'src/overlays/PAL/options/func_80115820.c'
body=target.read_text();macro=body[:body.index('#include')];body=body[body.index('void options_update_screen_adjust'):]
source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/options_calibration.h"
typedef struct {u32 words[5];} SpriteStorage;
typedef struct {SpriteStorage sprites[4];RrGpuSingleCommandPacket command;u32 tail[4];} PacketStream;
typedef struct {
 u32 guard[4];RrOptionsVisualState visual;RrProjectionScratchPrefix scratch;
 RrFrameStorage frames[2];RrRuntimeSettings settings;RrMenuItemState5C input[2];
 RrBadgeRow1C badges[2][2];RrMenuTextState text[2];RrUiUvRect regions[2][2];PacketStream streams[3];
 s16 timer;unsigned frameRoot,badgeRoot,textRoot,arena,sprites;u32 tail[4];
} State;
static State world,initial,expected;
#define options_menu_state world.visual
#define MenuFadeTimer world.timer
#define Options world.settings
#define FrameEnvs world.frames
#define PlayerControllers world.input
#define BadgeRows world.badges[world.badgeRoot]
#define DialogueState (&world.text[world.textRoot])
#define CurrentDrawBuf ((RrIconSpriteContext *)&world.frames[world.frameRoot])
#define options_vertical_arrow_uvs world.regions[0]
#define options_left_arrow_uv world.regions[1]
#undef RR_PROJECTION_SCRATCH
#define RR_PROJECTION_SCRATCH (&world.scratch)
''' + macro + body + r'''
typedef struct {unsigned kind;s32 args[8];State before;} Event;
static Event events[16];static unsigned scenario,profile,serial,count,seen,coverage[9],paths[5],exitKinds[2],buttonPaths[3],clamps[4];static int referenceRun;
static s32 word(u32 u){s32 x;memcpy(&x,&u,4);return x;}
static s16 half(u16 u){s16 x;memcpy(&x,&u,2);return x;}
static const u32 extrema[]={0,1,0x7FFFFFFFU,0x80000000U,0xFFFFFFFFU,0xFFFFFFFEU,0xA6A6A6A6U};
static const s32 points[]={INT32_MIN,INT32_MIN+1,-11,-10,-9,-1,0,1,9,10,11,31,32,33,INT32_MAX};
static void record(unsigned kind,const s32 *args){assert(kind<9);
 if(referenceRun){assert(count<16);events[count].kind=kind;memcpy(events[count].args,args,32);events[count++].before=world;}
 else{coverage[kind]++;assert(seen<count);Event *e=&events[seen++];assert(e->kind==kind&&!memcmp(e->args,args,32)&&!memcmp(&world,&e->before,sizeof(world)));}
}
static u8 *spriteCursor(void){return world.sprites<4?(u8*)&world.streams[world.arena].sprites[world.sprites]:(u8*)&world.streams[world.arena].command;}
static void effect(unsigned kind){unsigned n=serial++;if(!(profile&1))return;u32 v=(scenario+n*31+kind*17)*0x9E3779B9U;
 world.frameRoot=(scenario+n)&1;world.badgeRoot=(scenario+n+1)&1;world.textRoot=(scenario/2+n)&1;world.arena=(scenario+n)%3;
 world.visual.layout0C=word(extrema[(scenario+n+kind)%7]);world.visual.scene00=word(extrema[(scenario/7+n+kind)%7]);world.timer=half(scenario+n*31);
 world.settings.display.x34=points[(scenario+n)%15];world.settings.display.y38=points[(scenario/15+n)%15];
 world.input[0].buttons0A=(u16)v;world.input[0].buttons08=(u16)(v>>9);world.input[0].accept30=word(v);
 for(unsigned i=0;i<2;i++){world.frames[i].ordering.ordering70[0]=v+i*0x1234567U;world.badges[i][0].state10=((scenario+n+i)&1)?3:11;world.badges[i][0].x04=half(v);world.badges[i][0].y06=half(v>>16);world.text[i].b4=(s8)(u8)v;
  for(unsigned j=0;j<2;j++){world.regions[i][j].u=(u8)(v+j);world.regions[i][j].v=(u8)(v>>8);world.regions[i][j].w=(u8)(v>>16);world.regions[i][j].h=(u8)(v>>24);}}
 world.scratch.pointers.cursor=spriteCursor();world.scratch.pointers.order=world.frames[world.frameRoot].ordering.ordering70;
}
static void service(unsigned kind){const s32 args[8]={0};record(kind,args);effect(kind);}
void options_init_screen_adjust(void){service(0);}
void SetMotionControllerVisible(s32 a){const s32 args[8]={a};record(1,args);effect(1);}
void PlaySfx(s32 a){const s32 args[8]={a};record(2,args);effect(2);}
void AddScreenFadeTiles(u32 *p,s32 a,u32 b){assert(p==world.frames[world.frameRoot].ordering.ordering70&&(uintptr_t)p>UINT32_MAX);const s32 args[8]={world.frameRoot,a,(s32)b};record(3,args);effect(3);}
void options_init_main_menu(void){service(4);}
void *AddSpritePrim(u32 *head,void *cursor,s32 x,s32 y,const RrUiUvRect *region,u16 clut,s32 color){unsigned arena,which=99,n=world.sprites;u32 uv;assert(n<4&&head==world.frames[world.frameRoot].ordering.ordering70&&(uintptr_t)cursor>UINT32_MAX);
 for(arena=0;arena<3;arena++)if(cursor==&world.streams[arena].sprites[n])break;assert(arena<3);
 for(unsigned a=0;a<2;a++)for(unsigned b=0;b<2;b++)if(region==&world.regions[a][b])which=a*2+b;assert(which<4);memcpy(&uv,region,4);
 const s32 args[8]={world.frameRoot,arena,x,y,which,clut,color,word(uv)};record(5,args);
 for(unsigned i=0;i<5;i++)world.streams[arena].sprites[n].words[i]=(u32)x+(u32)y*3+(u32)color*7+uv+i;
 world.sprites++;effect(5);return world.sprites<4?(u8*)&world.streams[(world.arena+1)%3].sprites[world.sprites]:(u8*)&world.streams[(world.arena+1)%3].command;
}
void UpdateBadgeRowAnim(s32 a){const s32 args[8]={a};record(6,args);effect(6);}
void UpdateDialogue(s32 x,s32 y,RrMenuTextState *text){assert(text==&world.text[world.textRoot]&&(uintptr_t)text>UINT32_MAX);const s32 args[8]={x,y,world.textRoot};record(7,args);effect(7);}
void UpdateScreenFade(s32 flags){const s32 args[8]={flags};record(8,args);effect(8);}
static void oracle(void){s32 phase=world.visual.layout0C;paths[phase>=0&&phase<4?phase:4]++;
 if(phase==0){service(0);SetMotionControllerVisible(0);world.visual.layout0C=word((u32)world.visual.layout0C+1);}
 else if(phase==1){if(world.badges[world.badgeRoot][0].state10==3)world.visual.layout0C=2;}
 else if(phase==2){u16 buttons=world.input[0].buttons0A;s32 x=world.settings.display.x34,y=world.settings.display.y38;
  if(buttons&0x8000)x=word((u32)x-1);if(buttons&0x2000)x=word((u32)x+1);if(buttons&0x1000)y=word((u32)y-2);if(buttons&0x4000)y=word((u32)y+2);
  if(x<-10){x=-10;clamps[0]++;}else if(x>10){x=10;clamps[1]++;}if(y<0){y=0;clamps[2]++;}else if(y>32){y=32;clamps[3]++;}world.settings.display.x34=x;world.settings.display.y38=y;
  if(world.input[0].buttons08&0x800){buttonPaths[0]++;world.settings.display.x34=0;world.settings.display.y38=16;}
  else if(world.input[0].accept30){buttonPaths[1]++;PlaySfx(1);world.badges[world.badgeRoot][0].state10=4;world.timer=64;world.visual.layout0C=word((u32)world.visual.layout0C+1);}else buttonPaths[2]++;
  for(unsigned i=0;i<2;i++){world.frames[i].environment.screen.x=half((u32)world.settings.display.x34);world.frames[i].environment.screen.y=half((u32)world.settings.display.y38);}
 }else if(phase==3){s16 timer=half((u16)world.timer-1);world.timer=timer;AddScreenFadeTiles(world.frames[world.frameRoot].ordering.ordering70,(64-(s32)timer)*4,69);
  if(world.timer<=0&&world.badges[world.badgeRoot][0].state10==11){exitKinds[0]++;service(4);world.visual.layout0C=0;world.visual.scene00=word((u32)world.visual.scene00-1);return;}exitKinds[1]++;
 }
 void *cursor=world.scratch.pointers.cursor;
 for(unsigned i=0;i<4;i++)cursor=AddSpritePrim(world.frames[world.frameRoot].ordering.ordering70,cursor,i<2?160:136+(i-2)*48,i<2?128+i*96:176,&world.regions[i/2][i%2],32568,255);
 RrGpuSingleCommandPacket *packet=cursor;world.scratch.pointers.cursor=(u8*)packet;u32 old=world.frames[world.frameRoot].ordering.ordering70[0];
 packet->tag=0x01000000U+(old&0xFFFFFFU);packet->command=0xE1000259U;world.frames[world.frameRoot].ordering.ordering70[0]=(old&0xFF000000U)+((uintptr_t)packet%0x1000000U);world.scratch.pointers.cursor=(u8*)(packet+1);
 world.text[world.textRoot].b4=26;UpdateBadgeRowAnim(0);
 if(world.badges[world.badgeRoot][0].state10==3)UpdateDialogue(world.badges[world.badgeRoot][0].x04+10,world.badges[world.badgeRoot][0].y06+40,&world.text[world.textRoot]);UpdateScreenFade(0x1007A);
}
int main(void){assert(sizeof(void*)==8);_Static_assert(offsetof(PacketStream,command)==80,"command object");_Static_assert(sizeof(RrFrameStorage)==0x22778,"frame owner");
 for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<8;profile++){
 memset(&world,0xA6,sizeof(world));world.visual.layout0C=profile==0?0:profile==1?1:profile<4?2:profile<6?3:profile==6?(s32)scenario:word(scenario*0x9E3779B9U);
 world.visual.scene00=word(extrema[scenario%7]);world.timer=half(scenario);world.settings.display.x34=points[scenario%15];world.settings.display.y38=points[(scenario/15)%15];world.input[0].buttons0A=scenario;world.input[0].buttons08=(u16)(scenario<<6);world.input[0].accept30=(scenario>>6)&1;
 world.frameRoot=scenario&1;world.badgeRoot=(scenario>>1)&1;world.textRoot=(scenario>>2)&1;world.arena=scenario%3;world.sprites=0;
 for(unsigned i=0;i<2;i++){world.badges[i][0].state10=((scenario+i)&1)?3:11;world.text[i].p=NULL;}
 world.scratch.pointers.cursor=spriteCursor();world.scratch.pointers.order=world.frames[world.frameRoot].ordering.ordering70;
 initial=world;serial=count=seen=0;referenceRun=1;oracle();expected=world;world=initial;serial=0;referenceRun=0;options_update_screen_adjust();assert(count==seen&&!memcmp(&world,&expected,sizeof(world)));
 }
 for(unsigned i=0;i<9;i++)assert(coverage[i]);for(unsigned i=0;i<5;i++)assert(paths[i]);for(unsigned i=0;i<3;i++)assert(buttonPaths[i]);for(unsigned i=0;i<4;i++)assert(clamps[i]);assert(exitKinds[0]&&exitKinds[1]);
 puts("524288 cases passed; full buttons/timers/layout sweeps, wrapped clamp extrema, fresh full native roots/cursors, complete frame owners and real command packets");
}
'''
source=re.sub(r' __asm__\("\$\d+"\)','',source).replace(' __asm__(reg)','')
with TemporaryDirectory() as d:
 c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
 subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
