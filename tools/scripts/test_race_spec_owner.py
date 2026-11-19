"""Real context setters, spec blending and averaging on complete owners.

Independent integer models and original/linked MIPS check every PAL field after
all six calls. Native pointers retain their width; full owner/readonly guards,
source/destination aliases, both target slots and register/stack ABI are checked.
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
NAMES=('SetTrackStartNode','SetTrackLaneOffsets','SetCarSpecTable','BlendCarSpec','AverageCarSpec')
SPECS=(0x80180010,0x80190010);GRIDS=(0x801A0010,0x801A1010);LANES=(0x801B0010,0x801B1010)
STACK,STOP=0x803F0000,0x803FF000
CONTEXT,CARS=address_of('RaceContext'),address_of('ComCars')
POINTERS=(0xE8,0x224,0x24C,0x53C,0x540,0x544,0x548,0x54C,0x550,0x554,0x558,0x55C,0x564,0x568)
SIZE=0x57C+11*800
PREFIX=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "vehicle_wire.h"
#include "rr/scene_camera_state.h"
#include "game/course/trkres2.h"
#include "game/car/carspec1.h"
#include "game/car/carspec2.h"
typedef struct {
 u32 before[4];SceneRaceContext context;u32 afterContext[4];
 RaceVehicle cars[11];u32 afterCars[4];
 RaceSpecEntry30 specs[2][12];u32 afterSpecs[4];
 RaceStartGrid60 grids[2];u32 afterGrids[4];
 TrackLaneOffset8 lanes[2][4];u32 after[4];
} World;
static World world,fixed;
#define RaceContext world.context
#define ComCars world.cars
static unsigned scenario,stage;
static void put32(u8 *p,u32 n){memcpy(p,&n,4);}
static u32 spec_address(const RaceSpecEntry30 *p){
 if(!p)return 0;
 for(unsigned b=0;b<2;b++)for(unsigned i=0;i<12;i++)if(p==&world.specs[b][i])return (b?0x80190010U:0x80180010U)+48*i;
 assert(!"unknown full native spec pointer");return 0;
}
static u32 grid_address(const RaceStartGrid60 *p){
 if(!p)return 0;
 for(unsigned i=0;i<2;i++)if(p==&world.grids[i])return i?0x801A1010U:0x801A0010U;
 assert(!"unknown grid pointer");return 0;
}
static u32 lane_address(const TrackLaneOffset8 *p){
 if(!p)return 0;
 for(unsigned i=0;i<2;i++)if(p==world.lanes[i])return i?0x801B1010U:0x801B0010U;
 assert(!"unknown lane pointer");return 0;
}
static void snapshot(u8 *out){
 const unsigned nullCells[]={0xE8,0x224,0x24C,0x550,0x554,0x558,0x55C,0x564,0x568};
 memset(out,0xA5,0x57C);
 for(unsigned i=0;i<sizeof(nullCells)/sizeof(*nullCells);i++)put32(out+nullCells[i],0);
 put32(out+0x53C,grid_address(world.context.startGrid53C));
 put32(out+0x540,spec_address(world.context.carSpecs540));
 put32(out+0x544,spec_address(world.context.specTargets544[0]));
 put32(out+0x548,spec_address(world.context.specTargets544[1]));
 put32(out+0x54C,lane_address(world.context.laneOffsets54C));
 for(unsigned i=0;i<11;i++)rr_vehicle_wire_store(out+0x57C+800*i,&world.cars[i]);
 World unchanged=world;
 memcpy(unchanged.cars,fixed.cars,sizeof(unchanged.cars));
 unchanged.context.startGrid53C=fixed.context.startGrid53C;
 unchanged.context.carSpecs540=fixed.context.carSpecs540;
 unchanged.context.laneOffsets54C=fixed.context.laneOffsets54C;
 assert(!memcmp(&unchanged,&fixed,sizeof(fixed)));
}
'''
TAIL=r'''
void begin(unsigned number,const u8 *cars,const u8 *specs,u8 *out){
 scenario=number;stage=0;memset(&world,0xA5,sizeof(world));rr_vehicle_wire_reset();
 SceneRaceContext *p=&world.context;
 p->standings=NULL;p->cameraEC.tracks138=NULL;p->camera24C=NULL;
 p->startGrid53C=NULL;p->carSpecs540=NULL;p->laneOffsets54C=NULL;
 p->cameraNodes550=NULL;p->workspace554=NULL;p->lightPresets558=NULL;p->workspace55C=NULL;
 p->soundPayload564=NULL;p->lookupTables=NULL;
 memcpy(world.specs,specs,sizeof(world.specs));
 p->specTargets544[0]=&world.specs[(number>>3)&1][number%12];
 p->specTargets544[1]=&world.specs[(number>>4)&1][(number+5)%12];
 for(unsigned i=0;i<11;i++)rr_vehicle_wire_load(&world.cars[i],cars+800*i);
 assert(sizeof(void*)==8&&(uintptr_t)&world.specs[1][11]>UINT32_MAX);
 memcpy(&fixed,&world,sizeof(world));snapshot(out);
}
void advance(s32 a,s32 b,s32 c,s32 d,u8 *out){
 switch(stage++){
 case 0:SetTrackStartNode(&world.grids[scenario&1]);break;
 case 1:SetTrackLaneOffsets(world.lanes[(scenario>>1)&1]);break;
 case 2:SetCarSpecTable(world.specs[(scenario>>2)&1]);break;
 case 3:case 5:BlendCarSpec(a,b,c,d);break;
 case 4:AverageCarSpec(a,b,c);break;
 default:assert(!"too many pipeline calls");
 }
 snapshot(out);
}
'''
def trunc(n,d):return -(abs(n)//d)if n<0 else n//d

def blend(cars,specs,targets,a,b,c,d):
 base=specs[a*48:(a+1)*48];target=targets[d];offset=b*800+0x1F4
 factor=struct.unpack_from('<h',cars,offset+0x11E)[0]
 v=trunc(c*factor,1024)if c<1025 else c-2048
 def value(at,fmt):
  x=struct.unpack_from(fmt,base,at)[0];y=struct.unpack_from(fmt,target,at)[0]
  return x+trunc((y-x)*v,1024)
 struct.pack_into('<I',cars,offset+0xF8,(value(0,'<i')*256)&0xFFFFFFFF)
 for i in range(6):
  struct.pack_into('<H',cars,offset+0x100+2*i,(value(8+2*i,'<H')*4)&65535)
  struct.pack_into('<H',cars,offset+0x10C+2*i,value(0x14+2*i,'<H')&65535)
 for pos in (0x20,0x22):struct.pack_into('<H',cars,offset+0x118+pos-0x20,value(pos,'<h')&65535)

def average(cars,x,y,z):
 x=x*800+0x1F4;y=y*800+0x1F4;z=z*800+0x1F4
 a=struct.unpack_from('<i',cars,y+0xF8)[0];b=struct.unpack_from('<i',cars,z+0xF8)[0]
 struct.pack_into('<i',cars,x+0xF8,trunc(a+b,2))
 for base in (0x100,0x10C):
  for i in range(6):
   a=struct.unpack_from('<H',cars,y+base+2*i)[0];b=struct.unpack_from('<H',cars,z+base+2*i)[0]
   struct.pack_into('<H',cars,x+base+2*i,(a+b)//2)
 for base in (0x118,0x11A):
  a=struct.unpack_from('<h',cars,y+base)[0];b=struct.unpack_from('<h',cars,z+base)[0]
  struct.pack_into('<h',cars,x+base,trunc(a+b,2))

class RaceSpecOwnerTest(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.tmp=TemporaryDirectory();cls.addClassCleanup(cls.tmp.cleanup);directory=Path(cls.tmp.name)
  owners=source_owners(ROOT/'src/main/PAL');bodies=[definition_source(owners[n].read_text(),n)for n in NAMES]
  source=PREFIX+'\n'.join(bodies)+TAIL
  assert all(source.count(body)==1 for body in bodies)
  path=directory/'owner.c';path.write_text(source);library=path.with_suffix('.so')
  r=subprocess.run(['cc','-std=c11','-O2','-shared','-fPIC','-Wall','-Wextra','-Werror','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I',str(ROOT/'include'),'-I',str(ROOT/'tools/scripts'),str(path),'-o',str(library)],capture_output=True,text=True)
  if r.returncode:raise AssertionError(r.stderr)
  cls.lib=C.CDLL(str(library));cls.lib.begin.argtypes=[C.c_uint,C.c_char_p,C.c_char_p,C.c_void_p];cls.lib.advance.argtypes=[C.c_int32]*4+[C.c_void_p]
  cls.original=(ROOT/'assets/PAL/main.exe').read_bytes()[0x800:]
  elf=ELFFile(io.BytesIO((ROOT/'build/PAL/main.elf').read_bytes()));cls.linked=[(s['sh_addr'],s.data())for s in elf.iter_sections()if s['sh_flags']&2 and s['sh_type']=='SHT_PROGBITS']
 def write(self,a,b):self.u.mem_write(a&0x1FFFFFFF,bytes(b))
 def read(self,a,n):return bytes(self.u.mem_read(a&0x1FFFFFFF,n))
 def setup_cpu(self,linked):
  self.u=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);self.u.mem_map(0,0x400000);self.write(0x80010000,self.original)
  if linked:
   for a,b in self.linked:self.write(a,b)
  self.allowed=[]
  def store(cpu,access,a,size,value,user):
   a&=0x1FFFFFFF
   self.assertTrue(any((p&0x1FFFFFFF)<=a and a+size<=(p&0x1FFFFFFF)+n for p,n in self.allowed),(hex(a),size))
  self.cpu=Cpu(self.u,0,100,101,102,store)
 def invoke(self,name,args):
  cpu=self.cpu;cpu.reset();cpu.pc=address_of(name);cpu.r[29]=STACK;cpu.r[31]=STOP
  saved={r:0xA9810000+r for r in(*range(16,24),28,30)}
  for r,v in saved.items():cpu.r[r]=v
  for i,v in enumerate(args):cpu.r[4+i]=v&0xFFFFFFFF
  for _ in range(10000):
   if cpu.pc==STOP:
    self.assertEqual(cpu.r[29],STACK);self.assertTrue(all(cpu.r[r]==v for r,v in saved.items()));return
   cpu.step()
  self.fail('instruction limit')
 def test_pipeline_full_owners_aliases_native_original_linked(self):
  rng=random.Random(0xAC650544);cases=[]
  coefficients=(-1024,-1,0,1,1024,1025,2048,3072)
  for case in range(512):
   cars=bytearray(rng.randbytes(11*800));specs=bytearray(rng.randbytes(2*12*48))
   for i in range(11):
    struct.pack_into('<II',cars,i*800+0x20C,0,0)
    struct.pack_into('<i',cars,i*800+0x2EC,rng.randrange(-100000,100000))
    struct.pack_into('<h',cars,i*800+0x312,(-2048,-1,0,1,1024,2048,32767)[(case+i)%7])
   for i in range(24):struct.pack_into('<i',specs,i*48,rng.randrange(0,1000))
   cases.append((cars,specs))
  for linked in (False,True):
   self.setup_cpu(linked)
   for case,(initial,specs)in enumerate(cases):
    cars=bytearray(initial);context=bytearray(b'\xA5'*0x57C)
    for offset in POINTERS:struct.pack_into('<I',context,offset,0)
    addresses=(SPECS[(case>>3)&1]+case%12*48,SPECS[(case>>4)&1]+(case+5)%12*48)
    for i,a in enumerate(addresses):struct.pack_into('<I',context,0x544+4*i,a)
    targets=[specs[((case>>3)&1)*576+case%12*48:((case>>3)&1)*576+(case%12+1)*48],specs[((case>>4)&1)*576+(case+5)%12*48:((case>>4)&1)*576+((case+5)%12+1)*48]]
    self.write(CONTEXT,context);self.write(CARS,cars)
    for b,a in enumerate(SPECS):self.write(a,specs[b*576:(b+1)*576])
    for a in(*GRIDS,*LANES):self.write(a,b'\xA5'*0x60)
    out=C.create_string_buffer(SIZE);self.lib.begin(case,bytes(cars),bytes(specs),out)
    self.assertEqual(out.raw,bytes(context+cars))
    dest=case%11;other=(case+7)%11;avg=(dest if case&1 else(case+3)%11)
    calls=[('SetTrackStartNode',(GRIDS[case&1],),None),('SetTrackLaneOffsets',(LANES[(case>>1)&1],),None),('SetCarSpecTable',(SPECS[(case>>2)&1],),None),('BlendCarSpec',(case%12,dest,coefficients[case%8],case&1),None),('AverageCarSpec',(avg,dest,other),None),('BlendCarSpec',((case+5)%12,avg,coefficients[(case+3)%8],1-(case&1)),None)]
    for stage,(name,args,_)in enumerate(calls):
     self.allowed=[(STACK-1024,1088)]
     if stage<3:
      offset=(0x53C,0x54C,0x540)[stage];self.allowed.append((CONTEXT+offset,4));struct.pack_into('<I',context,offset,args[0])
     else:
      index=args[1]if name=='BlendCarSpec'else args[0];self.allowed.append((CARS+index*800+0x2EC,0x24))
      if name=='BlendCarSpec':blend(cars,specs[((case>>2)&1)*576:(((case>>2)&1)+1)*576],targets,*args)
      else:average(cars,*args)
     native_args=(0,0,0,0)if stage<3 else tuple(args)+(0,)*(4-len(args))
     self.lib.advance(*native_args,out);expected=bytes(context+cars)
     self.assertEqual(out.raw,expected,(case,stage,'native'))
     self.invoke(name,args);self.assertEqual(self.read(CONTEXT,0x57C)+self.read(CARS,8800),expected,(case,stage,'linked'if linked else'original'))
     for b,a in enumerate(SPECS):self.assertEqual(self.read(a,576),specs[b*576:(b+1)*576])
