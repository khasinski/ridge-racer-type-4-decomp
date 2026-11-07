"""Garage arena setup: native pointer layouts, stage capture and live upload roots."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest


class GarageArenaInitTest(unittest.TestCase):
    def test_stages_alignment_and_live_roots(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/662/func_8011A518.c').read_text()
        body = re.sub(r' __asm__\("\$[0-9]+"\)', '', body)
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
#include "rr/overlay662_arena_init.h"
typedef struct {
 u32 guard[4],rng;RrGarageSelectionPrefix garage;RrGpuRectangle rects[2];
 u8 *root;void *cursor,*first,*second;s32 stage;s16 phase,nextScene;u16 scene;u32 tail[4];
} State;
typedef struct {s32 kind,args[10];uintptr_t pointer;u32 hash;} Event;
typedef struct {State s;Event events[16];unsigned calls;} World;
static World actual,reference,initial,*w;
static u32 pool[8192],expectedPool[8192],initialPool[8192];
static unsigned scenario,mutation,stageSeen[3],alignmentSeen[4];
static const s32 counters[8]={-1,0,1,2,3,INT32_MAX,INT32_MIN,-2};
#define ovl662_D_801276A8 w->s.garage
#define D_800F4D74 w->s.root
#define LoadCursor w->s.cursor
#define D_800F3918 w->s.first
#define D_800F3910 w->s.second
#define ovl662_D_8012625C w->s.stage
#define D_800F4D8A w->s.phase
#define ModeId w->s.nextScene
#define ModeGroup w->s.scene
#define ovl662_D_8012624C w->s.rects[0]
#define ovl662_D_80126254 w->s.rects[1]
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static u32 next(void){u32 n=w->s.rng;n^=n<<13;n^=n>>17;n^=n<<5;return w->s.rng=n;}
static unsigned indexOf(const u32 *p){ptrdiff_t i=p-pool;assert(i>=0&&i<=8192-512);assert((uintptr_t)p>UINT32_MAX);return (unsigned)i;}
static Event *event(s32 kind){Event *e;unsigned i;u32 h=2166136261U;assert(w->calls<16);
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 e=&w->events[w->calls++];memset(e,0,sizeof(*e));e->kind=kind;e->hash=h;return e;
}
static void change(void){unsigned i;if(!mutation)return;
 w->s.stage=word(next());w->s.phase=(s16)next();w->s.scene=(u16)next();w->s.nextScene=(s16)next();
 w->s.root=(u8*)(pool+1024)+(scenario+w->calls)%4;w->s.cursor=pool+7168;
 w->s.garage.dirtyD3=(u8)next();w->s.garage.active21=next()%2;
 for(i=0;i<2;i++){
  w->s.garage.image38[i]=pool+2048+i*1024;
  w->s.garage.image40[i]=pool+4096+i*512+(next()%16);
  w->s.garage.palette48[i]=pool+6144+i*8;
 }
}
u8 *ovl662_func_8011B2D4(s32 index,u8 *cursor,s32 x,s32 y){Event *e=event(1);e->args[0]=index;e->args[1]=x;e->args[2]=y;assert(!cursor);change();return 0;}
void ovl662_func_8011E36C(s32 mode){event(2)->args[0]=mode;change();}
void ovl662_func_8011D278(s32 mode){event(3)->args[0]=mode;change();}
void ovl662_func_8011A414(void){event(4);change();}
void StoreImage(RrGpuRectangle *rectangle,const u32 *pixels){Event *e=event(5);e->args[0]=rectangle-w->s.rects;assert(e->args[0]>=0&&e->args[0]<2);memcpy(e->args+1,rectangle,8);e->pointer=(uintptr_t)pixels;indexOf(pixels);change();}
void ovl662_func_8011D120(void){event(6);w->s.garage.dirtyD3=1;change();}
void ClearScreenOverlayState(void){event(7);change();}
void InitBadgeRow(s32 index,s32 first,s16 x,s32 y,u16 width,u16 mode,u16 flags){Event *e=event(8);e->args[0]=index;e->args[1]=first;e->args[2]=x;e->args[3]=y;e->args[4]=width;e->args[5]=mode;e->args[6]=flags;change();}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e0,s32 f,s32 g,s32 h,s32 i,s32 j,...){Event *e=event(9);s32 args[10]={a,b,c,d,e0,f,g,h,i,j};memcpy(e->args,args,sizeof(args));change();}
''' + body + r'''
static void expected(void){s32 stage=word((u32)w->s.stage+1U);unsigned i;
 w->s.stage=stage;
 if(stage==1){u8 *b;RrGarageSelectionPrefix *g=&w->s.garage;
  ovl662_func_8011B2D4(-1,0,0,0);ovl662_func_8011E36C(-1);ovl662_func_8011D278(-1);ovl662_func_8011A414();
  b=(u8*)((((uintptr_t)w->s.root)/4+1)*4);assert((uintptr_t)b%4==0);
  for(i=0;i<2;i++){
   g->buffers24[i]=(u32*)(b+i*0x1080);g->tim2C[i]=(RrGarageTimBytes*)(b+0x840+i*0x1080);
   g->image38[i]=(u32*)(b+0x40+i*0x1080);g->image40[i]=(u32*)(b+0x880+i*0x1080);
   g->palette48[i]=(u32*)(b+0x14+i*0x1080);g->paletteBackup50[i]=(u32*)(b+0x854+i*0x1080);
   g->paletteScratch58[i]=(u32*)(b+0x2900+i*32);
  }
  g->scratch34=(u32*)(b+0x2100);w->s.cursor=b+0x2940;w->s.first=b;w->s.second=b+0x840;
  assert((uintptr_t)b>UINT32_MAX);alignmentSeen[(uintptr_t)w->s.root%4]++;stageSeen[0]++;
 }else if(stage==2){unsigned base;
  StoreImage(&w->s.rects[0],w->s.garage.image38[0]);StoreImage(&w->s.rects[1],w->s.garage.palette48[0]);
  base=indexOf(w->s.garage.image40[0]);for(i=0;i<512;i++)expectedPool[base+i]=0;stageSeen[1]++;
 }else if(stage==3){w->s.stage=0;ovl662_func_8011D120();w->s.garage.dirtyD3=0;ClearScreenOverlayState();
  InitBadgeRow(0,12,304,296,88,0,0);
  InitUiRow(0,140,136,28,32,0,0,1,0,104);InitUiRow(1,140,176,28,32,0,0,1,0,105);
  w->s.scene=3;w->s.nextScene=4;w->s.phase=0;stageSeen[2]++;
 }
}
int main(void){unsigned path,i,j;uint64_t cases=0;
 assert(sizeof(void*)==8&&sizeof(RrGarageSelectionPrefix)==0x138&&offsetof(RrGarageSelectionPrefix,paletteScratch58)==0x90);
 for(i=0;i<8192;i++)initialPool[i]=0x12345678U+i*73;
 for(scenario=0;scenario<4096;scenario++)for(path=0;path<8;path++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0xA518+scenario*17+path;
  for(i=0;i<4;i++){w->s.guard[i]=next();w->s.tail[i]=next();}
  for(i=0;i<2;i++){
   for(j=0;j<2;j++)w->s.rects[i].words[j]=next();
   w->s.garage.image38[i]=pool+2048+i*1024;w->s.garage.image40[i]=pool+4096+i*512;w->s.garage.palette48[i]=pool+6144+i*8;
  }
  w->s.root=(u8*)pool+scenario%4;w->s.cursor=pool+7168;w->s.first=pool+7200;w->s.second=pool+7300;
  w->s.stage=counters[path];w->s.phase=(s16)next();w->s.scene=(u16)next();w->s.nextScene=(s16)next();w->s.garage.dirtyD3=(u8)next();
  memcpy(pool,initialPool,sizeof(pool));memcpy(expectedPool,initialPool,sizeof(pool));actual=reference=initial;
  w=&actual;ovl662_func_8011A518();w=&reference;expected();
  assert(!memcmp(&actual,&reference,sizeof(actual)));assert(!memcmp(pool,expectedPool,sizeof(pool)));cases++;
 }
 assert(cases==65536);for(i=0;i<3;i++)assert(stageSeen[i]);for(i=0;i<4;i++)assert(alignmentSeen[i]);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p = Path(tmp)
            (p / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(p / 'test.c'), '-o', str(p / 'test')], check=True)
            subprocess.run([str(p / 'test')], check=True)
