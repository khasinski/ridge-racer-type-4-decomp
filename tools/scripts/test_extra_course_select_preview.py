"""Extra-course preview: complete owners and mutating typed service boundaries."""
from overlay_units import read_c_source
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
import subprocess
import re

class Overlay666PreviewTest(unittest.TestCase):
    def test_initialization_and_fresh_selection_roots(self):
        root=Path(__file__).resolve().parents[2]
        paths=['tools/scripts/fixtures/legacy/func_8003A198.c','tools/scripts/fixtures/legacy/func_80037040.c','src/overlays/PAL/extra_course_select/func_80115A88.c']
        bodies='\n'.join(re.sub(r'^extern[^\n]*;\n','',read_c_source(root/p),flags=re.M) for p in paths)
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/overlay666_preview.h"
static struct World {u32 head[4];RrRaceVehicle cars[2];RrProjectionScratchPrefix scratch;RrBadgeRow1C badges[3][2],*root;RrCourseChoice4 table[4][9];RrCurrentSelection choice;RrMenuGridPoint grid[4];void*base,*published;s32 frame,pulse,local;RrMenuSelectionWord32 index;s16 row,mode,flag;s8 clear1,clear2;u32 tail[4];} world,expected,checkpoint[6];
static unsigned scenario,calls,reference_calls;static u8 arenas[3][64],packets[64];static u32 orders[32];
#define PlayerCars world.cars
#define RR_PROJECTION_SCRATCH (&world.scratch)
#define BadgeRows world.root
#define extra_course_select_available_courses world.table
#define PlayerCourseChoices world.grid
#define SelectionRows world.choice
#define MenuArenaBase world.base
#define LoadCursor world.published
#define ModeFrame world.frame
#define ScreenFrame world.pulse
#define extra_course_select_selected_course world.index
#define extra_course_select_preview_phase world.local
#define CourseListIndex world.row
#define MenuItemCount world.mode
#define MenuCursor world.flag
#define MenuScriptStep world.clear1
#define LoadState world.clear2
'''+bodies+r'''
static u32 rng=0x15A88;static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s16 half(u32 x){u16 n=(u16)x;s16 v;memcpy(&v,&n,2);return v;}
static s32 word(u32 x){s32 v;memcpy(&v,&x,4);return v;}
static unsigned badge_index(RrBadgeRow1C*p){unsigned n;for(n=0;n<3;n++)if(p==world.badges[n])return n;assert(0);return 0;}
static void effect(struct World*w,unsigned n){unsigned j;
 for(j=0;j<sizeof(w->cars);j++)((u8*)w->cars)[j]=(u8)(scenario+j+n*31);
 for(j=0;j<sizeof(w->badges);j++)((u8*)w->badges)[j]=(u8)(scenario+j*13+n);
 w->root=world.badges[(scenario+n)%3];w->base=arenas[(scenario+n)%3]+(scenario%16);w->published=arenas[(scenario+n+1)%3];
 w->frame=word(scenario^0xA1234567U^n);w->pulse=word(scenario^0xB1234567U^n);w->index.word=(s32)((scenario/4+n)%9);w->local=(s32)n+13;w->row=(s16)((scenario+n)%4);w->mode=half(scenario+n);w->flag=half(scenario+n+1);w->clear1=(s8)n+1;w->clear2=(s8)n+2;
 w->scratch.pointers.cursor=packets+(scenario+n)%64;w->scratch.pointers.order=orders+(scenario+n)%32;
 w->scratch.word08=(s32)n+18;w->scratch.word0C=(s32)n+19;w->scratch.word10=(s32)n+20;w->scratch.word14=(s32)n+21;w->scratch.word18=(s32)n+22;w->scratch.word1C=(s32)n+23;w->scratch.word20=(s32)n+24;
 for(j=0;j<4;j++){((u8*)&w->choice)[j]=(u8)(scenario+n+j);w->grid[j].first=half(scenario+n+j);w->grid[j].second=half(scenario+n+j+1);}
}
static void boundary(unsigned n){assert(calls++==n);assert(memcmp(&world,&checkpoint[n],sizeof(world))==0);effect(&world,n);}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){assert(calls<2);assert(a==(s32)calls&&b==24&&c==(calls?128:88)&&d==68&&e==32&&f==0&&g==0&&h==1&&i==0&&j==(calls?85:84));boundary(calls);}
void SetupRenderer(s32 shift){assert(shift==5);boundary(2);}
void InitPreviewImageRows(void){boundary(3);}
void ResetCarState(RrRaceVehicle*p){assert(p==world.cars);assert((uintptr_t)p>UINT32_MAX);boundary(4);}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){assert(a==0&&b==1&&c==1);boundary(5);}
static void event(void){unsigned n=reference_calls++;assert(n<6);checkpoint[n]=expected;effect(&expected,n);}
static void reference(void){unsigned n;int glyph,t;
 expected.frame=expected.pulse=expected.index.word=expected.local=0;expected.flag=0;expected.mode=2;
 event();event();event();expected.clear1=expected.clear2=0;expected.pulse=0;event();
 expected.published=expected.base;n=badge_index(expected.root);expected.badges[n][0].half02=1;
 expected.scratch.word18=120;expected.scratch.word08=-16;expected.scratch.word0C=-248;expected.scratch.word1C=expected.scratch.word20=0;expected.scratch.word10=-96;
 event();expected.cars[0].pos20[1]=10;expected.cars[0].pos20[2]=308;expected.cars[0].rot54=2048;
 glyph=expected.table[expected.row][expected.index.word].glyph;t=glyph/160;
 expected.choice.selection.primary=(u8)t;expected.choice.selection.variant=(u8)(glyph/40-t*4);expected.choice.selection.pad02[0]=(u8)(glyph%40);
 expected.grid[0].first=expected.table[expected.row][expected.index.word].first;expected.grid[0].second=expected.table[expected.row][expected.index.word].second;event();
}
int main(void){unsigned j,k;for(scenario=0;scenario<65536;scenario++){
 memset(&world,0,sizeof(world));for(j=0;j<4;j++){world.head[j]=next();world.tail[j]=next();}
 for(j=0;j<4;j++)for(k=0;k<9;k++){world.table[j][k].glyph=half(scenario+j*233+k*901);world.table[j][k].first=(s8)(scenario+j+k);world.table[j][k].second=(s8)(scenario*7+j+k);}
 effect(&world,7);expected=world;calls=reference_calls=0;reference();extra_course_select_init_preview();assert(calls==6&&reference_calls==6);assert(memcmp(&world,&expected,sizeof(world))==0);
 assert((uintptr_t)world.base>UINT32_MAX&& (uintptr_t)world.root>UINT32_MAX);
 }return 0;}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
