from pathlib import Path
from tempfile import TemporaryDirectory
import ctypes as C,subprocess,sys,os,re,struct,random,json,hashlib
from unicorn import Uc,UC_ARCH_MIPS,UC_MODE_MIPS32,UC_MODE_LITTLE_ENDIAN,UC_HOOK_CODE
from unicorn.mips_const import *
ROOT=Path(__file__).resolve().parents[2];P=ROOT/'include/rr';os.chdir(ROOT)
source=Path(sys.argv[1]if len(sys.argv)>1 else ROOT/'tools/scripts/fixtures/views/func_80050BCC.c').resolve();count=int(sys.argv[2])if len(sys.argv)>2 else 4096
body=source.read_text();body=body[body.index('void SlotSetupBank'):];body=re.sub(r'\s+(?:__asm__|asm)\s*\("\$\d+"\)','',body)
# Make implicit read/write register inputs explicit for host diagnostics.
body=re.sub(r'((?:__asm__|asm)\(""\s*:\s*"\+r"\((\w+)\))',lambda m:'asm("" : : "r"('+m[2]+'));'+m[1],body)
# Target-only dead-register clobbers have identity semantics in host C.
body=re.sub(r'(?:__asm__|asm)\s*\(\s*""\s*:\s*:\s*:\s*"\$21"\s*,\s*"\$22"\s*,\s*"\$2"\s*,\s*"memory"\s*\)', 'asm("" : : : "memory")', body)
# Physical MIPS FP clobber has no logical operation in the host C model.
for reg in ['$30','$fp','fp']:body=body.replace('asm("" : : : "'+reg+'");','asm("" : : : "memory");')
services=[('SsVabClose','s32',['s16']),('PsyqBzero','void',['void*','s32']),('SsVabOpenHeadSticky','s32',['const u8*','s32','s32']),('SsVabTransBody','s32',['const u16*','s32']),('SsVabTransCompleted','s32',['s32']),('SlotLoadCurves','void',['RrAudioProgramData*','s32']),('SsUtGetVagAtr','s32',['s32','s16','s32','u8*'])]
class Setup(C.Structure):_fields_=[('bank00',C.c_void_p),('data04',C.c_void_p),('group08',C.c_void_p),('slot0C',C.c_int32)]
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();pack=lambda x:struct.pack('<I',x&0xffffffff)
with TemporaryDirectory(prefix='rr50bcc-')as td:
 d=Path(td);code='#include <stdint.h>\n#include <stddef.h>\n#include "rr/layout.h"\n#undef RR_ASSERT_SIZE\n#undef RR_ASSERT_OFFSET\n#define RR_ASSERT_SIZE(t,n)\n#define RR_ASSERT_OFFSET(t,f,n)\n#include "rr/audio_bank_setup.h"\nstruct {u8 a[16];RrAudioState v;u8 b[16];} state;\nstruct {u8 a[16];RrAudioBankSetup v;u8 b[16];} input;\nstruct {u8 a[16];u8 v[3064];u8 b[16];} resources;\nstruct {u8 a[16];RrAudioState v;u8 b[16];} rootBox;\n#define SoundState rootBox.v\n#define SoundSlots (SoundState.slots38)\ntypedef uintptr_t(*Callback)(unsigned,uintptr_t*);static Callback cb;void set_callback(Callback p){cb=p;}\n'
 # The actual canonical owner is surrounded by separate explicit guards below.
 code=code.replace('struct {u8 a[16];RrAudioState v;u8 b[16];} state;','')
 for i,(name,ret,types)in enumerate(services):
  code+=ret+' '+name+'('+','.join(t+' a'+str(j)for j,t in enumerate(types))+'){uintptr_t a[]={'+','.join('(uintptr_t)a'+str(j)for j in range(len(types)))+'};uintptr_t r=cb('+str(i)+',a);'+('return('+ret+')r;'if ret!='void'else'')+'}\n'
 code+=body+'\nvoid *owner(int n){return n==0?(void*)&SoundState:n==1?(void*)&input.v:(void*)resources.v;}\nvoid run(void){SlotSetupBank(&input.v);}\n'
 (d/'test.c').write_text(code);subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-Werror=uninitialized','-shared','-fPIC','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-Iinclude','-I'+str(P),str(d/'test.c'),'-o',str(d/'test.dylib')],check=True)
 lib=C.CDLL(str(d/'test.dylib'));lib.owner.restype=C.c_void_p;N=[lib.owner(i)for i in range(3)];assert all(n>0xffffffff for n in N)
 A=[0x800F4DB8,0x80180000,0x80190000];S=[0xA8D0,16,3064];u=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);u.mem_map(0,0x400000);u.mem_write(0x10000,Path('assets/PAL/main.exe').read_bytes()[0x800:]);elf=os.getenv('RR_COMPILED_ELF')
 if elf:
  from elftools.elf.elffile import ELFFile
  with open(elf,'rb')as f:e=ELFFile(f);t=e.get_section_by_name('.text');u.mem_write(t['sh_addr']&0x1fffffff,t.data())
 read=lambda a,n:bytes(u.mem_read(a&0x1fffffff,n));write=lambda a,b:u.mem_write(a&0x1fffffff,b)
 def pal(a):
  if a==0xffffffffffffffff:return 0xffffffff
  for n,base,size in zip(N,A,[S[0],C.sizeof(Setup),3064]):
   if n<=a<n+size:return base+a-n
  raise AssertionError(('unknown native pointer',hex(a)))
 def get(a,n,native):
  if not native:return read(a,n)
  for base,host,size in zip(A,N,[S[0],C.sizeof(Setup),3064]):
   if base<=a and a+n<=base+size:return C.string_at(host+a-base,n)
  raise AssertionError(('read',hex(a),n))
 def put(a,b,native):
  if not native:write(a,b);return
  for base,host,size in zip(A,N,[S[0],C.sizeof(Setup),3064]):
   if base<=a and a+len(b)<=base+size:C.memmove(host+a-base,b,len(b));return
  raise AssertionError(('write',hex(a),len(b)))
 def setInput(native,bank,data,group,slot):
  if native:
   x=Setup.from_address(N[1]);x.bank00=0xffffffffffffffff if bank==0xffffffff else N[2]+bank-A[2];x.data04=0xffffffffffffffff if data==0xffffffff else N[2]+data-A[2];x.group08=0xffffffffffffffff if group==0xffffffff else N[2]+group-A[2];x.slot0C=slot
  else:write(A[1],pack(bank)+pack(data)+pack(group)+pack(slot))
 def snapshot(native):
  if native:
   x=Setup.from_address(N[1]);assert C.string_at(N[1]+28,C.sizeof(Setup)-28)==b'\xa5'*(C.sizeof(Setup)-28);inp=pack(pal(x.bank00))+pack(pal(x.data04))+pack(pal(x.group08))+pack(x.slot0C)
  else:inp=read(A[1],16)
  guards=tuple((C.string_at(n-16,16)+C.string_at(n+size,16))if native else(read(a-16,16)+read(a+targetSize,16))for n,a,size,targetSize in zip(N,A,[S[0],C.sizeof(Setup),3064],S))
  return get(A[0],S[0],native),inp,get(A[2],3064,native),guards
 def service(i,args,native):
  nonlocalDummy=None
  name=services[i][0];seq=len(observed if native else events);normal=[]
  for j,(a,t)in enumerate(zip(args,services[i][2])):
   if '*'in t and name=='SsUtGetVagAtr':
    if not native:assert u.reg_read(UC_MIPS_REG_SP)<=a and a+24<=0x803e0000,('tone stack',case)
    else:assert a>0xffffffff
    normal.append('tone')
   elif '*'in t:normal.append(pal(a)if native else a)
   else:normal.append(a&0xffffffff)
  if name in ['SsVabOpenHeadSticky','SsVabTransBody']:normal.append(get(normal[0],32,native))
  if name=='SlotLoadCurves':normal.append(get(normal[0],0x5BC,native))
  event=(i,normal,snapshot(native));(observed if native else events).append(event);coverage.add(name)
  if name=='PsyqBzero':assert normal[:2]==[A[0]+0x38+bankIndex*0x620,0x620];put(normal[0],b'\0'*0x620,native)
  if name=='SlotLoadCurves':put(A[0]+0x74+normal[1]*0x620,get(normal[0],0x5BC,native),native)
  # Mutations stay in the complete canonical owner and can affect subsequent reloads.
  if case&1:
   for slot in range(2):
    put(A[0]+0x38+slot*0x620+4,struct.pack('<h',[-32768,-1,0,1,32767][(case+seq+slot)%5]),native)
    for j in range(10):put(A[0]+0x38+slot*0x620+0x5dc+j*2,struct.pack('<H',(case*97+seq*31+j*8191)&65535),native)
  if case&2 and i<6:setInput(native,A[2]+((case+seq)&1)*32 if (case+seq)%4 else 0xffffffff,A[2]+64+((case+seq+1)&1)*32 if (case+seq)%5 else 0xffffffff,A[2]+128+((case+seq)&1)*0x5BC if (case+seq)%3 else 0xffffffff,1-bankIndex)
  if name=='SsUtGetVagAtr':
   data=bytes((case*17+seq*29+j*31)&255 for j in range(24));a=args[3]
   for start,end in [(0,14),(16,24)]:
    if native:C.memmove(a+start,data[start:end],end-start)
    else:write(a+start,data[start:end])
  return (0x9a000000 | ((case*7919+seq*113+i*31)&65535))if name in ['SsVabOpenHeadSticky','SsVabTransBody']else 0
 def callback(i,p):
  try:return service(i,[p[j]for j in range(len(services[i][2]))],True)
  except BaseException as e:errors.append(e);return 0
 cb=C.CFUNCTYPE(C.c_uint64,C.c_uint,C.POINTER(C.c_uint64))(callback);lib.set_callback(cb);targets={__import__('c_functions').address_of(n):i for i,(n,_,_)in enumerate(services)}
 def hook(uc,pc,size,data):
  if pc not in targets:return
  i=targets[pc];regs=[UC_MIPS_REG_A0,UC_MIPS_REG_A1,UC_MIPS_REG_A2,UC_MIPS_REG_A3];args=[u.reg_read(r)for r in regs[:len(services[i][2])]];ra=u.reg_read(UC_MIPS_REG_RA);result=service(i,args,False)
  for r in [UC_MIPS_REG_AT,UC_MIPS_REG_V0,UC_MIPS_REG_V1,*regs,*range(UC_MIPS_REG_T0,UC_MIPS_REG_T7+1),UC_MIPS_REG_T8,UC_MIPS_REG_T9]:u.reg_write(r,(0xa5a50000+case*31+i*17+r)&0xffffffff)
  u.reg_write(UC_MIPS_REG_HI,0x52a55a25);u.reg_write(UC_MIPS_REG_LO,0x25a55a52);u.reg_write(UC_MIPS_REG_V0,result&0xffffffff);u.reg_write(UC_MIPS_REG_PC,ra)
 u.hook_add(UC_HOOK_CODE,hook);ctx=u.context_save();coverage=set()
 def initialize(native):
  if native:C.memmove(N[1],b'\xa5'*C.sizeof(Setup),C.sizeof(Setup))
  for n,a,size,targetSize in zip(N,A,[S[0],C.sizeof(Setup),3064],S):
   if native:C.memmove(n-16,b'\xa5'*16,16);C.memmove(n+size,b'\xa5'*16,16)
   else:write(a-16,b'\xa5'*16);write(a+targetSize,b'\xa5'*16)
  put(A[0],bytes(((case*17+j*29)&255)for j in range(S[0])),native);put(A[2],bytes(((case*7+j*31)&255)for j in range(3064)),native);setInput(native,A[2]+(case&1)*32 if case%3 else 0xffffffff,A[2]+64+(case&1)*32 if case%5 else 0xffffffff,A[2]+128+(case&1)*0x5BC if case%3 else 0xffffffff,bankIndex)
 for case in range(count):
  u.context_restore(ctx);bankIndex=case&1;events=[];observed=[];errors=[];initialize(False);write(0x803df000,b'\x5a'*0x2000)
  for r in range(UC_MIPS_REG_1,UC_MIPS_REG_31+1):u.reg_write(r,(0x5aa50000+case*31+r)&0xffffffff)
  u.reg_write(UC_MIPS_REG_A0,A[1]);u.reg_write(UC_MIPS_REG_SP,0x803e0000);u.reg_write(UC_MIPS_REG_RA,0x803ff000);saved={r:u.reg_read(r)for r in [*range(UC_MIPS_REG_S0,UC_MIPS_REG_S7+1),UC_MIPS_REG_GP,UC_MIPS_REG_SP,UC_MIPS_REG_FP,UC_MIPS_REG_RA]}
  u.emu_start(0x80050BCC,0x803ff000,count=10000);assert u.reg_read(UC_MIPS_REG_PC)==0x803ff000 and all(u.reg_read(r)==v for r,v in saved.items()),('return/callee',case,[(r,hex(v),hex(u.reg_read(r)))for r,v in saved.items()if u.reg_read(r)!=v],hex(u.reg_read(UC_MIPS_REG_PC)));expected=snapshot(False)
  initialize(True);lib.run()
  if errors:raise errors[0]
  assert len(events)==len(observed),('service count',case)
  for j,(a,b)in enumerate(zip(events,observed)):assert a==b,('service',case,j,services[a[0]][0])
  assert expected==snapshot(True),('final',case)
  if case and case%512==0:print(case,'cases',flush=True)
 assert coverage=={n for n,_,_ in services}if count>=512 else True
 result=dict(cases=count,engine=('compiled'if elf else'original')+' MIPS/native C',source_sha256=sha(source),header_sha256=sha(P/'audio_bank_setup.h'),test_sha256=sha(__file__),elf_sha256=sha(elf)if elf else None,full_host_pointers=True,full_owner_snapshots=True,mutating_callbacks=True,callee_saves_stack_return=True,caller_save_hi_lo_poison=True,services=sorted(coverage))
 if os.getenv('RR_AUDIT_JSON'):Path(os.environ['RR_AUDIT_JSON']).write_text(json.dumps(result,indent=2)+'\n')
 print(count,result['engine'],'cases passed;7 services',flush=True)
