"""Actual menu initialization/update with live roots and real input classification."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class ReplayMenuTest(unittest.TestCase):
    def test_native_menu(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/665/func_{a}.c').read_text()
                           for a in ('80114A24', '80114C4C'))
        bodies = re.sub(r'__asm__\("\$6"\)', '', bodies)
        reset = re.sub(r'^extern[^\n]*\n','',
                       (root / 'tools/scripts/fixtures/views/func_8003A198.c').read_text(),flags=re.M)
        classifier = (root / 'src/overlays/PAL/665/func_8011629C.c').read_text()
        classifier = classifier.replace('ovl665_func_8011629C','real_classify')
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
#include "rr/overlay665_menu.h"
typedef struct {
 u32 guard[4],rng;RrReplayMenuContextPrefix context;RrOverlay665VisualState visual;
 RrFrameOrderingPrefix frames[3];RrUiRow24 rows[3][40];RrMenuTextState text[3];
 RrMenuItemState5C inputs[2];RrMenuArenas arenas;u8 *resource;void *published;
 s32 frameRoot,uiRoot,textRoot,pulse;s16 fade;s8 clear1,clear2;u32 tail[4];
} State;
typedef struct {s32 op,args[10];u32 hash;} Event;
typedef struct {State s;unsigned calls;Event events[24];} World;
static World actual,reference,initial,*w;static unsigned scenario,mutation,kind;
static u8 resources[3][0x2D100],storage[3][128];static u16 textData[32];
RrIconSpriteContext *CurrentDrawBuf;RrUiRow24 *UiRows;RrMenuTextState *DialogueState;
#define RaceContext w->s.context
#define ovl665_D_80119768 w->s.visual
#define PlayerControllers w->s.inputs
#define MenuArenas w->s.arenas
#define CdDirectoryBuffer w->s.resource
#define LoadCursor w->s.published
#define MenuFadeTimer w->s.fade
#define ScreenFrame w->s.pulse
#define MenuScriptStep w->s.clear1
#define LoadState w->s.clear2
''' + reset + classifier + bodies + r'''
static u32 next(void){u32 x=w->s.rng;x^=x<<13;x^=x>>17;x^=x<<5;return w->s.rng=x;}
static s32 word(u32 x){s32 v;memcpy(&v,&x,4);return v;}
static s32 wrap(int64_t x){return word((u32)(uint64_t)x);}
static s16 half(s32 x){u16 n=(u16)(u32)x;s16 v;memcpy(&v,&n,2);return v;}
static const s32 phases[]={0,1,2,-1,3,INT32_MIN,INT32_MAX};
static const s16 fades[]={INT16_MIN,-1,0,1,2,63,64,INT16_MAX};
static void roots(void){CurrentDrawBuf=(RrIconSpriteContext*)&w->s.frames[w->s.frameRoot];UiRows=w->s.rows[w->s.uiRoot];DialogueState=&w->s.text[w->s.textRoot];}
static void effect(void){unsigned i,j;static const u8 types[]={0x41,0x73,0x23,0xE3,0,0x12};
 w->s.frameRoot=next()%3;w->s.uiRoot=next()%3;w->s.textRoot=next()%3;w->s.fade=fades[next()%8];w->s.pulse=word(next());w->s.clear1=next();w->s.clear2=next();
 w->s.visual.scene00=word(next());w->s.visual.selection08=next()%8;w->s.visual.layout0C=phases[next()%7];w->s.visual.detail10=word(next());w->s.visual.exit34.word=word(next());w->s.context.mode18=phases[next()%7];
 w->s.resource=resources[next()%3]+next()%32;w->s.arenas.first=storage[next()%3];w->s.arenas.second=storage[next()%3]+next()%32;w->s.published=storage[next()%3];
 for(i=0;i<3;i++){
  w->s.frames[i].ordering70[0]=next();w->s.frames[i].ordering70[702]=next();
  w->s.text[i].p=textData;w->s.text[i].b4=(s8)next();w->s.text[i].st=(s8)next();
  for(j=0;j<8;j++){w->s.rows[i][j].byte14=next()%4;w->s.rows[i][j].cells[0]=next();w->s.rows[i][j].half20=half(next());}
 }
 for(i=0;i<2;i++){w->s.inputs[i].a=next()%2;w->s.inputs[i].b=next()%2;w->s.inputs[i].type01=types[next()%6];w->s.inputs[i].kind1C=next()%2;w->s.inputs[i].flag40=next()%2;}
 roots();
}
static Event *event(s32 op){const u8 *p=(const u8*)&w->s;size_t i;u32 h=2166136261U,n;Event *e;assert(w->calls<24);
 for(i=0;i<sizeof(w->s);i+=4){memcpy(&n,p+i,4);h=(h^n)*16777619U;}
 e=&w->events[w->calls++];memset(e,0,sizeof(*e));e->op=op;e->hash=h;return e;
}
static void change(void){if(mutation)effect();}
void ClearScreenOverlayState(void){event(1);change();}
void InitMenuArenas(void){event(2);change();}
void SetupRenderer(s32 shift){event(3)->args[0]=shift;change();}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){Event *v=event(4);s32 args[]={a,b,c,d,e,f,g,h,i,j};memcpy(v->args,args,sizeof(args));change();}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){Event *e=event(5);e->args[0]=a;e->args[1]=b;e->args[2]=c;change();}
void RaceLoadReset(void){event(6);change();}
void SeqStartTrack(s32 mode){event(7)->args[0]=mode;change();}
void AddScreenFadeTiles(u32 *ordering,s32 intensity,u32 mode){Event *e=event(8);u32 *base=w->s.frames[w->s.frameRoot].ordering70;assert(ordering==base||ordering==base+702);e->args[0]=ordering-base;e->args[1]=intensity;e->args[2]=(s32)mode;*ordering^=(u32)intensity;change();}
void StepMenuValue(s32 a,s32 b,char *selection,s32 count,s32 flags){Event *e=event(9);assert(selection==(char*)&w->s.visual.selection08);e->args[0]=a;e->args[1]=b;e->args[2]=count;e->args[3]=flags;*selection=(char)(next()%8);change();}
s32 ovl665_func_8011629C(s32 index){Event *e=event(10);s32 result=real_classify(index);e->args[0]=index;e->args[1]=result;return result;}
void PlaySfx(s32 sound){event(11)->args[0]=sound;change();}
void SetMotionControllerVisible(s32 mode){event(12)->args[0]=mode;change();}
void SeqFadeVolume(s32 a,s32 b){Event *e=event(13);e->args[0]=a;e->args[1]=b;change();}
void ovl665_func_80116070(void){event(14);change();}
void DrawUiRow(s32 index){assert(index>=0&&index<8);event(15)->args[0]=index;change();}
void UpdateScreenFade(s32 flags){event(16)->args[0]=flags;change();}
static void expected(void){State *s=&w->s;unsigned i;
 if(!kind){static const s32 group[]={8,9,1,1,1,1,0},label[]={19,19,25,20,21,18,85};
  ClearScreenOverlayState();s->clear1=0;s->clear2=0;s->pulse=0;s->arenas.second=s->resource+0x2D000;InitMenuArenas();s->published=s->arenas.second;SetupRenderer(5);
  for(i=0;i<7;i++)InitUiRow(i,94,96+40*i,112,32,0,0,1,group[i],label[i]);
  SetMenuFrameOptions(1,1,1);if(s->context.mode18==1){s->visual.selection08=5;s->context.mode18=0;}s->fade=64;RaceLoadReset();SeqStartTrack(0);DialogueState->b4=-1;return;
 }
 {s32 phase=s->visual.layout0C;
 if(phase==0){
  if(s->fade>0){s->fade=half((s32)s->fade-1);AddScreenFadeTiles(s->frames[s->frameRoot].ordering70+702,(s32)s->fade*4,69);}
  if(UiRows[0].byte14==2&&s->fade==0){
   UiRows[s->visual.selection08].cells[0]=0;StepMenuValue(0,2,(char*)&s->visual.selection08,7,0);UiRows[s->visual.selection08].cells[0]=2;
   if(s->inputs[0].a){
    int reject=s->visual.selection08==1&&ovl665_func_8011629C(1)<0;
    if(reject)PlaySfx(4);
    else {PlaySfx(1);UiRows[s->visual.selection08].cells[0]=1;s->fade=64;s->visual.layout0C=wrap((int64_t)s->visual.layout0C+1);}
   }else if(s->inputs[0].b){PlaySfx(2);s->visual.selection08=6;s->fade=64;s->visual.layout0C=wrap((int64_t)s->visual.layout0C+1);}
  }
  SetMotionControllerVisible(1);
 }else if(phase==1){
  for(i=0;i<8;i++)if(UiRows[i].byte14==2)UiRows[i].byte14=3;
  if(s->visual.selection08==3||s->visual.selection08>=5)SeqFadeVolume(0,half((s32)s->fade-4));
  s->visual.layout0C=wrap((int64_t)s->visual.layout0C+1);
 }else if(phase==2){
  s->fade=half((s32)s->fade-1);AddScreenFadeTiles(s->frames[s->frameRoot].ordering70,(64-(s32)s->fade)*4,69);
  if(s->fade<=0){SetMotionControllerVisible(0);s->visual.exit34.word=0;s->visual.layout0C=0;s->visual.detail10=0;s->visual.scene00=wrap((int64_t)s->visual.scene00+1);if(s->visual.selection08==0||s->visual.selection08==1)ovl665_func_80116070();}
 }
 }
 if(ovl665_func_8011629C(1)<0){UiRows[1].half20=1;if(s->visual.selection08==1)UiRows[1].cells[0]=4;}else UiRows[1].half20=0;
 for(i=0;i<8;i++)DrawUiRow(i);UpdateScreenFade(0x10066);
}
int main(void){unsigned i;assert((uintptr_t)resources>UINT32_MAX);_Static_assert(sizeof(State)%4==0,"state words");
 for(scenario=0;scenario<32768;scenario++)for(kind=0;kind<2;kind++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0x14c4c+scenario*31+kind;effect();w->s.visual.layout0C=phases[scenario%7];w->s.fade=fades[(scenario/7)%8];w->s.visual.selection08=scenario%8;
  for(i=0;i<4;i++){w->s.guard[i]=scenario+i;w->s.tail[i]=~(scenario+i);}actual=reference=initial;
  w=&actual;roots();if(kind)ovl665_func_80114C4C();else ovl665_func_80114A24();w=&reference;roots();expected();assert(!memcmp(&actual,&reference,sizeof(actual)));
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
