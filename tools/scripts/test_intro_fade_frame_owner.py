"""Literal intro frame, tint, ripple controller and particle-fade writer.

Command expectations and independent packet/cursor checks cover full native
scene/frame/scratch owners. Glyph/tile/trig/particle services are controlled;
their internal rendering algorithms are outside this test.
"""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest
from c_functions import definition_source,source_owners
ROOT=Path(__file__).resolve().parents[2]
PREFIX=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/scene_camera_state.h"
#include "rr/frame_loop.h"
#include "rr/gpu_scratch_types.h"
#include "rr/gpu_address.h"
#include "rr/ui.h"
#include "game/gfx/fadespr.h"
#include "game/gfx/fadeset.h"
#include "game/effect/fxspr.h"
#include "game/demo/intro.h"
typedef union {GpuScratchPointers pointers;u64 alignment;u8 bytes[1024];} Scratch;
typedef union {GpuCommandPacket alignment;u8 bytes[512];} Packets;
typedef struct {
 u32 before[4];SceneRaceContext scene;u32 afterScene[4];ParticleSystem32C particles;u32 afterParticles[4];
 FrameStorage frames[2];u32 afterFrames[4];Scratch scratch;u32 afterScratch[4];Packets packets;u32 afterPackets[4];
 FrameStorage *current;DemoFadeCommand8 commands[2];u32 counter,mode;u8 palette[4];
 s32 widths[13];char texts[13][16];u32 after[4];
} World;
static World world,expected;
static unsigned scenario,particleCalls,spriteCalls,lineCalls,textCalls,tileCalls,randomCalls;
static int beforeOpacity,expectedIntensity,expectedY,expectedScale,expectedScaleMode;
#define RaceContext world.scene
#define ParticleSystem world.particles
#define CurrentDrawBuf world.current
#define DrawBuffers world.frames
#define SCRATCHPAD (&world.scratch)
#define IntroTimeline world.commands
#define FrameCounter world.counter
#define OverlayTextSpriteUv world.mode
#define HudPauseMenuTitleY world.palette
#define SparkParticleInitVel world.widths
#define HudPauseMenuItemCounts world.texts
static void put32(u8 *p,u32 n){memcpy(p,&n,4);}
void UpdateParticles(void){particleCalls++;}
s32 rsin(s32 angle){(void)angle;return 0;}
s32 rcos(s32 angle){(void)angle;return 0;}
s32 PsyqRand(void){return (s32)(scenario*13+randomCalls++);}
void PlaySfxBank2(s32 sound){(void)sound;assert(!"unexpected sound boundary");}
void EmitFadeSprite(s32 index,s32 x,s32 y,s32 intensity){
 unsigned call=spriteCalls++;unsigned command=scenario%12;
 if(call==0){assert(index==0&&x==44&&y==expectedY&&intensity==expectedIntensity);return;}
 int level=16*expectedIntensity/256;
 if(command==7){assert(call==1&&index==3&&x==66&&y==380&&intensity==level);return;}
 assert(command==8&&x==104&&y==316&&index==(call==1?2:1));
 assert(intensity==(call==1||expectedScaleMode!=0?level:0));
}
void AddLineF2Prim(const s16 coordinates[4],const u8 rgb[3],s32 blend,s32 depth){
 unsigned line=lineCalls++;assert(scenario%12==6&&line<4&&blend==0&&depth==10);
 assert(coordinates[0]==16&&coordinates[2]==34&&coordinates[1]==370+(int)(line/2)*40+(int)(line%2)&&coordinates[3]==coordinates[1]);
 int level=((255-beforeOpacity)*expectedIntensity/256)*expectedScale/256;
 for(unsigned i=0;i<3;i++)assert(rgb[i]==(u8)level);
}
void *AddTextSprites(u32 *ordering,void *cursor,s32 x,s32 y,const char *text,s32 clut,s32 spacing,s32 color){
 unsigned call=textCalls++;unsigned row=call/4,column=call%4;assert(call<8);
 assert(ordering==world.current->buffer.ordering70.words+10&&cursor==world.packets.bytes+call*16);
 assert((uintptr_t)cursor>UINT32_MAX&&text==world.texts[world.scene.fade3E0.ripplePreset58]);
 int h=row?(int)((scenario*13+1)%6)-3:(int)(scenario*13%16)-8;
 int level=world.scene.fade3E0.rippleIntensity60/(row?2:4);level=(level-16)/4;if(level<1)level=1;
 assert(x==158+(int)(column/2)-world.widths[world.scene.fade3E0.ripplePreset58]&&y==228+(int)(column%2)*2+h);
 assert(clut==0x7F38&&spacing==1&&color==((level<<1)|1));
 put32(cursor,0xACED0000U+call);if(call==3)world.current=&world.frames[1-(scenario&1)];
 return (u8*)cursor+16;
}
void *AddTileGrid2x2(u32 *ordering,void *cursor,s32 x,s32 y,s32 width,s32 height,s32 r,s32 g,s32 b){
 assert(tileCalls++==0&&ordering==world.current->buffer.ordering70.words+703);
 unsigned command=scenario%12;unsigned offset=(command==9||command==10)?136:0;
 assert(cursor==world.packets.bytes+offset&&(uintptr_t)cursor>UINT32_MAX);
 assert(x==0&&y==0&&width==320&&height==512);
 int expectedColor[3];for(unsigned i=0;i<3;i++)expectedColor[i]=((world.palette[i]*expected.scene.fade3E0.opacity0C/255)&255)*expectedScale/256;
 assert(r==expectedColor[0]&&g==expectedColor[1]&&b==expectedColor[2]);
 put32(cursor,0xCAFEBABEU);return (u8*)cursor+64;
}
'''
TAIL=r'''
static void reference(unsigned command){
 DemoFadeState64 *f=&expected.scene.fade3E0;
 switch(command){
 case 0:f->particlesEnabled14=1;break;
 case 1:expected.particles.fadeDuration08=32;expected.particles.fadeTimer0C=32;expected.particles.fadeStart10=256;expected.particles.fadeTarget14=0;expected.particles.brightness18=256;break;
 case 2:f->spriteMode18=1;break;case 3:f->spriteMode18=2;f->spriteFrame1C=0;break;
 case 4:f->opacityMode28=1;break;case 5:f->opacityMode28=2;break;
 case 6:f->linesEnabled30=1;break;case 7:f->sprite3Enabled38=1;break;
 case 8:f->sprite2Enabled34=1;f->mode3C=1;f->cycleFrame44=0;break;
 case 9:f->rippleMode54=1;f->rippleFrame5C=0;f->ripplePreset58=0;break;
 case 10:f->rippleMode54=2;f->rippleFrame5C=0;break;
 }
 if(f->scaleMode48==1){f->scale08-=16;if(f->scale08<=128){f->scale08=128;f->scaleMode48=2;}}
 if(command==9){f->rippleFrame5C=1;f->rippleIntensity60=15;}
 if(command==10){f->rippleFrame5C=1;f->rippleIntensity60=239;}
 if(f->spriteMode18==1){f->spriteIntensity24+=8;if(f->spriteIntensity24>=256){f->spriteIntensity24=256;f->spriteMode18=0;}}
 else if(f->spriteMode18==2){f->spriteFrame1C++;f->spriteY20=206-f->spriteFrame1C*66/16;}
 if(command==6)f->linesProgress32=1;
 if(command==7)f->sprite3Progress3A=1;
 if(command==8)f->sprite2Progress36=1;
 expectedIntensity=f->spriteIntensity24;expectedY=f->spriteY20;expectedScale=f->scale08;expectedScaleMode=f->scaleMode48;
 if(f->opacityMode28==1){f->opacity0C+=8;if(f->opacity0C>=255){f->opacity0C=255;f->opacityMode28=0;f->particlesEnabled14=0;}}
 else if(f->opacityMode28==2){f->opacity0C-=8;if(f->opacity0C<=0){f->opacity0C=0;f->opacityMode28=0;}}
 for(unsigned i=0;i<3;i++)f->color10[i]=(u8)(((expected.palette[i]*f->opacity0C/255)&255)*f->scale08/256);
 for(unsigned i=0;i<2;i++){expected.frames[i].environment.draw00.r=f->color10[0];expected.frames[i].environment.draw00.g=f->color10[1];expected.frames[i].environment.draw00.b=f->color10[2];}
 f->frame04++;if(f->mode3C==1)f->cycleFrame44++;
 unsigned offset=0;
 if(command==9||command==10){
  for(unsigned i=0;i<8;i++)put32(expected.packets.bytes+i*16,0xACED0000U+i);
  unsigned bank=1-(scenario&1);expected.current=&world.frames[bank];u32 *head=&expected.frames[bank].buffer.ordering70.words[10];
  put32(expected.packets.bytes+128,0x01000000U|(*head&0xFFFFFFU));put32(expected.packets.bytes+132,(expected.mode&0x9FFU)|0xE1000200U);
  *head=(*head&0xFF000000U)|0x180090U;offset=136;
 }
 put32(expected.packets.bytes+offset,0xCAFEBABEU);expected.scratch.pointers.cursor=world.packets.bytes+offset+64;
}
int main(void){
 const int opacities[]={0,7,8,127,247,248,255};const int scales[]={128,144,256};
 assert(sizeof(void*)==8&&(uintptr_t)world.packets.bytes>UINT32_MAX);
 for(scenario=0;scenario<1008;scenario++){
  unsigned command=scenario%12;memset(&world,0xA5,sizeof(world));memset(&world.scene.fade3E0,0,sizeof(DemoFadeState64));memset(&world.particles,0,sizeof(world.particles));
  particleCalls=spriteCalls=lineCalls=textCalls=tileCalls=randomCalls=0;
  world.current=&world.frames[scenario&1];world.scratch.pointers.cursor=world.packets.bytes;world.scratch.pointers.order=world.current->buffer.ordering70.words;
  world.counter=0x80000000U+scenario*7;world.mode=scenario*213U;
  world.commands[0]=(DemoFadeCommand8){(s32)scenario,(s32)command};world.commands[1]=(DemoFadeCommand8){-1,-1};
  for(unsigned i=0;i<13;i++){world.widths[i]=(s32)(i*17);memset(world.texts[i],(int)(i+'A'),16);}
  world.palette[0]=(u8)scenario;world.palette[1]=(u8)(scenario*17);world.palette[2]=(u8)(scenario*71);
  DemoFadeState64 *f=&world.scene.fade3E0;f->frame04=(s32)scenario;f->scale08=scales[(scenario/12)%3];f->opacity0C=opacities[(scenario/36)%7];f->spriteY20=140;f->spriteIntensity24=64;f->mode3C=2;f->ripplePreset58=(s32)(scenario%13);
  if(scenario&32)EnableDemoFadeSkip();
  beforeOpacity=f->opacity0C;memcpy(&expected,&world,sizeof(world));reference(command);
  GpuDmaRegion regions[]={{world.packets.bytes,0x80180010U,sizeof(world.packets)},{world.frames[0].bytes,0x80200000U,sizeof(FrameStorage)},{world.frames[1].bytes,0x80240000U,sizeof(FrameStorage)}};
  assert(GpuDmaSetRegions(regions,3));int result=IntroScriptStep();
  assert(result==(command==8?1:0));assert(particleCalls==(command==0));assert(spriteCalls==(command==7?2:command==8?3:1));assert(lineCalls==(command==6?4:0));assert(textCalls==((command==9||command==10)?8:0));assert(tileCalls==1);
  assert(!memcmp(&world,&expected,sizeof(world)));
 }
 assert(GpuDmaSetRegions(NULL,0));return 0;
}
'''
class IntroFadeFrameOwnerTest(unittest.TestCase):
 def test_literal_frame_ripple_tint_full_native_pointers_and_command_matrix(self):
  owners=source_owners(ROOT/'src/main/PAL');names=('SetParticleBrightFade','UpdateFadeTint','DrawIntroRippleSprites','IntroScriptStep','EnableDemoFadeSkip');bodies=[definition_source(owners[n].read_text(),n)for n in names];source=PREFIX+'\n'.join(bodies)+TAIL
  self.assertTrue(all(source.count(b)==1 for b in bodies))
  with TemporaryDirectory()as temporary:
   path=Path(temporary)/'intro.c';path.write_text(source);exe=path.with_suffix('');build=subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(path),str(ROOT/'src/native/gpu_address.c'),'-o',str(exe)],capture_output=True,text=True);self.assertEqual(build.returncode,0,build.stderr)
   run=subprocess.run([str(exe)],capture_output=True,text=True);self.assertEqual(run.returncode,0,run.stdout+run.stderr)
