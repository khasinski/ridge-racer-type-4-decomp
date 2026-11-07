"""Original (or RR_COMPILED_ELF) MIPS/native HUD22098 service-state oracle."""
from pathlib import Path
import os,re,sys,subprocess,struct,ctypes as C,json,hashlib
from unicorn import Uc,UC_ARCH_MIPS,UC_MODE_MIPS32,UC_MODE_LITTLE_ENDIAN,UC_HOOK_CODE
from unicorn.mips_const import *
ROOT=next(p for p in Path(__file__).resolve().parents if (p/'tools/scripts/cc.sh').is_file());os.chdir(ROOT);P=Path(os.getenv('RR_TEST_DIR',str(ROOT/'build/native_tests/controller_hud')));P.mkdir(parents=True,exist_ok=True)
source=Path(sys.argv[1]) if len(sys.argv)>1 else ROOT/'tools/scripts/fixtures/views/func_80022098.c';body=source.read_text().split('\n',1)[1];body=re.sub(r' __asm__\("\$\d+"\)','',body)
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
RrMenuItemState5C PlayerControllers[2];u32 ScreenFrame;s16 D_8009EE68[2];u8 D_8009EE5C[6],D_8009EE64[3];
static RrGpuScratchPointers scratch;static u32 streams[3][1024];
_Static_assert(sizeof(RrMenuItemState5C)==0x5c,"input owner");
_Static_assert(offsetof(RrMenuItemState5C,status10)==16,"status");
_Static_assert(offsetof(RrMenuItemState5C,steering3C)==60,"steering");
_Static_assert(sizeof(RrHudTriangle)==12,"triangle");
#undef RR_HUD_SCRATCH
#define RR_HUD_SCRATCH (&scratch)
typedef uintptr_t (*Callback)(unsigned,uintptr_t*);static Callback cb;void set_callback(Callback c){cb=c;}
s32 GetControllerWarning(s32 n){uintptr_t a[9]={(u32)n};return cb(0,a);}
void *AddDrawModePrim(u32*h,void*p,u16 page){uintptr_t a[9]={(uintptr_t)h,(uintptr_t)p,page};return(void*)cb(1,a);}
RrSpritePacketSignedClut *GameQueueSprite(u32*h,RrSpritePacketSignedClut*p,s32 x,s32 y,s32 w,s32 he,s32 u,s32 v,s32 c){uintptr_t a[9]={(uintptr_t)h,(uintptr_t)p,(u32)x,(u32)y,(u32)w,(u32)he,(u32)u,(u32)v,(u32)c};return(RrSpritePacketSignedClut*)cb(2,a);}
void *AddTilePrim(u32*h,void*p,s32 x,s32 y,s32 w,s32 he,s32 r,s32 g,s32 b){uintptr_t a[9]={(uintptr_t)h,(uintptr_t)p,(u32)x,(u32)y,(u32)w,(u32)he,(u32)r,(u32)g,(u32)b};return(void*)cb(3,a);}
RrPolyF3Packet *EmitPolyF3(u32*h,RrPolyF3Packet*p,s32 a,s32 b,s32 c,const u8*rgb,s32 f){uintptr_t args[9]={(uintptr_t)h,(uintptr_t)p,(u32)a,(u32)b,(u32)c,(uintptr_t)rgb,(u32)f};return(RrPolyF3Packet*)cb(4,args);}
void *object(unsigned n){switch(n){case 0:return PlayerControllers;case 1:return streams;case 2:return D_8009EE5C;case 3:return D_8009EE64;case 4:return D_8009EE68;case 5:return &ScreenFrame;case 6:return &scratch;default:abort();}}
size_t object_size(unsigned n){switch(n){case 0:return sizeof(PlayerControllers);case 1:return sizeof(streams);case 2:return sizeof(D_8009EE5C);case 3:return sizeof(D_8009EE64);case 4:return sizeof(D_8009EE68);case 5:return sizeof(ScreenFrame);case 6:return sizeof(scratch);default:abort();}}
'''
file=P/(source.stem+'_native.c');file.write_text(head+body)
subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-shared','-fPIC','-Iinclude','-I'+str(source.parent),'-fsanitize=undefined,bounds','-fno-sanitize-recover=all',str(file),'-o',str(file.with_suffix('.dylib'))],check=True)
lib=C.CDLL(str(file.with_suffix('.dylib')));lib.object.restype=C.c_void_p;lib.object_size.restype=C.c_size_t
N=[(lib.object(i),lib.object_size(i)) for i in range(7)];assert all(p>0xffffffff for p,s in N)
addresses=[0x800F3A38,0x80300000,0x8009EE5C,0x8009EE64,0x8009EE68,0x800ABEB0,0x1F800000]
regions=[(p,a,size) for (p,size),a in zip(N[:6],addresses[:6])]
u=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);u.mem_map(0,0x400000);u.mem_map(0x1F800000,0x1000);u.mem_write(0x10000,Path('assets/PAL/main.exe').read_bytes()[0x800:])
if os.getenv('RR_COMPILED_ELF'):
 from elftools.elf.elffile import ELFFile
 with open(os.environ['RR_COMPILED_ELF'],'rb') as f:u.mem_write(0x22098,ELFFile(f).get_section_by_name('.text').data())
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
 if native:ptrs=[pal(C.c_void_p.from_address(N[obj][0]+j*8).value) for obj,count in [(6,2)] for j in range(count)]
 else:ptrs=[struct.unpack('<I',read(addresses[obj]+j*4,4))[0] for obj,count in [(6,2)] for j in range(count)]
 return b''.join(parts)+struct.pack('<2I',*ptrs)
names=['GetControllerWarning','AddDrawModePrim','GameQueueSprite','AddTilePrim','EmitPolyF3'];counts=[1,3,9,9,7];ptrargs=[set(),{0,1},{0,1},{0,1},{0,1,5}];events=[];observed=[];case=0;errors=[];coverage=[0]*5;connection_results=set()
def service(e,args,native):
 arr=[pal(x) if native and j in ptrargs[e] else x&0xffffffff for j,x in enumerate(args)]
 trace=observed if native else events;trace.append((e,arr,snap(native)));serial=len(trace)
 if not native:coverage[e]+=1
 if case&32:
  for j in range(2):
   mem(0x800F3A38+j*92+60,struct.pack('<h',[-32768,-128,-127,-1,0,1,127,128,32767][(case+serial+j)%9]),native)
  mem(0x8009EE5C,bytes((case+serial*13+j*29)&255 for j in range(6)),native)
  mem(0x8009EE64,bytes((case+serial*31+j*7)&255 for j in range(3)),native)
  mem(0x8009EE68,struct.pack('<hh',[-32768,-1,0,1,2,32767][(case+serial)%6],[-32768,-1,0,1,2,32767][(case+serial+1)%6]),native)
  mem(0x800ABEB0,struct.pack('<I',(case*97+serial)&0xffffffff),native)
 ptrset(6,0,0x80300000+((case+serial+1)%3)*4096+((serial*53)%3500&~3),native)
 ptrset(6,1,0x80300000+((case+serial)%3)*4096,native)
 mem(0x80300000+((case+serial)%3)*4096+4000,struct.pack('<I',case*97+serial),native)
 if e==0:
  result=[-2147483648,-1,0,1,2,3,2147483647][(case//4)%7]&0xffffffff
  connection_results.add(result);return result
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
 for p,a,size in regions:mem(a,bytes((n*13+j*17)&255 for j in range(size)),native)
 ptrset(6,0,0x80300000+(n%3)*4096,native);ptrset(6,1,0x80300000+((n+2)%3)*4096,native)
 for j in range(2):
  mem(0x800F3A38+j*92+16,struct.pack('<i',[2,-2147483648,-1,0,1,2147483647][(n//112+j)%6]),native)
  mem(0x800F3A38+j*92+8,struct.pack('<H',n*131&65535),native)
  mem(0x800F3A38+j*92+60,struct.pack('<h',[-32768,-128,-127,-1,0,1,127,128,32767][(n+j)%9]),native)
 mem(0x8009EE68,struct.pack('<hh',[-32768,-1,0,1,2,32767][(n//8)%6],[-32768,-1,0,1,2,32767][(n//48)%6]),native)
 mem(0x800ABEB0,struct.pack('<I',[0,1,2,3,0x7fffffff,0x80000000,0xffffffff][n%7]),native)

count=int(sys.argv[2]) if len(sys.argv)>2 else 4096
for case in range(count):
 events=[];observed=[];init(case,False);u.reg_write(UC_MIPS_REG_A0,0x80300000+((case//4)%3)*4096);u.reg_write(UC_MIPS_REG_A1,[1,2,0,-1,-2147483648,2147483647][(case//28)%6]&0xffffffff);u.reg_write(UC_MIPS_REG_SP,0x803F0000);u.reg_write(UC_MIPS_REG_RA,0x803FF800)
 try:u.emu_start(0x80022098,0x803FF800,count=30000)
 except Exception:print('MIPS failure',case,hex(u.reg_read(UC_MIPS_REG_PC)));raise
 expected=snap(False);init(case,True);lib.DrawSteeringIndicator(C.c_void_p(host(0x80300000+((case//4)%3)*4096)),C.c_int([1,2,0,-1,-2147483648,2147483647][(case//28)%6]))
 if errors:raise errors[0]
 assert len(events)==len(observed),(case,len(events),len(observed))
 for j,(e,o) in enumerate(zip(events,observed)):
  assert e[:2]==o[:2],(case,j,names[e[0]],e[1],o[1])
  assert e[2]==o[2],(case,j,names[e[0]],'world mismatch')
 assert expected==snap(True),('final',case)
 if case and case%512==0:print(case,'cases',flush=True)
print(count,'cases passed;',('compiled' if os.getenv('RR_COMPILED_ELF') else 'original'),'MIPS/native full pointers, two full input owners, scratch pointers, packet arenas, full ROM shape/RGB/toggle owners, exact service args/snapshots, mutable callbacks, ABI clobbers',flush=True)

assert all(coverage),coverage
assert len(connection_results)==7,connection_results
result=dict(source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),header_sha256={str(f):hashlib.sha256(f.read_bytes()).hexdigest() for f in [Path('include/rr/controller_hud.h'),Path('include/rr/tables.h'),Path('include/rr/gpu.h'),Path('include/rr/sprite_emit.h')]},test_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),cases=count,service_counts=dict(zip(names,coverage)),connection_results=sorted(connection_results),engine='compiled' if os.getenv('RR_COMPILED_ELF') else 'original')
Path(os.getenv('RR_AUDIT_JSON',str(P/'audit.json'))).write_text(json.dumps(result,indent=2)+'\n')
