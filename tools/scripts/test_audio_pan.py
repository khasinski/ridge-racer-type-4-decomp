"""Actual panning body versus independent arithmetic and helper snapshots."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT = Path(__file__).resolve().parents[2]


class AudioPanTest(unittest.TestCase):
    def test_actual_body_with_full_width_inputs_and_mutating_helpers(self):
        source = (ROOT / 'tools/scripts/fixtures/views/func_8004EF04.c').read_text()
        source = re.sub(r'^#include.*\n', '', source, flags=re.M)
        source = re.sub(r' asm\("\$\d+"\)', '', source)
        source = re.sub(r'__asm__\(""[^;]*;', ';', source)
        prefix = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,s)
#define RR_ASSERT_OFFSET(t,f,o)
#include "rr/audio_pan.h"
_Static_assert(sizeof(RrAudioSettings)==52,"settings size");
_Static_assert(offsetof(RrAudioSettings,volume31)==49,"volume");
_Static_assert(offsetof(RrAudioSettings,mono32)==50,"mono");
typedef struct {s16 table[128];RrRuntimeSettings settings;s16 gain;u16 banks[3];u8 canary[18];} World;
typedef struct {s32 kind,args[8];World snapshot;} Entry;
static World initial,actual,expected,*w;
static Entry entries[2],saved[2];
static int count;
#define PanGainTable (w->table)
#define Options (w->settings)
#define SoundGain (w->gain)
#define SoundVabIdProg00 (w->banks[0])
#define SoundVabIdProg30 (w->banks[1])
#define SoundVabIdProg60 (w->banks[2])
static s32 emit(int kind,s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h){
 assert(count<2);Entry *p=&entries[count++];memset(p,0,sizeof(*p));
 p->kind=kind;p->args[0]=a;p->args[1]=b;p->args[2]=c;p->args[3]=d;
 p->args[4]=e;p->args[5]=f;p->args[6]=g;p->args[7]=h;p->snapshot=*w;
 w->gain^=0x7fff;w->settings.audio.volume31+=97;w->settings.audio.mono32^=1;
 for(int i=0;i<3;i++)w->banks[i]+=321;
 for(int i=0;i<128;i++)w->table[i]=(s16)((u16)w->table[i]+0x9f81U);
 return (s32)0xDEADBEEFU;
}
s32 SsUtKeyOn(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g){return emit(0,a,b,c,d,e,f,g,0);}
s32 SsUtKeyOnV(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h){return emit(1,a,b,c,d,e,f,g,h);}
static int narrow(u32 v){v&=65535;return v<32768?(int)v:(int)v-65536;}
static int word(u32 v){return v<=INT32_MAX?(int)v:(int)((int64_t)v-INT64_C(4294967296));}
static void reference(s32 program,s32 level,s32 pan,s32 pitch,s32 voice){
 int bank_index=program<30?0:program<60?1:2;
 int bank=narrow(w->banks[bank_index]);
 int p=narrow((u32)program-30U*(unsigned)bank_index);
 int index=(u32)pan%128U;
 int l=narrow((u32)((int64_t)w->table[index]*narrow((u32)level)/16129));
 int r=narrow((u32)((int64_t)w->table[127-index]*narrow((u32)level)/16129));
 if(w->settings.audio.mono32)l=r=(l+r)/2;
 l=narrow((u32)((int64_t)l*w->gain/128));
 r=narrow((u32)((int64_t)r*w->gain/128));
 l=narrow((u32)((int64_t)l*w->settings.audio.volume31/16));
 r=narrow((u32)((int64_t)r*w->settings.audio.volume31/16));
 int raw=word((u32)pitch+60U),note=narrow((u32)(raw/128)),fine=narrow((u32)(raw%128));
 if(voice==-1){emit(0,bank,p,0,note,fine,r,l,0);emit(0,bank,p,1,note,fine,r,l,0);}
 else{int v=narrow((u32)voice*2U);emit(1,v,bank,p,0,note,fine,r,l);emit(1,narrow((u32)voice*2U+1U),bank,p,1,note,fine,r,l);}
}
static u32 seed=0xAB1CD765;
static u32 random_word(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
int main(void){
 const u32 edges[]={0,1,29,30,59,60,127,128,0xffff,0x10000,0x7fffffff,0x80000000,0xffffffff,0x8000,0x7fff,0x7fffffc4,0x8000003c};
 unsigned modes[2]={0,0};
 for(unsigned k=0;k<131072;k++){
  unsigned char *bytes=(unsigned char*)&initial;
  for(size_t j=0;j<sizeof(initial);j++)bytes[j]=(unsigned char)random_word();
  initial.settings.audio.mono32=(k&2)?(s16)random_word():0;
  initial.settings.audio.volume31=(u8)k;initial.gain=(s16)(u16)k;
  s32 program=word(k<sizeof(edges)/sizeof(*edges)?edges[k]:random_word());
  s32 level=word((random_word()&0xffff0000U)|(u16)k);
  s32 pan=word(random_word());
  s32 pitch=word(k<sizeof(edges)/sizeof(*edges)?edges[k]:random_word());
  s32 voice=(k&1)?-1:word(random_word());
  actual=expected=initial;w=&actual;count=0;memset(entries,0,sizeof(entries));
  TriggerSfx(program,level,pan,pitch,voice);assert(count==2);memcpy(saved,entries,sizeof(saved));
  w=&expected;count=0;memset(entries,0,sizeof(entries));reference(program,level,pan,pitch,voice);
  assert(count==2);assert(!memcmp(saved,entries,sizeof(saved)));assert(!memcmp(&actual,&expected,sizeof(actual)));
  modes[voice==-1]++;
 }
 assert(modes[0]&&modes[1]);puts("131072 native panning cases passed");
}
'''
        # Compile the public body directly, stripping only register constraints
        # and empty compiler hints. All pointers retain native width.
        with TemporaryDirectory() as directory:
            c = Path(directory) / 'pan.c'
            exe = Path(directory) / 'pan'
            c.write_text(prefix + source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(ROOT / 'include'),
                            str(c), '-o', str(exe)], check=True, capture_output=True, text=True)
            result = subprocess.run([str(exe)], check=True, capture_output=True, text=True)
            self.assertIn('131072 native panning cases passed', result.stdout)
