from pathlib import Path
import ctypes as C,struct,re,subprocess,os,sys,random,json,hashlib
from elftools.elf.elffile import ELFFile
from unicorn import *
from unicorn.mips_const import *
P=Path(os.getenv("RR_TEST_DIR","build/goal_50/part32/native"));P.mkdir(parents=True,exist_ok=True);source=Path(sys.argv[1]) if len(sys.argv)>1 else Path("tools/scripts/fixtures/legacy/func_800534D8.c");count=int(sys.argv[2]) if len(sys.argv)>2 else 8192
body=source.read_text().split('\n',1)[1];body=re.sub(r' __asm__\("[a-z0-9$]+"\)','',body)
services=[('CdAudioPlayStep','void',[]),('CdAudioPauseStep','void',[]),('CdAudioResumeStep','void',[]),('CdAudioStopStep','void',[]),('CdAudioLoopStep','void',[]),('CdAudioFilterStep','void',[]),('CdSync','s32',['s32','u8*']),('CdIntToPos','u8*',['s32','u8*']),('CdControlF','s32',['s32','const u8*']),('CdDiskReady','s32',['s32']),('CdFlush','void',[]),('CdAudioMute','void',[]),('CdAudioRequestPause','void',[]),('DebugPrintStub','s32',['const char*','RrAudioState*']),('CdAudioSetVolumeRaw','void',['s32']),('CdAudioFadeStep','void',[]),('CdLastCom','s32',[]),('CdPosToInt','s32',['const u8*']),('CdAudioSetVolume','void',['u16'])]
head='''#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "rr/audio_cd_controller.h"
static struct {u8 before[64];RrAudioState owner;u8 after[64];} storage;
#define SoundState storage.owner
#define CdAudioStopRequest SoundState.request26B4
char XaRepeatMsg[12];
typedef uintptr_t(*Callback)(unsigned,uintptr_t*);static Callback cb;
void set_callback(Callback p){cb=p;}
void *owner(void){return &SoundState;}void *format(void){return XaRepeatMsg;}
void guards(void){memset(storage.before,0xA6,64);memset(storage.after,0x6A,64);}
unsigned check(void){unsigned j;for(j=0;j<64;j++)if(storage.before[j]!=0xA6||storage.after[j]!=0x6A)return 1;return 0;}
'''
code=head
for i,(name,ret,types) in enumerate(services):
 if name=='DebugPrintStub':params='const char *a0,...';prefix='va_list ap;RrAudioState *a1;va_start(ap,a0);a1=va_arg(ap,RrAudioState*);va_end(ap);'
 else:params=','.join(t+' a'+str(j) for j,t in enumerate(types)) or 'void';prefix=''
 code+=ret+' '+name+'('+params+'){'+prefix+'uintptr_t a[4]={'+(','.join('(uintptr_t)a'+str(j) for j in range(len(types))) or '0')+'};uintptr_t v=cb('+str(i)+',a);'+('return ('+ret+')v;' if ret!='void' else '')+'}\n'
code+=body
(P/'native.c').write_text(code)
subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-shared','-fPIC','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-Iinclude','-I'+str(P),str(P/'native.c'),'-o',str(P/'native.dylib')],check=True)
lib=C.CDLL(str((P/'native.dylib').resolve()));lib.owner.restype=C.c_void_p;lib.format.restype=C.c_void_p;owner=lib.owner();fmt=lib.format();assert owner>0xffffffff
BASE=0x800F4DB8;SIZE=0xA8D0
u=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);u.mem_map(0,0x400000);u.mem_write(0x10000,Path('assets/PAL/main.exe').read_bytes()[0x800:])
if os.getenv('RR_COMPILED_ELF'):
 with open(os.environ['RR_COMPILED_ELF'],'rb') as f:
  e=ELFFile(f)
  for n in ['.text','.rodata']:
   sec=e.get_section_by_name(n)
   if sec:u.mem_write(sec['sh_addr']&0x1fffffff,sec.data())
def rd(a,n,native):return C.string_at(a,n) if native else bytes(u.mem_read(a&0x1fffffff,n))
def wr(a,b,native):
 if native:C.memmove(a,b,len(b))
 else:u.mem_write(a&0x1fffffff,b)
def field(off,v,native,n=4):wr((owner if native else BASE)+off,(v&((1<<(8*n))-1)).to_bytes(n,'little'),native)
def val(off,native,n=4):return int.from_bytes(rd((owner if native else BASE)+off,n,native),'little')
def snap(native):
 if native:assert not lib.check(),('redzone',case)
 return rd(owner if native else BASE,SIZE,native)
