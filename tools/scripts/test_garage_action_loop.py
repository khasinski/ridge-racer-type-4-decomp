"""Complete action records, callback mutations and pointer-preserving table setup."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest


class GarageActionLoopTest(unittest.TestCase):
    def test_setup_dispatch_and_rendering(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_8011E36C.c').read_text()
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4||sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4||offsetof(t,f)==n,"PAL offset")
#include "rr/overlay662_action_loop.h"
typedef struct {
 u32 guard[4],rng;RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;
 RrGarageActionRecord16 records[53];RrGarageActionPrefix *actions[53];
 RrGarageButton buttons[46];RrGarageRectangle *table[46];u32 tail[4];
} State;
typedef struct {s32 kind,argument;u32 hash;} Event;
typedef struct {State s;Event e[128];unsigned calls;} World;
static World actual,reference,initial,*w;
static unsigned scenario,mutation,seen[8],brushSeen[5];
#define garage_logo_state w->s.garage
#define garage_logo_work w->s.work
#define garage_action_records w->s.records
#define garage_logo_actions w->s.actions
#define garage_button_records w->s.buttons
#define garage_logo_buttons w->s.table
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static u32 next(void){u32 n=w->s.rng;n^=n<<13;n^=n>>17;n^=n<<5;return w->s.rng=n;}
static unsigned actionIndex(RrGarageActionPrefix *p){unsigned i;assert((uintptr_t)p>UINT32_MAX);for(i=0;i<53;i++)if(p==&actual.s.records[i].prefix)return i;assert(0);return 0;}
static unsigned buttonIndex(RrGarageRectangle *p){unsigned i;assert((uintptr_t)p>UINT32_MAX);for(i=0;i<46;i++)if(p==&actual.s.buttons[i].rectangle)return i;assert(0);return 0;}
static void step0(s32);static void step1(s32);static void render0(s32);static void render1(s32);
static void change(void){unsigned i,k;if(!mutation)return;
 w->s.work.words[0]=(w->s.work.words[0]&~8U)|((scenario+w->calls)%7==0?8U:0U);
 w->s.work.words[4]=next()%53;w->s.garage.config1E=next()%5;w->s.garage.flagsD1=(u8)next();
 k=next()%53;w->s.actions[k]=&actual.s.records[next()%53].prefix;
 k=next()%53;w->s.records[k].render=(next()&1)?render0:render1;w->s.records[k].argument=word(next());
 for(i=0;i<46;i++)w->s.table[i]=&actual.s.buttons[(scenario+i+w->calls)%46].rectangle;
}
static void record(s32 kind,s32 argument){Event *e;unsigned i;u32 h=2166136261U;assert(w->calls<128);
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 e=&w->e[w->calls++];memset(e,0,sizeof(*e));e->kind=kind;e->argument=argument;e->hash=h;change();
}
static void step0(s32 a){assert(a==0);record(1,a);}static void step1(s32 a){assert(a==0);record(2,a);}
static void render0(s32 a){record(3,a);}static void render1(s32 a){record(4,a);}
void garage_draw_color_panel(void){record(5,0);}
''' + body + r'''
static void expected(s32 mode){unsigned i,selected;static const unsigned brushes[5]={9,11,12,14,4};
 if(mode){
  for(i=0;i<53;i++)w->s.actions[w->s.records[i].key]=&actual.s.records[i].prefix;
  for(i=0;i<46;i++)w->s.table[w->s.buttons[i].key0C]=&actual.s.buttons[i].rectangle;
  seen[0]++;return;
 }
 if(w->s.work.words[0]&2)seen[1]++;
 else if((w->s.work.words[0]&4)&&word(w->s.work.words[10])<4096)seen[2]++;
 else if(w->s.work.words[0]&8){unsigned index=actionIndex(w->s.actions[w->s.work.words[4]]);w->s.records[index].prefix.callback(0);seen[3]++;}
 else{
  for(i=0;i<53;i++){unsigned index=actionIndex(w->s.actions[i]);w->s.records[index].prefix.callback(0);if(w->s.work.words[0]&8)break;}
  seen[i==53?4:5]++;
 }
 assert(w->s.garage.config1E<5);brushSeen[w->s.garage.config1E]++;selected=brushes[w->s.garage.config1E];
 {static const unsigned reset[5]={4,14,12,11,9};for(i=0;i<5;i++)w->s.buttons[buttonIndex(w->s.table[reset[i]])].timer0E=0;}
 w->s.buttons[buttonIndex(w->s.table[selected])].timer0E=-1;
 if(!w->s.garage.flagsD1){
  for(i=0;i<53;i++){unsigned index=actionIndex(w->s.actions[i]);RrGarageStep callback=w->s.records[index].render;s32 arg=w->s.records[index].argument;callback(arg);}
  garage_draw_color_panel();seen[6]++;
 }else seen[7]++;
}
int main(void){unsigned modeIndex,i;uint64_t cases=0;static const s32 modes[4]={0,-1,1,INT32_MIN};static const s32 progress[7]={-1,0,4095,4096,4097,INT32_MIN,INT32_MAX};
 assert(sizeof(RrGarageActionRecord16)==24&&offsetof(RrGarageActionRecord16,prefix)==0&&sizeof(RrGarageActionPrefix)==sizeof(RrGarageStep));
 for(scenario=0;scenario<8192;scenario++)for(modeIndex=0;modeIndex<4;modeIndex++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0x1E36C+scenario*73;
  for(i=0;i<4;i++){w->s.guard[i]=next();w->s.tail[i]=next();}
  for(i=0;i<16;i++)w->s.work.words[i]=next();w->s.work.words[0]=0xABC00000U|(scenario&14U);w->s.work.words[4]=scenario%53;w->s.work.words[10]=(u32)progress[(scenario>>4)%7];
  w->s.garage.config1E=modeIndex?(u8)next():scenario%5;w->s.garage.flagsD1=(scenario&8)?(u8)(scenario|1):0;
  if((w->s.work.words[0]&2)||((w->s.work.words[0]&4)&&word(w->s.work.words[10])<4096)||!(w->s.work.words[0]&8))w->s.work.words[4]=next()|0x80000000U;
  for(i=0;i<53;i++){
   w->s.records[i].prefix.callback=((i+scenario)&1)?step0:step1;w->s.records[i].render=(i&1)?render0:render1;w->s.records[i].argument=word(next());
   w->s.records[i].key=(scenario&1)?next()%53:(i*17+scenario)%53;w->s.actions[i]=&actual.s.records[(i*23+scenario)%53].prefix;
  }
  for(i=0;i<46;i++){
   w->s.buttons[i].key0C=(scenario&1)?next()%46:(i*17+scenario)%46;w->s.buttons[i].timer0E=(s16)next();
   w->s.buttons[i].rectangle[0]=(u16)next();w->s.buttons[i].sprite08=(s16)next();
   w->s.table[i]=&actual.s.buttons[(scenario&2)?next()%46:(i*17+scenario)%46].rectangle;
  }
  actual=reference=initial;w=&actual;garage_update_actions(modes[modeIndex]);w=&reference;expected(modes[modeIndex]);
  assert(!memcmp(&actual,&reference,sizeof(actual)));cases++;
 }
 assert(cases==65536);for(i=0;i<8;i++)assert(seen[i]);for(i=0;i<5;i++)assert(brushSeen[i]);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p = Path(tmp)
            (p / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(p / 'test.c'), '-o', str(p / 'test')], check=True)
            subprocess.run([str(p / 'test')], check=True)
