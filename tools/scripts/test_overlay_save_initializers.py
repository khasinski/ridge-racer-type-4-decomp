"""Complete124-byte save owner, unlock filtering and three real initializers."""
from overlay_units import read_c_source
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class SaveInitializersTest(unittest.TestCase):
    def test_native_flags_names_full_save_and_course_records(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join(read_c_source(root / f'src/overlays/PAL/{i}/func_{a}.c')
                           for i,a in ((660,'80114C40'),(662,'80114B9C'),(663,'80114A60')))
        bodies = re.sub(r'^extern[^\n]*\n', '', bodies, flags=re.M)
        helpers = '\n'.join(re.sub(r'^extern[^\n]*\n','',
                           read_c_source(root / f'tools/scripts/fixtures/views/func_{a}.c'),flags=re.M)
                           for a in ('8002B75C','80037040','80014A8C'))
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
#include "rr/diagnostics.h"
#include "rr/overlay_menu_init.h"
#include "rr/menu_transition.h"
#include "rr/frame_environment.h"
#include "rr/save_progress.h"
#include "rr/course_choice_types.h"
#include "rr/menu_glyph.h"
#include "rr/overlay662_progress_init.h"
static const char format660[]="course",format662[]="garage",format663[]="records";
struct State {
 u32 guard[4];RrSaveProgressRecord7C save;RrCourseChoice4 choices[2][9];
 char name[12];RrMenuChoice4 current;u8 order[9];s32 ids[9],values[9],count;
 s16 row,col,local,setting,position,timer;u16 mode;s32 zoom,driverSetting;u32 tail[4];
};
static struct World {struct State s;unsigned calls;struct {s32 service,arg[4];u32 hash;} events[12];} actual,expected,initial,*w;
static unsigned scenario,mutation,kind;
static u8 seenCounts[10];
#define SaveProgress w->s.save
#define CourseChoiceTable w->s.choices
#define SelectionRows w->s.current
#define ovl660_record_name w->s.name
#define ovl660_preview_bank w->s.row
#define ovl660_preview_selection w->s.col
#define ovl660_preview_confirmation w->s.local
#define ovl660_available_course_count w->s.zoom
#define ovl663_course_count w->s.zoom
#define ovl663_course_order w->s.order
#define ovl663_available_courses w->s.ids
#define ovl663_course_unlock_levels w->s.values
#define ovl663_available_course_count w->s.count
#define CourseMenuSelection w->s.setting
#define ModeGroup w->s.mode
#define ModeId w->s.position
#define D_800F4D8A w->s.timer
#define ModelTexturePage w->s.driverSetting
#define ovl660_title format660
#define ovl662_D_8011487C format662
#define ovl663_scene_name format663
static s32 word(u32 bits){s32 result;memcpy(&result,&bits,4);return result;}
static void effect(unsigned n){
 unsigned i,j;u32 value=scenario*0x9e3779b9U+n*47;
 static const s32 flags[]={INT32_MIN,-1,0,1,15,16,255,256,257,INT32_MAX};
 static const s16 glyphs[]={INT16_MIN,-1,0,1,358,359,INT16_MAX};
 static const u8 permutation[]={1,0,2,3,8,4,6,7,5};
 memset(&w->s.save,(u8)value,sizeof(w->s.save));
 for(i=0;i<11;i++)w->s.save.prefix.progress.prefix00.fields.name[i]=(char)('A'+(scenario+n+i)%26);
 w->s.save.prefix.progress.prefix00.fields.name[scenario%12]=0;
 w->s.save.prefix.progress.prefix00.fields.name[11]=0;
 w->s.save.prefix.progress.prefix00.fields.configuration0C=flags[(scenario+n)%10];
 w->s.save.prefix.reserved50.words[0]=flags[(scenario/10+n)%10];
 for(i=0;i<9;i++){
  unsigned count=(scenario>>4)%10;
  w->s.order[i]=permutation[(i+(scenario+n)%9)%9];
  w->s.save.unlocked60[i]=i<count?(u8)(1+(value+i)%255):0;
  w->s.ids[i]=word(value+i);w->s.values[i]=word(~value+i);
 }
 for(i=0;i<2;i++)for(j=0;j<9;j++){
  w->s.choices[i][j].glyph=glyphs[(scenario+n+i+j)%7];
  w->s.choices[i][j].second=(s8)(value+i+j);w->s.choices[i][j].first=(s8)(value+i+j+1);
 }
 for(i=0;i<11;i++)w->s.name[i]=(char)(1+(value+i)%254);
 w->s.name[scenario%12]=0;w->s.name[11]=0;
 for(i=0;i<4;i++)w->s.current.bytes[i]=(u8)(value+i*53);
 w->s.row=(s16)value;w->s.col=(s16)(value+1);w->s.local=(s16)(value+2);w->s.setting=(s16)(value+3);
 w->s.position=(s16)(value+4);w->s.timer=(s16)(value+5);w->s.mode=(u16)(value+6);
 w->s.zoom=word(value);w->s.count=word(~value);w->s.driverSetting=word(value+1);
}
static void event(s32 service,s32 a,s32 b,s32 c,s32 d){
 unsigned i,n=w->calls;u32 hash=2166136261U;assert(n<12);
 for(i=0;i<sizeof(w->s);i++)hash=(hash^((u8*)&w->s)[i])*16777619U;
 w->events[n].service=service;w->events[n].arg[0]=a;w->events[n].arg[1]=b;w->events[n].arg[2]=c;w->events[n].arg[3]=d;w->events[n].hash=hash;
 w->calls++;if(mutation && service!=7)effect(n);
}
void SetDispMask(s32 mode){assert(mode==0);event(1,mode,0,0,0);}
void ClearScreenOverlayState(void){event(2,0,0,0,0);}
void SetupDrawDispEnvs(s32 r,s32 g,s16 b,u32 flags){event(3,r,g,b,(s32)flags);}
void ovl660_func_80114920(void){event(4,0,0,0,0);}
void ovl662_func_80114998(void){event(5,0,0,0,0);}
void ovl663_init_menu(void){
 if(w==&actual){assert(w->s.count>=0 && w->s.count<=9);seenCounts[w->s.count]=1;}
 event(6,0,0,0,0);
}
s32 DebugPrintStub(const char *format,...){assert(format==(kind==0?format660:kind==1?format662:format663));event(7,0,0,0,0);return 1;}
void SeqStartTrack(s32 mode){assert(mode==0);event(8,mode,0,0,0);}
''' + helpers + bodies + r'''
static void copy_name(char *destination,const char *source){unsigned n=0,j;while(source[n])n++;for(j=0;j<n;j++)destination[j]=source[j];while(destination[n])destination[n++]=0;}
static void reference(void){
 unsigned i,n;s32 flags;SetDispMask(0);
 if(kind==0){
  ClearScreenOverlayState();w->s.row=0;w->s.col=0;w->s.local=0;
  copy_name(w->s.name,w->s.save.prefix.progress.prefix00.fields.name);SetupDrawDispEnvs(0,0,0,2);
  flags=w->s.save.prefix.progress.prefix00.fields.configuration0C;w->s.setting=0;
  w->s.zoom=flags>=16?8:4;if((u32)flags&256)w->s.zoom=16;
  ovl660_func_80114920();
  {s32 glyph=w->s.choices[0][0].glyph;w->s.current.bytes[0]=(u8)(glyph/160);w->s.current.bytes[1]=(u8)(glyph/40-(glyph/160)*4);w->s.current.bytes[2]=(u8)(glyph%40);}
  w->s.mode=1;w->s.position=50;w->s.timer=0;DebugPrintStub(format660);SeqStartTrack(0);
 }else if(kind==1){
  ClearScreenOverlayState();ovl662_func_80114998();SetupDrawDispEnvs(0,0,0,2);w->s.driverSetting=25;
  w->s.mode=1;w->s.position=40;w->s.timer=0;DebugPrintStub(format662);
  if(w->s.save.prefix.reserved50.words[0]==1 && w->s.choices[1][7].glyph>=0){w->s.choices[1][8].glyph=359;w->s.choices[1][8].first=1;}
  SeqStartTrack(0);
 }else{
  SetupDrawDispEnvs(0,0,0,2);w->s.mode=1;w->s.position=74;w->s.timer=0;DebugPrintStub(format663);n=0;w->s.count=0;
  for(i=0;i<9;i++){unsigned key=w->s.order[i];u8 value=w->s.save.unlocked60[key];if(value){w->s.ids[n]=(s32)key;w->s.values[n]=value;n++;w->s.count=(s32)n;}}
  ovl663_init_menu();SeqStartTrack(0);
  flags=w->s.save.prefix.progress.prefix00.fields.configuration0C;w->s.zoom=flags>=16?8:4;if((u32)flags&256)w->s.zoom=16;
 }
}
int main(void){unsigned i;
 _Static_assert(sizeof(RrSaveProgressRecord7C)==124,"full save");
 _Static_assert(offsetof(RrSaveProgressRecord7C,history6C)==108,"history");
 assert((uintptr_t)actual.s.name>UINT32_MAX);
 for(kind=0;kind<3;kind++)for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;effect(9);
  for(i=0;i<4;i++){w->s.guard[i]=0x12345678U+i;w->s.tail[i]=0x87654321U-i;}
  actual=initial;expected=initial;w=&actual;
  if(kind==0)ovl660_func_80114C40();else if(kind==1)ovl662_func_80114B9C();else ovl663_init_scene();
  w=&expected;reference();assert(memcmp(&actual,&expected,sizeof(actual))==0);
 }
 for(i=0;i<10;i++)assert(seenCounts[i]);return 0;
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
