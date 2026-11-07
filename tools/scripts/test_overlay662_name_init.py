"""Name screen uses actual tail-copy semantics and two fresh badge rows."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class Overlay662NameInitTest(unittest.TestCase):
    def test_real_string_copy_and_mutating_badge_roots(self):
        root=Path(__file__).resolve().parents[2]
        copy=(root/'tools/scripts/fixtures/views/func_8002B75C.c').read_text()
        body=(root/'src/overlays/PAL/662/func_80118930.c').read_text()
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
#include "rr/overlay662_name_init.h"
typedef struct {s32 args[7],frame,counter,local;s8 a,b,c;u16 mode;s16 scene,timer,first,second;u8 name[12];RrBadgeRow1C *root;} Event;
static struct World {
 u32 guard[4];s32 frame,counter,local;s8 a,b,c;u16 mode;s16 scene,timer;
 RrSaveProgressRecord7C save;u32 before_name;u8 name[12];u32 after_name;
 RrBadgeRow1C badges[3][2],*root;unsigned calls;Event events[4];u32 tail[4];
} world,expected;
static unsigned scenario;
#define ModeFrame world.frame
#define ScreenFrame world.counter
#define ovl662_D_801273B4 world.local
#define ovl662_D_801273A2 world.a
#define ovl662_D_801273A3 world.b
#define ovl662_D_801273A0 world.c
#define ModeGroup world.mode
#define ModeId world.scene
#define D_800F4D8A world.timer
#define SaveProgress world.save
#define ovl662_D_801273A8 world.name
#define BadgeRows world.root
'''+copy+body+r'''
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static void event(struct World *w,const s32 args[7]){unsigned n=w->calls,j,bank;Event *e;assert(n<4);e=&w->events[w->calls++];
 for(j=0;j<7;j++)e->args[j]=args[j];e->frame=w->frame;e->counter=w->counter;e->local=w->local;e->a=w->a;e->b=w->b;e->c=w->c;
 e->mode=w->mode;e->scene=w->scene;e->timer=w->timer;e->root=w->root;
 bank=0;while(w->root!=world.badges[bank]){bank++;assert(bank<3);}
 e->first=w->badges[bank][0].half02;e->second=w->badges[bank][1].half02;memcpy(e->name,w->name,12);
 w->frame=word(scenario*0x9E3779B9U+n);w->counter=word(scenario*0x1234567U+n);w->local=word(scenario*0x87654321U+n);
 w->a=(s8)(scenario+n+1);w->b=(s8)(scenario+n+2);w->c=(s8)(scenario+n+3);
 w->mode=(u16)(scenario+n+4);w->scene=(s16)(scenario+n+5);w->timer=(s16)(scenario+n+6);
 for(j=0;j<sizeof(w->badges);j++)((u8*)w->badges)[j]=(u8)(scenario+n+j*17);
 for(j=0;j<12;j++)w->name[j]=(u8)(scenario+n+j*13);
 w->root=world.badges[(scenario+n)%3];
}
void ClearScreenOverlayState(void){s32 a[7]={0};assert(world.calls==0);
 assert(world.frame==0&&world.counter==0&&world.local==0&&world.a==0&&world.b==0&&world.c==0);event(&world,a);
}
void InitBadgeRow(s32 a,s32 b,s16 c,s32 d,u16 e,u16 f,u16 g){s32 args[7]={a,b,c,d,e,f,g};assert(world.calls==1||world.calls==2);event(&world,args);}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){s32 args[7]={a,b,c};assert(world.calls==3);event(&world,args);}
static void reference(void){unsigned j=0;
 expected.frame=expected.counter=expected.local=0;expected.a=expected.b=expected.c=0;
 while(expected.save.prefix.progress.prefix00.bytes[j]){expected.name[j]=expected.save.prefix.progress.prefix00.bytes[j];j++;}
 while(expected.name[j]){expected.name[j]=0;j++;}
 {s32 a[7]={0};event(&expected,a);}
 {s32 a[7]={0,4,172,312,216,0,0x806};event(&expected,a);}
 expected.badges[(scenario+1)%3][0].half02=1;
 {s32 a[7]={1,208,84,108,80,0,0x905};event(&expected,a);}
 expected.badges[(scenario+2)%3][1].half02=1;
 {s32 a[7]={0,1,1};event(&expected,a);}
 expected.mode=1;expected.scene=28;expected.timer=0;
}
int main(void){unsigned j,length;
 for(scenario=0;scenario<65536;scenario++){
  memset(&world,0,sizeof(world));length=scenario%9;
  for(j=0;j<4;j++){world.guard[j]=scenario*17+j;world.tail[j]=scenario*31+j;}
  world.before_name=0x12345678;world.after_name=0x87654321;
  for(j=0;j<sizeof(world.save);j++)((u8*)&world.save)[j]=(u8)(scenario+j*73);
  for(j=0;j<length;j++)world.save.prefix.progress.prefix00.bytes[j]=(u8)(1+(scenario+j*19)%255);
  world.save.prefix.progress.prefix00.bytes[length]=0;
  for(j=0;j<12;j++)world.name[j]=(u8)(1+(scenario+j*37)%255);
  world.name[scenario%12]=0;world.name[11]=0;
  world.frame=word(scenario*0x9E3779B9U);world.counter=word(scenario*0x1234567U);world.local=word(scenario*0x87654321U);
  world.a=world.b=world.c=(s8)scenario;world.mode=(u16)scenario;world.scene=world.timer=(s16)scenario;
  world.root=world.badges[scenario%3];
  memcpy(&expected,&world,sizeof(world));reference();ovl662_func_80118930();
  assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.root>UINT32_MAX);
 }
 return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
