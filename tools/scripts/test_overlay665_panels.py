"""Detail setup and panel drawing with genuine mixed packet storage and wire links."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class PanelsTest(unittest.TestCase):
    def test_native_detail_and_panel(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/665/func_{a}.c').read_text()
                           for a in ('80116070', '80115CFC'))
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
#include "rr/overlay665_panels.h"
typedef struct {u32 words[5];} GlyphStorage;
typedef struct {u32 words[7];} PairStorage;
typedef struct {
 GlyphStorage glyphs[8];RrGpuSingleCommandPacket first;
 PairStorage pairs[16];RrGpuSingleCommandPacket second;u32 tail[4];
} PacketStream;
typedef struct {
 u32 guard[4],rng;RrOverlay665VisualState visual;RrProjectionScratchPrefix scratch;
 PacketStream streams[3];RrFrameOrderingPrefix frames[3];RrBadgeRow1C badges[3][8];RrMenuTextState text[3];
 RrUiUvRect uv;s32 arena,frameRoot,badgeRoot,textRoot,accept,local;
 s16 fade;u8 work[12];unsigned glyphCalls,pairCalls;u32 tail[4];
} State;
typedef struct {s32 op,args[10];u32 hash;} Event;
typedef struct {State s;unsigned calls;Event events[40];} World;
static World world,initial,expected;static u16 textData[32];static unsigned scenario,mutation,kind;
#define ovl665_D_80119768 world.s.visual
#define ovl665_D_80119760 world.s.local
#define ScreenFadeState world.s.work
#define BadgeRows world.s.badges[world.s.badgeRoot]
#define DialogueState (&world.s.text[world.s.textRoot])
#define CurrentDrawBuf ((RrIconSpriteContext*)&world.s.frames[world.s.frameRoot])
#define MenuConfirmPressed world.s.accept
#define MenuFadeTimer world.s.fade
#define ovl665_D_80119028 world.s.uv
#undef RR_PROJECTION_SCRATCH
#define RR_PROJECTION_SCRATCH (&world.s.scratch)
''' + bodies + r'''
static u32 next(void){u32 x=world.s.rng;x^=x<<13;x^=x>>17;x^=x<<5;return world.s.rng=x;}
static s32 word(u32 u){s32 v;memcpy(&v,&u,4);return v;}
static s32 wrap(int64_t v){return word((u32)(uint64_t)v);}
static s16 half(s32 v){u16 u=(u16)(u32)v;s16 h;memcpy(&h,&u,2);return h;}
static u8 *glyph_cursor(void){PacketStream *p=&world.s.streams[world.s.arena];return world.s.glyphCalls<8?(u8*)&p->glyphs[world.s.glyphCalls]:(u8*)&p->first;}
static u8 *pair_cursor(void){PacketStream *p=&world.s.streams[world.s.arena];return world.s.pairCalls<16?(u8*)&p->pairs[world.s.pairCalls]:(u8*)&p->second;}
static const s32 phases[]={0,1,2,3,-1,INT32_MIN,INT32_MAX};
static const s16 fades[]={INT16_MIN,-1,0,1,63,64,INT16_MAX};
static const s16 states[]={0,3,4,11};
static void effect(void){unsigned i,j;
 world.s.arena=next()%3;world.s.frameRoot=next()%3;world.s.badgeRoot=next()%3;world.s.textRoot=next()%3;
 world.s.visual.layout0C=phases[next()%7];world.s.visual.scene00=word(next());world.s.visual.reset2C=word(next());world.s.local=word(next());world.s.fade=fades[next()%7];world.s.accept=next()%2;
 for(i=0;i<3;i++){
  world.s.frames[i].ordering70[0]=next();world.s.text[i].p=textData;world.s.text[i].b4=(s8)next();world.s.text[i].st=(s8)next();
  world.s.badges[i][0].half02=half(next());world.s.badges[i][0].state10=states[next()%4];world.s.badges[i][0].x04=half(next());world.s.badges[i][0].y06=half(next());
  world.s.visual.view74.position00.words[i]=word(next());
 }
 world.s.visual.view74.rotation10=half(next());world.s.visual.view74.rotation12=half(next());world.s.visual.view74.rotation14=half(next());
 world.s.uv.u=next();world.s.uv.v=next();world.s.uv.w=next();world.s.uv.h=next();
 for(j=0;j<12;j++)world.s.work[j]=next();
 world.s.scratch.pointers.order=world.s.frames[world.s.frameRoot].ordering70;
 world.s.scratch.pointers.cursor=world.s.glyphCalls<8?glyph_cursor():pair_cursor();
}
static Event *event(s32 op){size_t i;u32 h=2166136261U,n;const u8 *p=(const u8*)&world.s;Event *e;assert(world.calls<40);
 for(i=0;i<sizeof(world.s);i+=4){memcpy(&n,p+i,4);h=(h^n)*16777619U;}
 e=&world.events[world.calls++];memset(e,0,sizeof(*e));e->op=op;e->hash=h;return e;
}
static void change(void){if(mutation)effect();}
void ClearScreenOverlayState(void){event(1);change();}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){Event *v=event(2);s32 args[]={a,b,c,d,e,f,g,h,i,j};memcpy(v->args,args,sizeof(args));change();}
void InitBadgeRow(s32 a,s32 b,s16 c,s32 d,u16 e,u16 f,u16 g){Event *v=event(3);s32 args[]={a,b,c,d,e,f,g};memcpy(v->args,args,sizeof(args));change();}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){Event *e=event(4);e->args[0]=a;e->args[1]=b;e->args[2]=c;change();}
void ovl665_func_80115C8C(void){event(5);change();}
void SetMotionControllerVisible(s32 mode){event(6)->args[0]=mode;change();}
void PlaySfx(s32 sound){event(7)->args[0]=sound;change();}
void AddScreenFadeTiles(u32 *head,s32 intensity,u32 mode){Event *e=event(8);assert(head==world.s.frames[world.s.frameRoot].ordering70);e->args[0]=intensity;e->args[1]=mode;*head^=(u32)intensity;change();}
void ovl665_func_80114A24(void){event(9);change();}
void *AddSpritePrim(u32 *head,void *cursor,s32 x,s32 y,const RrUiUvRect *uv,u16 clut,s32 color){
 Event *e=event(10);unsigned n=world.s.glyphCalls,i;GlyphStorage *p;int arena=-1;
 assert(n<8&&head==world.s.frames[world.s.frameRoot].ordering70&&uv==&world.s.uv);
 for(i=0;i<3;i++)if(cursor==&world.s.streams[i].glyphs[n])arena=i;assert(arena>=0);p=&world.s.streams[arena].glyphs[n];
 e->args[0]=arena;e->args[1]=x;e->args[2]=y;e->args[3]=clut;e->args[4]=color;memcpy(&e->args[5],uv,4);
 /* Typed GPU service simulation; these words deliberately are opaque, not
  * claimed to be emitted hardware sprites. The caller's command packets below
  * are real packet objects and are checked word-for-word by the reference. */
 for(i=0;i<5;i++)p->words[i]=(u32)x+(u32)y*3+(u32)color*7+i;
 world.s.glyphCalls++;change();return glyph_cursor();
}
void DrawHudDigit(s32 glyph,s32 x,s32 y,s32 color){
 Event *e=event(11);unsigned i,n=world.s.pairCalls;PairStorage *p;
 assert(n<16&&world.s.scratch.pointers.cursor==pair_cursor());p=&world.s.streams[world.s.arena].pairs[n];
 e->args[0]=glyph;e->args[1]=x;e->args[2]=y;e->args[3]=color;
 for(i=0;i<7;i++)p->words[i]=(u32)glyph+(u32)x*3+(u32)y*5+(u32)color*7+i;
 world.s.pairCalls++;change();world.s.scratch.pointers.cursor=pair_cursor();
}
void UpdateBadgeRowAnim(s32 row){event(12)->args[0]=row;change();}
void UpdateDialogue(s32 x,s32 y,RrMenuTextState *text){Event *e=event(13);assert(text==DialogueState);e->args[0]=x;e->args[1]=y;change();}
void UpdateScreenFade(s32 flags){event(14)->args[0]=flags;change();}
static void command(RrGpuSingleCommandPacket *p,u32 value){
 u32 *head=world.s.frames[world.s.frameRoot].ordering70;u32 old=*head;
 p->tag=0x01000000U+(old&0xFFFFFFU);p->command=value;
 *head=(old&0xFF000000U)+(u32)((uintptr_t)p%0x1000000U);
 world.s.scratch.pointers.cursor=(u8*)(p+1);
}
static void reference(void){unsigned i;State *s=&world.s;
 if(!kind){static const s32 rows[8][6]={{56,160,64,0,1,22},{128,160,64,0,0,109},{200,160,64,0,1,23},{92,360,64,0,0,110},{164,360,64,0,0,111},{128,80,52,1,0,106},{200,80,28,1,0,107},{200,120,28,1,0,108}};
  ClearScreenOverlayState();for(i=0;i<8;i++)InitUiRow(i,rows[i][0],rows[i][1],rows[i][2],32,0,0,rows[i][3],rows[i][4],rows[i][5]);
  s->work[0]=2;InitBadgeRow(0,16,96,76,40,0,3329);s->local=0;BadgeRows[0].half02=1;
  for(i=0;i<3;i++)s->visual.view74.position00.words[i]=0;s->visual.view74.rotation10=0;s->visual.view74.rotation12=0;s->visual.view74.rotation14=0;s->visual.reset2C=0;SetMenuFrameOptions(1,1,0);return;
 }
 {s32 phase=s->visual.layout0C;
  if(phase==0){ovl665_func_80115C8C();SetMotionControllerVisible(0);s->visual.layout0C=wrap((int64_t)s->visual.layout0C+1);}
  else if(phase==1){if(BadgeRows[0].state10==3)s->visual.layout0C=2;}
  else if(phase==2){if(s->accept){PlaySfx(1);BadgeRows[0].state10=4;s->fade=64;s->visual.layout0C=wrap((int64_t)s->visual.layout0C+1);}}
  else if(phase==3){s->fade=half((s32)s->fade-1);AddScreenFadeTiles(s->frames[s->frameRoot].ordering70,(64-(s32)s->fade)*4,69);if(s->fade<=0&&BadgeRows[0].state10==11){ovl665_func_80114A24();s->visual.layout0C=0;s->visual.scene00=wrap((int64_t)s->visual.scene00-1);return;}}
 }
 {void *cursor=s->scratch.pointers.cursor;
  for(i=0;i<8;i++)cursor=AddSpritePrim(s->frames[s->frameRoot].ordering70,cursor,24+36*i,144,&s->uv,32568,255-(i*223+6)/7);
  s->scratch.pointers.cursor=cursor;command(cursor,0xE1000239U);
 }
 for(i=0;i<8;i++){DrawHudDigit(0,28+36*i,192,128);DrawHudDigit(i+1,34+36*i,192,128);}
 command((RrGpuSingleCommandPacket*)s->scratch.pointers.cursor,0xE1000238U);
 DialogueState->b4=27;UpdateBadgeRowAnim(0);
 if(BadgeRows[0].state10==3){s32 x=BadgeRows[0].x04,y=BadgeRows[0].y06;UpdateDialogue(x+10,y+40,DialogueState);}
 UpdateScreenFade(65660);
}
int main(void){unsigned i,j;
 _Static_assert(offsetof(PacketStream,first)==160,"first command");_Static_assert(offsetof(PacketStream,second)==616,"second command");_Static_assert(sizeof(State)%4==0,"state words");assert((uintptr_t)&world.s.streams>UINT32_MAX);
 for(scenario=0;scenario<16384;scenario++)for(kind=0;kind<2;kind++)for(mutation=0;mutation<2;mutation++){
  memset(&world,0,sizeof(world));world.s.rng=0x15cfc+scenario*31+kind;effect();world.s.visual.layout0C=phases[scenario%7];world.s.fade=fades[(scenario/7)%7];world.s.accept=scenario%2;
  for(i=0;i<3;i++){world.s.streams[i].first.tag=next();world.s.streams[i].first.command=next();world.s.streams[i].second.tag=next();world.s.streams[i].second.command=next();for(j=0;j<4;j++)world.s.streams[i].tail[j]=next();}
  for(i=0;i<4;i++){world.s.guard[i]=scenario+i;world.s.tail[i]=~(scenario+i);}initial=world;reference();expected=world;world=initial;
  if(kind)ovl665_func_80115CFC();else ovl665_func_80116070();assert(!memcmp(&world,&expected,sizeof(world)));
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
