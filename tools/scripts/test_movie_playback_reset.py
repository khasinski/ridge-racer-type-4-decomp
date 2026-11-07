"""STR reset/recovery MMIO traces, native pointers and live command tables."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class StrResetTest(unittest.TestCase):
    def test_reset_and_recovery(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/movie_playback/func_{a}.c').read_text()
                           for a in ('80116558', '801168A4'))
        bodies = re.sub(r'__asm__\("\$2"\)', '', bodies)
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
static u32 read_port(u32 *);
static void write_port(u32 *,u32);
#define RR_STR_READ32(port) read_port(port)
#define RR_STR_WRITE32(port,value) write_port(port,value)
#include "rr/overlay668_dma.h"
#include "rr/diagnostics.h"
static const char invalid[]="invalid mode %d",recover[]="recover %s",message[]="timeout";
typedef struct {
 u32 guard[4],rng,regs[4];RrStrQuantizationCommand quant;RrStrMatrixCommand matrix;u32 tail[4];
} State;
typedef struct {s32 op,index;u32 value,hash,words[32];} Event;
typedef struct {State s;unsigned count;Event events[12];} World;
static World actual,reference,initial,*w;
u32 *movie_playback_D_80116E54,*movie_playback_D_80116E28,*movie_playback_D_80116E34;
#define movie_playback_D_80116D10 w->s.quant
#define movie_playback_D_80116D94 w->s.matrix
#define movie_playback_D_801148C0 invalid
#define movie_playback_D_801148FC recover
''' + bodies + r'''
static u32 next(void){u32 x=w->s.rng;x^=x<<13;x^=x>>17;x^=x<<5;return w->s.rng=x;}
static u32 hash(void){size_t i;u32 h=2166136261U;for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;return h;}
static Event *event(s32 op,s32 index,u32 value){Event *e;assert(w->count<12);e=&w->events[w->count++];memset(e,0,sizeof(*e));e->op=op;e->index=index;e->value=value;e->hash=hash();return e;}
static u32 read_port(u32 *p){u32 v=*p;event(1,p-w->s.regs,v);*p^=0x13579bdfU;return v;}
static void write_port(u32 *p,u32 value){event(2,p-w->s.regs,value);*p=value;}
void movie_playback_func_80116648(const void *command,u32 words){
 Event *e;unsigned i;int which=command==&w->s.quant?0:1;
 assert(command==(which?(const void*)&w->s.matrix:(const void*)&w->s.quant));assert(words==32);
 e=event(3,which,words);memcpy(e->words,command,128);
 for(i=0;i<32;i++)w->s.matrix.matrix[i]^=next();w->s.matrix.header.word^=next();
 /* A submission can replace the RAM pointer cells before the next call. */
 movie_playback_D_80116E54=w->s.regs+next()%4;movie_playback_D_80116E28=w->s.regs+next()%4;movie_playback_D_80116E34=w->s.regs+next()%4;
}
s32 DebugPrintStub(const char *format,...){va_list a;va_start(a,format);
 if(format==invalid){s32 mode=va_arg(a,s32);event(4,0,(u32)mode);}
 else {assert(format==recover&&va_arg(a,const char*)==message);event(4,1,0);}
 va_end(a);w->s.regs[0]^=next();return 1;
}
static void expected(unsigned kind,s32 mode){
 if(kind)DebugPrintStub(recover,message);
 else if(mode!=0&&mode!=1){DebugPrintStub(invalid,mode);return;}
 write_port(movie_playback_D_80116E54,0x80000000U);write_port(movie_playback_D_80116E28,0);write_port(movie_playback_D_80116E34,0);
 if(kind||mode==1)(void)read_port(movie_playback_D_80116E34);
 write_port(movie_playback_D_80116E54,0x60000000U);
 if(!kind&&mode==0){movie_playback_func_80116648(&w->s.quant,32);movie_playback_func_80116648(&w->s.matrix,32);}
}
static void ports(unsigned code){movie_playback_D_80116E54=w->s.regs+(code&3);movie_playback_D_80116E28=w->s.regs+((code>>2)&3);movie_playback_D_80116E34=w->s.regs+((code>>4)&3);}
int main(void){unsigned scenario,kind,i;static const s32 modes[]={0,1,-1,2,INT32_MIN,INT32_MAX};
 assert((uintptr_t)actual.s.regs>UINT32_MAX);
 for(scenario=0;scenario<65536;scenario++)for(kind=0;kind<2;kind++){
  s32 mode=modes[(scenario/64)%6];memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0x16558+scenario*29+kind;
  for(i=0;i<4;i++){w->s.regs[i]=next();w->s.guard[i]=next();w->s.tail[i]=next();}
  w->s.quant.header.word=next();w->s.matrix.header.word=next();for(i=0;i<16;i++){w->s.quant.luma[i]=next();w->s.quant.chroma[i]=next();}
  for(i=0;i<32;i++)w->s.matrix.matrix[i]=next();actual=reference=initial;
  w=&actual;ports(scenario);if(kind)assert(movie_playback_func_801168A4(message)==0);else movie_playback_func_80116558(mode);
  w=&reference;ports(scenario);expected(kind,mode);assert(!memcmp(&actual,&reference,sizeof(actual)));
 }
 return 0;
}
'''
        with TemporaryDirectory() as d:
            p = Path(d)
            (p / 'test.c').write_text(source)
            run = subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                                  '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                                  str(p / 'test.c'), '-o', str(p / 'test')], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)
            subprocess.run([str(p / 'test')], check=True)
