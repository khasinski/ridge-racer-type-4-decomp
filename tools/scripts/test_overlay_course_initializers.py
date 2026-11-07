"""Real course initializers, string-tail copy and glyph decomposition."""
from overlay_units import read_c_source
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class CourseInitializersTest(unittest.TestCase):
    def test_native_names_group_capacity_and_preservation(self):
        root = Path(__file__).resolve().parents[2]
        bodies = []
        for index,address in ((661,'80115DB8'),(666,'80114B14')):
            body = read_c_source(root / f'src/overlays/PAL/{index}/func_{address}.c')
            body = re.sub(r'^extern[^\n]*\n', '', body, flags=re.M)
            # The fixture owns both existing views of the same selection storage.
            body = body.replace('#include "rr/overlay666_preview.h"',
                                '#include "rr/course_choice_types.h"')
            bodies.append(body)
        helpers = '\n'.join(read_c_source(root / f'tools/scripts/fixtures/views/func_{a}.c')
                            for a in ('8002B75C','80037040'))
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
#include "rr/menu_choice_types.h"
#include "rr/current_selection_types.h"
#include "rr/save_progress.h"
#include "rr/frame_environment.h"
#include "rr/diagnostics.h"
static char player2[12]="PLAYER 2";
static const char format661[]="job_vs_car_info0\n",format666[]="job_ex_crs_info0\n";
union Selection {RrSelectionRows complete;RrMenuChoice4 prefix;RrCurrentSelection owner;};
struct State {
 u32 guard[4];RrSaveProgressRecord7C save;char names[24];union Selection choice;
 RrCourseChoice4 source[2][9],versus[2][2][9],extra[4][9];
 s16 selections[2],rows[2],columns[2],counts[4];
 s16 clearABEC8,clearAC648,clearF3914,clearF3826,modeAC59C,flag,position,selected;u16 course;
 s32 laps,marker;u32 tail[4];
};
static struct World {struct State s;unsigned calls;struct {s32 service,args[4];u32 hash;} events[12];} actual,expected,initial,*w;
static unsigned scenario,mutation,kind;static u8 seenCounts[4][10];
#define SaveProgress w->s.save
#define CourseChoiceTable w->s.source
#define ovl661_D_80118458 w->s.versus
#define ovl666_available_courses w->s.extra
#define ovl661_D_80118420 w->s.names
#define ovl661_D_8011487C player2
#define ovl661_D_80114888 format661
#define ovl666_scene_name format666
#define ovl661_D_80118418 w->s.selections
#define ovl661_D_80118450 w->s.rows
#define ovl661_D_8011844C w->s.columns
#define ovl666_course_counts w->s.counts
#define SelectedCarIndex w->s.clearABEC8
#define CourseMenuSelection w->s.clearAC648
#define CourseIndex w->s.clearF3914
#define NumCars w->s.modeAC59C
#define MenuCursor w->s.flag
#define NumLaps w->s.laps
#define ovl661_D_80118414 w->s.marker
#define ModeGroup w->s.course
#define ModeId w->s.position
#define ModeSubState w->s.selected
#define CourseListIndex w->s.clearF3826
static void effect(unsigned n){
 unsigned i,j,k;u32 value=scenario*0x9e3779b9U+n*47U;
 for(i=0;i<2;i++){
  w->s.selections[i]=(s16)(value+i);w->s.rows[i]=(s16)(value+i+1);w->s.columns[i]=(s16)(value+i+2);
  for(j=0;j<9;j++){
   RrCourseChoice4 *p=&w->s.source[i][j];
   p->glyph=(s16)((scenario+j*43+n*71)%32000);p->second=(s8)(value+j*13);p->first=(s8)(value+j*17);
   if((scenario&7)==0)p->glyph=(s16)(((scenario/8)%4)*40+(j<((scenario>>5)%10)?j:39));
   else if((scenario&7)==1)p->glyph=(s16)(39+((scenario+j)%4)*40);
   else if((scenario&15)==2)p->glyph=(s16)(-1-(s32)((scenario+j)%32000));
   for(k=0;k<2;k++){w->s.versus[k][i][j].glyph=(s16)(value+j+k);w->s.versus[k][i][j].second=(s8)(value+j+k+3);w->s.versus[k][i][j].first=(s8)(value+j+k+5);}
  }
 }
 for(i=0;i<4;i++){
  w->s.counts[i]=(s16)(value+i);
  for(j=0;j<9;j++){w->s.extra[i][j].glyph=(s16)(value+j);w->s.extra[i][j].second=(s8)(value+j+1);w->s.extra[i][j].first=(s8)(value+j+2);}
 }
 for(i=0;i<sizeof(w->s.names);i++)w->s.names[i]=(char)(1+(value+i)%254);
 w->s.names[11]=0;w->s.names[23]=0;
 for(i=0;i<16;i++)w->s.save.prefix.progress.prefix00.bytes[i]=(u8)('A'+(scenario+i+n)%26);
 w->s.save.prefix.progress.prefix00.bytes[scenario%12]=0;w->s.save.prefix.progress.prefix00.bytes[11]=0;
 for(i=0;i<sizeof(w->s.choice);i++)((u8*)&w->s.choice)[i]=(u8)(value+i*23);
 w->s.clearABEC8=(s16)value;w->s.clearAC648=(s16)(value+1);w->s.clearF3914=(s16)(value+2);w->s.clearF3826=(s16)(value+8);w->s.modeAC59C=(s16)(value+3);
 w->s.flag=(s16)(value+4);w->s.position=(s16)(value+5);w->s.selected=(s16)(value+6);w->s.course=(s16)(value+7);
 w->s.laps=(s32)value;w->s.marker=(s32)~value;
}
static void event(s32 service,s32 a,s32 b,s32 c,s32 d){
 unsigned i,n=w->calls;u32 h=2166136261U;assert(n<12);
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 w->events[n].service=service;w->events[n].args[0]=a;w->events[n].args[1]=b;w->events[n].args[2]=c;w->events[n].args[3]=d;w->events[n].hash=h;
 w->calls++;if(mutation && service!=7)effect(n);
}
void SetDispMask(s32 mode){assert(mode==0);event(1,mode,0,0,0);}
void ClearScreenOverlayState(void){event(2,0,0,0,0);}
void SetupDrawDispEnvs(s32 r,s32 g,s16 b,u32 flags){event(3,r,g,b,(s32)flags);}
void ovl661_func_80115598(s32 mode){assert(mode==0);event(4,mode,0,0,0);}
void SeqStartTrack(s32 mode){assert(mode==0);event(5,mode,0,0,0);}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){event(6,a,b,c,0);}
s32 DebugPrintStub(const char *format,...){
 unsigned i;assert(format==(kind?format666:format661));
 if(kind && w==&actual)for(i=0;i<4;i++){assert(w->s.counts[i]>=0 && w->s.counts[i]<=9);seenCounts[i][w->s.counts[i]]=1;}
 event(7,0,0,0,0);return 1;
}
void ovl666_init_menu(void){event(8,0,0,0,0);}
''' + helpers + '\n#define SelectionRows w->s.choice.complete\n' + bodies[0] + \
                 '\n#undef SelectionRows\n#define SelectionRows w->s.choice.owner\n' + bodies[1] + r'''
