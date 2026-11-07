"""Garage arena setup: real reset body, fresh loader pointers and full host addresses."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayArenaTest(unittest.TestCase):
    def test_live_arena_publication(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_80116148.c').read_text()
        reset = (root / 'tools/scripts/fixtures/legacy/func_8003A198.c').read_text()
        reset = re.sub(r'^extern[^\n]*;\n', '', reset, flags=re.M)
        reset = reset.replace('ResetScreenCounters', 'rr_real_reset')
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
#include "rr/menu_arenas.h"
typedef struct {s32 kind,a,b,c;void *observed;} Event;
static u8 arena[3][0x2d080],original_arena[3][0x2d080];
static struct World {
 u32 before[8];RrMenuArenas slots;u8 *base;void *active;
 u8 *base_after_reset,*scratch_after_load,*scratch_after_ui,*active_after_ui;
 s8 flag0,flag1;s32 counter;u32 calls;Event events[4];u32 after[8];
} world,expected;
#define MenuArenas world.slots
#define CdDirectoryBuffer world.base
#define LoadCursor world.active
#define LoadState world.flag0
#define MenuScriptStep world.flag1
#define ScreenFrame world.counter
''' + reset + r'''
static void event(struct World *w,s32 kind,s32 a,s32 b,s32 c,void *observed){
 Event *e;assert(w->calls<4);e=&w->events[w->calls++];e->kind=kind;e->a=a;e->b=b;e->c=c;e->observed=observed;
}
static void reset_effect(struct World *w){event(w,1,0,0,0,w->active);w->base=w->base_after_reset;}
static void load_effect(struct World *w){
 event(w,2,0,0,0,w->slots.second);w->slots.second=w->scratch_after_load;
 w->active=w->base_after_reset;w->base=w->scratch_after_load;
}
static void ui_effect(struct World *w,s32 a,s32 b,s32 c){
 event(w,3,a,b,c,w->active);w->slots.second=w->scratch_after_ui;w->active=w->active_after_ui;
}
static void finish_effect(struct World *w){event(w,4,0,0,0,w->active);w->counter=77;}
void ResetScreenCounters(void){rr_real_reset();assert(world.flag0==0 && world.flag1==0 && world.counter==0);reset_effect(&world);}
void InitMenuArenas(void){load_effect(&world);}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){ui_effect(&world,a,b,c);}
void garage_init_team_select(void){finish_effect(&world);}
''' + body + r'''
static u32 seed=0x16148;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
int main(void){unsigned k,i,j;
 for(i=0;i<3;i++)for(j=0;j<sizeof(arena[0]);j++)arena[i][j]=(u8)next();
 memcpy(original_arena,arena,sizeof(arena));
 assert(sizeof(void*)==8 && sizeof(RrMenuArenas)==16);
 for(k=0;k<65536;k++){
  unsigned index=k%3,offset=k&127;
  memset(&world,0,sizeof(world));
  for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
  world.slots.first=&arena[0][next()%sizeof(arena[0])];
  world.slots.second=&arena[1][next()%sizeof(arena[0])];
  world.base=&arena[(index+1)%3][offset];world.active=&arena[2][next()%sizeof(arena[0])];
  world.base_after_reset=&arena[index][offset];
  world.scratch_after_load=&arena[(index+2)%3][next()%sizeof(arena[0])];
  world.scratch_after_ui=&arena[index][next()%sizeof(arena[0])];
  world.active_after_ui=&arena[(index+1)%3][next()%sizeof(arena[0])];
  world.flag0=next();world.flag1=next();world.counter=next();
  memcpy(&expected,&world,sizeof(world));
  expected.flag0=0;expected.flag1=0;expected.counter=0;reset_effect(&expected);
  expected.slots.second=&arena[index][offset+0x2d000];
  load_effect(&expected);expected.active=expected.slots.second;
  ui_effect(&expected,0,1,1);finish_effect(&expected);
  garage_init_collection();
  assert(memcmp(&world,&expected,sizeof(world))==0);
  assert((uintptr_t)world.base>UINT32_MAX && (uintptr_t)world.slots.second>UINT32_MAX);
 }
 assert(memcmp(arena,original_arena,sizeof(arena))==0);
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
