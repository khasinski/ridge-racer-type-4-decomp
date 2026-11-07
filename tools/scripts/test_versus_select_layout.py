"""Full two-player layout initializer with real reset, glyph and count helpers."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class Overlay661LayoutTest(unittest.TestCase):
    def test_native_layouts(self):
        root = Path(__file__).resolve().parents[2]
        bodies = (root / 'src/overlays/PAL/versus_select/func_80115598.c').read_text()
        bodies = re.sub(r'__asm__\("\$2"\)', '', bodies)
        helpers = '\n'.join(re.sub(r'^extern[^\n]*\n','',
                            (root / f'tools/scripts/fixtures/legacy/func_{a}.c').read_text(),flags=re.M)
                            for a in ('8003A198','80037040','800370BC'))
        helpers = helpers.replace("void UnpackChoiceGlyph(", "void real_glyph(")
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
#include "rr/overlay661_layout.h"
typedef struct {
 u32 guard[4];RrRaceVehicle cars[2];RrProjectionScratchPrefix scratch;
 RrMenuArenas arenas;RrSelectionRows choice;RrMenuGridPoint grid[4];RrCourseChoice4 table[2][2][9];s16 counts[2][2],groups[2],courses[2];s32 player;
 u8 *resource;void *published,*base;s32 frame,pulse;
 s16 index,local,count,selection,selectionFlag,course,mode,flag;s8 clear1,clear2;u8 map[8];u32 tail[4];
} State;
typedef struct {s32 op,args[10];u32 hash;} Event;
typedef struct {State s;unsigned calls;Event events[40];} World;
static World actual,reference,initial,*w;
static unsigned scenario,kind,mutation,seenCounts;static unsigned seenChoices[2][2][9];
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
#define SelectionPlayer w->s.player
#define versus_select_subchoice_state w->s.local
#define versus_select_choice_counts w->s.counts
#define versus_select_choice_index w->s.courses
#define versus_select_choice_group w->s.groups
#define versus_select_choice_tables w->s.table
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
 for(i=0;i<4;i++){((u8*)&w->s.choice)[i]=v+i;w->s.grid[i].first=half(v+i);w->s.grid[i].second=half(v+i+1);}
 for(i=0;i<8;i++)w->s.map[i]=v+i;
 w->s.player=(scenario+n)%2;
 for(i=0;i<2;i++){w->s.groups[i]=((scenario>>(i+1))+n)%2;w->s.courses[i]=((scenario/(i?72:8))+n)%9;}
 for(i=0;i<4;i++)for(j=0;j<9;j++){
  count=(scenario/9+n+i)%10;
  s16 glyph=half(v+i*901+j*233);if(glyph==-1)glyph=-2;
  w->s.table[i/2][i%2][j].glyph=j==count?-1:(j==8?(count==9?359:0):glyph);
  w->s.table[i/2][i%2][j].first=(s8)(v+i+j);w->s.table[i/2][i%2][j].second=(s8)(v+i+j+1);
 }
}
static Event *event(s32 op){size_t i;u32 h=2166136261U;Event *e;assert(w->calls<40);
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
void UnpackChoiceGlyph(s8 *out,s32 glyph){unsigned p=w->s.player,g=w->s.groups[p],c=w->s.courses[p];assert(p<2&&g<2&&c<9);seenChoices[p][g][c]++;real_glyph(out,glyph);}
static void expected(void){unsigned i,p,g;State *s=&w->s;
 static const s32 rows[26][10]={{0,24,88,68,32,0,0,1,0,83},{1,24,128,68,32,0,0,1,0,54},{2,24,168,68,32,0,0,1,0,85},{3,24,88,68,32,0,0,1,0,83},{4,24,128,68,32,0,0,1,0,101},{5,24,168,68,32,0,0,1,0,53},{6,24,208,68,32,0,0,1,0,85},{7,116,88,28,32,0,0,1,0,55},{8,116,128,28,32,0,0,1,0,56},{9,24,88,68,32,0,0,1,0,83},{10,24,128,68,32,0,0,1,0,54},{11,24,168,68,32,0,0,1,0,85},{12,24,88,68,32,0,0,1,0,83},{13,24,128,68,32,0,0,1,0,101},{14,24,168,68,32,0,0,1,0,53},{15,24,208,68,32,0,0,1,0,85},{16,116,88,28,32,0,0,1,0,55},{17,116,128,28,32,0,0,1,0,56},{18,12,64,68,32,0,0,1,0,84},{19,12,104,68,32,0,0,1,0,3},{20,12,144,68,32,0,0,1,1,13},{21,12,184,68,32,0,0,1,0,85},{22,12,368,208,32,0,0,8,4,0},{23,104,104,68,32,0,0,1,5,3},{24,104,144,68,32,0,0,1,6,23},{25,104,184,68,32,0,0,1,7,6}};
 s->frame=0;s->pulse=0;s->flag=0;s->mode=3;reset();s->arenas.second=s->resource+0x2D000;InitMenuArenas();s->player=kind;s->published=s->arenas.second;
 for(i=0;i<26;i++)InitUiRow(rows[i][0],rows[i][1],rows[i][2],rows[i][3],rows[i][4],rows[i][5],rows[i][6],rows[i][7],rows[i][8],i==22?s->selection:rows[i][9]);
 SetupRenderer(5);s->flag=0;s->mode=3;reset();InitPreviewImageRows();s->local=0;s->published=s->base;if(!(s->player&1))s->arenas.first=s->base;else s->published=s->arenas.first;
 s->scratch.word18=120;s->scratch.word08=-16;s->scratch.word0C=-248;s->scratch.word1C=0;s->scratch.word20=0;s->scratch.word10=-96;
 for(i=0;i<2;i++){ResetCarState(s->cars+i);s->cars[i].pos20[1]=10;s->cars[i].pos20[2]=308;s->cars[i].rot54=2048;}
 for(p=0;p<2;p++)for(g=0;g<2;g++){unsigned n=0;while(n<9&&s->table[p][g][n].glyph!=-1){if(n==8&&s->table[p][g][8].glyph!=359)break;n++;}s->counts[p][g]=n;}
 if(!s->counts[kind][1])s->groups[kind]=0;
 p=s->player&1;if(s->courses[p]>=s->counts[p][s->groups[p]])s->courses[p]=0;
 {RrCourseChoice4 c=s->table[p][s->groups[p]][s->courses[p]];int t=c.glyph/160;u8 *out=(u8*)&s->choice+4*s->player;out[0]=(u8)t;out[1]=(u8)(c.glyph/40-4*t);out[2]=(u8)(c.glyph%40);s->grid[s->player].first=c.first;s->grid[s->player].second=c.second;}
}
int main(void){unsigned i;assert((uintptr_t)resources>UINT32_MAX);
 for(scenario=0;scenario<65536;scenario++)for(kind=0;kind<2;kind++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;effect(0);w->s.selection=half(scenario);for(i=0;i<4;i++){w->s.guard[i]=scenario+i;w->s.tail[i]=~(scenario+i);}actual=reference=initial;
  w=&actual;versus_select_init_selection_screen(kind);
  {unsigned p,g;for(p=0;p<2;p++)for(g=0;g<2;g++){assert(w->s.counts[p][g]>=0&&w->s.counts[p][g]<=9);seenCounts|=1U<<w->s.counts[p][g];}}w=&reference;expected();assert(!memcmp(&actual,&reference,sizeof(actual)));
 }
 assert(seenCounts==1023);{unsigned p,g,c;for(p=0;p<2;p++)for(g=0;g<2;g++)for(c=0;c<9;c++)assert(seenChoices[p][g][c]);}return 0;
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

if __name__ == "__main__":
    unittest.main()
