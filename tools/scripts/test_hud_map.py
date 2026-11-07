"""Original (or RR_COMPILED_ELF) MIPS/native HUD21E2C service-state oracle."""
from pathlib import Path
import os,re,sys,subprocess,struct,ctypes as C,json,hashlib
from unicorn import Uc,UC_ARCH_MIPS,UC_MODE_MIPS32,UC_MODE_LITTLE_ENDIAN,UC_HOOK_CODE
from unicorn.mips_const import *
root=next(p for p in Path(__file__).resolve().parents if (p/'tools/scripts/cc.sh').is_file());os.chdir(root)
P=Path(os.getenv('RR_TEST_DIR','build/tests/hud_map'));P.mkdir(parents=True,exist_ok=True)
source=Path(sys.argv[1]) if len(sys.argv)>1 else root/'tools/scripts/fixtures/views/func_80021E2C.c';body=source.read_text().split('\n',1)[1];body=re.sub(r' __asm__\("\$\d+"\)','',body)
head=r'''
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/controller_hud.h"
__typeof__(CurrentDrawBuf) CurrentDrawBuf;RrBankRecord320 *CarTable[16];s16 NumCars,LinkRoleSlot;RrUiUvRect D_8009EE58;
static RrGpuScratchPointers scratch;static RrFrameStorage frames[2];static RrBankRecord320 records[16];static u32 streams[3][1024];
#undef RR_HUD_SCRATCH
#define RR_HUD_SCRATCH (&scratch)
typedef uintptr_t (*Callback)(unsigned,uintptr_t*);static Callback cb;void set_callback(Callback c){cb=c;}
void *AddSpritePrim(u32*h,void*p,s32 x,s32 y,RrUiUvRect*g,u16 clut,s32 flags){uintptr_t a[9]={(uintptr_t)h,(uintptr_t)p,(u32)x,(u32)y,(uintptr_t)g,clut,(u32)flags};return (void*)cb(0,a);}
RrSpritePacketSignedClut *GameQueueSprite(u32*h,RrSpritePacketSignedClut*p,s32 x,s32 y,s32 w,s32 he,s32 u,s32 v,s32 c){uintptr_t a[9]={(uintptr_t)h,(uintptr_t)p,(u32)x,(u32)y,(u32)w,(u32)he,(u32)u,(u32)v,(u32)c};return (void*)cb(1,a);}
void *AddDrawModePrim(u32*h,void*p,u16 page){uintptr_t a[9]={(uintptr_t)h,(uintptr_t)p,page};return (void*)cb(2,a);}
void *object(unsigned n){switch(n){case 0:return frames;case 1:return records;case 2:return streams;case 3:return &D_8009EE58;case 4:return &NumCars;case 5:return &LinkRoleSlot;case 6:return &scratch;case 7:return &CurrentDrawBuf;case 8:return CarTable;default:abort();}}
size_t object_size(unsigned n){switch(n){case 0:return sizeof(frames);case 1:return sizeof(records);case 2:return sizeof(streams);case 3:return sizeof(D_8009EE58);case 4:return sizeof(NumCars);case 5:return sizeof(LinkRoleSlot);case 6:return sizeof(scratch);case 7:return sizeof(CurrentDrawBuf);case 8:return sizeof(CarTable);default:abort();}}
'''
file=P/(source.stem+'_native.c');file.write_text(head+body)
subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-shared','-fPIC','-Iinclude','-I'+str(source.parent),'-I'+str(Path(__file__).parent),'-fsanitize=undefined,bounds','-fno-sanitize-recover=all',str(file),'-o',str(file.with_suffix('.dylib'))],check=True)
lib=C.CDLL(str(file.with_suffix('.dylib')));lib.object.restype=C.c_void_p;lib.object_size.restype=C.c_size_t
N=[(lib.object(i),lib.object_size(i)) for i in range(9)];assert all(p>0xffffffff for p,s in N)
addresses=[0x80200000,0x80280000,0x80300000,0x8009EE58,0x800AC59C,0x800AD508,0x1F800000,0x800ACC14,0x800FFC20]
regions=[(p,a,size) for (p,size),a in zip(N[:6],addresses[:6])]
u=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);u.mem_map(0,0x400000);u.mem_map(0x1F800000,0x1000);u.mem_write(0x10000,Path('assets/PAL/main.exe').read_bytes()[0x800:])
if os.getenv('RR_COMPILED_ELF'):
 from elftools.elf.elffile import ELFFile
 with open(os.environ['RR_COMPILED_ELF'],'rb') as f:u.mem_write(0x21E2C,ELFFile(f).get_section_by_name('.text').data())
