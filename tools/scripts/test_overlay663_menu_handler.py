"""Menu phase transitions with mutable complete UI owners and typed service mocks."""
from overlay_units import read_c_source
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class Overlay663MenuHandlerTest(unittest.TestCase):
    def test_phases_and_mutations(self):
        root=Path(__file__).resolve().parents[2]
        body=read_c_source(root/'src/overlays/PAL/663/func_80115964.c')
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
#include "rr/overlay663_menu_handler.h"
#include "rr/frame_storage_types.h"
typedef struct {u32 guard[4];RrUiRow24 rows[2][4];s32 selection,limit,accepted;u16 phase,scene;s16 timer,nextScene;unsigned root,bank;u32 tail[4];} State;
typedef struct {s32 kind,args[7];u32 hash;} Event;
typedef struct {State s;Event events[16];unsigned calls;} World;
static World actual,reference,initial,*w;
static RrFrameStorage frames[2],frameSnapshot[2];
static s32 choices[9]={-91,0,0x12345678,2147483647,-7,11,22,33,44};
static unsigned scenario,mutation,phaseSeen[10];
#define ovl663_available_courses choices
#define ovl663_selected_record w->s.selection
#define ovl663_available_course_count w->s.limit
#define D_800F4D8A w->s.phase
#define ModeGroup w->s.scene
#define ModeId w->s.nextScene
#define MenuFadeTimer w->s.timer
#define MenuConfirmPressed w->s.accepted
#define UiRows (w->s.rows[w->s.root])
#define CurrentDrawBuf (&frames[w->s.bank])
static Event *event(int kind){unsigned i;u32 h=2166136261U;Event *e;assert(w->calls<16);
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 e=w->events+w->calls++;memset(e,0,sizeof(*e));e->kind=kind;e->hash=h;return e;
}
static void change(void){unsigned n=w->calls;if(!mutation)return;
 w->s.selection=(scenario+n)%9;w->s.limit=(int)(scenario>>2)%12-2;w->s.phase=(scenario+n)%10;
 w->s.root=(scenario+n)%2;w->s.bank=(scenario+n/2)%2;
 w->s.timer=(s16)(scenario+n*8191U);w->s.accepted=(scenario+n)%3-1;
 w->s.rows[w->s.root][0].byte14=(scenario+n)%5;
}
s32 LoadCoursePreviewStep(s32 resource){Event *e=event(1);e->args[0]=resource;change();return scenario&1?-1:0;}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){Event *e=event(2);e->args[0]=a;e->args[1]=b;e->args[2]=c;change();}
void StepMenuValue(s32 a,s32 b,char *selection,s32 limit,s32 flags){Event *e=event(3);assert(selection==(char*)&w->s.selection);e->args[0]=a;e->args[1]=b;e->args[2]=limit;e->args[3]=flags;change();}
void ResetScreenCounters(void){event(4);change();}
void ovl663_draw_preview(void){event(5);change();}
void PlaySfx(s32 a){event(6)->args[0]=a;change();}
void AddScreenFadeTiles(u32 *order,s32 fade,u32 flags){Event *e=event(7);assert(order==frames[w->s.bank].ordering.ordering70);assert((uintptr_t)order>UINT32_MAX);e->args[0]=w->s.bank;e->args[1]=fade;e->args[2]=flags;change();}
void ovl663_init_menu(void){event(8);change();}
void DrawMenuStepArrows(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g){Event *v=event(9);s32 args[]={a,b,c,d,e,f,g};memcpy(v->args,args,sizeof(args));change();}
void DrawUiRow(s32 a){event(10)->args[0]=a;change();}
void UpdateScreenFade(s32 a){event(11)->args[0]=a;change();}
''' + body + r'''
static void expected(void){State *s=&w->s;int old;
 int loaded=LoadCoursePreviewStep(choices[s->selection]);
 if(loaded){
  unsigned phase=s->phase;if(phase<10)phaseSeen[phase]++;
  if(phase==0){if(s->rows[s->root][0].byte14==2){SetMenuFrameOptions(1,1,0);s->phase=1;}}
  else if(phase==1){old=s->selection;StepMenuValue(1,4,(char*)&s->selection,s->limit,0);
   if(old==s->selection)ovl663_draw_preview();else ResetScreenCounters();
   s->rows[s->root][0].cells[0]=2;
   if(s->accepted){PlaySfx(1);SetMenuFrameOptions(0,1,0);s->phase=8;s->rows[s->root][0].cells[0]=1;s->timer=64;s->rows[s->root][0].byte14=3;}
  }else if(phase==8){u16 bits=(u16)s->timer-1;memcpy(&s->timer,&bits,2);
   AddScreenFadeTiles(frames[s->bank].ordering.ordering70,(64-s->timer)*4,69);
   if(s->timer<=0){ovl663_init_menu();s->scene=1;s->nextScene=74;s->phase=0;return;}
  }
 }
 if(s->limit>=2)DrawMenuStepArrows(160,384,16,1,s->selection,0,s->limit);
 DrawUiRow(0);UpdateScreenFade(0x10058);
}
int main(void){unsigned i;
 memset(frames,0xA7,sizeof(frames));memcpy(frameSnapshot,frames,sizeof(frames));
 for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));
  for(i=0;i<sizeof(initial.s);i++)((u8*)&initial.s)[i]=(u8)(scenario+i*31);
  initial.s.selection=(scenario>>1)%9;initial.s.limit=(int)(scenario>>4)%12-2;initial.s.phase=(scenario>>1)%10;
  initial.s.root=scenario%2;initial.s.bank=(scenario>>1)%2;initial.s.accepted=(scenario>>3)%3;
  {static const s16 timers[]={-32768,-32767,-1,0,1,2,63,64,65,32767};initial.s.timer=timers[(scenario/20)%10];}initial.s.rows[initial.s.root][0].byte14=(scenario>>2)%5;
  actual=reference=initial;w=&actual;ovl663_update_preview();w=&reference;expected();
  assert(!memcmp(&actual,&reference,sizeof(actual)));
 }
 assert(phaseSeen[0]&&phaseSeen[1]&&phaseSeen[8]);assert(!memcmp(frames,frameSnapshot,sizeof(frames)));return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
