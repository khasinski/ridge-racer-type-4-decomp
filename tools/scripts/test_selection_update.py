"""Actual selection update and callback, full native RAM/ROM owners, UBSan."""
from pathlib import Path
import re,subprocess,tempfile,unittest
root=Path(__file__).resolve().parents[2]
s=(root/'tools/scripts/fixtures/legacy/func_80039F98.c').read_text();s=re.sub(r'^#include[^\n]*\n|extern[^;]*;','',s,flags=re.M)
s=re.sub(r' asm\("\$\d+"\)','',s);s=re.sub(r'__asm__\("".*?\);','',s)
s,n=re.subn(r'\(u8 \*\)0x800B0000U','(actual+0x2B28)',s);assert n==3,n
s,n=re.subn(r'\(u8 \*\)0x800F0000U','((u8*)&row_owner)',s);assert n==1,n
s=s.replace('void UpdateSelectionRank(void)', 'void candidate(void)')
helper=(root/'tools/scripts/fixtures/legacy/func_8003989C.c').read_text();helper=re.sub(r'extern[^;]*;','',helper)
code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_OFFSET(t,f,o) _Static_assert(offsetof(t,f)==o,"native layout")
#include "rr/selection.h"
static u8 actual[0x3000],expected[0x3000],rom[2048];
static struct {u8 prefix[0x4D98];RrSelectionRows value;u8 guard[16];} row_owner;
#define native_rows_value row_owner.value
static RrSelectionRows expected_rows;
#define SelectionState ((s8*)actual)
#define SelectionRank actual[0x14]
#define SelectionRows native_rows_value
#define SelectionRankWeights (rom+128)
#define SelectionRankThresholds ((s8*)rom+144)
#define CourseSaveProgressIndex (rom+116)
''' + helper + s+r'''
static void reference(void) {
 s32 phase=(s8)expected[0x17],level=expected[0x1E],rank=0,group,sum=0;
 int i;
 if(phase<=0)return;
 if(phase<4) {
  for(i=phase-1;i<=phase;i++)sum+=rom[128+i*4+(s8)expected[i]];
 }
 group=phase/2;if(group==1)group=expected[0x14]+1;
 if(phase<4) {
  for(rank=0;rank<4;rank++) {
   s32 upper=(s8)rom[144+group*4+rank];
   s32 lower=rank<3?(s8)rom[144+group*4+rank+1]:-1;
   if(sum<=upper && sum>lower) {level+=(phase<2?4:7)-rank;break;}
  }
 } else {level+=7;rank=level<16;}
 expected[0x14]=rank;
 memcpy(expected+0x20+expected[0x15]*4,expected+0x1C,4);
 expected[0x1E]=(u8)level;
 memcpy(&expected_rows,expected+0x1C,4);
 if(phase==6) {
  int v=expected[0x1E]==18?rom[116+expected[0x1C]*2+(expected[0x1D]>>1)]:1;
  if(v>8)v=8;expected[0x12]=v;
  if(v==1)expected[0x13]=expected[0x18];
 }
 for(i=0;i<7;i++)expected_rows.rows[i].parameter=(u8)level;
}
int main(void) {
 unsigned phase,level,seed,k;u32 random;
 assert(sizeof(RrSelectionStatePrefix)==0x20);
 assert(offsetof(typeof(row_owner),value)==0x4D98);
 memset(row_owner.prefix,0xD7,sizeof(row_owner.prefix));
 memset(row_owner.guard,0xE8,sizeof(row_owner.guard));
 for(seed=0;seed<4;seed++) {
  random=seed+1;
  for(k=0;k<sizeof(rom);k++){random=random*1664525U+1013904223U;rom[k]=(u8)(random>>24);}
  for(phase=0;phase<256;phase++)for(level=0;level<256;level++) {
   memset(actual,0xA7,sizeof(actual));memset(&native_rows_value,0xB8,sizeof(native_rows_value));
   for(k=0;k<4;k++)actual[k]=(phase+seed+k)&3;
   actual[0x14]=(phase+level)&255;actual[0x15]=(phase*3+level)&255;
   actual[0x17]=phase;actual[0x1C]=phase;actual[0x1D]=level;actual[0x1E]=level;
   memcpy(expected,actual,sizeof(actual));expected_rows=native_rows_value;
   reference();candidate();
   assert(row_owner.prefix[0x4D97]==0xD7 && row_owner.guard[0]==0xE8 && row_owner.guard[15]==0xE8);
   assert(!memcmp(expected,actual,sizeof(actual)) && !memcmp(&native_rows_value,&expected_rows,sizeof(native_rows_value)));
  }
 }
 /* Signed choice bytes may address the surrounding readable ROM owner. */
 for(phase=1;phase<=3;phase++)for(level=0;level<256;level++)for(k=0;k<256;k++) {
  memset(actual,0xA7,sizeof(actual));memset(&native_rows_value,0xB8,sizeof(native_rows_value));
  actual[phase-1]=(u8)level;actual[phase]=(u8)k;
  actual[0x14]=(u8)level;actual[0x15]=(u8)k;actual[0x17]=(u8)phase;
  actual[0x1C]=3;actual[0x1D]=7;actual[0x1E]=(u8)(level+k);
  memcpy(expected,actual,sizeof(actual));expected_rows=native_rows_value;
  reference();candidate();
   assert(row_owner.prefix[0x4D97]==0xD7 && row_owner.guard[0]==0xE8 && row_owner.guard[15]==0xE8);
  assert(!memcmp(expected,actual,sizeof(actual)) && !memcmp(&native_rows_value,&expected_rows,sizeof(native_rows_value)));
 }
 return 0;
}
'''
class SelectionUpdateTests(unittest.TestCase):
    def test_actual_update_with_real_callback(self):
        with tempfile.TemporaryDirectory() as tmp:
            c=Path(tmp)/'state.c';exe=Path(tmp)/'state';c.write_text(code)
            subprocess.run(['cc','-std=gnu99','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(c),'-o',str(exe)],check=True,capture_output=True)
            subprocess.run([str(exe)],check=True,capture_output=True)
if __name__=='__main__':unittest.main()
