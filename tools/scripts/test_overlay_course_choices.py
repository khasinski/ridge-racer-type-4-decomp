"""Real course screens and glyph/count helpers; simulated scene services."""
from overlay_units import read_c_source
from overlay_names import name as OVN
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class CourseChoicesTest(unittest.TestCase):
    def test_native_screens_and_signed_course_records(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join(read_c_source(root / f'src/overlays/PAL/{OVN(i)}/func_{a}.c')
                           for i,a in ((660,'801163C4'),(661,'80115F8C'),(667,'80116BB4')))
        bodies = re.sub(r'^extern[^\n]*\n', '', bodies, flags=re.M)
        glyph = read_c_source(root / 'tools/scripts/fixtures/views/func_80037040.c')
        count = read_c_source(root / 'tools/scripts/fixtures/views/func_800370BC.c')
        source = r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/course_choices.h"
#include "rr/race.h"
#include "rr/overlay665_render_types.h"
static u8 arenas[4][4096];static u32 heads[4][256];
struct State {
 u32 guards[4];RrMenuArenas arenas;void *current,*backing;
 RrSelectionRows selection;RrMenuGridPoint points[4];u8 ui[4][32];
 RrCourseChoice4 main[2][9],table661[2][2][9],table667[2][2][9];
 s32 player; s16 rows661[2],cols661[2],rows667[2],cols667[2];
 RrRaceVehicle vehicles[2];RrPreviewRenderScratch scratch;
 s32 word2de4,wordabeb0;s16 halfc64a,halfc5e0,row660,col660,reset660,counts660[2];
 u32 tail[4];
};
static struct World {struct State s;unsigned calls;struct {u32 hash;s32 service,arg;} events[8];} actual,expected,*w;
static unsigned scenario,mutation;
#define MenuArenas w->s.arenas
#define LoadCursor w->s.current
#define MenuArenaBase w->s.backing
#define SelectionRows w->s.selection
#define PlayerCourseChoices w->s.points
#define ImageRows w->s.ui[scenario%4]
#define CourseChoiceTable w->s.main
#define versus_select_choice_tables w->s.table661
#define CourseMenuChoices w->s.table667
#define SelectionPlayer w->s.player
#define versus_select_choice_group w->s.rows661
#define versus_select_choice_index w->s.cols661
#define CourseMenuPage w->s.rows667
#define CourseMenuCursor w->s.cols667
#define PlayerCars w->s.vehicles
#define RR_COURSE_SCRATCH (&w->s.scratch.projection)
#define RR_PROJECTION_SCRATCH RR_COURSE_SCRATCH
#define ModeFrame w->s.word2de4
#define ScreenFrame w->s.wordabeb0
#define MenuCursor w->s.halfc64a
#define MenuItemCount w->s.halfc5e0
#define time_attack_course_select_preview_bank w->s.row660
#define time_attack_course_select_preview_selection w->s.col660
#define time_attack_course_select_preview_confirmation w->s.reset660
#define time_attack_course_select_available_car_counts w->s.counts660
static s16 half(u32 bits){s16 result;u16 h=(u16)bits;memcpy(&result,&h,2);return result;}
static void effect(unsigned n){
 unsigned i,j,k;u32 value=scenario*0x9e3779b9U+n*47U;
 w->s.player=(scenario+n)%4;w->s.row660=(scenario+n)%2;w->s.col660=(scenario+n)%9;
 w->s.arenas.first=arenas[(scenario+n)%4];w->s.arenas.second=arenas[(scenario+n+1)%4];
 w->s.backing=arenas[(scenario+n+2)%4];w->s.current=arenas[(scenario+n+3)%4];
 w->s.word2de4=(s32)value;w->s.wordabeb0=(s32)~value;
 w->s.halfc64a=half(value);w->s.halfc5e0=half(value+1);w->s.reset660=half(value+2);
 for(i=0;i<2;i++){
  w->s.rows661[i]=((scenario>>4)+n+i)%2;w->s.cols661[i]=((scenario>>7)+n+i)%9;
  w->s.rows667[i]=((scenario>>5)+n+i+1)%2;w->s.cols667[i]=((scenario>>8)+n+i+3)%9;
  w->s.counts660[i]=half(value+i);
  for(j=0;j<9;j++){
   RrCourseChoice4 *p=&w->s.main[i][j];p->glyph=half(value+j*71+i);p->first=(s8)(value+j);p->second=(s8)(value+j*13);
   if(j==(scenario+n+i)%10)p->glyph=-1;
   if(j==8 && ((scenario+n)&1))p->glyph=0x167;
   for(k=0;k<2;k++){
    p=&w->s.table661[i][k][j];p->glyph=half(value+j*67+k*11+i);p->first=(s8)(value+j*37);p->second=(s8)(value+j*19);
    p=&w->s.table667[i][k][j];p->glyph=half(~value+j*79+k*17+i);p->first=(s8)(value+j*43);p->second=(s8)(value+j*23);
   }
  }
 }
 for(i=0;i<sizeof(w->s.vehicles);i++)((u8*)w->s.vehicles)[i]=(u8)(value+i*73);
 w->s.scratch.projection.pointers.cursor=arenas[(scenario+n)%4];
 w->s.scratch.projection.pointers.order=heads[(scenario+n)%4];
}
static void event(s32 service,s32 arg){
 unsigned i,n=w->calls;u32 h=2166136261U;assert(n<8);
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 w->events[n].hash=h;w->events[n].service=service;w->events[n].arg=arg;w->calls++;
 if(mutation)effect(n);
}
void SetupRenderer(s32 shift){assert(shift==5);event(1,shift);}
void ResetScreenCounters(void){event(2,0);}
void InitPreviewImageRows(void){event(3,0);}
void ResetCarState(RrRaceVehicle *vehicle){assert(vehicle==&w->s.vehicles[0]);event(4,0);}
''' + glyph + count + bodies + r'''
static void reference_glyph(s8 *output,s32 value){
 s32 first=value/160,second=value/40-first*4,last=value%40;
 output[0]=(s8)first;output[1]=(s8)second;output[2]=(s8)last;
}
static s32 reference_count(const RrCourseChoice4 *p){unsigned i;for(i=0;i<9;i++)if(p[i].glyph==-1 || (i==8 && p[i].glyph!=0x167))return i;return 9;}
static void reference(unsigned kind){
 if(kind==0){
  w->s.word2de4=0;w->s.wordabeb0=0;SetupRenderer(5);w->s.halfc64a=0;w->s.halfc5e0=3;
  ResetScreenCounters();InitPreviewImageRows();
  w->s.scratch.projection.word18=120;w->s.scratch.projection.word08=-16;
  w->s.scratch.projection.word0C=-248;w->s.scratch.projection.word10=-96;
  w->s.scratch.projection.word1C=0;w->s.scratch.projection.word20=0;
  w->s.current=w->s.backing;ResetCarState(&w->s.vehicles[0]);
  w->s.vehicles[0].pos20[1]=10;w->s.vehicles[0].pos20[2]=308;w->s.vehicles[0].rot54=2048;
  w->s.reset660=0;
  reference_glyph((s8*)&w->s.selection,w->s.main[w->s.row660][w->s.col660].glyph);
  w->s.points[0].first=w->s.main[w->s.row660][w->s.col660].first;
  w->s.points[0].second=w->s.main[w->s.row660][w->s.col660].second;
  w->s.counts660[0]=reference_count(w->s.main[0]);w->s.counts660[1]=reference_count(w->s.main[1]);
 }else{
  RrCourseChoice4 *p;s32 slot,side;ResetScreenCounters();w->s.current=w->s.arenas.first;
  slot=w->s.player;side=slot%2;
  p=kind==1?&w->s.table661[side][w->s.rows661[side]][w->s.cols661[side]]:
            &w->s.table667[side][w->s.rows667[side]][w->s.cols667[side]];
  reference_glyph((s8*)&w->s.selection+slot*4,p->glyph);
  w->s.points[slot].first=p->first;w->s.points[slot].second=p->second;
  w->s.ui[scenario%4][1]=6;
 }
}
int main(void){
 unsigned kind,i;static struct World initial;
 assert((uintptr_t)arenas>UINT32_MAX);
 for(kind=0;kind<3;kind++)for(scenario=0;scenario<131072;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;effect(9);
  for(i=0;i<sizeof(w->s.selection);i++)((u8*)&w->s.selection)[i]=(u8)(scenario+i*43);
  for(i=0;i<4;i++){w->s.guards[i]=0x12345678U+i;w->s.tail[i]=0x87654321U-i;}
  actual=initial;expected=initial;w=&actual;
  if(kind==0)time_attack_course_select_init_preview();else if(kind==1)versus_select_apply_player_choice();else serial_link_load_player_choice();
  w=&expected;reference(kind);assert(memcmp(&actual,&expected,sizeof(actual))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as directory:
            p = Path(directory)
            (p / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(p / 'test.c'), '-o', str(p / 'test')], check=True)
            subprocess.run([str(p / 'test')], check=True)


if __name__ == '__main__':
    unittest.main()
