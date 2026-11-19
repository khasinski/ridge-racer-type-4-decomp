"""Literal fade/particle producers on one full scene and audio owner.

An independent integer model checks native C and original/linked MIPS after
all service boundaries and each pipeline call. Complete native owners retain
guards; no production body is rewritten. Hardware/audio services are controlled.
"""
import ctypes as C
import io
from pathlib import Path
import random
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest
from elftools.elf.elffile import ELFFile
from unicorn import Uc, UC_ARCH_MIPS, UC_MODE_MIPS32, UC_MODE_LITTLE_ENDIAN
from c_functions import address_of, definition_source, source_owners
from test_vehicle_initializer import Cpu
ROOT=Path(__file__).resolve().parents[2]
NAMES=('InitDemoFadeContext','InitParticles','SetParticleBrightFade','ResetDemoFadeState','StartDemoFadeIn','SelectDemoFadePreset','UpdateFadeTint','StartDemoFadeOutAndCloseVab','EnableDemoFadeSkip')
OPS=('InitDemoFadeContext','ResetDemoFadeState','StartDemoFadeIn','SelectDemoFadePreset','UpdateFadeTint','StartDemoFadeOutAndCloseVab','EnableDemoFadeSkip','InitParticles')
STACK,STOP=0x803F0000,0x803FF000
STATE=972
PREFIX=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/scene_camera_state.h"
#include "rr/audio_state_types.h"
#include "rr/ui.h"
#include "game/gfx/fadespr.h"
#include "game/gfx/fadeset.h"
#include "game/gfx/fadectl.h"
#include "game/gfx/fadein.h"
#include "game/gfx/fadeend.h"
#include "game/effect/ptinit.h"
struct OptionsDefaultTail16 {int a[4];};
typedef struct {
 u32 before[4];SceneRaceContext scene;u32 afterScene[4];
 AudioState audio;u32 afterAudio[4];ParticleSystem32C particles;u32 afterParticles[4];
 s32 vsync;struct OptionsDefaultTail16 tail;u8 palette[4];u32 after[4];
} World;
static World world,fixed;
#define RaceContext world.scene
#define SoundState world.audio
#define ParticleSystem world.particles
#define VSyncThreshold world.vsync
#define OptionsDefaultTailData world.tail
#define HudPauseMenuTitleY world.palette
static unsigned scenario,eventCount;
enum {STATE=972};
typedef struct {s32 kind,args[4];u8 state[STATE];} Event;
static Event events[16];
static void snapshot(u8 *p){
 memcpy(p,&world.scene.sceneStateB0,4);p+=4;memcpy(p,&world.scene.fade3E0,100);p+=100;
 memcpy(p,&world.scene.sceneStep444,4);p+=4;memcpy(p,&world.particles,0x32C);p+=0x32C;
 memcpy(p,&world.audio.banks24,20);p+=20;memcpy(p,&world.audio.seqHandle2864,4);p+=4;
 memcpy(p,&world.audio.seqTrack2868,4);p+=4;memcpy(p,&world.vsync,4);p+=4;
 memcpy(p,&world.tail,16);p+=16;memcpy(p,world.palette,4);
 World other=world;other.scene.sceneStateB0=fixed.scene.sceneStateB0;
 other.scene.fade3E0=fixed.scene.fade3E0;other.scene.sceneStep444=fixed.scene.sceneStep444;
 other.audio.banks24=fixed.audio.banks24;other.audio.seqHandle2864=fixed.audio.seqHandle2864;
 other.particles=fixed.particles;other.audio.seqTrack2868=fixed.audio.seqTrack2868;other.vsync=fixed.vsync;other.tail=fixed.tail;
 assert(!memcmp(&other,&fixed,sizeof(fixed)));
}
static void record(s32 kind,s32 a,s32 b,s32 c,s32 d){
 assert(eventCount<16);Event *e=&events[eventCount++];e->kind=kind;e->args[0]=a;e->args[1]=b;e->args[2]=c;e->args[3]=d;snapshot(e->state);
}
static void changed(unsigned kind){
 if(kind==0&&(scenario&1))for(unsigned i=0;i<4;i++)world.tail.a[i]=(world.tail.a[i]+3)%10;
 if(kind==1&&(scenario&2))world.scene.fade3E0.opacityMode28=2;
 if(kind==2&&(scenario&4))world.audio.seqHandle2864=(s32)((u32)world.audio.seqHandle2864^0x7FFF0001U);
 if(kind==4&&(scenario&8))world.audio.banks24.ids[3]=-77;
 if(kind==5&&(scenario&16))world.scene.fade3E0.scale08=128;
 if(kind==6&&(scenario&32)){world.scene.fade3E0.opacityMode28=2;world.scene.fade3E0.soundPlayed50=0;}
}
void PsyqBzero(void *p,s32 n){
 s32 which=p==&world.scene.fade3E0?0:1;
 assert((which==0&&n==sizeof(DemoFadeState64))||(which==1&&p==&world.particles&&n==sizeof(world.particles)));
 assert((uintptr_t)p>UINT32_MAX);record(0,which,n,0,0);memset(p,0,(size_t)n);changed(0);
}
void SetupDrawDispEnvs(s32 r,s32 g,s16 b,u32 flags){record(1,r,g,b,(s32)flags);changed(1);}
void SsSeqStop(s16 handle){record(2,handle,0,0,0);changed(2);}
void SsSeqClose(s32 handle){record(3,handle,0,0,0);changed(3);}
void SsVabClose(s16 handle){record(4,handle,0,0,0);changed(4);}
void ResetSeqFadeState(void){record(5,0,0,0,0);changed(5);}
void PlaySfxBank2(s32 sound){record(6,sound,0,0,0);changed(6);}
'''
TAIL=r'''
void begin(unsigned number,const u8 *p){
 memset(&world,0xA5,sizeof(world));scenario=number;eventCount=0;
 SceneRaceContext *s=&world.scene;s->standings=NULL;s->cameraEC.tracks138=NULL;s->camera24C=NULL;
 s->startGrid53C=NULL;s->carSpecs540=NULL;s->specTargets544[0]=NULL;s->specTargets544[1]=NULL;s->laneOffsets54C=NULL;
 s->cameraNodes550=NULL;s->workspace554=NULL;s->lightPresets558=NULL;s->workspace55C=NULL;s->soundPayload564=NULL;s->lookupTables=NULL;
 memcpy(&s->sceneStateB0,p,4);p+=4;memcpy(&s->fade3E0,p,100);p+=100;memcpy(&s->sceneStep444,p,4);p+=4;
 memcpy(&world.particles,p,0x32C);p+=0x32C;memcpy(&world.audio.banks24,p,20);p+=20;memcpy(&world.audio.seqHandle2864,p,4);p+=4;
 memcpy(&world.audio.seqTrack2868,p,4);p+=4;memcpy(&world.vsync,p,4);p+=4;
 memcpy(&world.tail,p,16);p+=16;memcpy(world.palette,p,4);memcpy(&fixed,&world,sizeof(world));
}
unsigned advance(unsigned op,s32 arg,u8 *out){
 eventCount=0;
 switch(op){case 0:InitDemoFadeContext(arg);break;case 1:ResetDemoFadeState();break;case 2:StartDemoFadeIn(arg);break;
 case 3:SelectDemoFadePreset(arg);break;case 4:UpdateFadeTint(arg);break;case 5:StartDemoFadeOutAndCloseVab();break;
 case 6:EnableDemoFadeSkip();break;case 7:InitParticles();break;default:assert(0);}
 memcpy(out,&eventCount,4);unsigned n=eventCount*sizeof(Event);memcpy(out+4,events,n);snapshot(out+4+n);return 4+n+STATE;
}
'''
def sint(n,bits=32):return C.c_int16(n).value if bits==16 else C.c_int32(n).value

def trunc(n,d):return -(abs(n)//d)if n<0 else n//d

class Model:
 def __init__(self,state,scenario):self.s=bytearray(state);self.scenario=scenario;self.events=[]
 def word(self,p):return struct.unpack_from('<i',self.s,p)[0]
 def put(self,p,n):struct.pack_into('<I',self.s,p,n&0xFFFFFFFF)
 def half(self,p):return struct.unpack_from('<h',self.s,p)[0]
 def short(self,p,n):struct.pack_into('<H',self.s,p,n&65535)
 def f(self,p):return self.word(4+p)
 def setf(self,p,n):self.put(4+p,n)
 def record(self,kind,*args):self.events.append(struct.pack('<5i',kind,*(tuple(args)+(0,)*(4-len(args))))+bytes(self.s))
 def change(self,kind):
  if kind==0 and self.scenario&1:
   for i in range(4):self.put(952+4*i,(self.word(952+4*i)+3)%10)
  if kind==1 and self.scenario&2:self.setf(0x28,2)
  if kind==2 and self.scenario&4:self.put(940,self.word(940)^0x7FFF0001)
  if kind==4 and self.scenario&8:self.short(926,-77)
  if kind==5 and self.scenario&16:self.setf(8,128)
  if kind==6 and self.scenario&32:self.setf(0x28,2);self.setf(0x50,0)
 def service(self,kind,*args):self.record(kind,*args);self.change(kind)
 def clear(self,which):
  size=(100,0x32C)[which];self.record(0,which,size);start=(4,108)[which];self.s[start:start+size]=b'\0'*size;self.change(0)
 def particles(self):
  self.service(1,0,0,0,3);self.put(948,128);self.put(108,0);self.clear(1);self.put(108+24,256)
 def bright(self,duration,start,target):
  for offset,value in((8,duration),(12,duration),(16,start),(20,target),(24,start)):self.put(108+offset,value)
 def reset(self):
  self.setf(0x54,0);self.setf(0x60,0)
  if self.f(0x28)==1 or(self.f(0x28)==0 and self.f(12)==0):
   if self.f(0x14)==0:self.bright(1,0,256);self.setf(0x14,1)
  if self.f(0x28)==2 or(self.f(0x28)==0 and self.f(12)==255):
   if self.f(0x14):self.bright(0,0,0);self.setf(0x14,0)
  for off,value in((0x20,140),(0x24,256),(0x18,0),(0x3C,1),(4,3000),(0x44,0)):self.setf(off,value)
  for off,value in((0x30,1),(0x32,16),(0x3A,16),(0x36,16),(0x38,1),(0x34,1)):self.short(4+off,value)
  if not self.f(0x50):self.service(6,self.f(0x4C));self.setf(0x50,1)
 def call(self,op,arg):
  self.events=[]
  if op==0:
   captured=[self.word(952+4*i)for i in range(4)];self.clear(0);self.setf(0x20,206);self.setf(0x4C,arg);self.setf(8,256);self.put(0,0);self.particles()
   if self.word(940)>=0:self.service(2,sint(self.word(940),16));self.service(3,sint(self.word(940),16));self.put(940,-1)
   for bank in captured:
    handle=self.half(920+2*bank)
    if handle>=0:self.service(4,handle);self.short(920+2*bank,-1)
   self.service(5);self.put(944,-1)
  elif op==1:self.reset()
  elif op==2:
   if self.f(0x3C)==0:self.reset()
   self.setf(0x58,arg+2);self.setf(0x54,1);self.setf(0x5C,0)
   if self.f(0x28)==1 or(self.f(0x28)==0 and self.f(12)==255):self.setf(0x28,2);self.setf(0,1);self.bright(32,0,256);self.setf(0x14,1)
   self.setf(0x18,3);self.setf(0x3C,2)
  elif op==3:
   for off in(0x24,0x18,0x5C,0x60):self.setf(off,0)
   self.setf(0x54,1)
   if arg in(0,1,2):self.setf(0x58,(11,8,12)[arg])
   self.setf(0x14,1);self.setf(4,3000);self.setf(0x3C,2)
  elif op==4:
   self.s[20]=trunc(self.s[968]*arg,255)&255;self.s[21]=trunc(self.s[969]*arg,255)&255
   first=self.s[20]*self.f(8);self.s[22]=trunc(self.s[970]*arg,255)&255
   second=self.s[21]*self.f(8);self.s[20]=trunc(first,256)&255
   third=self.s[22]*self.f(8);self.s[21]=trunc(second,256)&255;self.s[22]=trunc(third,256)&255
  elif op==5:
   self.setf(0x54,2);self.setf(0x5C,0);self.bright(32,256,0);self.setf(0x3C,3);self.setf(0x40,0)
   handle=self.half(934)
   if handle>=0:self.service(4,handle);self.short(934,-1)
  elif op==6:self.setf(0x48,1)
  elif op==7:self.particles()
  return struct.pack('<I',len(self.events))+b''.join(self.events)+bytes(self.s)

class DemoFadeOwnerTest(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.tmp=TemporaryDirectory();cls.addClassCleanup(cls.tmp.cleanup);directory=Path(cls.tmp.name);owners=source_owners(ROOT/'src/main/PAL');bodies=[definition_source(owners[n].read_text(),n)for n in NAMES];source=PREFIX+'\n'.join(bodies)+TAIL
  assert all(source.count(b)==1 for b in bodies);
  path=directory/'fade.c';path.write_text(source);library=path.with_suffix('.so');r=subprocess.run(['cc','-std=c11','-O2','-shared','-fPIC','-Wall','-Wextra','-Werror','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(path),'-o',str(library)],capture_output=True,text=True)
  if r.returncode:raise AssertionError(r.stderr)
  cls.lib=C.CDLL(str(library));cls.lib.begin.argtypes=[C.c_uint,C.c_char_p];cls.lib.advance.argtypes=[C.c_uint,C.c_int32,C.c_void_p];cls.lib.advance.restype=C.c_uint
  cls.original=(ROOT/'assets/PAL/main.exe').read_bytes()[0x800:];elf=ELFFile(io.BytesIO((ROOT/'build/PAL/main.elf').read_bytes()));cls.linked=[(s['sh_addr'],s.data())for s in elf.iter_sections()if s['sh_flags']&2 and s['sh_type']=='SHT_PROGBITS']
 def write(self,a,b):self.u.mem_write(a&0x1FFFFFFF,bytes(b))
 def read(self,a,n):return bytes(self.u.mem_read(a&0x1FFFFFFF,n))
 def setup_cpu(self,linked):
  self.u=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);self.u.mem_map(0,0x400000);self.write(0x80010000,self.original)
  if linked:
   for a,b in self.linked:self.write(a,b)
  self.scene=address_of('RaceContext');self.audio=address_of('SoundState');self.particles=address_of('ParticleSystem');self.motion=address_of('MotionSequenceIndex');self.vsync=address_of('VSyncThreshold');self.skip=address_of('DemoFadeSkipEnabled');self.tail=address_of('OptionsDefaultTailData');self.palette=address_of('HudPauseMenuTitleY')
  self.services={address_of(n):i for i,n in enumerate(('PsyqBzero','SetupDrawDispEnvs','SsSeqStop','SsSeqClose','SsVabClose','ResetSeqFadeState','PlaySfxBank2'))}
  ranges=[(self.scene+0xB0,4),(self.scene+0x3E0,100),(self.particles,0x32C),(self.audio+0x24,20),(self.audio+0x2864,4),(self.motion,4),(self.vsync,4),(STACK-1024,1088)]
  def store(cpu,access,a,n,value,user):
   a&=0x1FFFFFFF;self.assertTrue(any((p&0x1FFFFFFF)<=a and a+n<=(p&0x1FFFFFFF)+z for p,z in ranges),(hex(a),n))
  self.cpu=Cpu(self.u,0,100,101,102,store)
 def snapshot(self):return self.read(self.scene+0xB0,4)+self.read(self.scene+0x3E0,100)+self.read(self.scene+0x444,4)+self.read(self.particles,0x32C)+self.read(self.audio+0x24,20)+self.read(self.audio+0x2864,4)+self.read(self.motion,4)+self.read(self.vsync,4)+self.read(self.tail,16)+self.read(self.palette,4)
 def invoke(self,name,arg,scenario):
  cpu=self.cpu;cpu.reset();cpu.pc=address_of(name);cpu.r[29]=STACK;cpu.r[31]=STOP;cpu.r[4]=arg&0xFFFFFFFF;events=[]
  saved={r:0xA7310000+r for r in(*range(16,24),28,30)}
  for r,v in saved.items():cpu.r[r]=v
  for _ in range(20000):
   if cpu.pc==STOP:
    self.assertEqual(cpu.r[29],STACK);self.assertTrue(all(cpu.r[r]==v for r,v in saved.items()));return struct.pack('<I',len(events))+b''.join(events)+self.snapshot()
   if cpu.pc in self.services:
    kind=self.services[cpu.pc];args=[sint(v)for v in cpu.r[4:8]];before=self.snapshot();m=Model(before,scenario)
    if kind==0:
     which=0 if cpu.r[4]==self.scene+0x3E0 else 1;self.assertEqual(cpu.r[4],(self.scene+0x3E0,self.particles)[which]);self.assertEqual(args[1],(100,0x32C)[which]);m.clear(which)
    elif kind==1:m.service(1,args[0],args[1],sint(args[2],16),args[3])
    elif kind in(2,4):m.service(kind,sint(args[0],16))
    elif kind in(3,6):m.service(kind,args[0])
    else:m.service(kind)
    events.extend(m.events);self.install(bytes(m.s));return_to=cpu.r[31]
    for r in(*range(1,16),24,25):cpu.r[r]=0xA5CD0000+r
    cpu.r[2]=0;cpu.hi=0x92AACCCC;cpu.lo=0x5822BBBB;cpu.pc=return_to;continue
   cpu.step()
  self.fail('instruction limit')
 def install(self,state):
  spans=((self.scene+0xB0,4),(self.scene+0x3E0,100),(self.scene+0x444,4),(self.particles,0x32C),(self.audio+0x24,20),(self.audio+0x2864,4),(self.motion,4),(self.vsync,4),(self.tail,16),(self.palette,4));pos=0
  for a,n in spans:self.write(a,state[pos:pos+n]);pos+=n
  self.assertEqual(pos,STATE)
 def test_full_owner_pipeline_live_services_native_original_linked(self):
  rng=random.Random(0x3E064);inputs=[]
  for case in range(256):
   state=bytearray(rng.randbytes(STATE));struct.pack_into('<i',state,12,(-4096,-1,0,1,128,256,4096)[case%7]);struct.pack_into('<i',state,940,(-1,0,1,32767,65535,0x7FFFFFFF)[case%6])
   for offset,value in((0x28,case%4),(0x0C,(0,255,-1,8)[(case//4)%4]),(0x14,case%2),(0x50,(case//2)%2),(0x3C,(case//8)%4)):
    struct.pack_into('<i',state,4+offset,value)
   for i in range(10):struct.pack_into('<h',state,920+2*i,(-1,0,1,127,32767)[(case+i)%5])
   for i in range(4):struct.pack_into('<i',state,952+4*i,(case+3*i)%10)
   inputs.append(bytes(state))
  for linked in(False,True):
   self.setup_cpu(linked)
   for case,initial in enumerate(inputs):
    self.write(self.scene,b'\xA5'*0x57C);self.write(self.audio,b'\xA5'*0xA8D0);self.install(initial);self.lib.begin(case,initial);model=Model(initial,case)
    calls=((2,case%13-6),(1,0),(4,(-32768,-1,0,255,32767)[case%5]),(0,sint(case*12345)),(1,0),(2,(-1000,-1,0,1,1000)[case%5]),(3,case%5),(4,(-32768,-1,0,1,128,255,256,32767)[case%8]),(5,0),(6,0),(7,0))
    for op,arg in calls:
     expected=model.call(op,arg);output=C.create_string_buffer(20000);length=self.lib.advance(op,arg,output);self.assertEqual(output.raw[:length],expected,(case,op,'native'))
     self.assertEqual(self.invoke(OPS[op],arg,case),expected,(case,op,'linked'if linked else'original'))
