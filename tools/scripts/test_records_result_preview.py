"""Preview state machine, temporary selector and three defined RGB channels."""
from overlay_units import read_c_source
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class RecordsResultPreviewTest(unittest.TestCase):
    def test_states_roots_and_all_fade_timers(self):
        root=Path(__file__).resolve().parents[2]
        body=read_c_source(root/'src/overlays/PAL/records/func_80115C34.c')
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
#include "rr/records_result_preview.h"
_Static_assert(sizeof(RrPreviewRgb3)==4&&__alignof__(RrPreviewRgb3)==4,"color word ABI");
typedef struct {
 u32 guard[4];RrRaceVehicle cars[2];RrCurrentSelection choice;RrUiRow24 ui[2][4];RrBadgeRow1C badges[2][2];u8 modes[2][3];
 s32 frame,accepted,entry;u16 scene;s16 next,phase;unsigned uiRoot,badgeRoot,modeRoot;u32 tail[4];
} State;
typedef struct {s32 kind,args[5];u32 hash;} Event;
typedef struct {State s;Event events[16];unsigned calls;} World;
static World actual,reference,initial,*w;
static unsigned scenario,mutation,seenPhases,seenDraw,seenExit,seenTimers[8];
static const u16 phases[]={0,1,2,3,8,32768,65535};
static const u8 modes[]={0,1,2,3,5,127,128,255};
static const u32 frames[]={0,1,0x7FFFFFFFU,0x80000000U,0xFFFFFFFFU};
#define PlayerCars w->s.cars
#define SelectionRows w->s.choice
#define UiRows (w->s.ui[w->s.uiRoot])
#define BadgeRows (w->s.badges[w->s.badgeRoot])
#define ImageRows (w->s.modes[w->s.modeRoot])
#define ModeFrame w->s.frame
#define MenuConfirmPressed w->s.accepted
#define records_record_kind w->s.entry
#define ModeGroup w->s.scene
#define ModeId w->s.next
#define ModeSubState w->s.phase
static s32 word(u32 u){s32 v;memcpy(&v,&u,4);return v;}
static s16 half(u32 u){u16 bits=u;s16 v;memcpy(&v,&bits,2);return v;}
static void effect(unsigned n){unsigned i,r;
 for(i=0;i<sizeof(w->s.cars);i++)((u8*)w->s.cars)[i]=(u8)(scenario+n*19+i*31);
 for(i=0;i<sizeof(w->s.choice);i++)((u8*)&w->s.choice)[i]=(u8)(scenario+n*37+i*17);
 for(i=0;i<sizeof(w->s.ui);i++)((u8*)w->s.ui)[i]=(u8)(scenario+n*51+i*7);
 for(i=0;i<sizeof(w->s.badges);i++)((u8*)w->s.badges)[i]=(u8)(scenario+n*71+i*13);
 for(r=0;r<2;r++){w->s.modes[r][0]=(u8)(scenario+n+r);w->s.modes[r][1]=modes[(scenario/256+n*3+r)%8];w->s.modes[r][2]=(u8)(scenario+n*37+r*11);}
 w->s.uiRoot=(scenario+n)%2;w->s.badgeRoot=(scenario/3+n)%2;w->s.modeRoot=(scenario/7+n)%2;
 w->s.phase=half(phases[(scenario/2048+n)%7]);w->s.scene=(u16)(scenario+n*23);w->s.next=half(scenario+n*41);
 w->s.frame=word(frames[(scenario+n)%5]);w->s.accepted=word(scenario&16?1:0);w->s.entry=word(scenario*0x9E3779B9U+n*131U);
 w->s.ui[w->s.uiRoot][3].byte14=(scenario/4+n)%4;
 w->s.badges[w->s.badgeRoot][0].state10=(scenario/32+n)%14;
}
static Event *event(s32 kind){unsigned i;u32 h=2166136261U;Event *e;assert(w->calls<16);
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 e=w->events+w->calls++;memset(e,0,sizeof(*e));e->kind=kind;e->hash=h;return e;
}
static void change(void){if(mutation)effect(w->calls);}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){Event *e=event(1);e->args[0]=a;e->args[1]=b;e->args[2]=c;change();}
void PlaySfx(s32 a){event(2)->args[0]=a;change();}
void records_init_selection(s32 a){event(3)->args[0]=a;if(w==&actual)seenExit=1;change();}
void SetupMenuCarCamera(s32 a){event(4)->args[0]=a;change();}
void SetupViewport(s32 a){event(5)->args[0]=a;change();}
void DrawSelectionIconGrid(s32 a){Event *e=event(6);e->args[0]=a;assert(w->s.choice.selection.pad02[1]==2);change();}
void PrepareCarDrawPose(RrRaceVehicle *car){event(7);assert(car==w->s.cars);assert((uintptr_t)car>UINT32_MAX);assert(w->s.choice.selection.pad02[1]==0);change();}
void DrawPreviewCar(RrRaceVehicle *car,RrPreviewRgb3 color,s32 scale,s32 flags){Event *e=event(8);
 assert(car==w->s.cars);e->args[0]=color.r;e->args[1]=color.g;e->args[2]=color.b;e->args[3]=scale;e->args[4]=flags;
 if(w==&actual){seenDraw=1;if(w->s.modes[w->s.modeRoot][1]==1){unsigned t=w->s.modes[w->s.modeRoot][2];seenTimers[t/32]|=1U<<(t%32);}}
 change();
}
void DrawUiRow(s32 a){event(9)->args[0]=a;change();}
void UpdateBadgeRowAnim(s32 a){event(10)->args[0]=a;change();}
s32 UpdateImageRow(s32 a){event(11)->args[0]=a;change();return 0;}
void UpdateScreenFade(s32 a){event(12)->args[0]=a;change();}
''' + body + r'''
static void expected(void){State *s=&w->s;unsigned p=(u16)s->phase;
 s->frame=word((u32)s->frame+1U);
 if(p==0){if(((u8*)&s->ui[s->uiRoot])[128]==2){SetMenuFrameOptions(1,1,0);s->phase=1;}}
 else if(p==1){((u8*)&s->ui[s->uiRoot])[120]=2;
  if(s->accepted){PlaySfx(1);SetMenuFrameOptions(0,1,0);((u8*)&s->ui[s->uiRoot])[120]=1;((u8*)&s->ui[s->uiRoot])[128]=3;s->phase=2;s->badges[s->badgeRoot][0].state10=4;s->modes[s->modeRoot][1]=3;}
 }else if(p==2){if(s->badges[s->badgeRoot][0].state10==11&&s->modes[s->modeRoot][1]==5){records_init_selection(s->entry);s->scene=1;s->next=75;s->phase=0;}}
 s->cars[0].rot54=(u32)(word((u32)s->cars[0].rot54+8U))&4095;
 SetupMenuCarCamera(1);SetupViewport(6);
 {unsigned mode=s->modes[s->modeRoot][1];if(mode==1||mode==2){RrPreviewRgb3 c;int scale;
  s->choice.selection.pad02[1]=2;DrawSelectionIconGrid(0);s->choice.selection.pad02[1]=0;PrepareCarDrawPose(s->cars);
  if(s->modes[s->modeRoot][1]==1){int64_t t=s->modes[s->modeRoot][2];c.r=(u8)(255-t*255/30);c.g=c.r;c.b=c.r;scale=t*256/30;}
  else{c.r=128;c.g=128;c.b=128;scale=256;}
  DrawPreviewCar(s->cars,c,scale,0);
 }}
 DrawUiRow(3);UpdateBadgeRowAnim(0);UpdateImageRow(0);UpdateScreenFade(0x10022);
}
int main(void){unsigned i;
 for(scenario=0;scenario<16384;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;effect(0);for(i=0;i<4;i++){initial.s.guard[i]=scenario+i;initial.s.tail[i]=~(scenario+i);}
  actual=reference=initial;w=&actual;records_update_result_preview();w=&reference;expected();assert(!memcmp(&actual,&reference,sizeof(actual)));
  seenPhases|=1U<<((scenario/2048)%7);
 }
 assert(seenPhases==127&&seenDraw&&seenExit);for(i=0;i<8;i++)assert(seenTimers[i]==0xFFFFFFFFU);return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
