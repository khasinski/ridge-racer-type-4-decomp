"""Track projection controller against original and linked MIPS.

Services are stateful ABI mocks. Incoming aggregate padding is not a coordinate;
only the low signed halfword consumed by the resident assembly is compared.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import hashlib,json,os,struct,subprocess
from elftools.elf.elffile import ELFFile
from unicorn import Uc,UC_ARCH_MIPS,UC_MODE_MIPS32,UC_MODE_LITTLE_ENDIAN,UC_HOOK_CODE,UC_HOOK_MEM_WRITE
from unicorn.mips_const import *
ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'src/overlays/PAL/movie_playback/func_80115460.c'
CASES=512
C=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "rr/movie_playback_track_project.h"
static struct {u8 pre[16];RrReplayCameraContextPrefix c;u8 post[16];} camera;
static struct {u8 pre[16];RrTrackSample3C table[3][8];u8 post[16];} samples,saved;
#define RaceContext camera.c
RrTrackSample3C *TrackNodes;
s32 TrackNodeCount;
static u8 expected[0x4B8];
static unsigned scenario,event;
static s32 initial[3],oldnode,count;
static const s32 values[8]={(-2147483647-1),-7,-4,-1,0,1,7,2147483647};
static const s32 counts[8]={7,0,-1,2147483647,(-2147483647-1),1,65535,-2};
static s32 word(u32 x){s32 v;memcpy(&v,&x,4);return v;}
static s16 half(u16 x){s16 v;memcpy(&v,&x,2);return v;}
static void check(void){assert(!memcmp(&camera.c,expected,sizeof expected));}
static void change(unsigned salt){unsigned i;for(i=0;i<sizeof expected;i++)expected[i]=(u8)(scenario*29+i*7+salt);memcpy(&camera.c,expected,sizeof expected);}
s32 FindTrackNode(s32 *position,s32 node,s32 n){
 unsigned i;assert(event++==0);check();assert(node==oldnode&&n==count);
 assert((uintptr_t)position>UINT32_MAX);
 for(i=0;i<3;i++)assert(position[i]==initial[i]>>2);
 change(31);TrackNodes=samples.table[(scenario+1)%3];
 /* The local search vector may be overwritten; later calculations use globals. */
 for(i=0;i<3;i++)position[i]=word(0xDEADBEEF+i);
 return scenario%8;
}
void ProjectToTrackSegment(RrTrackOffsetArgument x,RrTrackOffsetArgument z,s32 node,s16 *fraction,s16 *lateral,s32 *height){
 s16 a,b;u32 n=scenario%8;
 assert(event++==1&&node==(s32)n&&fraction==&camera.c.fraction49C&&lateral==&camera.c.lateral49E);
 memcpy(expected+0x498,&n,4);check();assert((uintptr_t)height>UINT32_MAX);
 a=half((u16)(camera.c.position450[0]/4-(u16)samples.table[(scenario+1)%3][n].x08));
 b=half((u16)(camera.c.position450[2]/4-(u16)samples.table[(scenario+1)%3][n].z0C));
 assert(x.value==a&&z.value==b);
 change(63);*fraction=half(scenario*37);*lateral=half(~(scenario*71));*height=word(scenario*0x9E3779B9U);
 memcpy(expected,&camera.c,sizeof expected);TrackNodes=samples.table[(scenario+2)%3];TrackNodeCount=word(~scenario);
}
'''
TAIL=r'''
int main(void){unsigned i,b,j;_Static_assert(sizeof(void*)==8,"full native pointers");
 _Static_assert(sizeof(RrTrackOffsetArgument)==4&&_Alignof(RrTrackOffsetArgument)==2,"PAL aggregate ABI");
 for(scenario=0;scenario<512;scenario++){
  memset(&camera,0xA5,sizeof camera);memset(&samples,0x5A,sizeof samples);change(0);
  for(i=0;i<3;i++)camera.c.position450[i]=initial[i]=values[(scenario+i*3)%8];
  camera.c.node498=oldnode=scenario%8;TrackNodeCount=counts[(scenario/8)%8];count=word((u32)TrackNodeCount+1);
  for(b=0;b<3;b++)for(j=0;j<8;j++){
   u8 *p=(u8*)&samples.table[b][j];for(i=0;i<60;i++)p[i]=(u8)(scenario*11+b*17+j*19+i*3);
   samples.table[b][j].x08=word(scenario*0x9E3779B9U+b*65537+j*97);
   samples.table[b][j].z0C=word(~(scenario*0x85EBCA6BU+b*131071+j*193));
  }
  saved=samples;TrackNodes=samples.table[scenario%3];event=0;memcpy(expected,&camera.c,sizeof expected);
  movie_playback_camera_locate_track(0);assert(event==2);check();assert(!memcmp(&samples,&saved,sizeof samples));
  for(i=0;i<16;i++)assert(camera.pre[i]==0xA5&&camera.post[i]==0xA5);
  assert(TrackNodes==samples.table[(scenario+2)%3]);
  printf("%08x %u ",(u32)TrackNodeCount,8*((scenario+2)%3));
  for(i=0;i<sizeof expected;i++)printf("%02x",((u8*)&camera.c)[i]);puts("");
 }return 0;
}
'''
def signed(v,bits=32):
 v&=(1<<bits)-1
 return v-(1<<bits) if v&(1<<(bits-1)) else v
def trunc4(v):return v//4 if v>=0 else -((-v)//4)
def main():
 with TemporaryDirectory() as tmp:
  c=Path(tmp)/'test.c';exe=Path(tmp)/'test';c.write_text(C+SOURCE.read_text()+TAIL)
  subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(c),'-o',str(exe)],check=True)
  native=subprocess.check_output([str(exe)],text=True).splitlines()
 blob=(ROOT/'disc/PAL/R4.BIN').read_bytes();sector=struct.unpack_from('<I',blob,4+668*4)[0];off=sector*2048+0x80115460-0x80114878;code=blob[off:off+244]
 elf=os.environ.get('RR_COMPILED_ELF')
 if elf:
  with open(elf,'rb') as f:code=ELFFile(f).get_section_by_name('.text').data()
 uc=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);uc.mem_map(0,0x200000)
 start=0x80115460;ctx=0x800AC650;root=0x800ABEA4;countaddr=0x800ABECC;stop=0x80010000;sp=0x801F0000
 table=lambda b:0x800D1000+b*8*60
 write=lambda a,b:uc.mem_write(a&0x1fffffff,bytes(b))
 read=lambda a,n:bytes(uc.mem_read(a&0x1fffffff,n))
 w32=lambda a,v:write(a,struct.pack('<I',v&0xffffffff))
 r32=lambda a:struct.unpack('<I',read(a,4))[0]
 reg=lambda i:uc.reg_read(UC_MIPS_REG_0+i)&0xffffffff
 put=lambda i,v:uc.reg_write(UC_MIPS_REG_0+i,v&0xffffffff)
 write(start,code);state={};seen_offsets=set();seen_counts=set();seen_positions=set()
 def hook(u,pc,size,_):
  if pc not in [0x80026A74,0x800464E8,stop]:return
  if pc==stop:u.emu_stop();return
  s=state;scenario=s['scenario'];expected=s['expected']
  if pc==0x80026A74:
   assert s['event']==0 and read(ctx,0x4B8)==expected,(scenario,s['event'],hex(reg(31)),hex(reg(4)),[(i,a,b) for i,(a,b) in enumerate(zip(read(ctx,0x4B8),expected)) if a!=b][:12])
   assert struct.unpack('<3i',read(reg(4),12))==tuple(v>>2 for v in s['initial'])
   assert reg(5)==scenario%8 and reg(6)==s['count']
   seen_counts.add(reg(6));seen_positions.update(s['initial'])
   expected=bytearray((scenario*29+i*7+31)&255 for i in range(0x4B8));write(ctx,expected);w32(root,table((scenario+1)%3))
   for i in range(3):w32(reg(4)+i*4,0xDEADBEEF+i)
   result=scenario%8;s['event']=1
  else:
   n=scenario%8;expected[0x498:0x49C]=struct.pack('<I',n)
   assert s['event']==1 and read(ctx,0x4B8)==expected
   x=signed(trunc4(signed(r32(ctx+0x450)))-(r32(table((scenario+1)%3)+n*60+8)&65535),16)
   z=signed(trunc4(signed(r32(ctx+0x458)))-(r32(table((scenario+1)%3)+n*60+12)&65535),16)
   assert signed(reg(4),16)==x and signed(reg(5),16)==z and reg(6)==n
   assert reg(7)==ctx+0x49C and r32(reg(29)+16)==ctx+0x49E
   out=r32(reg(29)+20);assert sp-64<=out and out+4<=sp
   seen_offsets.add((x<0,z<0));expected=bytearray((scenario*29+i*7+63)&255 for i in range(0x4B8));write(ctx,expected)
   write(ctx+0x49C,struct.pack('<HH',(scenario*37)&65535,(~(scenario*71))&65535));expected[0x49C:0x4A0]=read(ctx+0x49C,4)
   w32(out,scenario*0x9E3779B9);w32(root,table((scenario+2)%3));w32(countaddr,~scenario);result=0;s['event']=2
  s['expected']=expected;ret=reg(31)
  for r in [2,3,4,5,6,7,8,9,10,11,12,13,14,15,24,25]:put(r,0xDEAD0000+r)
  put(2,result);u.reg_write(UC_MIPS_REG_HI,0x12345678);u.reg_write(UC_MIPS_REG_LO,0x87654321);s['resume']=ret;u.emu_stop()
 def store(u,access,address,size,value,_):
  a=address&0x1fffffff
  assert ((ctx+0x498)&0x1fffffff)<=a and a+size<=((ctx+0x49C)&0x1fffffff) or (sp&0x1fffffff)-64<=a and a+size<=(sp&0x1fffffff),hex(address)
 uc.hook_add(UC_HOOK_CODE,hook);uc.hook_add(UC_HOOK_MEM_WRITE,store)
 values=[-2147483648,-7,-4,-1,0,1,7,2147483647];counts=[7,0,-1,2147483647,-2147483648,1,65535,-2]
 for scenario in range(CASES):
  write(ctx-16,b'\xA5'*(0x4B8+32));expected=bytearray((scenario*29+i*7)&255 for i in range(0x4B8));initial=[values[(scenario+i*3)%8] for i in range(3)]
  expected[0x450:0x45C]=struct.pack('<3i',*initial);expected[0x498:0x49C]=struct.pack('<I',scenario%8);write(ctx,expected)
  w32(countaddr,counts[(scenario//8)%8]);w32(root,table(scenario%3))
  write(table(0)-16,b'\x5A'*(1440+32))
  for b in range(3):
   for j in range(8):
    p=table(b)+j*60;write(p,bytes((scenario*11+b*17+j*19+i*3)&255 for i in range(60)))
    w32(p+8,scenario*0x9E3779B9+b*65537+j*97);w32(p+12,~(scenario*0x85EBCA6B+b*131071+j*193))
  saved=read(table(0)-16,1472)
  state.update(scenario=scenario,event=0,expected=expected,initial=initial,count=(counts[(scenario//8)%8]+1)&0xffffffff)
  for r in range(1,32):put(r,0xBEEF0000+r)
  put(4,0);put(29,sp);put(31,stop);state['resume']=start
  for boundary in range(3):uc.emu_start(state['resume'],stop,count=1000)
  assert uc.reg_read(UC_MIPS_REG_PC)==stop and state['event']==2
  line=f'{r32(countaddr):08x} {(r32(root)-table(0))//60} {read(ctx,0x4B8).hex()}'
  assert line==native[scenario],scenario
  assert saved==read(table(0)-16,1472) and read(ctx-16,16)==b'\xA5'*16 and read(ctx+0x4B8,16)==b'\xA5'*16
  for r in range(16,24):assert reg(r)==0xBEEF0000+r
  assert reg(29)==sp
 assert len(seen_offsets)==4 and len(seen_counts)==8 and seen_positions==set(values)
 sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
 deps=[SOURCE,Path(__file__).resolve()]+list((ROOT/'include').rglob('*.h'))
 audit=dict(cases=CASES,engine='compiled MIPS' if elf else 'original MIPS',source_sha256=sha(SOURCE),test_sha256=sha(__file__),full_host_pointers=True,full_owner_guards=True,caller_save_hi_lo_poison=True,mips_store_bounds=True,position_profiles=values,count_profiles=sorted(seen_counts),offset_sign_profiles=sorted(seen_offsets),aggregate_padding='ignored high halfword; consumed low signed halfword compared',sanitizers=['undefined','bounds'],dependencies={str(p):sha(p) for p in deps})
 if elf:audit['elf_sha256']=sha(elf)
 if os.environ.get('RR_AUDIT_JSON'):Path(os.environ['RR_AUDIT_JSON']).write_text(json.dumps(audit,indent=2)+'\n')
 print(f'PASS {CASES} {audit["engine"]}/native cases; live camera and track roots, aggregate ABI')
if __name__=='__main__':main()
