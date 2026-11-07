"""Frame submission: full native pointers versus original/linked MIPS.

Acquire, submit and release are stateful software boundaries, not hardware tests.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import hashlib,json,os,re,struct,subprocess,sys
from unicorn import Uc,UC_ARCH_MIPS,UC_MODE_MIPS32,UC_MODE_LITTLE_ENDIAN,UC_HOOK_CODE,UC_HOOK_MEM_WRITE
from unicorn.mips_const import *
from elftools.elf.elffile import ELFFile
ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'src/overlays/PAL/movie_playback/func_801150D8.c'
CASES=300
C=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/overlay668_frame_submit.h"
static struct {u8 pre[16];RrStrDisplayConfig c;u8 post[16];} cfg;
static struct {u8 pre[16];RrStrStreamPrefix s[3];u8 post[16];} streams, saved;
static u8 backing[3][6][64],saved_backing[3][6][64];
static RrLibQueueRecord32 queues[3],saved_queues[3];
RrStrStreamPrefix *StrStreamState;
static unsigned scenario,polls,limit,event;
static u8 expected[40];
static const u32 selectors[6]={0,1,2,0xffffffffU,0x80000000U,0x10000};
static const s32 results[6]={0,1,-1,65537,(-2147483647-1),2147483647};
static void check(void){assert(!memcmp(&cfg.c,expected,40));}
static void change(unsigned salt){unsigned i;for(i=0;i<40;i++)expected[i]=(u8)(scenario*17+polls*13+i*7+salt);memcpy(&cfg.c,expected,40);}
u8 *movie_playback_func_80115164(RrStrDisplayConfig *c){
 u32 selector;assert(c==&cfg.c&&event==0);check();polls++;change(0);
 selector=selectors[(scenario/6+polls)%6];memcpy(expected,&selector,4);memcpy(&cfg.c,expected,40);
 StrStreamState=&streams.s[(scenario+polls)%3];
 return polls==limit?backing[scenario%3][5]:0;
}
void movie_playback_func_80116958(u8 *data,void *commands){
 u32 selector=selectors[(scenario/6+polls)%6]<1U;
 assert(event++==0&&polls==limit&&data==backing[scenario%3][5]);
 memcpy(expected,&selector,4);check();
 assert(commands==streams.s[(scenario+polls)%3].commands10[selector]);
 assert((uintptr_t)data>UINT32_MAX&&(uintptr_t)commands>UINT32_MAX);
 change(31);StrStreamState=&streams.s[(scenario+polls+1)%3];
}
s32 StFreeRing(void *data){
 assert(event++==1&&data==backing[scenario%3][5]);check();change(63);
 StrStreamState=&streams.s[(scenario+polls+2)%3];return results[(scenario/36)%6];
}
'''
TAIL=r'''
int main(void){unsigned i,j;const unsigned limits[6]={1,2,1999,2000,2001,17};
 _Static_assert(sizeof(void*)==8,"full native pointers");
 _Static_assert(sizeof(RrStrDisplayConfig)==40,"config owner");
 _Static_assert(offsetof(RrStrStreamPrefix,commands10)==16,"commands native");
 _Static_assert(offsetof(RrStrStreamPrefix,buffers18)==32,"buffers native");
 _Static_assert(offsetof(RrStrStreamPrefix,queue20)==48,"queue native");
 _Static_assert(sizeof(RrStrStreamPrefix)==56,"stream native");
 for(scenario=0;scenario<300;scenario++){
  memset(&cfg,0xA5,sizeof cfg);memset(&streams,0x5A,sizeof streams);
  memset(backing,0x37,sizeof backing);memset(queues,0x63,sizeof queues);
  for(i=0;i<3;i++){
   for(j=0;j<2;j++){streams.s[i].commands10[j]=backing[i][j];streams.s[i].buffers18[j]=backing[i][j+2];}
   streams.s[i].queue20=&queues[i];
  }
  saved=streams;memcpy(saved_backing,backing,sizeof backing);memcpy(saved_queues,queues,sizeof queues);
  polls=event=0;limit=limits[scenario%6];memcpy(expected,&cfg.c,40);StrStreamState=&streams.s[scenario%3];
  s32 result=movie_playback_func_801150D8(&cfg.c);
  assert(polls==(limit>2000?2000:limit));assert(event==(limit>2000?0:2));check();
  assert(result==(limit>2000?-1:results[(scenario/36)%6]));
  assert(!memcmp(&streams,&saved,sizeof streams));assert(!memcmp(backing,saved_backing,sizeof backing));assert(!memcmp(queues,saved_queues,sizeof queues));
  for(i=0;i<16;i++)assert(cfg.pre[i]==0xA5&&cfg.post[i]==0xA5);
  printf("%08x %u %u %ld ",(u32)result,polls,event,(long)(StrStreamState-streams.s));
  for(i=0;i<40;i++)printf("%02x",((u8*)&cfg.c)[i]);puts("");
 }return 0;
}
'''
def main():
 body=re.sub(r'__asm__\("\$\d+"\)','',SOURCE.read_text())
 with TemporaryDirectory() as tmp:
  c=Path(tmp)/'test.c';exe=Path(tmp)/'test';c.write_text(C+body+TAIL)
  subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(c),'-o',str(exe)],check=True)
  native=subprocess.check_output([str(exe)],text=True).splitlines()
 blob=(ROOT/'disc/PAL/R4.BIN').read_bytes();sector=struct.unpack_from('<I',blob,4+668*4)[0]
 code=blob[sector*2048+0x801150D8-0x80114878:sector*2048+0x801150D8-0x80114878+140]
 elf=os.environ.get('RR_COMPILED_ELF')
 if elf:
  with open(elf,'rb') as f:code=ELFFile(f).get_section_by_name('.text').data()
 uc=Uc(UC_ARCH_MIPS,UC_MODE_MIPS32|UC_MODE_LITTLE_ENDIAN);uc.mem_map(0,0x200000)
 # KSEG0 aliases map to the same physical RAM in Unicorn MIPS.
 start=0x801150D8;config=0x800D0010;root=0x800AC708;stop=0x80010000;sp=0x801F0000
 stream=lambda i:0x800D1000+i*0x100
 back=lambda i,j:0x800D2000+i*0x400+j*64
 queue=lambda i:0x800D4000+i*0x40
 write=lambda a,b:uc.mem_write(a&0x1fffffff,bytes(b))
 read=lambda a,n:bytes(uc.mem_read(a&0x1fffffff,n))
 w32=lambda a,v:write(a,struct.pack('<I',v&0xffffffff))
 reg=lambda i:uc.reg_read(UC_MIPS_REG_0+i)&0xffffffff
 put=lambda i,v:uc.reg_write(UC_MIPS_REG_0+i,v&0xffffffff)
 write(start,code)
 selectors=[0,1,2,0xffffffff,0x80000000,0x10000];results=[0,1,0xffffffff,65537,0x80000000,0x7fffffff]
 state={};seen_selectors=set();seen_results=set()
 def hook(u,pc,size,_):
  if pc not in [0x80115164,0x80116958,0x8008F764,stop]:return
  if pc==stop:u.emu_stop();return
  s=state;scenario=s['scenario'];polls=s['polls'];expected=s['expected']
  if pc==0x80115164:
   assert reg(4)==config and s['event']==0
   assert read(config,40)==expected
   polls+=1;s['polls']=polls
   expected=bytearray((scenario*17+polls*13+i*7)&255 for i in range(40))
   expected[:4]=struct.pack('<I',selectors[(scenario//6+polls)%6]);write(config,expected);w32(root,stream((scenario+polls)%3))
   result=back(scenario%3,5) if polls==s['limit'] else 0
  elif pc==0x80116958:
   selector=int(selectors[(scenario//6+polls)%6]<1);seen_selectors.add(selector);expected[:4]=struct.pack('<I',selector)
   assert s['event']==0 and polls==s['limit'] and reg(4)==back(scenario%3,5)
   assert reg(5)==back((scenario+polls)%3,selector) and read(config,40)==expected
   s['event']=1;expected=bytearray((scenario*17+polls*13+i*7+31)&255 for i in range(40));write(config,expected);w32(root,stream((scenario+polls+1)%3));result=0
  else:
   assert s['event']==1 and reg(4)==back(scenario%3,5) and read(config,40)==expected
   s['event']=2;expected=bytearray((scenario*17+polls*13+i*7+63)&255 for i in range(40));write(config,expected);w32(root,stream((scenario+polls+2)%3));result=results[(scenario//36)%6];seen_results.add(result)
  s['expected']=expected
  ret=reg(31)
  for r in [2,3,4,5,6,7,8,9,10,11,12,13,14,15,24,25]:put(r,0xDEAD0000+r)
  put(2,result);u.reg_write(UC_MIPS_REG_HI,0x12345678);u.reg_write(UC_MIPS_REG_LO,0x87654321);u.reg_write(UC_MIPS_REG_PC,ret)
 def store(u,access,address,size,value,_):
  a=address&0x1fffffff
  assert (config&0x1fffffff)<=a and a+size<=(config&0x1fffffff)+4 or (sp&0x1fffffff)-32<=a and a+size<=(sp&0x1fffffff),hex(address)
 uc.hook_add(UC_HOOK_CODE,hook);uc.hook_add(UC_HOOK_MEM_WRITE,store)
 limits=[1,2,1999,2000,2001,17]
 for scenario in range(CASES):
  write(config-16,b'\xA5'*72);write(0x800D1000,b'\x5A'*0x300);write(0x800D2000,b'\x37'*0xc00);write(0x800D4000,b'\x63'*0xc0)
  for i in range(3):
   for j in range(2):w32(stream(i)+16+j*4,back(i,j));w32(stream(i)+24+j*4,back(i,j+2))
   w32(stream(i)+32,queue(i))
  saved=read(0x800D1000,0x30c0);w32(root,stream(scenario%3))
  state.update(scenario=scenario,polls=0,event=0,limit=limits[scenario%6],expected=bytearray(b'\xA5'*40))
  for r in range(1,32):put(r,0xBEEF0000+r)
  put(4,config);put(29,sp);put(31,stop)
  uc.emu_start(start,stop,count=100000)
  assert uc.reg_read(UC_MIPS_REG_PC)==stop
  line=f'{reg(2):08x} {state["polls"]} {state["event"]} {(struct.unpack("<I",read(root,4))[0]-stream(0))//0x100} {read(config,40).hex()}'
  assert line==native[scenario],(scenario,line,native[scenario])
  assert saved==read(0x800D1000,0x30c0)
  assert read(config-16,16)==b'\xA5'*16 and read(config+40,16)==b'\xA5'*16
  for r in range(16,24):assert reg(r)==0xBEEF0000+r
  assert reg(29)==sp
 assert seen_selectors=={0,1} and seen_results==set(results)
 sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
 deps=[SOURCE,Path(__file__).resolve()]+list((ROOT/'include').rglob('*.h'))
 audit=dict(selected_commands=sorted(seen_selectors),release_results=sorted(seen_results),cases=CASES,engine='compiled MIPS' if elf else 'original MIPS',source_sha256=sha(SOURCE),test_sha256=sha(__file__),full_host_pointers=True,full_owner_guards=True,caller_save_hi_lo_poison=True,mips_store_bounds=True,poll_limits=limits,sanitizers=['undefined','bounds'],dependencies={str(p):sha(p) for p in deps})
 if elf:audit['elf_sha256']=sha(elf)
 if os.environ.get('RR_AUDIT_JSON'):Path(os.environ['RR_AUDIT_JSON']).write_text(json.dumps(audit,indent=2)+'\n')
 print(f'PASS {CASES} {audit["engine"]}/native cases; full pointers, owners, timeout and release status')
if __name__=='__main__':main()
