"""Progress controller: valid PAL phases, actual completion wrapper and live roots."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest


class ProgressControllerTest(unittest.TestCase):
    def test_valid_phases_and_mutable_services(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/garage/func_{a}.c').read_text()
                           for a in ('8011662C', '801161FC'))
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4||sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4||offsetof(t,f)==n,"PAL offset")
#include "rr/garage_progress_controller.h"
typedef struct {
 u32 guards[4],rng; s32 frame,tick,resource;
 s16 phase,timer,nextScene,mode;u16 scene;RrMenuChoice4 choice;
 RrMenuItemState5C controllers[2];RrUiRow24 ui[2][4];RrBadgeRow1C badges[2][4];RrMenuGlyphRow1C glyphs[2][4];
 u16 counts[4],grid[4][4];unsigned uiRoot,badgeRoot,glyphRoot,frameRoot;u32 tail[4];
} State;
typedef struct {s32 kind,args[8];u32 hash;} Event;
typedef struct {State s;Event e[24];unsigned calls;} World;
static World actual,reference,initial,*w;
static RrFrameOrderingPrefix frames[2],frameSnapshot[2];
static unsigned scenario,mutation,phaseSeen[9],branchSeen[8],timerSeen[8];
static const s16 phases[6]={0,1,2,3,4,8};
static const u16 timers[8]={0,1,63,64,32767,32768,65534,65535};
#define ModeFrame w->s.frame
#define ScreenFrame w->s.tick
#define garage_select_arrows_unused_arg w->s.resource
#define ModeSubState w->s.phase
#define ModeGroup w->s.scene
#define ModeId w->s.nextScene
#define MenuFadeTimer w->s.timer
#define garage_color_preview_state w->s.mode
#define SelectionRows w->s.choice
#define CourseListSavedIndex w->s.choice.bytes[1]
#define PlayerControllers w->s.controllers
#define UiRows w->s.ui[w->s.uiRoot]
#define BadgeRows w->s.badges[w->s.badgeRoot]
#define GlyphRows w->s.glyphs[w->s.glyphRoot]
#define CurrentDrawBuf ((RrIconSpriteContext*)&frames[w->s.frameRoot])
#define garage_team_car_counts w->s.counts
#define garage_team_maker_car_counts w->s.grid
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static s16 half(u16 n){s16 v;memcpy(&v,&n,2);return v;}
static u32 next(void){u32 x=w->s.rng;x^=x<<13;x^=x>>17;x^=x<<5;return w->s.rng=x;}
static Event *event(s32 kind){Event *e;unsigned i;u32 h=2166136261U;assert(w->calls<24);
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 e=&w->e[w->calls++];memset(e,0,sizeof(*e));e->kind=kind;e->hash=h;return e;
}
static void change(void){unsigned i,j,n=w->calls;if(!mutation)return;
 w->s.frame=word(next());w->s.tick=word(next());w->s.resource=word(next());w->s.phase=half(next());w->s.mode=(s16)(next()%3)-1;
 w->s.uiRoot=next()%2;w->s.badgeRoot=next()%2;w->s.glyphRoot=next()%2;w->s.frameRoot=next()%2;
 w->s.timer=half(next());w->s.choice.bytes[0]=next()%4;w->s.choice.bytes[1]=next()%4;
 w->s.controllers[0].a=next()%2;w->s.controllers[0].b=next()%2;
 w->s.ui[w->s.uiRoot][0].byte14=(scenario+n)%7;
 w->s.badges[w->s.badgeRoot][0].state10=((scenario+n)%3==0)?3:((scenario+n)%3==1)?11:-1;
 for(i=0;i<4;i++){w->s.counts[i]=(u16)next();for(j=0;j<4;j++)w->s.grid[i][j]=(u16)next();}
}
s32 LoadGlyphRowsStep(void){static const s32 values[]={0,1,-1,65537};Event *e=event(1);s32 result=values[scenario%4];e->args[0]=result;change();return result;}
void DrawMenuStepArrows(s32 x,s32 y,s32 width,s32 mode,s32 selected,s32 first,s32 second){
 Event *e=event(2);e->args[0]=x;e->args[1]=y;e->args[2]=width;e->args[3]=mode;e->args[4]=selected;e->args[5]=first;e->args[6]=second;change();
}
s32 UpdateGlyphRow(s32 selected){static const s32 values[]={0,1,-1,65536};Event *e=event(3);s32 result=values[(scenario>>2)%4];e->args[0]=selected;e->args[1]=result;change();return result;}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){Event *e=event(4);e->args[0]=a;e->args[1]=b;e->args[2]=c;change();}
void garage_step_team_cursor(void){event(5);change();}
void garage_step_maker_cursor(void){event(6);change();}
void PlaySfx(s32 sound){event(7)->args[0]=sound;change();}
void garage_init_maker_select(s32 base){event(8)->args[0]=base;change();}
void garage_init_car_list(void){event(9);change();}
void garage_init_team_select(void){event(10);change();}
void AddScreenFadeTiles(u32 *order,s32 fade,u32 flags){Event *e=event(11);assert((uintptr_t)order>UINT32_MAX);assert(order==frames[w->s.frameRoot].ordering70);e->args[0]=w->s.frameRoot;e->args[1]=fade;e->args[2]=flags;change();}
void garage_init_car_info(void){event(12);change();}
void DrawUiRow(s32 a){event(13)->args[0]=a;change();}
void UpdateBadgeRowAnim(s32 a){event(14)->args[0]=a;change();}
void UpdateScreenFade(s32 flags){event(15)->args[0]=flags;change();}
''' + bodies + r'''
static s32 completion(unsigned mode){s32 selected=w->s.choice.bytes[mode];
 DrawMenuStepArrows(160,204,48,0,selected,0,4);return UpdateGlyphRow(selected);
}
static void expected(void){s32 flags=65538;unsigned phase=(u16)w->s.phase;
 w->s.frame=word((u32)w->s.frame+1);w->s.tick=word((u32)w->s.tick+1);
 if(phase==0){s32 result=LoadGlyphRowsStep();if(result==1){SetMenuFrameOptions(1,1,1);w->s.phase=1;}}
 else if(phase==1||phase==3){s32 completed=completion(phase==3);s32 captured=w->s.badges[w->s.badgeRoot][0].state10;
  if(phase==3)flags=65542;
  if(captured==3){
   if(completed){if(phase==1)garage_step_team_cursor();else garage_step_maker_cursor();}
   if(w->s.controllers[0].a){
    s32 count=phase==1?half(w->s.counts[w->s.choice.bytes[0]]):half(w->s.grid[w->s.choice.bytes[0]][w->s.choice.bytes[1]]);
    if(count<=0){PlaySfx(4);branchSeen[0]++;}
    else{PlaySfx(1);w->s.frame=0;w->s.phase=phase==1?2:4;w->s.ui[w->s.uiRoot][0].byte14=3;w->s.badges[w->s.badgeRoot][0].state10=4;branchSeen[1]++;}
   }else if(w->s.controllers[0].b){
    PlaySfx(2);if(phase==1){w->s.phase=8;w->s.timer=64;}else{w->s.mode=1;w->s.phase=4;}
    w->s.ui[w->s.uiRoot][0].byte14=3;w->s.badges[w->s.badgeRoot][0].state10=4;branchSeen[2]++;
   }
  }
 }else if(phase==2){flags=65542;
  if(w->s.badges[w->s.badgeRoot][0].state10==11&&w->s.ui[w->s.uiRoot][0].byte14==5){garage_init_maker_select(0);w->s.phase=3;branchSeen[3]++;}
 }else if(phase==4){flags=65542;
  if(w->s.badges[w->s.badgeRoot][0].state10==11&&w->s.ui[w->s.uiRoot][0].byte14==5){
   if(!w->s.mode){garage_init_car_list();branchSeen[4]++;return;}
   else{unsigned i;for(i=0;i<4;i++)w->s.glyphs[w->s.glyphRoot][i].phase18=word((u32)w->s.glyphs[w->s.glyphRoot][i].phase18-196608U);
    garage_init_team_select();w->s.phase=1;w->s.mode=0;branchSeen[5]++;
   }
  }
 }else{assert(phase==8);w->s.timer=half((u16)w->s.timer-1U);SetMenuFrameOptions(0,1,1);
  AddScreenFadeTiles(frames[w->s.frameRoot].ordering70,(64-(s32)w->s.timer)*4,69);
  if(w->s.timer<=0){garage_init_car_info();w->s.scene=1;w->s.nextScene=64;w->s.phase=0;branchSeen[6]++;}else branchSeen[7]++;
 }
 DrawUiRow(0);UpdateBadgeRowAnim(0);UpdateScreenFade(flags);
}
int main(void){unsigned p,i,j,k;uint64_t cases=0;
 memset(frames,0xA5,sizeof(frames));memcpy(frameSnapshot,frames,sizeof(frames));
 for(scenario=0;scenario<8192;scenario++)for(p=0;p<6;p++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0x161FC+scenario*73+p*101;
  for(i=0;i<4;i++){w->s.guards[i]=next();w->s.tail[i]=next();w->s.counts[i]=(u16)next();for(j=0;j<4;j++)w->s.grid[i][j]=(u16)next();}
  for(i=0;i<2;i++)for(j=0;j<4;j++){
   w->s.ui[i][j].byte14=(scenario%3==0)?5:(scenario%3==1)?3:0;
   w->s.badges[i][j].state10=(scenario%3==0)?11:(scenario%3==1)?3:-1;
   w->s.glyphs[i][j].phase18=word(next());
  }
  w->s.frame=word(next());w->s.tick=word(next());if(scenario%8==0){w->s.frame=INT32_MAX;w->s.tick=-1;}
  w->s.resource=word(next());w->s.phase=phases[p];w->s.timer=half(timers[scenario%8]);timerSeen[scenario%8]++;
  w->s.mode=(s16)((scenario>>3)%3)-1;w->s.scene=(u16)next();w->s.nextScene=half(next());
  for(i=0;i<4;i++)w->s.choice.bytes[i]=next()%4;
  w->s.controllers[0].a=(scenario>>4)&1;w->s.controllers[0].b=(scenario>>5)&1;
  w->s.uiRoot=(scenario>>6)&1;w->s.badgeRoot=(scenario>>7)&1;w->s.glyphRoot=(scenario>>8)&1;w->s.frameRoot=(scenario>>9)&1;
  actual=reference=initial;w=&actual;garage_update_collection_select();w=&reference;expected();
  assert(!memcmp(&actual,&reference,sizeof(actual)));assert(!memcmp(frames,frameSnapshot,sizeof(frames)));phaseSeen[phases[p]]++;cases++;
 }
 assert(cases==98304);for(i=0;i<6;i++)assert(phaseSeen[phases[i]]);for(i=0;i<8;i++){assert(branchSeen[i]);assert(timerSeen[i]);}return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p = Path(tmp)
            (p / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(p / 'test.c'), '-o', str(p / 'test')], check=True)
            subprocess.run([str(p / 'test')], check=True)
