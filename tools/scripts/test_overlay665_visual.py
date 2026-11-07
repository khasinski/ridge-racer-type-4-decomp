"""Exact160-byte visual state and actual signed32 sine/cosine closure."""
from pathlib import Path
import re
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

PREFIX=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/overlay665_visual.h"
'''

class Overlay665VisualTest(unittest.TestCase):
    def run_native(self,root,source):
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

    def test_visual_initialization_and_full_pointer_scratch(self):
        root=Path(__file__).resolve().parents[2]
        reset=re.sub(r'^extern[^\n]*;\n','',(root/'tools/scripts/fixtures/legacy/func_8003A198.c').read_text(),flags=re.M)
        body=(root/'src/overlays/PAL/665/func_80114918.c').read_text()
        source=PREFIX+r'''
typedef struct {u32 guard[4];RrOverlay665VisualState visual;RrProjectionScratchPrefix scratch;s32 counter;s8 flag1,flag2;s32 mode;u16 selector;s16 scene,phase;u32 tail[4];} State;
typedef struct {s32 kind,args[4];void *buffer;State state;} Event;
static struct World {State s;u32 calls;Event events[3];u32 guard[4];} world,expected;
static u8 arenas[3][256];static u32 ordering[3][704];
#define ovl665_D_80119768 world.s.visual
#define RR_PROJECTION_SCRATCH (&world.s.scratch)
#define ScreenFrame world.s.counter
#define LoadState world.s.flag1
#define MenuScriptStep world.s.flag2
#define D_800F4CA8 world.s.mode
#define ModeGroup world.s.selector
#define ModeId world.s.scene
#define D_800F4D8A world.s.phase
''' + reset + body + r'''
static u32 seed=0x14918,scenario;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 word(u32 bits){s32 v;memcpy(&v,&bits,4);return v;}
static s16 half(u32 raw){u16 bits=(u16)raw;s16 v;memcpy(&v,&bits,2);return v;}
static void effect(State *s,unsigned step){unsigned i;
 for(i=0;i<sizeof(s->visual);i++)((u8*)&s->visual)[i]=(u8)(scenario+step*73+i*17);
 s->scratch.pointers.cursor=arenas[(scenario+step)%3]+64+step;s->scratch.pointers.order=ordering[(scenario+step+1)%3];
 s->scratch.word08=word(scenario^0xA1234567U^step);s->scratch.word0C=word(scenario^0xB1234567U^step);s->scratch.word10=word(scenario^0xC1234567U^step);s->scratch.word14=word(scenario^0xD1234567U^step);s->scratch.word18=word(scenario^0xE1234567U^step);s->scratch.word1C=word(scenario^0xF1234567U^step);s->scratch.word20=word(scenario^0x91234567U^step);
 s->counter=word(scenario*0x9E3779B9U+step);s->flag1=(s8)step;s->flag2=(s8)(step+3);s->mode=word(scenario*0xABCDEF01U+step);s->selector=(u16)(scenario+step);s->scene=half(scenario*73U+step);s->phase=half(scenario*40503U+step);
}
static void observe(struct World *v,s32 kind,s32 a,s32 b,s32 c,s32 d){Event *e;unsigned step=v->calls;assert(step<3);e=&v->events[v->calls++];memset(e,0,sizeof(*e));e->kind=kind;e->args[0]=a;e->args[1]=b;e->args[2]=c;e->args[3]=d;e->buffer=kind==2?(void*)&world.s.visual:0;memcpy(&e->state,&v->s,sizeof(v->s));
 if(kind==2)memset(&v->s.visual,0,160);else effect(&v->s,step+1);
}
void SetDispMask(s32 mode){assert(world.calls==0 && mode==0);observe(&world,0,mode,0,0,0);}
void SetupDrawDispEnvs(s32 r,s32 g,s16 b,u32 flags){assert(world.calls==1 && r==0 && g==0 && b==0 && flags==2);observe(&world,1,r,g,b,(s32)flags);}
void PsyqBzero(void *buffer,s32 size){assert(world.calls==2 && buffer==&world.s.visual && size==160);observe(&world,2,size,0,0,0);}
static void reference(void){unsigned i,j;State *s=&expected.s;static const s16 directions[3][3]={{4500,-4096,-8000},{8000,8000,-8000},{-2637,1602,2694}};static const u8 colors[3]={192,128,0};
 observe(&expected,0,0,0,0,0);observe(&expected,1,0,0,0,2);observe(&expected,2,160,0,0,0);
 for(i=0;i<3;i++)for(j=0;j<3;j++){s->visual.lighting4C.matrix.bytes[i*4+j]=colors[i];s->visual.lighting4C.matrix.signedHalves[6+i*4+j]=directions[i][j];}
 for(j=0;j<3;j++)s->visual.lighting4C.ambient[j]=64;
 s->scratch.word08=0;s->scratch.word0C=0;s->scratch.word10=0;s->scratch.word18=0;s->scratch.word1C=0;s->scratch.word20=0;
 s->counter=0;s->flag1=0;s->flag2=0;s->mode=2;s->selector=2;s->scene=2;s->phase=0;
}
int main(void){unsigned k,i;
 _Static_assert(sizeof(RrOverlay665VisualState)==160 && offsetof(RrOverlay665VisualState,lighting4C)==0x4C,"real scalar visual owner");
 _Static_assert(offsetof(RrOverlay665VisualState,lighting4C)+offsetof(RrPreviewLightInputFields36,vectors)==0x58,"original direction rows");
 _Static_assert(offsetof(RrOverlay665VisualState,lighting4C)+offsetof(RrPreviewLighting40,ambient)==0x70,"original ambient bytes");
 for(k=0;k<65536;k++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();scenario=k;world.calls=0;world.s.scratch.pointers.cursor=arenas[k%3]+32;world.s.scratch.pointers.order=ordering[(k+1)%3];
  assert((uintptr_t)world.s.scratch.pointers.cursor>UINT32_MAX && (uintptr_t)world.s.scratch.pointers.order>UINT32_MAX);
  memcpy(&expected,&world,sizeof(world));reference();ovl665_func_80114918();assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        self.run_native(root,source)

    def test_compass_actual_trig_full_word_angles_and_centers(self):
        root=Path(__file__).resolve().parents[2]
        exe=(root/'assets/PAL/main.exe').read_bytes()
        table=struct.unpack_from('<4097h',exe,0x800+0x800A50C0-0x80010000)
        self.assertEqual((min(table[3072:]),max(table[3072:])),(0,4096))
        trig=(root/'tools/scripts/fixtures/legacy/trig_sine.c').read_text()+(root/'tools/scripts/fixtures/legacy/func_800904F4.c').read_text()
        body=(root/'src/overlays/PAL/665/func_801169BC.c').read_text()
        source=PREFIX+r'''
typedef struct {u16 points[4];u8 rgb[3];u8 pad;const u8 *color;s32 blend,depth;RrOverlay665VisualState before;} Event;
static struct World {u32 guard[4];RrOverlay665VisualState visual;u8 color[4];u32 calls;Event event;u32 tail[4];} world,expected;
#define ovl665_D_80119768 world.visual
#define ovl665_D_8011973C world.color
static const s16 rom[4097]={''' + ','.join(map(str,table)) + r'''};
#define D_800A50C0 rom
#define D_800A58C0 (rom+0x400)
#define D_800A60C0 (rom+0x800)
#define SinTable (rom+0xC00)
''' + trig + body + r'''
static u32 seed=0x169BC;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 word(u32 bits){s32 v;memcpy(&v,&bits,4);return v;}
static s32 sine_reference(u32 phase){unsigned quadrant=(phase&4095U)/1024U,at=phase%1024U;switch(quadrant){case 0:return rom[3072+at];case 1:return rom[4096-at];case 2:return -rom[3072+at];default:return -rom[4096-at];}}
static void effect(struct World *v,const u16 points[4]){Event *e=&v->event;assert(v->calls==0);v->calls++;memset(e,0,sizeof(*e));memcpy(e->points,points,8);memcpy(e->rgb,v->color,3);e->color=world.color;e->blend=0;e->depth=0;memcpy(&e->before,&v->visual,sizeof(v->visual));
 v->visual.centerX38^=0xA55A;v->visual.centerY3A^=0x5AA5;((u8*)&v->visual.scale3C)[0]^=0xEF;v->color[0]^=0x31;v->color[1]^=0x73;v->color[2]^=0xA7;
}
void AddLineF2Prim(const s16 points[4],const u8 rgb[3],s32 blend,s32 depth){assert(rgb==world.color && blend==0 && depth==0);assert((uintptr_t)points>UINT32_MAX && (uintptr_t)rgb>UINT32_MAX);effect(&world,(u16 *)points);}
static void reference(s32 angle){u16 points[4];int64_t sine=sine_reference((u32)angle&4095U),cosine=sine_reference(((u32)angle+1024U)&4095U),x=expected.visual.centerX38,y=expected.visual.centerY3A;
 assert(sine>=-4096 && sine<=4096 && cosine>=-4096 && cosine<=4096);
 points[0]=(u16)(x+sine/128);points[1]=(u16)(y-cosine/64);points[2]=(u16)(x+sine*40/8192);points[3]=(u16)(y-cosine*40/4096);effect(&expected,points);
}
int main(void){unsigned k,i;for(k=0;k<262144;k++){
 s32 angle;for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();world.calls=0;world.visual.centerX38=(u16)k;world.visual.centerY3A=(u16)(k*40503U+17U);
 if(k<65536)angle=(s32)k;else if(k<131072)angle=-(s32)(k-65536)-1;else angle=word(next());
 if(k==131072)angle=INT32_MIN;else if(k==131073)angle=INT32_MAX;
 memcpy(&expected,&world,sizeof(world));reference(angle);ovl665_func_801169BC(angle);assert(memcmp(&world,&expected,sizeof(world))==0);
 }return 0;
}
'''
        self.run_native(root,source)

if __name__=='__main__':unittest.main()
