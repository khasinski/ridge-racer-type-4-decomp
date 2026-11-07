"""Garage animation controller, native C versus scheduled MIPS instructions.

The GTE command is an opaque stateful test boundary: transfer inputs and output
stores are checked, while GTE arithmetic, timing, FLAG and IR3 are not emulated.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import hashlib,json,os,re,struct,subprocess
from elftools.elf.elffile import ELFFile
from unicorn.mips_const import UC_MIPS_REG_0,UC_MIPS_REG_PC
from unicorn import Uc,UC_ARCH_MIPS,UC_MODE_MIPS32,UC_MODE_LITTLE_ENDIAN
from mips_preview_engine import Mips,signed
ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'src/overlays/PAL/garage/func_8011DA90.c'
CASES=4096
C=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#define RR_GARAGE_GTE_LERP(f,s,d) interpolate(f,s,d)
#include "rr/overlay662_garage_fade.h"
typedef struct {u8 pre[16];RrGarageWorkingStorage work;RrMenuItemState5C pads[2];u8 post[16];} World;
static World actual;
static unsigned scenario,sound_calls,gte_calls;
#define garage_D_80127668 actual.work
#define PlayerControllers actual.pads
static const u32 factors[8]={0,1,4095,4096,4097,0xffffffffU,0x80000000U,0x7fffffffU};
static const u32 steps[8]={0,1,2,4096,0xffffffffU,0x80000000U,0x7fffffffU,65535};
static void dump(void){unsigned i;for(i=0;i<sizeof actual;i++)printf("%02x",((u8*)&actual)[i]);puts("");}
void PlaySfx(s32 sound){
 unsigned i;assert(sound==2&&sound_calls++==0);printf("S ");dump();
 if((scenario/1024)&1){
  for(i=0;i<64;i++)((u8*)&actual.work)[i]=(u8)(scenario*29+i*7+31);
  for(i=0;i<184;i++)((u8*)actual.pads)[i]=(u8)(scenario*29+(80+i)*7+31);
 }
}
static void interpolate(u32 factor,const u32 *source,u32 *destination){
 u16 result[2];assert(gte_calls++==0&&factor==actual.work.words[10]);
 assert(source==&actual.work.words[8]&&destination==&actual.work.words[5]);
 assert((uintptr_t)source>UINT32_MAX&&(uintptr_t)destination>UINT32_MAX);
 printf("G %08x %08x %08x ",factor,source[0],source[1]);dump();
 result[0]=(u16)(scenario*137+0x8000);result[1]=(u16)~(scenario*211+0x7fff);
 memcpy(destination,result,4);
}
'''
TAIL=r'''
int main(void){unsigned i;unsigned flags[4]={0,2,4,6};
 _Static_assert(sizeof(RrMenuItemState5C)==92,"full pad owner");
 _Static_assert(sizeof(World)==280,"whole fixed-width world");
 for(scenario=0;scenario<4096;scenario++){
  for(i=0;i<sizeof actual;i++)((u8*)&actual)[i]=(u8)(scenario*29+i*7);
  actual.work.words[0]=flags[scenario%4];actual.work.words[10]=factors[(scenario/4)%8];actual.work.words[11]=steps[(scenario/32)%8];
  actual.pads[0].type01=(scenario/256)%2?0x12:0x41;actual.pads[0].buttons02=(scenario/512)%2?4:0;
  sound_calls=gte_calls=0;garage_func_8011DA90();assert(gte_calls==1);
  for(i=0;i<16;i++){assert(actual.pre[i]==(u8)(scenario*29+i*7));assert(actual.post[i]==(u8)(scenario*29+(264+i)*7));}
  printf("F ");dump();
 }return 0;
}
'''
def main():
 body=re.sub(r' __asm__\("\$\d+"\)','',SOURCE.read_text())
 with TemporaryDirectory() as tmp:
  c=Path(tmp)/'test.c';exe=Path(tmp)/'test';c.write_text(C+body+TAIL)
  subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(c),'-o',str(exe)],check=True)
  native=iter(subprocess.check_output([str(exe)],text=True).splitlines())
 blob=(ROOT/'disc/PAL/R4.BIN').read_bytes();sector=struct.unpack_from('<I',blob,4+662*4)[0];off=sector*2048+0x8011DA90-0x80114878;code=blob[off:off+328]
 elf=os.environ.get('RR_COMPILED_ELF')
 if elf:
  with open(elf,'rb') as f:code=ELFFile(f).get_section_by_name('.text').data()
 memory=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);memory.mem_map(0,0x200000)
 start=0x8011DA90;work=0x80127668;pad=0x800F3A38;sp=0x801F0000;stop=0x80010000
 write=lambda a,b:memory.mem_write(a&0x1fffffff,bytes(b))
 read=lambda a,n:bytes(memory.mem_read(a&0x1fffffff,n))
 w32=lambda a,v:write(a,struct.pack('<I',v&0xffffffff))
 r32=lambda a:struct.unpack('<I',read(a,4))[0]
 whole=lambda:read(work-16,80)+read(pad,200)
 write(start,code);seen_sound=set();seen_branches=set();protocols=0;sound_total=0;mutating_sound_total=0;seen_sound_mutation=set()
 def store(cpu,access,address,size,value,_):
  a=address&0x1fffffff
  assert (work&0x1fffffff)<=a and a+size<=(work&0x1fffffff)+64 or (sp&0x1fffffff)-32<=a and a+size<=(sp&0x1fffffff),hex(address)
 class ProtocolMips(Mips):
  def step(self):
   nonlocal protocols
   word=self.load(self.pc,4)
   if word>>26==1:
    assert self.delay is None
    rs=(word>>21)&31;rt=(word>>16)&31;assert rt in [0,1]
    take=signed(self.r[rs])<0 if rt==0 else signed(self.r[rs])>=0
    self.delay=(self.pc+4+(signed(word,16)<<2))&0xffffffff if take else self.pc+8
    self.pc+=4;return
   if word>>26!=18:return super().step()
   assert self.delay is None
   rs=(word>>21)&31;rt=(word>>16)&31;rd=(word>>11)&31
   if rs==4:self.data[rd]=self.r[rt]
   elif rs==6:self.control[rd]=self.r[rt]
   elif rs==0:
    if rt:self.r[rt]=self.data[rd]&0xffffffff
   else:
    assert word==0x4A980011 and rs==20,hex(word)
    factor=self.data[8];a=self.data[9]&65535|(self.data[10]&65535)<<16;b=self.control[21]&65535|(self.control[22]&65535)<<16
    assert self.data[9]==(signed(a,16)&0xffffffff) and self.data[10]==(signed(a>>16,16)&0xffffffff)
    assert self.control[21]==(signed(b,16)&0xffffffff) and self.control[22]==(signed(b>>16,16)&0xffffffff)
    assert factor==r32(work+40) and a==r32(work+32) and b==r32(work+36)
    line=f'G {factor:08x} {a:08x} {b:08x} {whole().hex()}';assert next(native)==line,scenario
    self.data[9]=signed(scenario*137+0x8000,16)&0xffffffff;self.data[10]=signed(~(scenario*211+0x7fff),16)&0xffffffff;protocols+=1
   self.pc+=4
 cpu=ProtocolMips(memory,0,100,101,102,store)
 # Validate both REGIMM conditions and their delay slot against Unicorn.
 branch_checks=0;bench=0x80010100
 for condition in [0,1]:
  for value in [0,1,4095,4096,0xffffffff,0x80000000,0x7fffffff,0xffff0000]:
   bench=0x80010100+branch_checks*0x40
   words=[(1<<26)|(4<<21)|(condition<<16)|2,0x24021234,0x24035678,0]
   write(bench,struct.pack('<4I',*words));cpu.reset();cpu.r[4]=value;cpu.pc=bench
   for r in range(32):memory.reg_write(UC_MIPS_REG_0+r,0)
   memory.reg_write(UC_MIPS_REG_0+4,value);memory.emu_start(bench,bench+12,count=100)
   for _ in range(4):
    if cpu.pc==bench+12:break
    cpu.step()
   assert cpu.pc==bench+12
   assert cpu.r[2]==memory.reg_read(UC_MIPS_REG_0+2)==0x1234 and cpu.r[3]==memory.reg_read(UC_MIPS_REG_0+3)
   branch_checks+=1
 factors=[0,1,4095,4096,4097,0xffffffff,0x80000000,0x7fffffff];steps=[0,1,2,4096,0xffffffff,0x80000000,0x7fffffff,65535]
 for scenario in range(CASES):
  initial=bytes((scenario*29+i*7)&255 for i in range(280));write(work-16,initial[:80]);write(pad,initial[80:])
  flags=[0,2,4,6][scenario%4];factor=factors[(scenario//4)%8];step=steps[(scenario//32)%8]
  w32(work,flags);w32(work+40,factor);w32(work+44,step)
  write(pad+1,bytes([0x12 if (scenario//256)%2 else 0x41]));write(pad+2,struct.pack('<H',4 if (scenario//512)%2 else 0))
  expanding=bool(flags&2 or (read(pad+1,1)[0]!=0x12 and struct.unpack('<H',read(pad+2,2))[0]&4));seen_branches.add(expanding)
  cpu.reset();cpu.data=[0xCDEF0000+i for i in range(32)];cpu.control=[0xFECD0000+i for i in range(32)]
  for r in range(1,32):cpu.r[r]=0xBEEF0000+r
  cpu.r[29]=sp;cpu.r[31]=stop;cpu.pc=start;sound=False
  for executed in range(300):
   if cpu.pc==stop:break
   if cpu.pc==0x8004F24C:
    assert cpu.r[4]==2 and not sound and cpu.delay is None;sound=True;sound_total+=1
    assert next(native)=='S '+whole().hex(),scenario
    seen_sound_mutation.add(bool((scenario//1024)&1))
    if (scenario//1024)&1:
     mutating_sound_total+=1
     write(work,bytes((scenario*29+i*7+31)&255 for i in range(64)))
     write(pad,bytes((scenario*29+(80+i)*7+31)&255 for i in range(184)))
    ret=cpu.r[31]
    for r in [2,3,4,5,6,7,8,9,10,11,12,13,14,15,24,25]:cpu.r[r]=0xDEAD0000+r
    cpu.hi=0x12345678;cpu.lo=0x87654321;cpu.pc=ret
   else:cpu.step()
  assert cpu.pc==stop and cpu.r[29]==sp
  for r in range(16,24):assert cpu.r[r]==0xBEEF0000+r
  assert next(native)=='F '+whole().hex(),scenario
  assert read(work-16,16)==initial[:16] and read(pad+184,16)==initial[264:]
  seen_sound.add(sound)
 assert next(native,None) is None and seen_sound=={False,True} and seen_branches=={False,True} and protocols==CASES and seen_sound_mutation=={False,True} and branch_checks==16
 sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
 deps=[SOURCE,Path(__file__).resolve(),ROOT/'tools/scripts/mips_preview_engine.py']+list((ROOT/'include').rglob('*.h'))
 audit=dict(cases=CASES,engine='compiled integer MIPS with opaque COP2 boundary' if elf else 'original integer MIPS with opaque COP2 boundary',source_sha256=sha(SOURCE),test_sha256=sha(__file__),full_host_pointers=True,full_owner_guards=True,sound_calls=sound_total,mutating_sound_calls=mutating_sound_total,regimm_validation_blocks=branch_checks,coprocessor_protocols=protocols,opaque_coprocessor=True,caller_save_hi_lo_poison=True,mips_store_bounds=True,factor_profiles=factors,step_profiles=steps,sanitizers=['undefined','bounds'],dependencies={str(p):sha(p) for p in deps})
 if elf:audit['elf_sha256']=sha(elf)
 if os.environ.get('RR_AUDIT_JSON'):Path(os.environ['RR_AUDIT_JSON']).write_text(json.dumps(audit,indent=2)+'\n')
 print(f'PASS {CASES} {audit["engine"]}/native cases; full snapshots, live sound service, GTE transfer protocol')
if __name__=='__main__':main()
