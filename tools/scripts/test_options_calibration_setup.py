"""Calibration setup: complete shared settings and live badge roots."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class CalibrationSetupTest(unittest.TestCase):
    def test_native_setup(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/options/func_80115798.c').read_text()
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
#include "rr/options_calibration.h"
static const char format[]="calibration";
typedef struct {u32 guard[4],rng;RrRuntimeSettings settings;RrBadgeRow1C rows[3][8];s32 root;s16 flag;u32 tail[4];} State;
typedef struct {s32 op,args[7];u32 hash;} Event;
typedef struct {State s;unsigned calls;Event events[4];} World;
static World actual,reference,initial,*w;static unsigned mutation;
RrBadgeRow1C *BadgeRows;
#define Options w->s.settings
#define MenuCursor w->s.flag
#define options_screen_offset_fmt format
''' + body + r'''
static u32 next(void){u32 x=w->s.rng;x^=x<<13;x^=x>>17;x^=x<<5;return w->s.rng=x;}
static s32 word(u32 u){s32 v;memcpy(&v,&u,4);return v;}
static void effect(void){unsigned i,j;static const s32 values[]={INT32_MIN,INT32_MAX,-11,-10,-1,0,1,10,11,16,32,33};
 for(i=0;i<60;i++)w->s.settings.bytes[i]=next();
 w->s.settings.display.x34=values[next()%12];w->s.settings.display.y38=values[next()%12];
 for(i=0;i<3;i++)for(j=0;j<8;j++)w->s.rows[i][j].half02=(s16)next();
 w->s.root=next()%3;BadgeRows=w->s.rows[w->s.root];w->s.flag=(s16)next();
}
static Event *event(s32 op){size_t i;u32 h=2166136261U;Event *e;assert(w->calls<4);for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;e=&w->events[w->calls++];memset(e,0,sizeof(*e));e->op=op;e->hash=h;return e;}
static void change(void){if(mutation)effect();}
void ClearScreenOverlayState(void){event(1);change();}
void InitBadgeRow(s32 a,s32 b,s16 c,s32 d,u16 e,u16 f,u16 g){Event *v=event(2);s32 args[]={a,b,c,d,e,f,g};memcpy(v->args,args,sizeof(args));change();}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){Event *e=event(3);e->args[0]=a;e->args[1]=b;e->args[2]=c;change();}
s32 DebugPrintStub(const char *message,...){va_list args;Event *e=event(4);assert(message==format);va_start(args,message);e->args[0]=va_arg(args,s32);e->args[1]=va_arg(args,s32);va_end(args);return 1;}
static void expected(void){
 ClearScreenOverlayState();w->s.flag=0;InitBadgeRow(0,12,304,296,88,0,0);
 w->s.rows[w->s.root][0].half02=1;SetMenuFrameOptions(1,1,0);
 DebugPrintStub(format,w->s.settings.display.x34,w->s.settings.display.y38);
}
int main(void){unsigned scenario,i;_Static_assert(sizeof(RrRuntimeSettings)==60,"settings extent");_Static_assert(offsetof(RrDisplaySettings3C,x34)==52,"signed word x");
 assert((uintptr_t)actual.s.rows>UINT32_MAX);
 for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0x15798+scenario*31;effect();
  w->s.settings.display.x34=word(scenario*0x9e3779b9U);w->s.settings.display.y38=word(~scenario*0x85ebca6bU);
  for(i=0;i<4;i++){w->s.guard[i]=scenario+i;w->s.tail[i]=~(scenario+i);}actual=reference=initial;
  w=&actual;BadgeRows=w->s.rows[w->s.root];options_init_screen_adjust();w=&reference;BadgeRows=w->s.rows[w->s.root];expected();assert(!memcmp(&actual,&reference,sizeof(actual)));
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
