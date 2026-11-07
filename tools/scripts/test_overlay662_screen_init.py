"""Screen initializer retains full arena pointers and fresh callback effects."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest

class Overlay662ScreenInitTest(unittest.TestCase):
    def test_real_reset_and_mutating_setup(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/662/func_80118ED8.c').read_text()
        reset=re.sub(r'^extern[^\n]*;\n','',(root/'tools/scripts/fixtures/legacy/func_8003A198.c').read_text(),flags=re.M)
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/overlay662_screen_init.h"
static u8 arenas[3][0x2D001];
typedef struct {s32 a[10];void *published;RrMenuArenas arenas;u16 mode;s16 scene,counter;} Event;
static struct World {
 u32 guard[4];s32 frame,word;s16 local,setting,style;
 s8 flag0,flag1;u16 mode;s16 scene,counter;
 RrMenuArenas arenas;u8 *base;void *published,*resource;
 unsigned calls;Event events[7];u32 tail[4];
} world,expected;
static unsigned scenario;
#define ModeFrame world.frame
#define ScreenFrame world.word
#define ovl662_D_801272D4 world.local
#define MenuCursor world.setting
#define MenuItemCount world.style
#define LoadState world.flag0
#define MenuScriptStep world.flag1
#define ModeGroup world.mode
#define ModeId world.scene
#define D_800F4D8A world.counter
#define MenuArenas world.arenas
#define CdDirectoryBuffer world.base
#define LoadCursor world.published
#define MenuArenaBase world.resource
'''+reset+body+r'''
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static void event(struct World *w,const s32 a[10]){unsigned n=w->calls,j;Event *e;
 assert(n<7);e=&w->events[w->calls++];for(j=0;j<10;j++)e->a[j]=a[j];
 e->published=w->published;e->arenas=w->arenas;e->mode=w->mode;e->scene=w->scene;e->counter=w->counter;
 w->frame=word(scenario*0x1234567U+n);w->word=word(scenario*0x9E3779B9U+n);
 w->local=(s16)(scenario+n);w->setting=(s16)(scenario+n+7);w->style=(s16)(scenario+n+13);
 w->flag0=(s8)(scenario+n+1);w->flag1=(s8)(scenario+n+2);
 w->mode=(u16)(scenario+n+3);w->scene=(s16)(scenario+n+4);w->counter=(s16)(scenario+n+5);
 w->base=arenas[(scenario+n+1)%3];w->arenas.first=arenas[(scenario+n+2)%3]+n;
 w->arenas.second=arenas[(scenario+n)%3]+n+9;
 w->published=arenas[(scenario+n+2)%3]+n+17;w->resource=arenas[(scenario+n+1)%3]+n+29;
}
void InitMenuArenas(void){s32 a[10]={0};
 assert(world.calls==0);assert(world.frame==0&&world.word==0&&world.local==0&&world.setting==0&&world.style==3);
 assert(world.flag0==0&&world.flag1==0);assert(world.arenas.second==world.base+0x2D000);
 event(&world,a);
}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){s32 args[10]={a,b,c,d,e,f,g,h,i,j};
 assert(world.calls>=1&&world.calls<=3);if(world.calls==1)assert(world.published==world.arenas.second);event(&world,args);
}
void InitPreviewImageRows(void){s32 a[10]={37110};assert(world.calls==4);event(&world,a);}
void InitBadgeRow(s32 a,s32 b,s16 c,s32 d,u16 e,u16 f,u16 g){s32 args[10]={a,b,c,d,e,f,g};assert(world.calls==5);event(&world,args);}
void SetupRenderer(s32 shift){s32 a[10]={shift};assert(world.calls==6);event(&world,a);}
static void reference(void){s32 a[10]={0};unsigned n;
 expected.frame=expected.word=0;expected.local=expected.setting=0;expected.style=3;
 expected.flag0=expected.flag1=0;expected.arenas.second=expected.base+0x2D000;
 event(&expected,a);expected.published=expected.arenas.second;
 for(n=0;n<3;n++){s32 row[10]={(s32)n,24,88+(s32)n*40,68,32,0,0,1,0,n==0?83:n==1?42:85};event(&expected,row);}
 {s32 args[10]={37110};event(&expected,args);}
 {s32 args[10]={1,24,232,84,34,0,0x501};event(&expected,args);}
 {s32 args[10]={5};event(&expected,args);}
 expected.mode=1;expected.scene=68;expected.arenas.first=expected.resource;expected.published=expected.resource;expected.counter=0;
}
int main(void){unsigned j;
 for(scenario=0;scenario<65536;scenario++){
  memset(&world,0,sizeof(world));
  for(j=0;j<4;j++){world.guard[j]=scenario*17+j;world.tail[j]=scenario*41+j;}
  world.frame=word(scenario*0x1234567U);world.word=word(scenario*0x9E3779B9U);
  world.local=world.setting=world.style=(s16)scenario;world.flag0=world.flag1=(s8)scenario;
  world.mode=(u16)scenario;world.scene=world.counter=(s16)scenario;
  world.base=arenas[scenario%3];world.arenas.first=arenas[(scenario+1)%3];world.arenas.second=arenas[(scenario+2)%3];
  world.published=world.arenas.first;world.resource=world.arenas.second;
  memcpy(&expected,&world,sizeof(world));reference();ovl662_func_80118ED8();
  assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.published>UINT32_MAX);
 }
 return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
