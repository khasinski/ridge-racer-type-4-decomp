"""Coupled dispatcher/pixel C with real hit testing and mutable service state."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess
import unittest


class GaragePixelDispatchTest(unittest.TestCase):
    def test_callbacks_byte_coordinates_and_captured_operation(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/garage/func_{a}.c').read_text()
                           for a in ('80124E4C', '80124974'))
        bodies = re.sub(r' __asm__\("\$[0-9]+"\)', '', bodies)
        hit = (root / 'src/overlays/PAL/garage/func_8011E6C8.c').read_text().replace(
            'garage_cursor_outside_rect(', 'real_hit(')
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
#include "rr/overlay662_pixel_dispatch.h"
typedef struct {u32 guard[4],rng;RrGarageSelectionPrefix garage;RrGarageWorkingStorage work;RrMenuItemState5C controllers[2];u32 tail[4];} State;
typedef struct {s32 kind,args[7];u32 hash;} Event;
typedef struct {State s;Event e[12];unsigned calls;} World;
static World actual,reference,initial,*w;
static u32 pool[8192],expectedPool[8192],initialPool[8192];
static unsigned scenario,mutation,entry,seen[16],colors[256],xSeen[64],ySeen[64],brushSeen[5],maskSeen[3];
#define garage_logo_state w->s.garage
#define garage_logo_work w->s.work
#define PlayerControllers w->s.controllers
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static s16 half(u16 n){s16 v;memcpy(&v,&n,2);return v;}
static u32 next(void){u32 n=w->s.rng;n^=n<<13;n^=n>>17;n^=n<<5;return w->s.rng=n;}
static unsigned byteIndex(const u8 *p){uintptr_t i=(uintptr_t)p-(uintptr_t)pool;assert((uintptr_t)p>UINT32_MAX&&i<sizeof(pool));return (unsigned)i;}
static u8 *payload(void){return (u8*)(w==&actual?pool:expectedPool);}
static void coordinates(unsigned x,unsigned y){u16 a=(u16)next(),b=(u16)next();
 w->s.garage.reset08[0]=half(a);w->s.garage.reset08[6]=half(((u16)next()&0xFF00U)|((x-(u8)a)&255));
 w->s.garage.reset08[1]=half(b);w->s.garage.reset08[7]=half(((u16)next()&0xFF00U)|((y-(u8)b)&255));
}
static unsigned currentPixel(void){RrGarageSelectionPrefix *g=&w->s.garage;
 unsigned x=((u16)g->reset08[0]+(u16)g->reset08[6])&255,y=((u16)g->reset08[1]+(u16)g->reset08[7])&255;
 u32 *base=g->layer23?g->image40[g->active21]:g->image38[g->active21];assert(x<64&&y<64);return byteIndex((u8*)base)+y*32+x/2;
}
static Event *event(s32 kind){Event *e;unsigned i;u32 h=2166136261U;assert(w->calls<12);
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 e=&w->e[w->calls++];memset(e,0,sizeof(*e));e->kind=kind;e->hash=h;return e;
}
static void change(void){unsigned i;if(!mutation)return;
 w->s.garage.active21=next()%2;w->s.garage.layer23=(next()&1)?0:255;
 w->s.garage.config20=next()%4;w->s.garage.config1E=next()%8;w->s.garage.config1F=next()%5;w->s.garage.config22=(u8)next();
 for(i=0;i<2;i++){w->s.garage.image38[i]=pool+((scenario+i+w->calls)%4)*512;w->s.garage.image40[i]=pool+((scenario+i+w->calls+2)%4)*512;}
 w->s.garage.scratch34=(scenario&1)?w->s.garage.image38[0]:pool+2048;
 w->s.work.words[1]=(w->s.work.words[1]&~1U)|(next()%2);
 w->s.controllers[0].type01=(next()&1)?0x12:(u8)next();w->s.controllers[0].buttons02=(u16)next();
 coordinates(next()%64,next()%64);
}
void garage_map_cursor_to_cell(void){event(1);change();}
''' + hit + r'''
s32 garage_cursor_outside_rect(s32 x,s32 y,s32 width,s32 height){Event *e=event(2);s32 result=real_hit(x,y,width,height);
 e->args[0]=x;e->args[1]=y;e->args[2]=width;e->args[3]=height;e->args[4]=result;change();return result;
}
void garage_save_undo_copy(void){unsigned i=currentPixel();event(3)->args[0]=i;payload()[i]^=255;w->s.garage.dirtyD3=1;change();}
void FloodFill4bpp(u8 *pixel,u32 operation){Event *e=event(4);unsigned i=byteIndex(pixel);e->args[0]=i;e->args[1]=word(operation);e->args[2]=payload()[i];change();}
static void service(s32 kind){event(kind);change();}
void garage_pencil_stroke(void){service(10);}void garage_line_stroke(void){service(11);}
void garage_erase_stroke(void){service(13);}void garage_move_layer_drag(void){service(14);}
void garage_pick_color(void){service(20);}void garage_alt_erase_stroke(void){service(21);}void garage_alt_undo_toggle(void){service(22);}
''' + bodies + r'''
static void expectedPixel(void){unsigned x,y,index,old,color;u32 operation;
 if(garage_cursor_outside_rect(82,71,128,256)){seen[0]++;return;}
 if(!(w->s.work.words[1]&1)){seen[1]++;return;}
 x=((u16)w->s.garage.reset08[0]+(u16)w->s.garage.reset08[6])%256;
 y=((u16)w->s.garage.reset08[1]+(u16)w->s.garage.reset08[7])%256;
 assert(x<64&&y<64);index=currentPixel();old=(payload()[index]>>(4*(x%2)))&15;
 color=w->s.controllers[0].type01!=0x12&&(w->s.controllers[0].buttons02&3)==3?0:w->s.garage.config22;
 xSeen[x]++;ySeen[y]++;colors[color]++;seen[2+(x&1)]++;
 if(old==color){seen[4]++;return;}
 operation=(u32)x+(u32)y*256U+(u32)old*65536U+(u32)color*16777216U;
 garage_save_undo_copy();FloodFill4bpp((u8*)pool+index,operation);seen[5]++;
}
static void expectedDispatcher(void){unsigned i,index;garage_map_cursor_to_cell();
 index=byteIndex((u8*)w->s.garage.scratch34)/4;assert(index<=8192-512);for(i=0;i<512;i++)expectedPool[index+i]=0;
 if(w->s.garage.config20==2)seen[6]++;
 else switch(w->s.garage.config1E){
  case 0:brushSeen[0]++;garage_pencil_stroke();break;case 1:brushSeen[1]++;garage_line_stroke();break;
  case 2:brushSeen[2]++;expectedPixel();break;case 3:brushSeen[3]++;garage_erase_stroke();break;case 4:brushSeen[4]++;garage_move_layer_drag();break;default:seen[7]++;break;
 }
 if(w->s.garage.config20==1)seen[8]++;
 else switch(w->s.garage.config1F){case 0:maskSeen[0]++;garage_pick_color();break;case 1:maskSeen[1]++;garage_alt_erase_stroke();break;case 2:maskSeen[2]++;garage_alt_undo_toggle();break;default:seen[9]++;break;}
}
int main(void){unsigned i,j;uint64_t cases=0;static const s32 codes[8]={0,1,2,4,8,-1,INT32_MIN,INT32_MAX};
 for(i=0;i<8192;i++)initialPool[i]=0x9A456BC7U+i*73;
 for(scenario=0;scenario<16384;scenario++)for(entry=0;entry<2;entry++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0x24974+scenario*73;
  for(i=0;i<4;i++){w->s.guard[i]=next();w->s.tail[i]=next();}
  for(i=0;i<16;i++)w->s.work.words[i]=next();w->s.work.words[1]=(w->s.work.words[1]&~1U)|(scenario%5!=0);
  w->s.work.points.cursor14.x=(scenario%3)?(s16)(82+scenario%128):81;w->s.work.points.cursor14.y=(s16)(71+scenario%256);
  w->s.garage.active21=(scenario>>1)&1;w->s.garage.layer23=(scenario&1)?0:255;
  w->s.garage.config20=scenario%256;w->s.garage.config1E=(scenario>>2)%8;w->s.garage.config1F=(scenario>>5)%5;w->s.garage.config22=(scenario>>4)%256;
  for(i=0;i<2;i++){w->s.garage.image38[i]=pool+i*512;w->s.garage.image40[i]=pool+(2+i)*512;}
  w->s.garage.scratch34=(scenario&1)?w->s.garage.image38[0]:pool+2048;
  w->s.controllers[0].type01=(scenario&2)?0x12:0;w->s.controllers[0].buttons02=(u16)((next()&~3U)|(scenario%4));
  coordinates(scenario%64,(scenario>>6)%64);
  memcpy(pool,initialPool,sizeof(pool));
  if((scenario&128)&&w->s.garage.config22<16){unsigned k=currentPixel(),shift=4*(scenario%2);((u8*)pool)[k]=(((u8*)pool)[k]&~(15U<<shift))|(w->s.garage.config22<<shift);}
  memcpy(expectedPool,pool,sizeof(pool));actual=reference=initial;
  w=&actual;if(entry==0){void (*callback)(s32)=garage_canvas_input;callback(codes[scenario%8]);}else garage_fill_at_cursor();
  w=&reference;if(entry==0)expectedDispatcher();else expectedPixel();
  assert(!memcmp(&actual,&reference,sizeof(actual)));assert(!memcmp(pool,expectedPool,sizeof(pool)));cases++;
 }
 assert(cases==65536);for(i=0;i<10;i++)assert(seen[i]);for(i=0;i<5;i++)assert(brushSeen[i]);for(i=0;i<3;i++)assert(maskSeen[i]);
 for(i=0;i<64;i++){assert(xSeen[i]);assert(ySeen[i]);}for(i=0;i<256;i++)assert(colors[i]);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p = Path(tmp)
            (p / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(p / 'test.c'), '-o', str(p / 'test')], check=True)
            subprocess.run([str(p / 'test')], check=True)
