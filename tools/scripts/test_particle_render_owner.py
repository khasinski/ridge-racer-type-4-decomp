"""Actual particle renderer and all three gradient emitters on full owners.

Independent wire construction checks every packet and ordering head, both frame
banks, full native DMA mapping, complete context/system/scratch owners and guards.
No renderer service is mocked and no production body is rewritten.
"""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest
from c_functions import definition_source, source_owners
ROOT=Path(__file__).resolve().parents[2]
PREFIX=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/scene_camera_state.h"
#include "rr/ui.h"
#include "rr/frame_storage_types.h"
#include "rr/gpu_scratch_types.h"
#include "rr/gpu_address.h"
#include "rr/gouraud_quad.h"
#include "rr/ui_primitives.h"
#include "game/course/trktext.h"
typedef union {GpuScratchPointers pointers;u64 alignment;u8 bytes[1024];} Scratch;
typedef union {GpuPolyFT4Packet alignment;u8 bytes[512];} PacketArena;
typedef struct {
 u32 before[4];SceneRaceContext scene;u32 afterScene[4];ParticleSystem32C system;u32 afterSystem[4];
 FrameStorage frames[2];u32 afterFrames[4];PacketArena packets;u32 afterPackets[4];
 Scratch scratch;u32 afterScratch[4];FrameStorage *current;GpuColorCommand clip;u32 after[4];
} World;
static World world,expected;
#define RaceContext world.scene
#define ParticleSystem world.system
#define CurrentDrawBuf world.current
#define SCRATCHPAD (&world.scratch)
#define ViewViewportClipRect world.clip
enum {ORDERING_TABLE_WORD=RR_FIELD_OFFSET(FrameOrderingPrefix,ordering70)/sizeof(u32)};
'''
TAIL=r'''
static unsigned cursor,bank;
static void put16(u8 *p,int n){u16 x=(u16)n;memcpy(p,&x,2);}
static void put32(u8 *p,u32 n){memcpy(p,&n,4);}
static void rgb(u8 *p,const u8 c[3]){memcpy(p,c,3);}
static void link_packet(unsigned depth,unsigned length){
 u32 *head=&expected.frames[bank].buffer.ordering70.words[depth];
 put32(expected.packets.bytes+cursor,((length/4-1)<<24)|(*head&0xFFFFFFU));
 *head=(*head&0xFF000000U)|((0x180010U+cursor)&0xFFFFFFU);cursor+=length;
}
static void textured(int left,int right,int y,unsigned width,int speed,const u8 color[3]){
 u8 *p=expected.packets.bytes+cursor;rgb(p+4,color);p[7]=0x2E;
 put16(p+8,left);put16(p+10,y-24);put16(p+16,right);put16(p+18,y-24);
 put16(p+24,left);put16(p+26,y+24);put16(p+32,right);put16(p+34,y+24);
 int u=(speed/2)*16+48;p[12]=(u8)u;p[13]=112;p[20]=(u8)(u+width);p[21]=112;
 p[28]=(u8)u;p[29]=160;p[36]=(u8)(u+width);p[37]=160;put16(p+14,0x7EB8);put16(p+22,0xA5);link_packet(70,40);
}
static void quad(int x,int y,int w,int h,int slanted,int reverse,const u8 *a,const u8 *b,const u8 *c,const u8 *d){
 /* Callers first narrow the rectangle to its four signed wire halfwords. */
 x=(s16)(u16)x;y=(s16)(u16)y;w=(s16)(u16)w;h=(s16)(u16)h;
 u8 *p=expected.packets.bytes+cursor;p[7]=0x3A;rgb(p+4,a);rgb(p+12,b);rgb(p+20,c);rgb(p+28,d);
 put16(p+8,x);put16(p+10,y);put16(p+16,x+w);put16(p+18,y+(slanted&&!reverse?h/2:0));
 put16(p+24,x);put16(p+26,y+h);put16(p+32,x+w);put16(p+34,y+(slanted&&reverse?h/2:h));link_packet(65,36);
}
static void reference(unsigned slot){
 const UiParticle20 *e=&expected.system.entries2C[slot];u8 colors[4][3];const u8 *input=&e->r10;
 for(unsigned channel=0;channel<3;channel++){
  int64_t level=(int64_t)input[channel]*expected.system.brightness18/256;
  level=level*expected.scene.fade3E0.scale08/256;u8 base=(u8)level;
  int values[4]={base,base/2,base/4,base/4+(base/4)/2};
  for(unsigned i=0;i<4;i++)colors[i][channel]=(u8)((int64_t)values[i]*e->scale1A/256);
 }
 cursor=0;
 if(e->velY1E>=4)textured(e->x0C-9,e->x0C+3,e->y0E,12,e->speed16,colors[3]);
 textured(e->x0C-16,e->x0C,e->y0E,16,e->speed16,colors[2]);
 if(e->velY1E>=5)textured(e->x0C+e->rot18-12,e->x0C+e->rot18+4,e->y0E,16,e->speed16,colors[0]);
 if(e->velY1E>=5){
  int width=e->rot18<24?e->rot18:24;int extra=e->rot18-width;int x=e->x0C+3+extra;int half=e->speed16/2;const u8 *clip=&expected.clip.r;
  quad(x,e->y0E-half-16,width,16,1,0,clip,clip,colors[2],colors[2]);
  quad(x,e->y0E-half,width,half,0,0,colors[1],colors[2],colors[0],colors[1]);
  quad(x,e->y0E,width,half,0,0,colors[0],colors[1],colors[1],colors[2]);
  quad(x,e->y0E+half,width,16,1,1,colors[2],colors[2],clip,clip);
  if(extra>0){
   x=e->x0C+3;
   quad(x,e->y0E-half-16,extra,16,0,0,clip,clip,colors[2],colors[2]);
   quad(x,e->y0E,extra,half,0,0,colors[0],colors[0],colors[1],colors[1]);
   quad(x,e->y0E-half,extra,half,0,0,colors[1],colors[1],colors[0],colors[0]);
   quad(x,e->y0E+half,extra,16,0,0,colors[2],colors[2],clip,clip);
  }
 }
 assert(cursor<=sizeof(expected.packets));expected.scratch.pointers.cursor=world.packets.bytes+cursor;
}
int main(void){
 const s16 positions[]={-32768,-1024,-1,0,1,1024,32767};const s16 rotations[]={-32768,-1,0,1,23,24,25,100,32767};
 const s16 speeds[]={-32768,-7,-1,0,1,7,32767};const s16 phases[]={-1,0,3,4,5,6,32767};
 const s32 levels[]={-256,-1,0,1,128,255,256,1024};
 assert(sizeof(void*)==8&&(uintptr_t)world.packets.bytes>UINT32_MAX);
 for(unsigned scenario=0;scenario<4096;scenario++){
  memset(&world,0xA5,sizeof(world));bank=scenario%2;unsigned slot=(scenario/2)%24;
  for(unsigned i=0;i<sizeof(world.packets.bytes);i++)world.packets.bytes[i]=(u8)(scenario*13+i*37);
  world.current=&world.frames[bank];world.scratch.pointers.cursor=world.packets.bytes;world.scratch.pointers.order=world.frames[bank].buffer.ordering70.words;
  world.scene.fade3E0.scale08=levels[(scenario/8)%8];world.system.brightness18=levels[(scenario/64)%8];
  world.clip=(GpuColorCommand){(u8)scenario,(u8)(scenario*3),(u8)(scenario*7),0xA5};
  UiParticle20 *p=&world.system.entries2C[slot];p->r10=(u8)scenario;p->g11=(u8)(scenario*17);p->b12=(u8)(scenario*73);
  p->x0C=positions[(scenario/3)%7];p->y0E=positions[(scenario/11)%7];p->rot18=rotations[(scenario/7)%9];p->speed16=speeds[(scenario/13)%7];p->velY1E=phases[(scenario/5)%7];p->scale1A=(s16)levels[(scenario/17)%8];
  u32 alias=(scenario&2)?0xA0000000U:0x80000000U;
  GpuDmaRegion regions[]={{world.packets.bytes,alias|0x180010U,sizeof(world.packets)}, {world.frames[0].bytes,alias|0x200000U,sizeof(FrameStorage)}, {world.frames[1].bytes,alias|0x240000U,sizeof(FrameStorage)}};
  assert(GpuDmaSetRegions(regions,3));memcpy(&expected,&world,sizeof(world));reference(slot);DrawParticle((s32)slot);
  assert(!memcmp(&world,&expected,sizeof(world)));
 }
 assert(GpuDmaSetRegions(NULL,0));return 0;
}
'''
class ParticleRenderOwnerTest(unittest.TestCase):
 def test_actual_particle_and_gradient_pipeline_full_owners_dma_and_guards(self):
  owners=source_owners(ROOT/'src/main/PAL');names=('AddVerticalGradientRect','AddGouraudRect','AddSlantedGradientRect','DrawParticle');bodies=[definition_source(owners[n].read_text(),n)for n in names];source=PREFIX+'\n'.join(bodies)+TAIL
  self.assertTrue(all(source.count(b)==1 for b in bodies))
  with TemporaryDirectory()as temporary:
   path=Path(temporary)/'particles.c';path.write_text(source);exe=path.with_suffix('')
   build=subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(path),str(ROOT/'src/native/gpu_address.c'),'-o',str(exe)],capture_output=True,text=True);self.assertEqual(build.returncode,0,build.stderr)
   run=subprocess.run([str(exe)],capture_output=True,text=True);self.assertEqual(run.returncode,0,run.stdout+run.stderr)
