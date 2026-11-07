"""Result-screen setup: shared progress owner, real standings and live tables.

Row initialization/rendering, result classification and transitions are typed
simulated boundaries. The six-entry standings selector executes its actual C.
"""
from overlay_units import read_c_source
from overlay_names import name as OVN
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest


class OverlayResultSetupTest(unittest.TestCase):
    def test_progress_flags_label_banks_standings_and_mutable_services(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join(re.sub(r'^extern[^\n]*;\n', '',
                                  read_c_source(root / f'src/overlays/PAL/{OVN(ov)}/func_{addr}.c'),
                                  flags=re.M)
                           for ov, addr in [(660, '801162D8'), (660, '801154D8'),
                                            (666, '80115388')])
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
#include "rr/menu_text_rows.h"
#include "rr/overlay_menu.h"
#include "rr/menu_selection.h"
#include "rr/standings.h"
#include "rr/save_progress.h"
RrMenuTextRow60 SharedWorkBuffer[55];
struct State {
 u32 guard[4];RrSaveProgressRecord7C save;RrMenuChoice4 choice;
 s32 frame,counter,bank666,labels666[33],match,standings[6],first660,second660,labels660[55],uiMode,screenFlag;
 u16 first;s16 selection,mode,gameMode,flash;u32 tail[4];
};
struct Event {s32 kind,args[5];u32 hash;};
static struct World {struct State state;unsigned calls,draws;struct Event events[59];} actual,expected,initial,*w;
static unsigned scenario,mutation,overlay;
#define SaveProgress w->state.save
#define SelectionRows w->state.choice
#define ModeFrame w->state.frame
#define ScreenFrame w->state.counter
#define extra_course_select_result_kind w->state.bank666
#define extra_course_select_result_labels w->state.labels666
#define TotalRaceTime w->state.match
#define LapTimes w->state.standings
#define BestLapIndex w->state.first
#define time_attack_course_select_race_record_rank w->state.first660
#define time_attack_course_select_lap_record_rank w->state.second660
#define time_attack_course_select_result_labels w->state.labels660
#define Pad0Protocol w->state.uiMode
#define time_attack_course_select_result_transition w->state.screenFlag
#define MenuCursor w->state.selection
#define MenuItemCount w->state.mode
#define CourseIndex w->state.gameMode
#define time_attack_course_select_result_kind w->state.flash
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static s16 half(u32 n){u16 bits=(u16)n;s16 v;memcpy(&v,&bits,2);return v;}
static s32 event(s32 kind,s32 a,s32 b,s32 c,s32 d,s32 e) {
 unsigned i,n=w->calls;u32 hash=2166136261U;struct Event *record;
 assert(n<59);record=&w->events[w->calls++];
 for(i=0;i<sizeof(w->state);i++)hash=(hash^((u8*)&w->state)[i])*16777619U;
 record->kind=kind;record->args[0]=a;record->args[1]=b;record->args[2]=c;
 record->args[3]=d;record->args[4]=e;record->hash=hash;
 if(kind==1)w->draws++;
 if(mutation) {
  for(i=0;i<sizeof(w->state.save);i++)((u8*)&w->state.save)[i]=(u8)(scenario*19+n*23+i*29);
  w->state.frame=word(scenario*0x9E3779B9U+n);w->state.counter=word(scenario*0x1234567U+n);
  w->state.bank666=(s32)((scenario+n)%3);
  for(i=0;i<4;i++)w->state.choice.bytes[i]=(u8)((scenario+n+i)%4);
  for(i=0;i<33;i++)w->state.labels666[i]=word(scenario*0x87654321U+n*13+i*37);
  w->state.match=word(scenario*0xABCDEF01U+n*31);
  for(i=0;i<6;i++)w->state.standings[i]=word(scenario*0x3456789U+n*7+i*3);
  w->state.first=(u16)((scenario+n)%6);
  w->state.first660=word(scenario*0x1234567U+n*17);
  w->state.second660=word(scenario*0x7654321U+n*23);
  for(i=0;i<55;i++)w->state.labels660[i]=word(scenario*0x98765431U+n*43+i*7);
  w->state.uiMode=(s32)((scenario+n)%5)-1;w->state.screenFlag=word(scenario+n*33);
  w->state.selection=half(scenario+n*47);w->state.mode=half(scenario+n*53);
  w->state.gameMode=(scenario+n)%2?7:half(scenario+n*59);w->state.flash=half(scenario+n*61);
 }
 return (s32)((scenario+n)%7);
}
s32 InitMenuTextRows(RrMenuTextRow60 *rows) {
 assert(rows==SharedWorkBuffer && (uintptr_t)rows>UINT32_MAX);
 return event(0,0,0,0,0,0);
}
void SetMenuTextRow(s32 row,s32 position,s32 group,s32 mode,s32 label) {
 unsigned n=w->draws;
 if(overlay) {
  assert(n<33 && row==(s32)(n<22?n:n+11));assert(position==(s32)(n%11));
  assert(group==(n<22?0:2));
 } else {
  assert(n<55 && row==(s32)n);
  assert(position==(s32)(n%11) && group==((n>=11&&n<44)?2:0));
 }
 assert(mode==0);(void)event(1,row,position,group,mode,label);
}
s32 time_attack_course_select_func_80115FA8(s32 selector,s32 value) {
 assert(!overlay && w->calls<2 && selector==(s32)w->calls);
 return event(3,selector,value,0,0,0);
}
void SetMenuFrameOptions(s32 a,s32 b,s32 c) {
 assert(a==0 && b==1 && c==0 && w->draws==(overlay?33U:55U));
 (void)event(2,a,b,c,0,0);
}
'''+bodies+r'''
static void reference_666(void) {
 unsigned i,bank;
 (void)event(0,0,0,0,0,0);w->state.selection=0;w->state.mode=3;
 bank=(unsigned)w->state.bank666;
 if(bank==0) {
  unsigned first=w->state.choice.bytes[0],second=w->state.choice.bytes[1];
  w->state.save.prefix.flags5C[second]=1;
  w->state.save.prefix.progress.progress10[first][second]|=1U<<19;
 }
 for(i=0;i<33;i++) {
  s32 label=i<11?w->state.labels666[bank*11+i]:i<22?-1:w->state.labels666[i];
  (void)event(1,(s32)(i<22?i:i+11),(s32)(i%11),i<22?0:2,0,label);
 }
 (void)event(2,0,1,0,0,0);w->state.frame=0;w->state.counter=0;
}
static void reference_660(void) {
 unsigned i,best=0,offset;
 s32 mode;
 w->state.frame=0;
 if(w->state.match!=-1) {
  for(i=0;i<6;i++) {
   if(w->state.standings[i]<0)break;
   if(w->state.standings[i]<w->state.standings[best])best=i;
  }
  w->state.first=(u16)best;
  w->state.first660=event(3,0,w->state.standings[best],0,0,0);
  w->state.second660=event(3,1,w->state.match,0,0,0);
 } else {w->state.first660=6;w->state.second660=6;}
 (void)event(0,0,0,0,0,0);w->state.selection=0;w->state.mode=3;
 offset=w->state.gameMode==7?11:0;
 for(i=0;i<55;i++) {
  unsigned index=i<11?i+offset:i+11;
  s32 label=i<44?w->state.labels660[index]:-1;
  (void)event(1,(s32)i,(s32)(i%11),(i>=11&&i<44)?2:0,0,label);
 }
 w->state.flash=0;mode=w->state.uiMode;
 if(mode==1||mode==2)w->state.flash=(s16)mode;
 w->state.screenFlag=0;(void)event(2,0,1,0,0,0);
}
int main(void) {
 unsigned i;
 _Static_assert(sizeof(RrSaveProgressRecord60)==96,"save owner");
 _Static_assert(offsetof(RrSaveProgressRecord60,flags5C)==92,"flag offset");
 for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++) {
  memset(&initial,0,sizeof(initial));
  for(i=0;i<sizeof(initial.state);i++)((u8*)&initial.state)[i]=(u8)(scenario*17+i*29);
  initial.state.bank666=(s32)(scenario%3);
  for(i=0;i<4;i++)initial.state.choice.bytes[i]=(u8)((scenario>>(i*2))%4);
  initial.state.match=(scenario%3==0)?-1:(scenario&1)?INT32_MIN:INT32_MAX;
  initial.state.first=(u16)(scenario%6);
  for(i=0;i<6;i++) {
   initial.state.standings[i]=(s32)((scenario+i*5)%17);
   if(i==(scenario>>4)%7)initial.state.standings[i]=-1;
  }
  initial.state.gameMode=(scenario&1)?7:half(scenario);
  initial.state.uiMode=(s32)(scenario%5)-1;
  for(overlay=0;overlay<2;overlay++) {
   actual=initial;expected=initial;
   w=&actual;if(overlay)extra_course_select_init_results();else time_attack_course_select_func_801154D8();
   w=&expected;if(overlay)reference_666();else reference_660();
   assert(memcmp(&actual,&expected,sizeof(actual))==0);
  }
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            c = Path(temp) / 'result_setup.c'
            exe = Path(temp) / 'result_setup'
            c.write_text(source)
            subprocess.run(['cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror','-Wno-error=deprecated-non-prototype',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            '-I', str(root / 'include'), str(c), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
