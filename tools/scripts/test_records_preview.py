"""Vehicle preview setup: genuine race/scratch owners and fresh badge roots."""
from overlay_units import read_c_source
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class RecordsPreviewTest(unittest.TestCase):
    def test_mutating_boundaries_and_complete_owners(self):
        root=Path(__file__).resolve().parents[2]
        body=read_c_source(root/'src/overlays/PAL/records/func_801157B8.c')
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
#include "rr/records_preview.h"
typedef struct {s32 args[10],local,frame,position[3],rotation[3],scratch[7];s8 flag;RrBadgeRow1C *root;u8 *cursor;u32 *order;} Event;
static struct World {
 u32 guard[4];RrRaceVehicle cars[2];RrProjectionScratchPrefix scratch;
 RrBadgeRow1C badges[3][2],*root;s32 local,frame;s8 flag;
 unsigned calls;Event events[4];u32 tail[4];
} world,expected;
static u8 packets[64];static u32 orders[32];static unsigned scenario;
#define PlayerCars world.cars
#define RR_PROJECTION_SCRATCH (&world.scratch)
#define BadgeRows world.root
#define records_selected_record world.local
#define ModeFrame world.frame
#define ScreenFadeState world.flag
'''+body+r'''
static u32 rng=0x157B8;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static void effect(struct World *w,unsigned n){unsigned j;u8 *bytes=(u8*)w->cars;
 for(j=0;j<sizeof(w->cars);j++)bytes[j]=(u8)(scenario*13+n*37+j);
 for(j=0;j<sizeof(w->badges);j++)((u8*)w->badges)[j]=(u8)(scenario+n+j*7);
 w->local=word(scenario*0x9E3779B9U+n);w->frame=word(scenario*0x1234567U+n);w->flag=(s8)(scenario+n);
 w->scratch.pointers.cursor=packets+(scenario+n)%64;w->scratch.pointers.order=orders+(scenario+n)%32;
 w->scratch.word08=word(scenario+n);w->scratch.word0C=word(scenario+n+1);w->scratch.word10=word(scenario+n+2);
 w->scratch.word14=word(scenario+n+3);w->scratch.word18=word(scenario+n+4);w->scratch.word1C=word(scenario+n+5);w->scratch.word20=word(scenario+n+6);
 w->root=world.badges[(scenario+n)%3];
}
static void event(struct World *w,const s32 args[10]){unsigned n=w->calls,j;Event *e;assert(n<4);e=&w->events[w->calls++];
 for(j=0;j<10;j++)e->args[j]=args[j];
 e->local=w->local;e->frame=w->frame;e->flag=w->flag;e->root=w->root;
 for(j=0;j<3;j++)e->position[j]=w->cars[0].pos20[j];
 e->rotation[0]=w->cars[0].rot50;e->rotation[1]=w->cars[0].rot54;e->rotation[2]=w->cars[0].rot58;
 e->scratch[0]=w->scratch.word08;e->scratch[1]=w->scratch.word0C;e->scratch[2]=w->scratch.word10;e->scratch[3]=w->scratch.word14;
 e->scratch[4]=w->scratch.word18;e->scratch[5]=w->scratch.word1C;e->scratch[6]=w->scratch.word20;
 e->cursor=w->scratch.pointers.cursor;e->order=w->scratch.pointers.order;effect(w,n);
}
void ClearScreenOverlayState(void){s32 a[10]={0};assert(world.calls==0);assert(world.local==0);event(&world,a);}
void SetupRenderer(s32 shift){s32 a[10]={shift};assert(world.calls==1);assert(world.flag==2&&world.frame==0);event(&world,a);}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){s32 args[10]={a,b,c,d,e,f,g,h,i,j};assert(world.calls==2);event(&world,args);}
void InitBadgeRow(s32 a,s32 b,s16 c,s32 d,u16 e,u16 f,u16 g){s32 args[10]={a,b,c,d,e,f,g};assert(world.calls==3);event(&world,args);}
static void reference(void){s32 a[10]={0};expected.local=0;event(&expected,a);expected.flag=2;expected.frame=0;
 {s32 args[10]={3};event(&expected,args);}
 expected.scratch.word08=expected.scratch.word0C=expected.scratch.word10=0;
 expected.scratch.word18=expected.scratch.word1C=expected.scratch.word20=0;
 expected.cars[0].pos20[0]=0;expected.cars[0].pos20[1]=-200;expected.cars[0].pos20[2]=2024;
 expected.cars[0].rot50=40;expected.cars[0].rot54=expected.cars[0].rot58=0;
 {s32 args[10]={0,12,64,68,32,0,0,1,0,83};event(&expected,args);}
 {s32 args[10]={0,12,128,76,34,0,0xC01};event(&expected,args);}
 expected.badges[(scenario+3)%3][0].half02=1;
}
int main(void){unsigned j;
 for(scenario=0;scenario<65536;scenario++){
  memset(&world,0,sizeof(world));effect(&world,7);
  for(j=0;j<4;j++){world.guard[j]=next();world.tail[j]=next();}
  memcpy(&expected,&world,sizeof(world));reference();records_init_preview();
  assert(memcmp(&world,&expected,sizeof(world))==0);
  assert((uintptr_t)world.root>UINT32_MAX&&(uintptr_t)world.scratch.pointers.cursor>UINT32_MAX);
 }
 return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
