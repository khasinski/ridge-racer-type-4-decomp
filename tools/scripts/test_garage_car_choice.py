"""Full choice group and save owner with actual name/glyph helpers."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class CarChoiceTest(unittest.TestCase):
    def test_native_choice_initialization_and_update(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/garage/func_{a}.c').read_text()
                           for a in ('801153B0', '80118E6C'))
        helpers = '\n'.join((root / f'tools/scripts/fixtures/views/func_{a}.c').read_text()
                            for a in ('8002B75C', '80037040'))
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
#include "rr/overlay662_car_choice.h"
static const char format[]="job_garage_car_info0\n";
typedef struct {
 u32 guard[4],rng;RrSaveProgressRecord7C save;RrCourseChoice4 choices[2][9];
 RrMenuGridPoint points[4];RrMenuChoice4 current;char name[12];
 s16 index,first,second,position,timer;u16 mode;u32 tail[4];
} State;
typedef struct {s32 op,arg;u32 hash;} Event;
typedef struct {State s;unsigned calls;Event events[4];} World;
static World actual,reference,initial,*w;static unsigned mutation;
#define SaveProgress w->s.save
#define CourseChoiceTable w->s.choices
#define PlayerCourseChoices w->s.points
#define SelectionRows w->s.current
#define ModeGroup w->s.mode
#define ModeId w->s.position
#define ModeSubState w->s.timer
#define garage_car_index w->s.index
#define garage_logo_number w->s.first
#define garage_clut_choice w->s.second
#define garage_player_name_copy w->s.name
#define garage_car_info_scene_name format
''' + bodies + helpers + r'''
static u32 next(void){u32 x=w->s.rng;x^=x<<13;x^=x>>17;x^=x<<5;return w->s.rng=x;}
static void change(void){unsigned i,j;
 for(i=0;i<2;i++)for(j=0;j<9;j++){
  w->s.choices[i][j].glyph=(s16)next();w->s.choices[i][j].first=(s8)next();w->s.choices[i][j].second=(s8)next();
 }
 for(i=0;i<4;i++){w->s.current.bytes[i]=next();w->s.points[i].first=(s16)next();w->s.points[i].second=(s16)next();}
 for(i=0;i<11;i++){w->s.name[i]='a'+next()%26;w->s.save.prefix.progress.prefix00.fields.name[i]='A'+next()%26;}
 w->s.name[11]=0;w->s.name[next()%12]=0;w->s.save.prefix.progress.prefix00.fields.name[11]=0;w->s.save.prefix.progress.prefix00.fields.name[next()%12]=0;
 w->s.index=next()%9;w->s.first=(s16)next();w->s.second=(s16)next();w->s.position=(s16)next();w->s.timer=(s16)next();w->s.mode=next();
}
static void event(s32 op,s32 arg){size_t i;u32 hash=2166136261U;Event *e;assert(w->calls<4);
 for(i=0;i<sizeof(w->s);i++)hash=(hash^((u8*)&w->s)[i])*16777619U;
 e=&w->events[w->calls++];e->op=op;e->arg=arg;e->hash=hash;
 if(mutation&&op!=3)change();
}
void SetDispMask(s32 mode){assert(mode==0);event(1,mode);}
void garage_init_car_info(void){event(2,0);}
void garage_init_car_select(void){event(4,0);}
s32 DebugPrintStub(const char *message,...){assert(message==format);event(3,0);return 1;}
static void expected(unsigned kind){
 if(!kind){unsigned n=0,i;s32 glyph;SetDispMask(0);w->s.index=0;
  while(w->s.save.prefix.progress.prefix00.fields.name[n])n++;
  for(i=0;i<n;i++)w->s.name[i]=w->s.save.prefix.progress.prefix00.fields.name[i];
  while(w->s.name[n])w->s.name[n++]=0;
  garage_init_car_info();glyph=w->s.choices[1][0].glyph;
  w->s.current.bytes[0]=(u8)(glyph/160);w->s.current.bytes[1]=(u8)(glyph/40-4*(glyph/160));w->s.current.bytes[2]=(u8)(glyph%40);
  w->s.points[0].first=w->s.choices[1][w->s.index].first;w->s.points[0].second=w->s.choices[1][w->s.index].second;
  w->s.mode=1;w->s.position=64;w->s.timer=0;DebugPrintStub(format);
 }else{
  RrCourseChoice4 c=w->s.choices[1][w->s.index];w->s.first=c.first;w->s.second=c.second;
  w->s.points[0].first=c.first;w->s.points[0].second=c.second;garage_init_car_select();
 }
}
int main(void){unsigned scenario,kind;u32 seen=0;
 _Static_assert(sizeof(RrSaveProgressRecord7C)==124,"save owner");assert((uintptr_t)actual.s.name>UINT32_MAX);
 for(scenario=0;scenario<65536;scenario++)for(kind=0;kind<2;kind++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0x153b0+scenario*19+kind;change();w->s.index=scenario%9;
  w->s.choices[1][0].glyph=(s16)scenario;
  actual=reference=initial;w=&actual;if(kind)garage_enter_car_select();else garage_enter_car_info();
  w=&reference;expected(kind);assert(!memcmp(&actual,&reference,sizeof(actual)));seen|=1U<<(scenario%9);
 }
 assert(seen==511);return 0;
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
