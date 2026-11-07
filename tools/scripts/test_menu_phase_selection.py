from pathlib import Path
from tempfile import TemporaryDirectory
import sys,os,ctypes as C,subprocess,struct,hashlib,json
from unicorn import Uc,UC_ARCH_MIPS,UC_MODE_MIPS32,UC_MODE_LITTLE_ENDIAN
from unicorn.mips_const import *
P=Path(__file__).resolve().parent
ROOT=next(p for p in P.parents if (p/'tools/scripts/cc.sh').is_file())
SOURCE=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'tools/scripts/fixtures/views/func_8002F5BC.c'
source=SOURCE.read_text();body=source[source.index('void SetNextModeByGameType'):]
fixture='''#include "types.h"
#include <string.h>
static union { unsigned int align; unsigned char b[16]; } phase,menu,aux;
#define ModeMenuCursorSaved (*(s16*)(phase.b+6))
#define D_800AD50A (*(u16*)(menu.b+6))
#define D_800FFC18 (*(s16*)(aux.b+6))
'''+body+'''
void setup(const void *p){memcpy(phase.b,p,16);memcpy(menu.b,(const char*)p+16,16);memcpy(aux.b,(const char*)p+32,16);}
void capture(void *p){memcpy(p,phase.b,16);memcpy((char*)p+16,menu.b,16);memcpy((char*)p+32,aux.b,16);}
'''
with TemporaryDirectory(prefix='rr2f5bc-') as td:
 d=Path(td);(d/'native.c').write_text(fixture)
 subprocess.run(['cc','-std=gnu11','-O2','-shared','-fPIC','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I'+str(ROOT/'include'),str(d/'native.c'),'-o',str(d/'native.dylib')],check=True)
 lib=C.CDLL(str(d/'native.dylib'));out=C.create_string_buffer(48)
 u=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);u.mem_map(0,0x400000);u.mem_write(0x10000,(ROOT/'assets/PAL/main.exe').read_bytes()[0x800:])
 if os.getenv('RR_COMPILED_ELF'):
  from elftools.elf.elffile import ELFFile
  with open(os.environ['RR_COMPILED_ELF'],'rb') as f:
   e=ELFFile(f)
   for name in ['.text','.rodata']:
    s=e.get_section_by_name(name)
    if s:u.mem_write(s['sh_addr']&0x1fffffff,s.data())
 expected_ids={0:0x105,1:0x131,2:0x137,3:0x127,4:0x149,5:0x147,6:0x201,7:0x14E,8:0x13B}
 addresses=[0xF4D26,0xAD504,0xFFC12]
 count=int(sys.argv[2]) if len(sys.argv)>2 else 65536
 for i in range(count):
  value=i if count==65536 else [0,1,2,3,4,5,6,7,8,9,32767,32768,65535][i%13]
  before=bytearray(((i*29+j*37+91)&255) for j in range(48));struct.pack_into('<H',before,6,value)
  expected=bytearray(before);struct.pack_into('<H',expected,22,expected_ids.get(value,0x131))
  if value==5:struct.pack_into('<H',expected,38,0)
  lib.setup(bytes(before))
  for j,a in enumerate(addresses):u.mem_write(a,bytes(before[j*16:j*16+16]))
  for r in range(UC_MIPS_REG_1,UC_MIPS_REG_31+1):u.reg_write(r,(0xA5A50000+i*31+r)&0xffffffff)
  u.reg_write(UC_MIPS_REG_SP,0x80300000);u.reg_write(UC_MIPS_REG_RA,0x802F0000)
  u.emu_start(0x8002F5BC,0x802F0000,count=200);assert u.reg_read(UC_MIPS_REG_PC)==0x802F0000
  lib.SetNextModeByGameType();lib.capture(out)
  actual=b''.join(bytes(u.mem_read(a,16)) for a in addresses)
  assert out.raw==actual==bytes(expected),(value,out.raw.hex(),actual.hex(),expected.hex())
 result={'source':str(SOURCE),'source_sha256':hashlib.sha256(SOURCE.read_bytes()).hexdigest(),'cases':count,'all_s16_inputs':count==65536,'engine':'compiled' if os.getenv('RR_COMPILED_ELF') else 'original','independent_expected_ids':expected_ids,'guards':'complete16B ownersaround each2B scalar','source_bounds':'no indexed memory or arithmetic domain restrictions'}
 audit=Path(os.getenv('RR_AUDIT_JSON',str(ROOT/'build/native_tests/menu_phase_selection.json')));audit.parent.mkdir(parents=True,exist_ok=True);audit.write_text(json.dumps(result,indent=2)+'\n')
 print(f'{count} original/compiled MIPS versus native and independent expected cases passed; full scalar guards unchanged')
