"""Two complete layout initializers with real reset, glyph and count helpers."""
from overlay_units import read_c_source
from overlay_names import name as OVN
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class CarLayoutsTest(unittest.TestCase):
    def test_native_layouts(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join(read_c_source(root / f'src/overlays/PAL/{OVN(o)}/func_{a}.c')
                           for o,a in ((660,'80114920'),(662,'8011512C')))
        bodies = re.sub(r'__asm__\("\$2"\)', '', bodies)
        helpers = '\n'.join(re.sub(r'^extern[^\n]*\n','',
                            read_c_source(root / f'tools/scripts/fixtures/views/func_{a}.c'),flags=re.M)
                            for a in ('8003A198','80037040','800370BC','80032E9C'))
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
#include "rr/time_attack_course_select_course_layout.h"
#include "rr/garage_car_layout.h"
typedef struct {
 u32 guard[4];RrRaceVehicle cars[2];RrProjectionScratchPrefix scratch;
 RrMenuArenas arenas;RrMenuChoice4 choice;RrMenuGridPoint grid[4];RrCourseChoice4 table[2][9];
 u8 *resource;void *published,*base;s32 frame,pulse;
 s16 index,local,count,selection,selectionFlag,course,mode,flag;s8 clear1,clear2;u8 map[8];u32 tail[4];
} State;
typedef struct {s32 op,args[10];u32 hash;} Event;
typedef struct {State s;unsigned calls;Event events[16];} World;
static World actual,reference,initial,*w;
static unsigned scenario,kind,mutation,seenCounts;
static u8 resources[3][0x2D100],arenas[3][128],packets[128];static u32 orders[32];
#define PlayerCars w->s.cars
#undef RR_PROJECTION_SCRATCH
#define RR_PROJECTION_SCRATCH (&w->s.scratch)
#define CdDirectoryBuffer w->s.resource
#define MenuArenas w->s.arenas
#define MenuArenaBase w->s.base
#define LoadCursor w->s.published
#define SelectionRows w->s.choice
#define PlayerCourseChoices w->s.grid
#define CourseChoiceTable w->s.table
#define ModeFrame w->s.frame
#define ScreenFrame w->s.pulse
#define CourseMenuSelection w->s.selection
#define MenuCursor w->s.flag
#define MenuItemCount w->s.mode
#define ReverseCourse w->s.selectionFlag
#define CourseIndex w->s.course
#define MenuScriptStep w->s.clear1
#define LoadState w->s.clear2
#define CourseIndexMap w->s.map
#define garage_car_index w->s.index
#define garage_color_preview_state w->s.local
#define garage_car_list_count w->s.count
''' + helpers + bodies + r'''
static s32 word(u32 u){s32 v;memcpy(&v,&u,4);return v;}
static s16 half(u32 u){u16 n=u;s16 v;memcpy(&v,&n,2);return v;}
static void effect(unsigned n){unsigned i,j;u32 v=scenario*0x9e3779b9U+n*47;unsigned count=(scenario/9+n)%10;
 for(i=0;i<sizeof(w->s.cars);i++)((u8*)w->s.cars)[i]=(u8)(v+i*31);
 w->s.resource=resources[(scenario+n)%3]+scenario%32;w->s.base=arenas[(scenario+n+1)%3]+scenario%32;w->s.published=arenas[(scenario+n+2)%3];
 w->s.arenas.first=arenas[(scenario+n)%3];w->s.arenas.second=arenas[(scenario+n+1)%3];
 w->s.scratch.pointers.cursor=packets+(scenario+n)%128;w->s.scratch.pointers.order=orders+(scenario+n)%32;
 w->s.scratch.word08=word(v);w->s.scratch.word0C=word(v+1);w->s.scratch.word10=word(v+2);w->s.scratch.word14=word(v+3);w->s.scratch.word18=word(v+4);w->s.scratch.word1C=word(v+5);w->s.scratch.word20=word(v+6);
 w->s.frame=word(v+7);w->s.pulse=word(v+8);w->s.index=(scenario+n)%9;w->s.local=half(v+9);w->s.count=half(v+10);w->s.selection=half(v+11);w->s.selectionFlag=half(v+12);w->s.course=half(v+13);w->s.mode=half(v+14);w->s.flag=half(v+15);w->s.clear1=v;w->s.clear2=v+1;
 for(i=0;i<4;i++){w->s.choice.bytes[i]=v+i;w->s.grid[i].first=half(v+i);w->s.grid[i].second=half(v+i+1);}
 for(i=0;i<8;i++)w->s.map[i]=v+i;
 for(i=0;i<2;i++)for(j=0;j<9;j++){
  s16 glyph=half(v+i*901+j*233);if(glyph==-1)glyph=-2;
  w->s.table[i][j].glyph=j==count?-1:(j==8?(count==9?359:0):glyph);
  w->s.table[i][j].first=(s8)(v+i+j);w->s.table[i][j].second=(s8)(v+i+j+1);
 }
}
static Event *event(s32 op){size_t i;u32 h=2166136261U;Event *e;assert(w->calls<16);
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 e=&w->events[w->calls++];memset(e,0,sizeof(*e));e->op=op;e->hash=h;return e;
}
static void change(void){if(mutation)effect(w->calls);}
void ClearScreenOverlayState(void){event(1);change();}
void InitMenuArenas(void){event(2);change();}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){Event *v=event(3);s32 args[]={a,b,c,d,e,f,g,h,i,j};memcpy(v->args,args,sizeof(args));change();}
void SetupRenderer(s32 shift){Event *e=event(4);e->args[0]=shift;change();}
void InitPreviewImageRows(void){event(5);change();}
void ResetCarState(RrRaceVehicle *car){Event *e=event(6);assert(car==w->s.cars||car==w->s.cars+1);e->args[0]=car-w->s.cars;assert((uintptr_t)car>UINT32_MAX);change();}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){Event *e=event(7);e->args[0]=a;e->args[1]=b;e->args[2]=c;if(w==&actual){assert(w->s.count>=0&&w->s.count<=9);seenCounts|=1U<<w->s.count;}change();}
static void reset(void){w->s.clear1=0;w->s.clear2=0;w->s.pulse=0;}
static void expected(void){unsigned i;State *s=&w->s;
 if(kind)ClearScreenOverlayState();s->frame=0;s->pulse=0;s->flag=0;s->mode=kind?4:3;
 if(!kind){int selected=s->selection;s->selectionFlag=selected>=8;s->course=s->map[(u32)selected&7];}
 reset();s->arenas.second=s->resource+0x2D000;InitMenuArenas();s->published=s->arenas.second;
 if(!kind){
  static const s32 rows[12][4]={{12,64,68,83},{12,104,68,102},{12,144,68,85},{12,368,208,0},{24,88,68,84},{24,128,68,54},{24,168,68,85},{24,88,68,84},{24,128,68,53},{24,168,68,85},{116,88,28,55},{116,128,28,56}};
  for(i=0;i<12;i++)InitUiRow(i,rows[i][0],rows[i][1],rows[i][2],32,0,0,i==3?8:1,i==3?4:0,i==3?(s->selection&7):rows[i][3]);
  s->scratch.word18=512;s->scratch.word0C=-416;s->scratch.word1C=0;s->scratch.word20=0;s->scratch.word08=0;s->scratch.word10=0;
  for(i=0;i<3;i++)s->cars[0].pos20[i]=0;
 }else{
  static const s32 labels[]={21,102,22,85};int glyph,t,count;
  for(i=0;i<4;i++)InitUiRow(i,24,88+40*i,68,32,0,0,1,0,labels[i]);
  SetupRenderer(5);reset();InitPreviewImageRows();s->local=0;
  s->scratch.word18=120;s->scratch.word08=-16;s->scratch.word0C=-248;s->scratch.word1C=0;s->scratch.word20=0;s->scratch.word10=-96;
  s->published=s->base;s->arenas.first=s->base;
  for(i=0;i<2;i++){ResetCarState(s->cars+i);s->cars[i].pos20[1]=10;s->cars[i].pos20[2]=308;s->cars[i].rot54=2048;}
  glyph=s->table[1][s->index].glyph;t=glyph/160;s->choice.bytes[0]=(u8)t;s->choice.bytes[1]=(u8)(glyph/40-4*t);s->choice.bytes[2]=(u8)(glyph%40);
  s->grid[0].first=s->table[1][s->index].first;s->grid[0].second=s->table[1][s->index].second;
  count=0;while(count<9&&s->table[1][count].glyph!=-1){if(count==8&&s->table[1][8].glyph!=359)break;count++;}s->count=count;SetMenuFrameOptions(0,1,1);
 }
}
int main(void){unsigned i;assert((uintptr_t)resources>UINT32_MAX);
 for(scenario=0;scenario<65536;scenario++)for(kind=0;kind<2;kind++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;effect(0);w->s.selection=half(scenario);for(i=0;i<4;i++){w->s.guard[i]=scenario+i;w->s.tail[i]=~(scenario+i);}actual=reference=initial;
  w=&actual;if(kind)garage_init_car_info();else time_attack_course_select_init_menu();w=&reference;expected();assert(!memcmp(&actual,&reference,sizeof(actual)));
 }
 assert(seenCounts==1023);return 0;
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