static void reference_copy(char *destination,const char *source){
 unsigned i=0,j;while(source[i])i++;for(j=0;j<i;j++)destination[j]=source[j];
 while(destination[i])destination[i++]=0;
}
static void reference_glyph(s8 *destination,s32 glyph){
 destination[0]=(s8)(glyph/160);destination[1]=(s8)(glyph/40-(glyph/160)*4);destination[2]=(s8)(glyph%40);
}
static void reference(void){unsigned i,j,k;
 SetDispMask(0);ClearScreenOverlayState();
 if(kind==0){
  for(i=0;i<2;i++)w->s.selections[i]=w->s.rows[i]=w->s.columns[i]=0;
  w->s.clearABEC8=0;w->s.clearAC648=0;w->s.clearF3914=0;w->s.laps=3;w->s.modeAC59C=2;
  for(i=0;i<2;i++)for(j=0;j<9;j++){w->s.versus[0][i][j]=w->s.source[i][j];w->s.versus[1][i][j]=w->s.source[0][j];}
  reference_copy(w->s.names,(char*)w->s.save.prefix.progress.prefix00.bytes);reference_copy(w->s.names+12,player2);
  SetupDrawDispEnvs(0,0,0,2);ovl661_func_80115598(0);
  reference_glyph((s8*)&w->s.choice.complete,w->s.source[0][0].glyph);
  reference_glyph((s8*)&w->s.choice.complete+4,w->s.source[0][0].glyph);
  SeqStartTrack(0);w->s.marker=8;w->s.course=1;w->s.position=56;w->s.selected=0;
  DebugPrintStub(format661);SetMenuFrameOptions(0,1,1);
 }else{
  SetupDrawDispEnvs(0,0,0,2);
  for(i=0;i<4;i++)for(j=0;j<9;j++)w->s.extra[i][j].glyph=-1;
  for(i=0;i<4;i++){
   k=0;w->s.counts[i]=0;
   for(j=0;j<9;j++){
    s32 value=w->s.source[1][j].glyph;
    reference_glyph((s8*)w->s.choice.prefix.bytes,value);
    if((u8)(value/40-(value/160)*4)==i && (u8)(value%40)<39){w->s.extra[i][k++]=w->s.source[1][j];w->s.counts[i]=(s16)k;}
   }
  }
  w->s.clearAC648=0;w->s.clearF3826=0;DebugPrintStub(format666);ovl666_init_menu();
  w->s.course=1;w->s.position=79;w->s.selected=0;SeqStartTrack(0);
 }
}
int main(void){unsigned i,j;
 assert((uintptr_t)actual.s.names>UINT32_MAX);
 for(kind=0;kind<2;kind++)for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;effect(9);
  for(i=0;i<4;i++){w->s.guard[i]=0x12345678U+i;w->s.tail[i]=0x87654321U-i;}
  actual=initial;expected=initial;w=&actual;
  if(kind)ovl666_init_scene();else ovl661_func_80115DB8();
  w=&expected;reference();assert(memcmp(&actual,&expected,sizeof(actual))==0);
 }
 for(i=0;i<4;i++)for(j=0;j<10;j++)assert(seenCounts[i][j]);
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
