"""Actual original/compiled MIPS vs native packet emitter + three real helper bodies."""
from pathlib import Path
import os,re,sys,subprocess,ctypes as C,struct,json,hashlib
from unicorn import Uc,UC_ARCH_MIPS,UC_MODE_MIPS32,UC_MODE_LITTLE_ENDIAN,UC_HOOK_CODE
from unicorn.mips_const import *
ROOT=next(p for p in Path(__file__).resolve().parents if (p/'tools/scripts/cc.sh').is_file());os.chdir(ROOT);source=Path(sys.argv[1]) if len(sys.argv)>1 else ROOT/'tools/scripts/fixtures/views/func_8001FEC0.c';triangle='2071' in str(source);entry=0x8002071C if triangle else 0x8001FEC0
P=Path(os.getenv('RR_TEST_DIR',str(ROOT/'build/native_tests/packet_emitters')));P.mkdir(exist_ok=True,parents=True)
header='rr/sprite_emit.h';body=source.read_text().split('\n',1)[1];body=re.sub(r' __asm__\("\$\d+"\)','',body)
head=r'''
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "HEADER"
static u32 arena[2048];void *memory(void){return arena;}size_t memory_size(void){return sizeof(arena);}
typedef void (*Callback)(unsigned,uintptr_t*);static Callback cb;void set_callback(Callback c){cb=c;}
void SetSprt(void*p){uintptr_t a[2]={(uintptr_t)p};cb(0,a);((u8*)p)[3]=4;((u8*)p)[7]=0x64;}
void SetPolyF3(void*p){uintptr_t a[2]={(uintptr_t)p};cb(0,a);((u8*)p)[3]=4;((u8*)p)[7]=0x20;}
void SetShadeTex(void*p,s32 flags){uintptr_t a[2]={(uintptr_t)p,(u32)flags};cb(1,a);((u8*)p)[7]=flags?((u8*)p)[7]|1:((u8*)p)[7]&0xFE;}
void SetSemiTrans(void*p,s32 flags){uintptr_t a[2]={(uintptr_t)p,(u32)flags};cb(1,a);((u8*)p)[7]=flags?((u8*)p)[7]|2:((u8*)p)[7]&0xFD;}
void AddPrim(u32*ordering,u32*p){uintptr_t a[2]={(uintptr_t)ordering,(uintptr_t)p};cb(2,a);*p=(*p&0xFF000000U)|(*ordering&0xFFFFFFU);*ordering=(*ordering&0xFF000000U)|((uintptr_t)p&0xFFFFFFU);}
'''.replace('HEADER',header)
file=P/(source.stem+'_native.c');file.write_text(head+body)
subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-shared','-fPIC','-Iinclude','-I'+str(source.parent),'-fsanitize=undefined,bounds','-fno-sanitize-recover=all',str(file),'-o',str(file.with_suffix('.dylib'))],check=True)
lib=C.CDLL(str(file.with_suffix('.dylib')));lib.memory.restype=C.c_void_p;lib.memory_size.restype=C.c_size_t
base=lib.memory();size=lib.memory_size();assert base>0xffffffff;palbase=0x81000000|(base&0xFFFFFF)
u=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);u.mem_map(0,0x4000000);u.mem_write(0x10000,Path('assets/PAL/main.exe').read_bytes()[0x800:])
if os.getenv('RR_COMPILED_ELF'):
 from elftools.elf.elffile import ELFFile
 with open(os.environ['RR_COMPILED_ELF'],'rb') as f:u.mem_write(entry&0x1FFFFFFF,ELFFile(f).get_section_by_name('.text').data())
def read(a,n):return bytes(u.mem_read(a&0x1FFFFFFF,n))
def write(a,b):u.mem_write(a&0x1FFFFFFF,b)
def pal(p):assert base<=p<=base+size;return palbase+p-base
def mem(a,b,native):
 assert palbase<=a and a+len(b)<=palbase+size
 if native:C.memmove(base+a-palbase,b,len(b))
 else:write(a,b)
def snap(native):return C.string_at(base,size) if native else read(palbase,size)
case=0;events=[];observed=[];errors=[];packet=order=rgb=0
ctors=[0x80095B54 if triangle else 0x80095B94,0x80095AF4 if triangle else 0x80095B24,0x80095A44]
def service(e,args,native):
 normalized=[pal(x) if native and (j==0 or e==2) else x&0xFFFFFFFF for j,x in enumerate(args)]
 trace=observed if native else events;trace.append((e,normalized,snap(native)))
 if case&1:
  serial=len(trace);mem(packet+10,bytes((case*17+serial*19+j*37)&255 for j in range(10)),native)
  mem(order,struct.pack('<I',(case*0x123457+serial*0x1579B)&0xFFFFFFFF),native)
  if triangle:mem(rgb,bytes((case+serial*29+j*31)&255 for j in range(3)),native)
