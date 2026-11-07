"""Standings text setup with actual row reset and mutating display services."""
from overlay_units import read_c_source
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class StandingsTextSetupTest(unittest.TestCase):
    def test_native_setup_and_live_phase_changes(self):
        root = Path(__file__).resolve().parents[2]
        body = read_c_source(root / 'src/overlays/PAL/race_results/func_801150CC.c')
        body = re.sub(r'^extern[^\n]*\n', '', body, flags=re.M)
        reset = read_c_source(root / 'tools/scripts/fixtures/views/func_800402F4.c')
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
#include "rr/menu_choice.h"
struct State {u32 guard[4];RrMenuSettings2C settings;RrMenuTextRow60 rows[48];
 RrMenuTextRow60 *published;s32 count,labels[34],local,frame;s16 flag,mode;u32 tail[4];};
static struct World {struct State s;unsigned calls;struct {s32 service,a[5];u32 hash;} event[32];} actual,expected,initial,*w;
static unsigned scenario,mutation;
#define SelectionState w->s.settings
#define SharedWorkBuffer w->s.rows
#define MenuTextRows w->s.published
#define MenuTextRowCount w->s.count
#define race_results_result_labels w->s.labels
#define race_results_preview_timer w->s.local
#define ModeFrame w->s.frame
#define MenuCursor w->s.flag
#define MenuItemCount w->s.mode
static s32 word(u32 value){s32 result;memcpy(&result,&value,4);return result;}
static void effect(unsigned n){
 unsigned i;u32 value=scenario*0x9e3779b9U+n*47U;
 static const s16 modes[]={4,-32768,3,32767,5,-1,0,4};
 w->s.settings.mode1A=modes[(scenario+n)%8];w->s.settings.phase17=(s8)(scenario+n*53);
 w->s.count=(scenario+n)%49;w->s.local=word(value);w->s.frame=word(~value);
 w->s.flag=(s16)value;w->s.mode=(s16)~value;
 for(i=0;i<34;i++)w->s.labels[i]=word(value+i*0x1234567U);
}
static void event(s32 service,s32 a,s32 b,s32 c,s32 d,s32 e){
 unsigned i,n=w->calls;u32 hash=2166136261U;assert(n<32);
 for(i=0;i<sizeof(w->s);i++)hash=(hash^((u8*)&w->s)[i])*16777619U;
 w->event[n].service=service;w->event[n].a[0]=a;w->event[n].a[1]=b;
 w->event[n].a[2]=c;w->event[n].a[3]=d;w->event[n].a[4]=e;w->event[n].hash=hash;
 w->calls++;if(mutation)effect(n);
}
void race_results_rank_standings(void){event(1,0,0,0,0,0);}
void PsyqBzero(void *destination,u32 bytes){
 assert(destination==w->s.rows && bytes<=sizeof(w->s.rows));
 event(2,(s32)bytes,0,0,0,0);memset(destination,0,bytes);
}
void SetMenuTextRow(s32 row,s32 position,s32 group,s32 mode,s32 label){
 unsigned i;u32 values[4];assert(row>=0 && row<48);
 event(3,row,position,group,mode,label);
 values[0]=(u32)position;values[1]=(u32)group;values[2]=(u32)mode;values[3]=(u32)label;
 for(i=0;i<16;i++)w->s.rows[row].bytes[i]=(u8)(values[i/4]>>(8*(i%4)));
}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){event(4,a,b,c,0,0);}
''' + reset + body + r'''
static void reference(void){
 unsigned i;u32 bytes;s32 offset;
 race_results_rank_standings();bytes=(u32)w->s.count*60U;
 /* Normalize the expected published pointer to the actual allocation. Its
  * full native value is part of every following service-state snapshot. */
 w->s.published=actual.s.rows;PsyqBzero(w->s.rows,bytes);
 w->s.flag=0;w->s.mode=2;offset=w->s.settings.mode1A==4?0:11;
 for(i=0;i<11;i++){
  s32 index=(s32)i+offset;
  if(w->s.settings.phase17>=4 && i==4 && offset==0)index=22;
  SetMenuTextRow(i,i,0,0,w->s.labels[index]);
 }
 for(i=0;i<11;i++)SetMenuTextRow(i+33,i,2,0,w->s.labels[i+23]);
 SetMenuFrameOptions(0,1,0);w->s.local=-2;w->s.frame=0;
}
int main(void){
 unsigned i;
 for(scenario=0;scenario<32768;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;effect(9);
  for(i=0;i<sizeof(w->s.rows);i++)((u8*)w->s.rows)[i]=(u8)(scenario+i*43);
  for(i=0;i<4;i++){w->s.guard[i]=0x12345678U+i;w->s.tail[i]=0x87654321U-i;}
  initial.s.published=initial.s.rows;
  actual=initial;expected=initial;w=&actual;assert((uintptr_t)w->s.rows>UINT32_MAX);
  race_results_init_results();w=&expected;reference();
  assert(memcmp(&actual,&expected,sizeof(actual))==0);
  assert(actual.s.published==actual.s.rows && (uintptr_t)actual.s.published>UINT32_MAX);
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
