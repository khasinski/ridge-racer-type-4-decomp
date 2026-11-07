"""RPM/gear response against original and fully linked integer MIPS."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess,struct,random,re,json,hashlib,sys,os
sys.path.insert(0,"tools/scripts")
from mips_preview_engine import Mips,signed
from unicorn import *
from unicorn.mips_const import *
from elftools.elf.elffile import ELFFile
root=Path(__file__).resolve().parents[2];os.chdir(root)
part=root/'include/rr';source=root/'tools/scripts/fixtures/views/func_800483D8.c'
body=source.read_text();body=body[body.index('void UpdateEngineRpm'):];body=re.sub(r' __asm__\("\$\d+"\)','',body)
prefix='''
#include <stddef.h>
#include <stdio.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"offset")
#include "rr/vehicle_rpm.h"
#include "rr/cpu_md_leaf.h"
#include "rr_asm.h"
#include <stdint.h>
s16 CourseListIndex,RaceType;
typedef struct {u8 before[16];RrRaceVehicle vehicle;u8 after[16];} World;
'''
tail='''
int main(void){World w; _Static_assert(sizeof(RrDriveVehicleView)==800 && sizeof(RrDriveControlView)==0x12C,"view sizes"); _Static_assert(sizeof(w)==832,"world");_Static_assert(offsetof(RrDriveControlView,latch60)==0x60,"native latch offset");_Static_assert(sizeof(void*)==8,"full native pointer");assert((uintptr_t)&w.vehicle>UINT32_MAX);
while(fread(&w,1,832,stdin)==832){if(fread(&CourseListIndex,2,1,stdin)!=1||fread(&RaceType,2,1,stdin)!=1)return 2;
UpdateEngineRpm(&w.vehicle);if(fwrite(&w,1,832,stdout)!=832)return 3;}return 0;}
'''
rng=random.Random(0x483D8);cases=[];counts={};limits=[0,1,-1,32767,-32768,6000,10240,-20];speeds=[0,256,-256,2190*256,2191*256,0x7fffffff,-0x80000000,255,-255,8500*256]
for i in range(8192):
 data=bytearray(rng.randbytes(836));put=lambda off,fmt,v:struct.pack_into('<'+fmt,data,16+off,v)
 mode=i%4;speed=speeds[(i//4)%len(speeds)] if i<4096 else rng.randrange(-0x80000000,0x80000000)
 if mode==3 and ((speed+255 if speed<0 else speed)>>8)&65535==0:speed=256
 put(0x2ec,'i',speed)
 count=1+(i//3)%7;gear=(i//18)%count;put(0x2f0,'i',count);put(0x25c,'h',gear)
 for j in range(6):put(0x300+2*j,'H',([0,1,65535,2000,8500,10000][(i//6+j)%6] if i<4096 else rng.randrange(65536)))
 put(0x1d8,'h',limits[(i//8)%8] if i<4096 else rng.randrange(-32768,32768))
 hold=[0,1,11,12,13,-1,32767,-32768][(i//64)%8];put(0x23c,'h',hold)
 shift=[-1,0,1][(i//512)%3];put(0x258,'h',shift)
 put(0x25a,'h',limits[(i//24)%8]);put(0x248,'h',i%2)
 put(0x24a,'H',[0,19,20,4000,6000,10240,65535][(i//2)%7]);put(0x254,'h',[0,1,-1][(i//7)%3])
 put(0x280,'h',limits[(i//96)%8])
 struct.pack_into('<hh',data,832,3 if mode in [2,3] else 2,6 if mode in [1,3] else 5)
 cases.append(bytes(data));key=(gear==0,gear==count-1,hold==0,shift>0,mode==3);counts[str(key)]=counts.get(str(key),0)+1
with TemporaryDirectory() as tmp:
 c=Path(tmp)/'test.c';exe=Path(tmp)/'test';c.write_text(prefix+body+tail)
 subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-Iinclude','-I'+str(part),str(c),'-o',str(exe)],check=True)
 native=subprocess.run([str(exe)],input=b''.join(cases),capture_output=True,check=True).stdout
assert len(native)==832*len(cases)
def verify(code,label):
 uc=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);uc.mem_map(0,0x200000)
 start=0x800483d8;vehicle=0x800d0010;sp=0x801f0000;stop=0x80010000
 write=lambda a,b:uc.mem_write(a&0x1fffffff,bytes(b));read=lambda a,n:bytes(uc.mem_read(a&0x1fffffff,n))
 write(start,code)
 def store(cpu,access,a,n,v,user):
  assert n==2 and a&0x1fffffff in [(vehicle+o)&0x1fffffff for o in [0x248,0x24a,0x254]],hex(a)
 class Cpu(Mips):
  def step(self):
   w=self.load(self.pc,4)
   if w>>26==0 and w&63==26:
    rs=(w>>21)&31;rt=(w>>16)&31;a=signed(self.r[rs]);b=signed(self.r[rt]);assert b!=0
    q=abs(a)//abs(b);q=-q if (a<0)!=(b<0) else q
    self.lo=q&0xffffffff;self.hi=(a-q*b)&0xffffffff
    destination=self.delay;self.delay=None;self.pc=(self.pc+4 if destination is None else destination)&0xffffffff;return
   if w>>26!=1:return super().step()
   assert self.delay is None;rs=(w>>21)&31;rt=(w>>16)&31;assert rt in [0,1]
   take=signed(self.r[rs])<0 if rt==0 else signed(self.r[rs])>=0
   self.delay=(self.pc+4+(signed(w,16)<<2))&0xffffffff if take else self.pc+8;self.pc+=4
 cpu=Cpu(uc,0,100,101,102,store)
 # Validate the local signed DIV extension against a straight Unicorn block;
 # isolated instructions avoid the previously documented branch translation issue.
 divuc=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);divuc.mem_map(0,4096)
 block=struct.pack('<III',(4<<21)|(5<<16)|26,(2<<11)|18,(3<<11)|16)
 divuc.mem_write(0,block);write(0x80000800,block)
 checks=random.Random(0xD1483D8)
 for j in range(1024):
  a=checks.randrange(0x100000000);b=checks.randrange(1,0x100000000)
  divuc.reg_write(UC_MIPS_REG_A0,a);divuc.reg_write(UC_MIPS_REG_A1,b);divuc.emu_start(0,12,count=3)
  cpu.reset();cpu.r[4]=a;cpu.r[5]=b;cpu.pc=0x80000800
  for n in range(3):cpu.step()
  assert cpu.r[2]==divuc.reg_read(UC_MIPS_REG_V0) and cpu.r[3]==divuc.reg_read(UC_MIPS_REG_V1)
 for i,data in enumerate(cases):
  write(vehicle-16,data[:832]);write(0x800f3826,data[832:834]);write(0x800f4d44,data[834:])
  cpu.reset();cpu.r=[0]+[0xbeef0000+r for r in range(1,32)];cpu.r[4]=vehicle;cpu.r[29]=sp;cpu.r[31]=stop;cpu.pc=start
  cpu.hi=0x12345678;cpu.lo=0x87654321
  for step in range(1000):
   if cpu.pc==stop:break
   cpu.step()
  assert cpu.pc==stop,(i,hex(cpu.pc))
  actual=read(vehicle-16,832);expected=native[i*832:(i+1)*832]
  assert actual==expected,(label,i,[(hex(j-16),a,b) for j,(a,b) in enumerate(zip(actual,expected)) if a!=b])
  assert cpu.r[29]==sp and cpu.r[31]==stop
  for r in list(range(16,24))+[28,30]:assert cpu.r[r]==0xbeef0000+r
  assert read(0x800f3826,2)==data[832:834] and read(0x800f4d44,2)==data[834:]
 print('PASS',label,len(cases),'whole record, guards, exact stores, callee-save registers, UBSan')
 return {'cases':len(cases),'sha256':hashlib.sha256(code).hexdigest()}
a=verify(Path('assets/PAL/main.exe').read_bytes()[0x38bd8:0x38ec4],'original MIPS/native')
elf=os.environ.get('RR_COMPILED_ELF');b=None
if elf:
 with open(elf,'rb') as f:
  e=ELFFile(f);section=e.get_section_by_name('.text');code=section.data();assert section['sh_addr']==0x800483D8
 b=verify(code,'compiled MIPS/native')
sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
audit={'original':a,'compiled':b,'source_sha256':sha(source),'header_sha256':sha(part/'vehicle_rpm.h'),'drive_header_sha256':sha(part/'vehicle_drive.h'),'md_header_sha256':sha(part/'cpu_md_leaf.h'),'pipeline_header_sha256':sha('include/rr_asm.h'),'engine_sha256':sha('tools/scripts/mips_preview_engine.py'),'test_sha256':sha(__file__),'branch_profiles':counts,'div_extension_unicorn_blocks':1024,'full_owner_guards':True,'full_host_pointer_addresses':True,'exact_store_bounds':True,'sanitizers':['undefined','bounds'],'contract':'Gear count1..7, accessible six-threshold table including top gear6. No special-mode zero divisor. Integer architectural evaluator; no physical timing model.'}
if elf:audit['elf_sha256']=sha(elf)
if os.environ.get('RR_AUDIT_JSON'):Path(os.environ['RR_AUDIT_JSON']).write_text(json.dumps(audit,indent=2)+'\n')
