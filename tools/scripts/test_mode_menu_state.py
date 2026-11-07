"""Exact MIPS/native state oracle for the mode-menu controller (PAL).

Requires unicorn from requirements.txt. Services compare complete guarded state
before each call; the completed string helper executes on both sides.
"""
from pathlib import Path
import ctypes as C,struct,subprocess,sys,json,time
from unicorn import Uc,UC_ARCH_MIPS,UC_MODE_MIPS32,UC_MODE_LITTLE_ENDIAN,UC_HOOK_CODE
from unicorn.mips_const import UC_MIPS_REG_SP,UC_MIPS_REG_RA,UC_MIPS_REG_PC,UC_MIPS_REG_A0,UC_MIPS_REG_A1,UC_MIPS_REG_A2,UC_MIPS_REG_A3,UC_MIPS_REG_V0
assert sys.byteorder=='little', 'The native fixture models PAL little-endian storage'
P=Path('build/tests/mode_menu_state');P.mkdir(parents=True,exist_ok=True);raw=Path('assets/PAL/main.exe').read_bytes();source=Path(sys.argv[2] if len(sys.argv)>2 else Path('tools/scripts/fixtures/views/func_8007ABCC.c')).read_text();body=source[source.index('void CardMenuModeEnter(void) {'):]
class Core(C.Structure):
 _fields_=[('head',C.c_uint32*4),('words',C.c_uint32*6),('half',C.c_uint16*13),('choices',C.c_int16*3),('name',C.c_ubyte*24),('text',C.c_ubyte*24),('stored',C.c_ubyte*132),('snapshot',C.c_ubyte*132),('records',C.c_int32*3),('badges',C.c_ubyte*112),('root',C.c_uint32),('tail',C.c_uint32*4)]
assert C.sizeof(Core)==528
word_addresses=[0x800A45B8,0x800AC5D4,0x800F2DE4,0x800F4CA8]
half_addresses=[0x800AC594,0x800F3A1C,0x800F3A26,0x800F3AF8,0x800F4C6A,0x800F4D3C,0x800F4D44,0x800F4D8A,0x800FF688,0x800FFC00,0x800FFC18,0x8010948C,0x8010948E]
fields=[]
for i,a in enumerate(word_addresses):fields.append((a,Core.words.offset+i*4,4))
for i,a in enumerate(half_addresses):fields.append((a,Core.half.offset+i*2,2))
for name,a,n in [('choices',0x80109478,6),('name',0x800A457C,24),('text',0x800F4D50,24),('stored',0x800F2E30,132),('snapshot',0x8010B258,132),('records',0x801272F0,12),('badges',0x80180000,56)]:fields.append((a,getattr(Core,name).offset,n))
fields.append((0x80180100,Core.badges.offset+56,56))
services=[(0x80035024,1),(0x8009AEAC,3),(0x80092CB8,1),(0x8001ECE8,4),(0x8002A604,0),(0x8003E928,10),(0x80053BB4,1),(0x8003D228,7),(0x8003A198,0),(0x8003B934,0),(0x8002A344,1),(0x8002AF8C,3)]
service_map={a:(i+1,n) for i,(a,n) in enumerate(services)}
u=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);u.mem_map(0,0x210000);u.mem_write(0x10000,raw[0x800:]);core=None;events=[];initialSelection=0;freshChecks=set()
def read(a,n):return bytes(u.mem_read(a&0x1FFFFFFF,n))
def write(a,b):u.mem_write(a&0x1FFFFFFF,b)
def into():
 for a,o,n in fields:write(a,C.string_at(C.addressof(core)+o,n))
 write(0x800F3AFC,struct.pack('<I',0x80180000+core.root*0x100))
def out():
 for a,o,n in fields:C.memmove(C.addressof(core)+o,read(a,n),n)
 addr=struct.unpack('<I',read(0x800F3AFC,4))[0];assert addr in [0x80180000,0x80180100];core.root=(addr-0x80180000)//0x100

