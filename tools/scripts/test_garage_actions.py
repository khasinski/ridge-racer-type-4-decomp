"""Garage bit-pair dispatch with native callback identities and live table changes."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayGarageActionsTest(unittest.TestCase):
    def test_priorities_and_live_callback_table(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_8011DCDC.c').read_text()
        sound = (root / 'tools/scripts/fixtures/views/func_8004F24C.c').read_text()
        sound = re.sub(r'^extern[^\n]*;\n', '', sound, flags=re.M)
        bridge = (root / 'tools/scripts/fixtures/views/func_8004F218.c').read_text()
        bridge = bridge.replace('#include "rr/audio_pan.h"', '')
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/garage_actions.h"
typedef struct {s32 kind,id,code,index,args[5];RrGarageActionPrefix *record;} Event;
static struct World {
 u32 before[8];RrGarageSelectionPrefix state;
 RrGarageActionPrefix records[16],*table[16];
 u32 sound_index,sound_record,sound_kind,next_index,next_record,next_kind;
 u32 calls,actions,sounds;Event log[3];u32 after[8];
} world,expected;
#define garage_logo_state world.state
#define garage_logo_actions world.table
static void action_a(s32 code);
static void action_b(s32 code);
static Event *event(struct World *w,s32 kind){Event *e;assert(w->calls<3);e=&w->log[w->calls++];e->kind=kind;e->index=w->state.current60;return e;}
static void sound_effect(struct World *w,s32 a,s32 b,s32 c,s32 d,s32 f){
 Event *e=event(w,1);e->args[0]=a;e->args[1]=b;e->args[2]=c;e->args[3]=d;e->args[4]=f;w->sounds++;
 w->state.current60=w->sound_index;w->table[w->sound_index]=&world.records[w->sound_record];
 w->records[w->sound_record].callback=w->sound_kind?action_b:action_a;
}
static void action_effect(struct World *w,s32 id,s32 code){
 Event *e=event(w,2);e->id=id;e->code=code;e->record=w->table[w->state.current60];
 w->state.phaseCC^=0xa5a55a5aU;
 if(++w->actions==1){
  w->state.current60=w->next_index;w->table[w->next_index]=&world.records[w->next_record];
  w->records[w->next_record].callback=w->next_kind?action_b:action_a;
 }else w->state.current60=-1;
}
static void action_a(s32 code){action_effect(&world,1,code);}
static void action_b(s32 code){action_effect(&world,2,code);}
void TriggerSfx(s32 a,s32 b,s32 c,s32 d,s32 f){sound_effect(&world,a,b,c,d,f);}
''' + bridge + sound + body + r'''
static u32 seed=0x1dcdc;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 word(u32 raw){s32 value;memcpy(&value,&raw,4);return value;}
static void model_action(s32 code){
 ptrdiff_t index=expected.table[expected.state.current60]-world.records;
 RrGarageStep callback;assert(index>=0 && index<16);callback=expected.records[index].callback;
 assert(callback==action_a || callback==action_b);action_effect(&expected,callback==action_a?1:2,code);
}
int main(void){unsigned k,i;
 assert(sizeof(RrGarageActionPrefix)==sizeof(RrGarageStep));
 for(k=0;k<131072;k++){
  u32 flags=k<65536?k:(next()&0xffff0000U)|(k-65536);
  memset(&world,0,sizeof(world));
  for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
  for(i=0;i<16;i++){
   world.records[i].callback=(i&1)?action_a:action_b;
   world.table[i]=&world.records[(i+k)&15];
  }
  world.state.current60=word(next());world.state.phaseCC=next();
  world.sound_index=next()&15;world.sound_record=next()&15;world.sound_kind=k&1;
  world.next_index=next()&15;world.next_record=next()&15;world.next_kind=!world.sound_kind;
  memcpy(&expected,&world,sizeof(world));
  if(flags)sound_effect(&expected,0,127,64,0x1e00,-1);
  if(flags%4!=0)model_action((flags&1)?1:2);
  if((flags/4)%4!=0)model_action((flags&4)?4:8);
  garage_dispatch_direction_input(flags);
  assert(memcmp(&world,&expected,sizeof(world))==0);
  assert((uintptr_t)&world.records[0]>UINT32_MAX);
  assert((uintptr_t)action_a>UINT32_MAX && (uintptr_t)action_b>UINT32_MAX);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            path = Path(temp)
            (path / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(path / 'test.c'), '-o', str(path / 'test')], check=True)
            subprocess.run([str(path / 'test')], check=True)


if __name__ == '__main__':
    unittest.main()
