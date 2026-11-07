"""Text-row initialization and overlay result screens with native pointers.

The real resident initializer is exercised with a memory-clearing BIOS boundary.
Overlay renderer, initializer and screen-transition calls are simulated, typed
boundaries which mutate live tables and result state between calls.
"""
from overlay_units import read_c_source
from overlay_names import name as OVN
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest


class OverlayTextRowsTest(unittest.TestCase):
    def run_source(self, source):
        root = Path(__file__).resolve().parents[2]
        source = r'''
#include <stddef.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
''' + source
        with TemporaryDirectory() as temp:
            c = Path(temp) / 'rows.c'
            exe = Path(temp) / 'rows'
            c.write_text(source)
            subprocess.run(['cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror','-Wno-error=deprecated-non-prototype',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            '-I', str(root / 'include'), str(c), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)

    def test_resident_clear_keeps_full_pointer_and_captured_count(self):
        root = Path(__file__).resolve().parents[2]
        body = read_c_source(root / 'tools/scripts/fixtures/views/func_800402F4.c')
        self.run_source(r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "rr/menu_text_rows.h"
static struct {u32 guard[4];RrMenuTextRow60 rows[55];u32 tail[4];} owners[2],before[2];
RrMenuTextRow60 *MenuTextRows;
s32 MenuTextRowCount;
static unsigned bank,count,mutation,calls;
void PsyqBzero(void *pointer,u32 bytes) {
 assert(calls++==0 && pointer==owners[bank].rows && bytes==count*60);
 assert(MenuTextRows==pointer && (uintptr_t)pointer>UINT32_MAX);
 memset(pointer,0,bytes);
 if(mutation){MenuTextRowCount=(s32)(55-count);MenuTextRows=owners[1-bank].rows;}
}
'''+body+r'''
int main(void) {
 unsigned i,b;
 for(bank=0;bank<2;bank++)for(count=0;count<=55;count++)for(mutation=0;mutation<2;mutation++) {
  for(i=0;i<sizeof(owners);i++)((u8*)owners)[i]=(u8)(i*17+count*29+bank);
  memcpy(before,owners,sizeof(owners));calls=0;MenuTextRowCount=(s32)count;
  MenuTextRows=owners[1-bank].rows;
  assert(InitMenuTextRows(owners[bank].rows)==(s32)(count*60));assert(calls==1);
  assert(MenuTextRows==owners[mutation?1-bank:bank].rows);
  memset(before[bank].rows,0,count*60);
  assert(memcmp(owners,before,sizeof(owners))==0);
  b=mutation?55-count:count;assert(MenuTextRowCount==(s32)b);
 }
 return 0;
}
''')

    def test_result_screens_fresh_labels_scores_rankings_and_service_order(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join(re.sub(r'^extern[^\n]*;\n', '',
                                  read_c_source(root / f'src/overlays/PAL/{OVN(ov)}/func_{addr}.c'),
                                  flags=re.M)
                           for ov, addr in [(661, '80117C78'), (667, '80118634')])
        self.run_source(r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/menu_text_rows.h"
#include "rr/overlay_menu.h"
RrMenuTextRow60 SharedWorkBuffer[55];
struct State {
 u32 guard[4];s32 frame,match,table661[22],table667[22];
 s16 selection,mode,scores[2],winners[2],wins[2],participants,ranks[8],counts[8],order[8],reset;
 u32 tail[4];
};
struct Event {s32 kind,args[5];u32 hash;};
static struct World {struct State state;unsigned calls;struct Event events[35];} actual,expected,initial,*w;
static unsigned scenario,mutation;
#define ModeFrame w->state.frame
#define TotalRaceTime w->state.match
#define MenuCursor w->state.selection
#define MenuItemCount w->state.mode
#define versus_select_results_text_ids w->state.table661
#define serial_link_result_labels w->state.table667
#define versus_select_finish_ranks w->state.scores
#define versus_select_result_order w->state.winners
#define versus_select_win_counts w->state.wins
#define NumCars w->state.participants
#define LinkResultPlayerByRank w->state.ranks
#define PlayerWinCounts w->state.counts
#define FinalCarRanks w->state.order
#define LinkState w->state.reset
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static s16 half(u32 n){u16 bits=(u16)n;s16 v;memcpy(&v,&bits,2);return v;}
static void event(s32 kind,s32 a,s32 b,s32 c,s32 d,s32 e) {
 unsigned i,n=w->calls;u32 hash=2166136261U;struct Event *record;
 assert(n<35);record=&w->events[w->calls++];
 for(i=0;i<sizeof(w->state);i++)hash=(hash^((u8*)&w->state)[i])*16777619U;
 record->kind=kind;record->args[0]=a;record->args[1]=b;record->args[2]=c;
 record->args[3]=d;record->args[4]=e;record->hash=hash;
 if(mutation) {
  w->state.frame=word(scenario*0x9E3779B9U+n);
  w->state.match=(scenario&1)?word(0x80000000U|scenario):(s32)scenario;
  w->state.selection=half(scenario+n*13);w->state.mode=half(scenario+n*17);
  w->state.participants=(s16)((scenario+n)%9);w->state.reset=half(scenario+n*23);
  for(i=0;i<22;i++) {
   w->state.table661[i]=word(scenario*0x1234567U+i*73+n*29);
   w->state.table667[i]=word(scenario*0x87654321U+i*13+n*37);
  }
  for(i=0;i<2;i++) {
   w->state.scores[i]=half(scenario+i*193+n*181);
   w->state.winners[i]=half(scenario+i*31+n*47);
   w->state.wins[i]=half(scenario+i*29+n*13);
  }
  for(i=0;i<8;i++) {
   w->state.ranks[i]=(s16)((scenario+n+i)%8);
   w->state.counts[i]=half(scenario+i*137+n*149);
   w->state.order[i]=(s16)((scenario+n+i*3)%8);
  }
 }
}
s32 InitMenuTextRows(RrMenuTextRow60 *rows) {
 assert(rows==SharedWorkBuffer && (uintptr_t)rows>UINT32_MAX);
 assert(w->calls==0);event(0,0,0,0,0,0);return -1;
}
void SetMenuTextRow(s32 row,s32 position,s32 group,s32 mode,s32 label) {
 unsigned n=w->calls-1;
 assert(n<33);
 assert(row==(s32)(n<22?n:n+11));assert(position==(s32)(n%11));
 assert(group==(n<22?0:2) && mode==0);
 event(1,row,position,group,mode,label);
}
void SetMenuFrameOptions(s32 a,s32 b,s32 c) {
 assert(w->calls==34 && a==0 && b==1 && c==0);event(2,a,b,c,0,0);
}
'''+bodies+r'''
static void reference(unsigned overlay) {
 unsigned i,index;
 if(overlay)w->state.frame=0;
 event(0,0,0,0,0,0);w->state.selection=0;w->state.mode=3;
 for(i=0;i<33;i++) {
  s32 label=i<11?(overlay?w->state.table667[i]:w->state.table661[i]):
            i<22?-1:(overlay?w->state.table667[i-11]:w->state.table661[i-11]);
  event(1,(s32)(i<22?i:i+11),(s32)(i%11),i<22?0:2,0,label);
 }
 if(w->state.match>=0) {
  if(overlay) {
   for(i=0;i<(unsigned)w->state.participants;i++)w->state.ranks[w->state.order[i]]=(s16)i;
   index=(unsigned)w->state.ranks[0];
   if(w->state.counts[index]<99)w->state.counts[index]=(s16)((s32)w->state.counts[index]+1);
  } else {
   unsigned left=w->state.scores[0]>w->state.scores[1];
   w->state.winners[0]=(s16)left;w->state.winners[1]=(s16)(1-left);
   index=left;
   if(w->state.wins[index]<99)w->state.wins[index]=(s16)((s32)w->state.wins[index]+1);
  }
 }
 if(overlay)w->state.reset=0;else w->state.frame=0;
 event(2,0,1,0,0,0);
}
int main(void) {
 unsigned i,overlay;
 for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++) {
  memset(&initial,0,sizeof(initial));
  for(i=0;i<sizeof(initial.state);i++)((u8*)&initial.state)[i]=(u8)(scenario*17+i*29);
  initial.state.match=(scenario&1)?INT32_MIN:INT32_MAX;
  initial.state.participants=(s16)((scenario>>1)%9);
  initial.state.scores[0]=half(scenario);initial.state.scores[1]=half(scenario^0x8000U);
  initial.state.wins[0]=half(scenario);initial.state.wins[1]=half(scenario+32768);
  for(i=0;i<8;i++) {
   initial.state.ranks[i]=(s16)((scenario+i)%8);
   initial.state.counts[i]=half(scenario+i*997);
   initial.state.order[i]=(s16)((scenario+i*3)%8);
  }
  for(overlay=0;overlay<2;overlay++) {
   actual=initial;expected=initial;
   w=&actual;if(overlay)serial_link_init_results();else versus_select_init_results();
   w=&expected;reference(overlay);
   assert(actual.calls==35 && memcmp(&actual,&expected,sizeof(actual))==0);
  }
 }
 return 0;
}
''')