def mutation(event,args):
 scenario=core.words[5];core.root^=(scenario>>(event&7))&1
 core.words[0]^=(event*0x10501)&0xFFFFFFFF
 core.words[1]^=(event*0x8040201)&0xFFFFFFFF
 core.words[2]^=(event*0x10203)&0xFFFFFFFF
 core.half[12]=(event+scenario)%3
 core.choices[(event+scenario)%3]=C.c_int16(core.choices[(event+scenario)%3]^(event*0x121)).value
 core.stored[(event*13+scenario)%132]^=scenario&255
 core.badges[(event*5+scenario)%112]^=(event+scenario)&255
 if event==6:core.half[10]=(scenario+args[0])&7
 elif event==8:core.half[10]^=(scenario>>3)&7
 if event==5:core.name[0]=65+scenario%26
 if event==1:core.half[8]=65535 if scenario&1 else 0
 if event==10:core.half[7]^=0xAAAA

def hook(uc,a,size,data):
 event,argc=service_map[a];out()
 regs=[UC_MIPS_REG_A0,UC_MIPS_REG_A1,UC_MIPS_REG_A2,UC_MIPS_REG_A3];args=[uc.reg_read(r)&0xFFFFFFFF for r in regs[:min(argc,4)]]
 if argc>4:args.extend(struct.unpack('<'+str(argc-4)+'I',read(uc.reg_read(UC_MIPS_REG_SP)+16,(argc-4)*4)))
 args+=([0]*(10-len(args)));
 if event==8 and args[0]==1 and args[6]==0x702:
  if initialSelection!=3 and core.half[10]==3:freshChecks.add('changed_to_three')
  if initialSelection==3 and core.half[10]!=3:freshChecks.add('changed_from_three')
 events.append(struct.pack('<11I',event,*args)+bytes(core));mutation(event,args);into()
 if event in [1,2,10]:uc.reg_write(UC_MIPS_REG_V0,core.words[4])
 uc.reg_write(UC_MIPS_REG_PC,uc.reg_read(UC_MIPS_REG_RA))
for a,_ in services:u.hook_add(UC_HOOK_CODE,hook,begin=a,end=a)

import rename_symbols as _RS
_reg=_RS.registry()
def D(a):return _reg.get(f'D_{a:08X}',f'D_{a:08X}')
macros=''
for i,a in enumerate(word_addresses):macros+=f'#define {D(a)} (((s32 *)active.words)[{i}])\n'
for i,a in enumerate(half_addresses):
 expr=f'active.half[{i}]' if i in [2,7] else f'((s16 *)active.half)[{i}]'
 macros+=f'#define {D(a)} ({expr})\n'
for name,a in [('choices',0x80109478),('name',0x800A457C),('text',0x800F4D50),('stored',0x800F2E30),('snapshot',0x8010B258),('records',0x801272F0)]:macros+=f'#define {D(a)} (active.{name})\n'
macros+='#define BadgeRows badgeRoot\n'
functions=[]
signatures=[('s32','UpdateLinkHandshake',['s32']),('s32','_sio_control',['s32','s32','unsigned long']),('void','SetDispMask',['s32']),('void','SetupDrawDispEnvs',['s32','s32','s16','u32']),('void','ClearScreenOverlayState',[]),('void','InitUiRow',['s32']*10),('void','SeqStartTrack',['s32']),('void','InitBadgeRow',['s32','s32','s16','s32','u16','u16','u16']),('void','ResetScreenCounters',[]),('s32','LoadDialogueTableStep',[]),('void','UpdateScreenFade',['s32']),('void','SetMenuFrameOptions',['s32']*3)]
for event,(ret,name,types) in enumerate(signatures,1):
 params=','.join(t+' a'+str(i) for i,t in enumerate(types)) or 'void'
 params+=',...' if name=='InitUiRow' else ''
 args=','.join('(u32)a'+str(i) for i in range(len(types))) or '0'
 functions.append(f'{ret} {name}({params}){{u32 args[10]={{{args}}};service({event},args);'+('return (s32)active.words[4];' if ret=='s32' else '')+'}')
