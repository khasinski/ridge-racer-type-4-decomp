"""Four menu actions, live helper mutations and captured choice bytes."""
from overlay_units import read_c_source
from overlay_names import name as OVN
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayMenuActionsTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        entries = [(660, '801156A8'), (661, '80117E10'), (666, '80115510'),
                   (667, '801187DC'), (662, '8011662C')]
        bodies = '\n'.join(read_c_source(root / f'src/overlays/PAL/{OVN(i)}/func_{a}.c')
                           for i, a in entries)
        resident = read_c_source(root / 'tools/scripts/fixtures/views/func_8004F24C.c')
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/overlay_menu.h"
typedef struct {s32 kind,args[7];const void *pointer;} Event;
static struct World {
 u32 before[8];s16 selection,limit; s32 accepted;RrMenuChoice4 choice;
 s16 first_selection,choice_selection,second_selection,sound_selection,first_limit;
 s32 first_accept,choice_accept,second_accept,sound_accept;
 u8 changed_choice[4];u32 draws,calls;Event events[8];u32 after[8];
} world,expected;
#define MenuCursor world.selection
#define MenuItemCount world.limit
#define MenuConfirmPressed world.accepted
#define SelectionRows world.choice
#define CourseListSavedIndex world.choice.bytes[1]
static Event *event(struct World *w,s32 kind){Event *e;assert(w->calls<8);e=&w->events[w->calls++];e->kind=kind;return e;}
static void draw(struct World *w,s32 row,s32 a,s32 b){
 Event *e=event(w,1);e->args[0]=row;e->args[1]=a;e->args[2]=b;
 if(w->draws++==0){w->selection=w->first_selection;w->limit=w->first_limit;w->accepted=w->first_accept;}
 else {w->selection=w->second_selection;w->accepted=w->second_accept;}
}
static void choose(struct World *w,s32 a,s32 b,char *pointer,s32 limit,s32 flags){
 Event *e=event(w,2);e->args[0]=a;e->args[1]=b;e->args[2]=limit;e->args[3]=flags;e->pointer=pointer;
 assert(pointer==(char*)&world.selection);
 w->selection=w->choice_selection;w->accepted=w->choice_accept;
}
static void sound(struct World *w,s32 a,s32 b,s32 c,s32 d){
 Event *e=event(w,3);e->args[0]=a;e->args[1]=b;e->args[2]=c;e->args[3]=d;
 w->selection=w->sound_selection;w->accepted=w->sound_accept;
}
void SetMenuTextRowCursor(s32 a,s32 b,s32 c){draw(&world,a,b,c);}
void StepMenuValue(s32 a,s32 b,char *p,s32 c,s32 d){choose(&world,a,b,p,c,d);}
int PlaySfxVolPan(s32 a,s32 b,s32 c,s32 d){sound(&world,a,b,c,d);return -1;}
void DrawMenuStepArrows(s32 a,s32 b,s32 c,s32 d,s32 f,s32 g,s32 h){
 Event *e=event(&world,4);e->args[0]=a;e->args[1]=b;e->args[2]=c;e->args[3]=d;e->args[4]=f;e->args[5]=g;e->args[6]=h;
 memcpy(world.choice.bytes,world.changed_choice,4);
}
s32 UpdateGlyphRow(s32 selected){Event *e=event(&world,5);e->args[0]=selected;world.accepted^=0x12345678;return -123;}
''' + resident + bodies + r'''
static u32 seed=0x17e10;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half_value(u32 bits){u16 raw=bits;s16 v;memcpy(&v,&raw,2);return v;}
static s16 half(void){return half_value(next());}
static void initialize(u32 k){unsigned i;memset(&world,0,sizeof(world));
 for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
 world.selection=half_value(k);world.limit=half();world.accepted=next();
 world.first_selection=half_value(k*40503U);world.choice_selection=half_value(k*17U+39);
 world.second_selection=half_value(k*3U+5);world.sound_selection=half_value(k*7U+11);world.first_limit=half_value(k*11U+17);
 world.first_accept=next();world.choice_accept=next();
 world.second_accept=(k&1)?(s32)(next()|1):0;world.sound_accept=(k&2)?0:-1;
 for(i=0;i<4;i++){world.choice.bytes[i]=next();world.changed_choice[i]=next();}
}
static s32 results_menu(void){return time_attack_course_select_func_801156A8(0);}
static s32 retry_menu(void){return extra_course_select_update_retry_choice(0);}
static void menus(unsigned which){u32 k;const s32 offsets[]={37,38,37,39};
 s32 (*functions[])(void)={results_menu,versus_select_func_80117E10,retry_menu,serial_link_func_801187DC};
 for(k=0;k<65536;k++){
  s32 result,wanted,limit;initialize(k);memcpy(&expected,&world,sizeof(world));
  draw(&expected,(s32)expected.selection+offsets[which],0,0);
  limit=which==0?expected.limit:which==3?2:3;
  choose(&expected,0,2,(char*)&world.selection,limit,0);
  draw(&expected,(s32)expected.selection+offsets[which],1,1);
  wanted=-1;
  if(expected.accepted){sound(&expected,1,127,64,-1);wanted=expected.selection;}
  result=functions[which]();assert(result==wanted);
  assert(memcmp(&world,&expected,sizeof(world))==0);
  assert((uintptr_t)&world.selection>UINT32_MAX);
 }
}
static void choices(void){u32 k,m;
 for(m=0;m<2;m++)for(k=0;k<65536;k++){
  s32 captured;Event *e;initialize(k);
  world.choice.bytes[0]=k;world.choice.bytes[1]=k>>8;
  captured=m?world.choice.bytes[1]:world.choice.bytes[0];
  memcpy(&expected,&world,sizeof(world));e=event(&expected,4);
  e->args[0]=160;e->args[1]=204;e->args[2]=48;e->args[3]=0;e->args[4]=captured;e->args[5]=0;e->args[6]=4;
  memcpy(expected.choice.bytes,expected.changed_choice,4);
  e=event(&expected,5);e->args[0]=captured;expected.accepted^=0x12345678;
  assert(garage_func_8011662C(next(),m?(k&1?-1:1):0)==-123);
  assert(memcmp(&world,&expected,sizeof(world))==0);
 }
}
int main(int argc,char **argv){unsigned which;assert(argc==2);which=atoi(argv[1]);if(which<4)menus(which);else choices();return 0;}
'''
        path = Path(cls.temp.name)
        (path / 'test.c').write_text(source)
        cls.binary = path / 'test'
        subprocess.run(['cc', '-std=gnu11', '-O2', '-Wno-deprecated-non-prototype',
                        '-fsanitize=undefined', '-fno-sanitize-recover=all',
                        '-I', str(root / 'include'), str(path / 'test.c'),
                        '-o', str(cls.binary)], check=True)

    def test_time_attack_course_select_menu(self):
        subprocess.run([str(self.binary), '0'], check=True)

    def test_versus_select_menu(self):
        subprocess.run([str(self.binary), '1'], check=True)

    def test_extra_course_select_menu(self):
        subprocess.run([str(self.binary), '2'], check=True)

    def test_serial_link_menu(self):
        subprocess.run([str(self.binary), '3'], check=True)

    def test_captured_choice(self):
        subprocess.run([str(self.binary), '4'], check=True)


if __name__ == '__main__':
    unittest.main()