def ptr(a,native):
 if not a:return 0
 base=owner if native else BASE
 if base<=a<base+SIZE:return BASE+a-base
 if a==(fmt if native else 0x8001118C):return 0x8001118C
 return 'local'
case=0;events=[];observed=[];errors=[];coverage=set();phases=set();actions=set()
edges=[0,1,-1,2,3,5,16,29,30,360,361,32767,-32768,0x7fffffff,-0x80000000,0x40000000]
def service(i,args,native):
 name,ret,types=services[i];norm=[ptr(v,native) if '*' in t else v&0xffffffff for v,t in zip(args,types)];local=b''
 if name=='CdControlF' and args[1]:local=rd(args[1],2 if args[0]==13 else 3,native)
 if name=='CdPosToInt':local=rd(args[0],3,native)
 records=observed if native else events;records.append((i,norm,local,snap(native)));step=len(records);seed=case*13+step*17+i*7
 coverage.add(name)
 if case&32:
  # Every service can mutate independent real controller fields or TOC contents.
  field(0x2664,edges[seed%len(edges)],native);field(0x266c,edges[(seed+3)%len(edges)],native)
  field(0x2674,[-1,0,1,2,3,4,0x7fffffff][seed%7],native)
  field(0x2670,[-1,0,1,2,3,4,5,6,7][seed%9],native)
  field(0x269c,[-1,0,1,2,3][(seed//3)%5],native)
  field(0x2698,seed%256,native);field(0xa8cc,(seed//5)%16,native)
  for j,off in enumerate([0x2688,0x268c,0x2690,0x26a8,0x26d8,0x26dc,0x26e8,0x26f4]):field(off,edges[(seed+j*3)%len(edges)],native)
  field(0x28c8+(seed%4096)*8,edges[(seed+9)%len(edges)],native)
  field(0x26a0,seed&1,native);field(0x26ac,seed%6,native)
 if name=='CdSync':
  if args[1]:wr(args[1],bytes((seed+k*19)&255 for k in range(8)),native)
  return [0,1,2,5,-1,0x1000002][(case+step)%6]
 if name=='CdIntToPos':wr(args[1],bytes((seed+k*7)&255 for k in range(3)),native);return args[1]
 if name=='CdControlF':return [0,1,0x80000000][(case+step)%3]
 if name=='CdDiskReady':return [0,16,-1][case%3]
 if name=='CdLastCom':return 17 if case&1 else 0
 if name=='CdPosToInt':return edges[(case+step)%len(edges)]&0xffffffff
 if name=='DebugPrintStub':return 1
 return 0
CB=C.CFUNCTYPE(C.c_size_t,C.c_uint,C.POINTER(C.c_size_t))
@CB
def callback(i,args):
 try:return service(i,[args[j] for j in range(len(services[i][2]))],True)&0xffffffffffffffff
 except BaseException as e:errors.append(e);return 0
lib.set_callback(callback)
def _addr(name):
 import re as _re
 if _re.fullmatch(r'func_[0-9A-F]{8}',name):return __import__('c_functions').address_of(name)
 for l in open(__import__('pathlib').Path(__file__).resolve().parents[2]/'configs/PAL/sym.main.txt'):
  m=_re.match(r'^'+name+r'\s*=\s*0x([0-9A-Fa-f]+);',l)
  if m:return int(m.group(1),16)
 raise KeyError(name)
addrs={_addr(name):i for i,(name,_,_) in enumerate(services)};resume=None
for a in addrs:u.mem_write(a&0x1fffffff,struct.pack('<3I',0,0x03e00008,0))
def hook(uc,a,n,d):
 global resume
 if a==0x803ff000:uc.emu_stop();return
 if a in addrs:
  i=addrs[a];sp=uc.reg_read(UC_MIPS_REG_SP);args=[uc.reg_read(UC_MIPS_REG_A0+j) if j<4 else int.from_bytes(rd(sp+16+4*(j-4),4,False),'little') for j in range(len(services[i][2]))];resume=(i,args,uc.reg_read(UC_MIPS_REG_RA));uc.emu_stop();return
 # CODE decode avoids Unicorn's demonstrated MEM_WRITE/branch-delay bug.
 if 0x800534d8<=a<0x80053a18:
  word=int.from_bytes(rd(a,4,False),'little');op=word>>26
  if op in [40,41,42,43,46]:
   r=(word>>21)&31;imm=C.c_int16(word&65535).value;ea=(uc.reg_read(UC_MIPS_REG_0+r)+imm)&0xffffffff
   size={40:1,41:2,42:4,43:4,46:4}[op]
   if op in [42,46]:ea&=~3
   assert BASE<=ea and ea+size<=BASE+SIZE or 0x803dff00<=ea and ea+size<=0x803e0010,('write outside owner/stack',case,hex(a),hex(ea),size)
u.hook_add(UC_HOOK_CODE,hook)
rng=random.Random(534)
for case in range(count):
 initial=rng.randbytes(SIZE)
 for native in [False,True]:
  if native:lib.guards()
  wr(owner if native else BASE,initial,native)
  for j,off in enumerate([0x2664,0x266c,0x2670,0x2674,0x2688,0x268c,0x2690,0x269c,0x26a0,0x26a8,0x26ac,0x26b4,0x26d8,0x26e8,0x26f4]):field(off,edges[(case//(1+j%3)+j)%len(edges)],native)
  field(0x2698,(case//3)%256,native);field(0xa8cc,(case//7)%16,native)
  # Directed finite-state prefixes ensure every dispatch/action/CD callback occurs.
  if case<256:
   field(0x2664,-1 if case<64 else 1,native);field(0x2670,(case//8)%8-1,native);field(0x2674,(case//6)%6-1,native);field(0x269c,(case//2)%4-1,native)
  if case<2:
   field(0x2664,-1,native);field(0x2670,-1,native);field(0x26b4,2-case,native)
  if not native:
   phases.add(val(0x2674,False));actions.add(val(0x2670,False));events=[];u.reg_write(UC_MIPS_REG_SP,0x803e0000);u.reg_write(UC_MIPS_REG_RA,0x803ff000);pc=0x800534d8
   for calls in range(128):
    resume=None;u.emu_start(pc,0,count=50000)
    if resume is None:break
    i,args,pc=resume;r=service(i,args,False)
    for reg in [UC_MIPS_REG_AT,UC_MIPS_REG_V0,UC_MIPS_REG_V1,*range(UC_MIPS_REG_A0,UC_MIPS_REG_T7+1),UC_MIPS_REG_T8,UC_MIPS_REG_T9,UC_MIPS_REG_HI,UC_MIPS_REG_LO]:u.reg_write(reg,0xBAD00000+reg+len(events))
    u.reg_write(UC_MIPS_REG_V0,r&0xffffffff)
   assert resume is None and u.reg_read(UC_MIPS_REG_PC)==0x803ff000,case
   final=snap(False)
  else:
   observed=[];lib.CdAudioUpdate();assert not errors,errors
   if events!=observed:
    for k,(a,b) in enumerate(zip(events,observed)):
     if a!=b:
      off=next((j for j,(x,y)in enumerate(zip(a[3],b[3]))if x!=y),None);print('owner mismatch',k,hex(off) if off is not None else None,a[3][off:off+16].hex() if off is not None else '',b[3][off:off+16].hex() if off is not None else '',flush=True);break
   assert events==observed,('events',case,[(k,a[:3],b[:3])for k,(a,b) in enumerate(zip(events,observed))if a!=b][:1],len(events),len(observed))
   assert final==snap(True),('world',case)
 if case%512==0:print('PASS',case,flush=True)
print('PASS',count,'services',len(coverage),sorted(coverage),'phases',sorted(phases),'actions',sorted(actions),flush=True)

assert coverage=={x[0] for x in services}
assert {0,1,2,3}<=phases and {1,2,3,4,5,6}<=actions
if os.getenv('RR_AUDIT_JSON'):
 sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
 deps=subprocess.check_output(['cc','-MM','-Iinclude',str(source)],text=True).replace('\\\n',' ').split(':',1)[1].split()
 audit=dict(source=str(source),source_sha256=sha(source),test_sha256=sha(__file__),dependencies={f:sha(f) for f in deps},cases=count,engine='compiled MIPS' if os.getenv('RR_COMPILED_ELF') else 'original MIPS',services=sorted(coverage),phases=sorted(phases),actions=sorted(actions),owner_bytes=SIZE,owner_redzones=True,full_host_pointers=True,defined_arithmetic=True,mips_store_audit='CODE instruction decode SB/SH/SW/SWL/SWR',sanitizers=['undefined','bounds'])
 if os.getenv('RR_COMPILED_ELF'):audit['elf_sha256']=sha(os.environ['RR_COMPILED_ELF'])
 Path(os.environ['RR_AUDIT_JSON']).write_text(json.dumps(audit,indent=2)+'\n')