def read(a,n):return bytes(u.mem_read(a&0x1fffffff,n))
def write(a,b):u.mem_write(a&0x1fffffff,b)
def pal(p):
 for native,a,size in regions:
  if native<=p<native+size:return a+p-native
 raise AssertionError(('unowned native pointer',hex(p)))
def host(a):
 for native,base,size in regions:
  if base<=a<base+size:return native+a-base
 raise AssertionError(('unowned PAL pointer',hex(a)))
def ptrset(obj,index,a,native):
 if native:C.c_void_p.from_address(N[obj][0]+index*C.sizeof(C.c_void_p)).value=host(a)
 else:write(addresses[obj]+index*4,struct.pack('<I',a))
def mem(a,b,native):
 if native:C.memmove(host(a),b,len(b))
 else:write(a,b)
def snap(native):
 parts=[C.string_at(p,size) if native else read(a,size) for p,a,size in regions]
 if native:ptrs=[pal(C.c_void_p.from_address(N[obj][0]+j*8).value) for obj,count in [(6,2),(7,1),(8,16)] for j in range(count)]
 else:ptrs=[struct.unpack('<I',read(addresses[obj]+j*4,4))[0] for obj,count in [(6,2),(7,1),(8,16)] for j in range(count)]
 return b''.join(parts)+struct.pack('<19I',*ptrs)
names=['AddSpritePrim','GameQueueSprite','AddDrawModePrim'];counts=[7,9,3];ptrargs=[{0,1,4},{0,1},{0,1}];events=[];observed=[];case=0;errors=[]
def service(e,args,native):
 arr=[pal(x) if native and j in ptrargs[e] else x&0xffffffff for j,x in enumerate(args)]
 trace=observed if native else events;trace.append((e,arr,snap(native)));serial=len(trace)
 if not native:coverage[e]+=1
 if case&16:
  # Count remains positive, and table slots remain valid. Other live scalar payloads are adversarial.
  mem(0x800AC59C,struct.pack('<h',1+(case+serial)%8),native);mem(0x800AD508,struct.pack('<h',[-32768,-1,0,1,2,32767][(case+serial)%6]),native)
  mem(0x8009EE58,bytes((case+serial*13+j*29)&255 for j in range(4)),native)
  for j in range(16):
   ptrset(8,j,0x80280000+((case+serial+j)%16)*0x320,native)
   vals=[0x80000000,0x7fffffff,0xffffffff,0,1,1150,0xfffffb82]
   mem(0x80280000+j*0x320+0x20,struct.pack('<I',vals[(case+serial+j)%len(vals)]),native)
   mem(0x80280000+j*0x320+0x28,struct.pack('<I',vals[(case+serial+j*3)%len(vals)]),native)
   mem(0x80280000+j*0x320+0x1e4,struct.pack('<H',(case*31+serial+j)&65535),native)
  ptrset(7,0,0x80200000+((case+serial)&1)*0x22778,native)
 ptrset(6,0,0x80300000+((case+serial+1)%3)*4096+((serial*53)%3500&~3),native)
 ptrset(6,1,0x80300000+((case+serial)%3)*4096,native)
 mem(0x80200000+((case+serial)&1)*0x22778+0x70,struct.pack('<I',case*97+serial),native)
 result=0x80300000+((case+serial+2)%3)*4096+((serial*97)%3500&~3)
 return host(result) if native else result
CB=C.CFUNCTYPE(C.c_size_t,C.c_uint,C.POINTER(C.c_size_t))
def callback(e,args):
 try:return service(e,list(args[:counts[e]]),True)
 except BaseException as err:errors.append(err);return 0
cb=CB(callback);lib.set_callback(cb)
regs=[UC_MIPS_REG_A0,UC_MIPS_REG_A1,UC_MIPS_REG_A2,UC_MIPS_REG_A3];services={__import__('c_functions').address_of(n):i for i,n in enumerate(names)}
def hook(uc,a,size,data):
 e=services[a];count=counts[e];args=[uc.reg_read(r)&0xffffffff for r in regs[:min(count,4)]]
 if count>4:args+=list(struct.unpack('<'+'I'*(count-4),read(uc.reg_read(UC_MIPS_REG_SP)+16,4*(count-4))))
 result=service(e,args,False);ret=uc.reg_read(UC_MIPS_REG_RA)
 for r in [UC_MIPS_REG_AT,UC_MIPS_REG_V1,*regs,UC_MIPS_REG_T0,UC_MIPS_REG_T1,UC_MIPS_REG_T2,UC_MIPS_REG_T3,UC_MIPS_REG_T4,UC_MIPS_REG_T5,UC_MIPS_REG_T6,UC_MIPS_REG_T7,UC_MIPS_REG_T8,UC_MIPS_REG_T9]:uc.reg_write(r,(0xA5A50000+case*19+r+len(events))&0xffffffff)
 uc.reg_write(UC_MIPS_REG_HI,0x12345678);uc.reg_write(UC_MIPS_REG_LO,0x89abcdef);uc.reg_write(UC_MIPS_REG_V0,result);uc.reg_write(UC_MIPS_REG_PC,ret)
