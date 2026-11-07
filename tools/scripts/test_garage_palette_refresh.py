"""Palette refresh: complete owners, mutable sine/upload services and wrapping phases."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest


class PaletteRefreshTest(unittest.TestCase):
    def test_aliases_fresh_roots_and_signed_phases(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_8011CD18.c').read_text()
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
#include "rr/garage_palette_refresh.h"
typedef struct {u32 guard[4],rng;RrGarageSelectionPrefix garage;s32 phase;RrGpuRectangleFields rectangles[5];u32 tail[4];} State;
typedef struct {s32 kind,arg,result;unsigned index;u32 hash;} Event;
typedef struct {State s;Event events[8];unsigned count;} World;
static World actual,reference,initial,*w;
static RrPalette16 palettes[9],expectedPalettes[9],initialPalettes[9];
static u32 images[4][512];
static unsigned scenario,mutation,aliasSeen[27],sineSeen[8193],uploadSeen[5],phaseSeen[8];
static const s32 phases[8]={INT32_MIN,INT32_MIN+4095,-4097,-1,0,4095,INT32_MAX-170,INT32_MAX};
#define garage_logo_state w->s.garage
#define garage_clut_pulse_phase w->s.phase
#define garage_preview_image_vram_rect w->s.rectangles[0]
#define garage_layer0_image_vram_rect w->s.rectangles[1]
#define garage_layer1_image_vram_rect w->s.rectangles[2]
#define garage_logo_clut_upload_rect w->s.rectangles[3]
#define garage_pulse_clut_upload_rect w->s.rectangles[4]
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static u32 next(void){u32 n=w->s.rng;n^=n<<13;n^=n>>17;n^=n<<5;return w->s.rng=n;}
static unsigned paletteIndex(const void *p){uintptr_t address=(uintptr_t)p,start=(uintptr_t)palettes;assert(address>UINT32_MAX&&address>=start&&address<start+sizeof(palettes));assert((address-start)%sizeof(RrPalette16)==0);return (unsigned)((address-start)/sizeof(RrPalette16));}
static RrPalette16 *owner(const void *p){return (w==&actual?palettes:expectedPalettes)+paletteIndex(p);}
static Event *event(s32 kind){unsigned i;u32 hash=2166136261U;Event *e;RrPalette16 *p=w==&actual?palettes:expectedPalettes;
 assert(w->count<8);e=&w->events[w->count++];memset(e,0,sizeof(*e));e->kind=kind;
 for(i=0;i<sizeof(w->s);i++)hash=(hash^((u8*)&w->s)[i])*16777619U;
 for(i=0;i<sizeof(palettes);i++)hash=(hash^((u8*)p)[i])*16777619U;e->hash=hash;return e;
}
static void change(void){unsigned i,j;RrPalette16 *p=w==&actual?palettes:expectedPalettes;if(!mutation)return;
 w->s.phase=word(next());w->s.garage.active21=next()%2;w->s.garage.scratch34=images[next()%4];
 for(i=0;i<2;i++){
  w->s.garage.palette48[i]=palettes[next()%9].words;w->s.garage.paletteBackup50[i]=palettes[next()%9].words;w->s.garage.paletteScratch58[i]=palettes[next()%9].words;
  w->s.garage.image38[i]=images[next()%4];w->s.garage.image40[i]=images[next()%4];
 }
 for(i=0;i<9;i++)for(j=0;j<16;j++)p[i].entries[j]=(u16)next();
 for(i=0;i<5;i++){w->s.rectangles[i].x=(s16)next();w->s.rectangles[i].y=(s16)next();}
}
s32 rsin(s32 angle){unsigned index=(scenario+(w->count*2731U))%8193;Event *e=event(1);s32 result=(s32)index-4096;
 assert(angle>=-4095&&angle<=4095);e->arg=angle;e->result=result;sineSeen[index]++;change();return result;
}
void LoadImage(const RrGpuRectangleFields *rectangle,const void *pixels){unsigned index=(unsigned)(rectangle-w->s.rectangles);Event *e=event(2);
 assert(index<5);e->arg=index;uploadSeen[index]++;
 if(index<3){uintptr_t p=(uintptr_t)pixels,start=(uintptr_t)images;assert(p>UINT32_MAX&&p>=start&&p<start+sizeof(images));assert((p-start)%sizeof(images[0])==0);e->index=(unsigned)((p-start)/sizeof(images[0]));}
 else e->index=paletteIndex(pixels);
 change();
}
''' + body + r'''
static void expected(void){RrGarageSelectionPrefix *g=&w->s.garage;unsigned active=g->active21,i;void *capturedPalette=g->palette48[active],*capturedScratch=g->paletteScratch58[active],*capturedBackup=g->paletteBackup50[active];
 RrPalette16 *p=owner(capturedPalette),*s=owner(capturedScratch),*b=owner(capturedBackup);s32 angle,result;u32 component;
 p->entries[0]=0;s->entries[0]=32768;
 for(i=0;i<3;i++){
  s32 phase=word((u32)w->s.phase+85U*i);angle=(s32)((int64_t)phase%4096);result=rsin(angle);
  component=(u32)((result/128+32)/8);s->entries[0]=(u16)(s->entries[0]|(component<<(5*i)));
 }
 w->s.phase=word((u32)w->s.phase+32U);
 for(i=1;i<16;i++){u16 color=(u16)(p->entries[i]|32768);p->entries[i]=color;s->entries[i]=color;}
 memmove(b->raw.data,p->raw.data,32);
 LoadImage(&w->s.rectangles[0],g->scratch34);
 LoadImage(&w->s.rectangles[1],g->image38[g->active21]);
 LoadImage(&w->s.rectangles[2],g->image40[g->active21]);
 LoadImage(&w->s.rectangles[3],capturedPalette);
 LoadImage(&w->s.rectangles[4],capturedScratch);
}
int main(void){unsigned i,j,a;uint64_t cases=0;
 for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0xCD18U+scenario*73U;for(i=0;i<4;i++){w->s.guard[i]=next();w->s.tail[i]=next();}
  w->s.phase=(scenario&8)?word(next()):phases[scenario%8];if(!(scenario&8))phaseSeen[scenario%8]++;
  for(i=0;i<9;i++)for(j=0;j<16;j++)initialPalettes[i].entries[j]=(u16)next();
  w->s.garage.active21=(scenario>>2)&1;a=scenario%27;aliasSeen[a]++;
  for(i=0;i<2;i++){
   w->s.garage.palette48[i]=palettes[a%3].words;w->s.garage.paletteScratch58[i]=palettes[(a/3)%3].words;w->s.garage.paletteBackup50[i]=palettes[a/9].words;
   w->s.garage.image38[i]=images[i];w->s.garage.image40[i]=images[i+2];
  }
  w->s.garage.scratch34=images[3];
  actual=reference=initial;memcpy(palettes,initialPalettes,sizeof(palettes));memcpy(expectedPalettes,initialPalettes,sizeof(palettes));
  w=&actual;garage_upload_editor_vram();w=&reference;expected();
  assert(!memcmp(&actual,&reference,sizeof(actual)));assert(!memcmp(palettes,expectedPalettes,sizeof(palettes)));cases++;
 }
 assert(cases==131072);for(i=0;i<27;i++)assert(aliasSeen[i]);for(i=0;i<8193;i++)assert(sineSeen[i]);for(i=0;i<8;i++)assert(phaseSeen[i]);for(i=0;i<5;i++)assert(uploadSeen[i]);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p = Path(tmp)
            (p / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(p / 'test.c'), '-o', str(p / 'test')], check=True)
            subprocess.run([str(p / 'test')], check=True)