CB=C.CFUNCTYPE(None,C.c_uint,C.POINTER(C.c_size_t))
def callback(e,args):
 try:service(e,list(args[:1 if e==0 else 2]),True)
 except BaseException as err:errors.append(err)
cb=CB(callback);lib.set_callback(cb)
def hook(uc,a,s,d):
 e=ctors.index(a);args=[uc.reg_read(UC_MIPS_REG_A0)&0xFFFFFFFF]
 if e:args.append(uc.reg_read(UC_MIPS_REG_A1)&0xFFFFFFFF)
 service(e,args,False)
for a in ctors:u.hook_add(UC_HOOK_CODE,hook,begin=a,end=a)
fn=getattr(lib,'EmitPolyF3' if triangle else 'GameQueueSprite');fn.restype=C.c_void_p
if triangle:fn.argtypes=[C.c_void_p,C.c_void_p,C.c_int32,C.c_int32,C.c_int32,C.c_void_p,C.c_int32]
else:fn.argtypes=[C.c_void_p,C.c_void_p]+[C.c_int32]*7
count=int(sys.argv[2]) if len(sys.argv)>2 else 16384
for case in range(count):
 po=32+((case//32)%64)*24;packet=palbase+po
 order=packet+[0,4,8,12,16,96][(case//2)%6] if (case//2)%7<6 else palbase+4096
 rgb=packet+[-7,0,3,4,5,6,7,8,12,17,19,23][(case//12)%12]
 vals=[0x80000000,0x7FFFFFFF,0xFFFFFFFF,0,1,0x10000,0xFFFF,0xFFFF8000,(case*0x123457)&0xFFFFFFFF]
 ints=[vals[(case//3+j*3)%len(vals)] for j in range(7)]
 args=[order,packet,*ints[:3],rgb,ints[3]] if triangle else [order,packet,*ints]
 events=[];observed=[];initial=bytes((j*29+case*19)&255 for j in range(size));write(palbase,initial)
 for j,a in enumerate(args[:4]):u.reg_write([UC_MIPS_REG_A0,UC_MIPS_REG_A1,UC_MIPS_REG_A2,UC_MIPS_REG_A3][j],a)
 write(0x803F0010,struct.pack('<'+'I'*(len(args)-4),*args[4:]));u.reg_write(UC_MIPS_REG_SP,0x803F0000);u.reg_write(UC_MIPS_REG_RA,0x803FF800)
 try:u.emu_start(entry,0x803FF800,count=1000)
 except Exception:print('MIPS failure',case,hex(u.reg_read(UC_MIPS_REG_PC)));raise
 expected=snap(False);result=u.reg_read(UC_MIPS_REG_V0)&0xFFFFFFFF
 C.memmove(base,initial,size);nargs=[base+a-palbase if j<2 or (triangle and j==5) else a for j,a in enumerate(args)];actual=fn(*nargs)
 if errors:raise errors[0]
 assert len(events)==len(observed)==3,(case,len(events),len(observed))
 for j,(e,o) in enumerate(zip(events,observed)):
  assert e[:2]==o[:2],(case,j,e[:2],o[:2]);assert e[2]==o[2],(case,j,'helper pre-state')
 assert expected==snap(True),(case,'full guarded arena');assert pal(actual)==result==packet+20,(case,'returned cursor')
 if case and case%4096==0:print(case,'cases',flush=True)
print(count,'cases passed;',('compiled' if os.getenv('RR_COMPILED_ELF') else 'original'),'MIPS/native; three actual GPU helpers, full guards, wide inputs, head/packet/RGB aliases, helper callback mutations, pointer low24 publication',flush=True)

result=dict(source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),test_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),header_sha256={str(f):hashlib.sha256(f.read_bytes()).hexdigest() for f in [Path('include/rr/gpu.h'),Path('include/rr/sprite_emit.h')]},cases=count,engine='compiled' if os.getenv('RR_COMPILED_ELF') else 'original',entry=entry)
Path(os.getenv('RR_AUDIT_JSON',str(P/(source.stem+'_audit.json')))).write_text(json.dumps(result,indent=2)+'\n')
