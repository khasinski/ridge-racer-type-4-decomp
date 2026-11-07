"""Guarded native oracle for the extra_course_select menu controller."""
from overlay_units import read_c_source
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess

root = Path(__file__).resolve().parents[2]
body = read_c_source(root/'src/overlays/PAL/extra_course_select/func_80114CCC.c')
body = body[body.index('void extra_course_select_update_menu'):]
body = re.sub(r' __asm__\("\$\d+"\)', '', body)
source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#include "rr/frame_storage_types.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/extra_course_select_menu_controller.h"
_Static_assert(sizeof(RrUiRow24)==36,"UI stride");
_Static_assert(sizeof(RrMenuGlyphRow1C)==28,"glyph stride");
_Static_assert(sizeof(RrMenuItemState5C)==92,"input stride");
_Static_assert(sizeof(RrSaveProgressRecord7C)==124,"save layout");
_Static_assert(offsetof(RrMenuGlyphRow1C,phase18)==24,"glyph phase");
_Static_assert(offsetof(RrSaveProgressRecord60,flags5C)==92,"save flags");
_Static_assert(offsetof(RrFrameStorage,ordering.ordering70)==112,"order head");
typedef struct {
 u32 guard[4];s32 frame,tick;s16 timer,selected,glyph;u16 phase,enable;s16 scene;
 s16 available[4];RrUiRow24 rows[2][3];RrMenuGlyphRow1C glyphs[2][4];
 RrMenuItemState5C input[2];RrSaveProgressRecord7C save;RrFrameStorage contexts[2];
 unsigned rowRoot,glyphRoot,context;u32 tail[4];
} State;
static State actual,reference,*w;
#define ModeFrame w->frame
#define ScreenFrame w->tick
#define MenuFadeTimer w->timer
#define MenuCursor w->selected
#define CourseListIndex w->glyph
#define ModeSubState w->phase
#define ModeGroup w->enable
#define ModeId w->scene
#define extra_course_select_course_counts w->available
#define UiRows w->rows[w->rowRoot]
#define GlyphRows w->glyphs[w->glyphRoot]
#define PlayerControllers w->input
#define SaveProgress w->save
#define CurrentDrawBuf (&w->contexts[w->context])
''' + body + r'''
typedef struct {unsigned kind;s32 args[7];State before;} Event;
static Event events[32];
static unsigned scenario,profile,serial,count,seen,coverage[14],choices[4],exits[2],completion[2],states[5],pulse[2];
static int referenceRun;
static s32 word(u32 a){s32 b;memcpy(&b,&a,4);return b;}
static s16 half(u16 a){s16 b;memcpy(&b,&a,2);return b;}
static void record(unsigned kind,const s32 *args){assert(kind<14);
 if(referenceRun){assert(count<32);Event *e=&events[count++];e->kind=kind;memcpy(e->args,args,28);e->before=*w;}
 else{coverage[kind]++;assert(seen<count);Event *e=&events[seen++];assert(e->kind==kind&&!memcmp(args,e->args,28)&&!memcmp(w,&e->before,sizeof(*w)));}
}
static void effect(unsigned kind){unsigned n=serial++;if(!(profile&1))return;
 u32 v=(scenario+17*kind+31*n)*0x9E3779B9U;
 w->frame=word(v);w->tick=word(~v);w->selected=(v>>2)&1;w->glyph=(v>>4)&3;
 if(kind!=0||(scenario&4096))w->phase=(u16)v;
 w->enable=(u16)v;w->scene=half(v>>7);w->timer=half(scenario+31*n);
 w->rowRoot=(scenario+n)&1;w->glyphRoot=(scenario+n+1)&1;w->context=(scenario+n)&1;
 w->input[0].a=(scenario>>6)&1;w->input[0].b=(scenario>>7)&1;
 for(unsigned i=0;i<4;i++){w->available[i]=half((scenario/8+i+n)%5-2);w->save.prefix.flags5C[i]=half((scenario/64+i+n)%3-1);
  for(unsigned j=0;j<2;j++){w->glyphs[j][i].phase18=word(v+i+j);w->glyphs[j][i].x08=half(v+i);w->glyphs[j][i].y0A=half(v>>16);}}
 for(unsigned i=0;i<3;i++)for(unsigned j=0;j<2;j++)w->rows[j][i].byte14=(scenario/256+n+i+j)%8;
}
static void service(unsigned kind){const s32 args[7]={0};record(kind,args);effect(kind);}
s32 LoadGlyphRowsStep(void){static const u32 returns[]={0,1,2,0xFFFFFFFFU,0x10001,0xFFFF0001,0x80000000,0x80000001};service(0);return profile<10?1:word(returns[scenario%8]);}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){const s32 args[7]={a,b,c};record(1,args);effect(1);}
void StepMenuValue(s32 a,s32 b,char *p,s32 d,s32 e){assert(p==(char*)&w->selected&&(uintptr_t)p>UINT32_MAX);const s32 args[7]={a,b,0,d,e};record(2,args);effect(2);}
void extra_course_select_update_course_selection(void){service(3);}
void PlaySfx(s32 a){const s32 args[7]={a};record(4,args);effect(4);}
void SeqFadeVolume(s32 a,s32 b){const s32 args[7]={a,b};record(5,args);effect(5);}
void extra_course_select_init_preview(void){service(6);}
void AddScreenFadeTiles(u32 *p,s32 b,u32 c){unsigned k;for(k=0;k<2&&p!=&w->contexts[k].ordering.ordering70;k++){}assert(k<2&&(uintptr_t)p>UINT32_MAX);const s32 args[7]={k,b,(s32)c};record(7,args);effect(7);}
void SeqMuteVolume(void){service(8);}
void DrawMenuStepArrows(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g){const s32 args[7]={a,b,c,d,e,f,g};record(9,args);effect(9);}
void extra_course_select_draw_selection_pulse(s32 a,s32 b){const s32 args[7]={a,b};record(10,args);effect(10);}
s32 UpdateGlyphRow(s32 a){const s32 args[7]={a};record(11,args);effect(11);return word(scenario*0x1234567U);}
void DrawUiRow(s32 a){const s32 args[7]={a};record(12,args);effect(12);}
void UpdateScreenFade(s32 a){const s32 args[7]={a};record(13,args);effect(13);}
static void closeRows(void){for(unsigned i=0;i<3;i++)if(w->rows[w->rowRoot][i].byte14==2)w->rows[w->rowRoot][i].byte14=3;}
static void expected(void){w->frame=word((u32)w->frame+1);w->tick=word((u32)w->tick+1);
 int ready=LoadGlyphRowsStep();completion[ready==1]++;int title=0,rows=1;
 if(ready==1){unsigned phase=w->phase;states[phase==0?0:phase==1?1:phase==3?2:phase==8?3:4]++;title=1;
  if(phase==0){for(unsigned i=0;i<4;i++)w->glyphs[w->glyphRoot][i].phase18=word((u32)w->glyphs[w->glyphRoot][i].phase18+0x30000U);w->phase=1;
  }else if(phase==1){SetMenuFrameOptions(1,1,1);w->rows[w->rowRoot][w->selected].cells[0]=0;StepMenuValue(0,2,(char*)&w->selected,2,0);
   int available=w->available[w->glyph]>0;w->rows[w->rowRoot][0].half20=available?0:1;w->rows[w->rowRoot][w->selected].cells[0]=!available&&w->selected==0?4:2;
   extra_course_select_update_course_selection();int close=0;
   if(w->input[0].a){int selected=w->selected;if(selected==0){if(w->available[w->glyph]>0){choices[0]++;close=1;w->phase=3;PlaySfx(1);closeRows();}else{choices[1]++;PlaySfx(4);}}
    else if(selected==1){choices[2]++;close=2;PlaySfx(1);}
   }else if(w->input[0].b){choices[3]++;close=2;PlaySfx(2);}
   if(close==2){closeRows();w->phase=8;w->timer=64;SeqFadeVolume(0,60);}
  }else if(phase==3){SetMenuFrameOptions(0,1,1);if(w->rows[w->rowRoot][0].byte14==5){exits[0]++;extra_course_select_init_preview();w->enable=1;w->scene=80;w->phase=0;title=rows=0;}
  }else if(phase==8){title=0;SetMenuFrameOptions(0,1,1);s16 timer=half((u16)w->timer-1);w->timer=timer;AddScreenFadeTiles(&w->contexts[w->context].ordering.ordering70,(64-(s32)timer)*4,69);
   if(w->timer<=0){exits[1]++;w->enable=1;w->scene=3;w->phase=0;SeqMuteVolume();}
  }
 }
 if(title){DrawMenuStepArrows(160,204,48,0,w->glyph,0,4);int glyph=w->glyph;int active=w->save.prefix.flags5C[glyph]!=0;pulse[active]++;
  if(active){int x=w->glyphs[w->glyphRoot][glyph].x08+20,y=w->glyphs[w->glyphRoot][glyph].y0A+40;extra_course_select_draw_selection_pulse(x,y);glyph=w->glyph;}
  UpdateGlyphRow(glyph);
 }
 if(rows)for(unsigned i=0;i<3;i++)DrawUiRow(i);UpdateScreenFade(0x10092);
}
int main(void){assert(sizeof(void*)==8);for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<12;profile++){
 memset(&actual,0xA6,sizeof(actual));w=&actual;static const u16 phases[]={0,1,3,8};w->phase=profile<8?phases[profile/2]:profile<10?(u16)scenario:1;
 w->frame=word(scenario*0x1234567U);w->tick=word(~(scenario*0x2345678U));w->timer=half(scenario);w->selected=scenario&1;w->glyph=(scenario>>2)&3;
 w->rowRoot=scenario&1;w->glyphRoot=(scenario>>1)&1;w->context=(scenario>>2)&1;w->input[0].a=(scenario>>6)&1;w->input[0].b=(scenario>>7)&1;
 for(unsigned i=0;i<4;i++){w->available[i]=half((scenario/8+i)%5-2);w->save.prefix.flags5C[i]=half((scenario/64+i)%3-1);for(unsigned j=0;j<2;j++)w->glyphs[j][i].phase18=word(scenario*0x9876543U+i+j);}
 for(unsigned i=0;i<3;i++)for(unsigned j=0;j<2;j++)w->rows[j][i].byte14=(scenario/256+i+j)%8;
 reference=actual;serial=count=seen=0;referenceRun=1;w=&reference;expected();serial=0;referenceRun=0;w=&actual;extra_course_select_update_menu();
 assert(count==seen&&!memcmp(&actual,&reference,sizeof(actual)));
 }
 for(unsigned i=0;i<14;i++)assert(coverage[i]);for(unsigned i=0;i<5;i++)assert(states[i]);for(unsigned i=0;i<4;i++)assert(choices[i]);assert(exits[0]&&exits[1]&&completion[0]&&completion[1]&&pulse[0]&&pulse[1]);
 puts("786432 cases passed; complete phase/timer sweeps, four choices, two exits, full-word readiness, fresh UI/glyph/context roots and pre-service state");
}
'''
with TemporaryDirectory() as d:
    c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
    subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