head=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/menu_setup.h"
#include "rr/frame_environment.h"
#include "rr/mode_menu_state.h"
typedef struct {u32 head[4],words[6];u16 half[13];s16 choices[3];char name[24],text[24];RrMenuSettings2C stored[3],snapshot[3];s32 records[3];RrBadgeRow1C badges[2][2];u32 root,tail[4];} Core;
typedef struct {u32 event,args[10];Core before;} Event;
static Core active;static RrBadgeRow1C *badgeRoot;
static Event expectedEvents[32];static u32 eventCount,eventIndex,caseIndex;
static void mismatch(const void *a,const void *b,size_t n){const u8 *x=a,*y=b;for(size_t j=0;j<n;j++)if(x[j]!=y[j]){fprintf(stderr,"case%u event%u offset%zu got%u wanted%u\n",caseIndex,eventIndex,j,x[j],y[j]);break;}abort();}
static void service(u32 event,const u32 args[10]){
 assert(eventIndex<eventCount);Event *expected=&expectedEvents[eventIndex++];
 assert(badgeRoot==active.badges[active.root]);
 if(expected->event!=event||memcmp(expected->args,args,40)){fprintf(stderr,"case%u event%u unexpected service%u expected%u\n",caseIndex,eventIndex,event,expected->event);abort();}
 if(memcmp(&active,&expected->before,sizeof(Core)))mismatch(&active,&expected->before,sizeof(Core));
 u32 scenario=active.words[5];active.root^=(scenario>>(event&7))&1;
 active.words[0]^=event*0x10501U;active.words[1]^=event*0x8040201U;active.words[2]^=event*0x10203U;
 active.half[12]=(event+scenario)%3;active.choices[(event+scenario)%3]^=event*0x121;
 ((u8*)active.stored)[(event*13+scenario)%132]^=scenario&255;((u8*)active.badges)[(event*5+scenario)%112]^=(event+scenario)&255;
 if(event==6)active.half[10]=(scenario+args[0])&7;
 else if(event==8)active.half[10]^=(scenario>>3)&7;
 if(event==5)active.name[0]=65+scenario%26;
 if(event==1)active.half[8]=scenario&1?65535:0;
 if(event==10)active.half[7]^=0xAAAA;
 badgeRoot=active.badges[active.root];
}
'''
native=head+macros+'\n'+'\n'.join(functions)+'\n'+Path('tools/scripts/fixtures/views/func_8002B75C.c').read_text()+'\n'+body+r'''
int main(int argc,char**argv){
 assert(sizeof(Core)==528&&sizeof(Event)==572&&sizeof(RrMenuSettings2C)==44&&sizeof(RrBadgeRow1C)==28);assert((uintptr_t)&active>UINT32_MAX);
 FILE *file=fopen(argv[1],"rb");assert(file);Core before,expected;
 for(caseIndex=0;fread(&before,sizeof(Core),1,file)==1;caseIndex++){
  assert(fread(&expected,sizeof(Core),1,file)==1);assert(fread(&eventCount,4,1,file)==1);assert(eventCount<=32);assert(fread(expectedEvents,sizeof(Event),eventCount,file)==eventCount);
  memcpy(&active,&before,sizeof(Core));badgeRoot=active.badges[active.root];eventIndex=0;
  CardMenuModeEnter();assert(eventIndex==eventCount);assert(badgeRoot==active.badges[active.root]);
  if(memcmp(&active,&expected,sizeof(Core)))mismatch(&active,&expected,sizeof(Core));
 }
 assert(feof(file));fclose(file);printf("%u cases passed; exact guarded state before every service and after original MIPS execution; actual string helper\n",caseIndex);return 0;
}
'''
(P/'state_native.c').write_text(native)
subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-Iinclude','-fsanitize=undefined,bounds','-fno-sanitize-recover=all',str(P/'state_native.c'),'-o',str(P/'state_native')],check=True)
fixture=P/'state_vectors.bin'
if len(sys.argv)>3 and sys.argv[3]=='--reuse':subprocess.run([str(P/'state_native'),str(fixture)],check=True);sys.exit(0)

def initial(n):
 c=Core();v=(n^0xD37A1C59)&0xffffffff
 for j in range(C.sizeof(c)):
  v^=v<<13&0xffffffff;v^=v>>17;v^=v<<5&0xffffffff;C.c_ubyte.from_address(C.addressof(c)+j).value=v&255
 c.words[5]=n;c.root=n&1;c.half[12]=(n>>2)%3
 for j in range(24):c.name[j]=(65+(n+j)%26) if j<n%24 else 0
 if n<65536:c.half[7]=0;c.half[10]=n
 elif n<131072:c.half[7]=n-65536;c.half[10]=(n*17)&65535
 else:c.half[7]=1;c.half[10]=[0,1,2,3,5,6,7,65535][n%8]
 c.half[6]=4 if n&4 else (n*29)&65535;c.half[8]=65535 if n&8 else 0;c.half[2]=1 if n&16 else (n*7)&65535
 c.words[4]=[0,1,65537,0xFFFF0001,0x80000000,0xFFFFFFFF,2,0x7FFFFFFF][n%8]
 c.words[2]=[0,1,0xFFFFFFFF,0x7FFFFFFF,0x80000000,(n*0x912751)&0xFFFFFFFF][n%6]
 # Repeated meaningful modes under independent callback and pointer profiles.
 if n>=196608:c.half[7]=0;c.half[10]=[0,1,2,3,5,6,7,-1,4][n%9]&65535
 return c
count=int(sys.argv[1]) if len(sys.argv)>1 else 200704
start=time.monotonic();coverage={'phases':set(),'selections':set(),'services':set(),'ready':set(),'roots':set()}
with fixture.open('wb') as f:
 for n in range(count):
  core=initial(n);initialSelection=core.half[10];before=bytes(core);events=[];coverage['phases'].add(core.half[7]);coverage['roots'].add(core.root)
  if core.half[7]==0:coverage['selections'].add(core.half[10])
  if core.half[7]==1:coverage['ready'].add(core.words[4])
  into();write(0x801FEE00,b'\xA6'*0x300);u.reg_write(UC_MIPS_REG_SP,0x801FF000);u.reg_write(UC_MIPS_REG_RA,0x801FF800)
  try:u.emu_start(0x8007ABCC,0x801FF800,count=20000)
  except Exception:print('MIPS failure case',n,hex(u.reg_read(UC_MIPS_REG_PC)),flush=True);raise
  assert u.reg_read(UC_MIPS_REG_PC)==0x801FF800;out();assert len(events)<=32
  for event in events:coverage['services'].add(struct.unpack_from('<I',event)[0])
  f.write(before);f.write(bytes(core));f.write(struct.pack('<I',len(events)));f.write(b''.join(events))
  if n and n%8192==0:print('MIPS cases',n,flush=True)
print('MIPS oracle complete',count,'seconds',round(time.monotonic()-start,2),flush=True)
if count>=196617:
 assert len(coverage['phases'])==len(coverage['selections'])==65536
 assert freshChecks=={'changed_to_three','changed_from_three'},freshChecks
 assert coverage['services']==set(range(1,13)) and len(coverage['ready'])==8 and coverage['roots']=={0,1}
coverage['fresh_mode_checks']=freshChecks
(P/'state_coverage.json').write_text(json.dumps({k:sorted(v) if len(v)<20 else {'distinct':len(v)} for k,v in coverage.items()},indent=2)+'\n')
subprocess.run([str(P/'state_native'),str(fixture)],check=True)