for a in services:u.hook_add(UC_HOOK_CODE,hook,begin=a,end=a)
def init(n,native):
 for p,a,size in regions:mem(a,b'\xA6'*size,native)
 ptrset(7,0,0x80200000+(n&1)*0x22778,native);ptrset(6,0,0x80300000+(n%3)*4096,native);ptrset(6,1,0x80300000+((n+2)%3)*4096,native)
 for j in range(16):
  ptrset(8,j,0x80280000+j*0x320,native)
  vals=[0x80000000,0x7fffffff,0xffffffff,0,1,1150,0xfffffb82,n*0x123457&0xffffffff]
  mem(0x80280000+j*0x320+0x20,struct.pack('<I',vals[(n+j)%len(vals)]),native);mem(0x80280000+j*0x320+0x28,struct.pack('<I',vals[(n+j*3)%len(vals)]),native);mem(0x80280000+j*0x320+0x1e4,struct.pack('<H',(n*31+j*19)&65535),native)
 mem(0x800AC59C,struct.pack('<h',[-32768,-1,0,1,2,3,4,8][n%8]),native);mem(0x800AD508,struct.pack('<h',[-32768,-1,0,1,2,32767][(n//8)%6]),native)
coverage=[0,0,0]
count=int(sys.argv[2]) if len(sys.argv)>2 else 4096
for case in range(count):
 events=[];observed=[];init(case,False);u.reg_write(UC_MIPS_REG_A0,(case//48)%3);u.reg_write(UC_MIPS_REG_SP,0x803F0000);u.reg_write(UC_MIPS_REG_RA,0x803FF800)
 try:u.emu_start(0x80021E2C,0x803FF800,count=30000)
 except Exception:print('MIPS failure',case,hex(u.reg_read(UC_MIPS_REG_PC)));raise
 expected=snap(False);init(case,True);lib.DrawRaceMinimap((case//48)%3)
 if errors:raise errors[0]
 assert len(events)==len(observed),(case,len(events),len(observed))
 for j,(e,o) in enumerate(zip(events,observed)):
  assert e[:2]==o[:2],(case,j,names[e[0]],e[1],o[1])
  assert e[2]==o[2],(case,j,names[e[0]],'world mismatch')
 assert expected==snap(True),('final',case)
 if case and case%512==0:print(case,'cases',flush=True)
print(count,'cases passed;',('compiled' if os.getenv('RR_COMPILED_ELF') else 'original'),'MIPS/native full pointers, two full frame owners,16 records and pointer slots, exact service args/snapshots, mutable callbacks, ABI clobbers',flush=True)

audit={'source':str(source),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'test_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'cases':count,'engine':'compiled MIPS' if os.getenv('RR_COMPILED_ELF') else 'original MIPS','header_sha256':{name:hashlib.sha256((root/'include/rr'/name).read_bytes()).hexdigest() for name in ['controller_hud.h','record_banks.h','frame_storage_types.h','sprite_emit.h','gpu.h','icon_sprites.h','animated_sprite_grid.h','tables.h']},'service_counts':dict(zip(names,coverage)),'sanitizers':['undefined','bounds'],'checks':['full native pointers and both complete frame owners','16 complete record320 owners and16 pointer slots','every service argument and complete pre-service world','fresh counts,banks,records,table roots,cursor roots after callback mutation','all signed32 map coordinate boundaries and caller-save/HI/LO poison','layout0/1/2; nonpositive count boundaries andcounts1..8; signed16 bankedges']}
if os.getenv('RR_COMPILED_ELF'):audit['elf_sha256']=hashlib.sha256(Path(os.environ['RR_COMPILED_ELF']).read_bytes()).hexdigest()
if os.getenv('RR_AUDIT_JSON'):Path(os.environ['RR_AUDIT_JSON']).write_text(json.dumps(audit,indent=2)+'\n')
