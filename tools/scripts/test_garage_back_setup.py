"""Save/load-back setup: correct diagnostic ABI and fresh scene-state writes."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class GarageBackSetupTest(unittest.TestCase):
    def test_native_save_and_load_setup(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/garage/func_{a}.c').read_text()
                           for a in ('8011A898','8011A988'))
        bodies = re.sub(r'^extern[^\n]*\n', '', bodies, flags=re.M)
        source = r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/diagnostics.h"
#include "rr/overlay662_garage.h"
#include "rr/menu_setup.h"
static const char save_message[]="enter SaveBack\n",load_message[]="enter LoadBack\n";
static u32 buffers[4][512];
struct State {u32 guard[4];RrGarageSelectionPrefix garage;s16 row,position,selection;u32 tail[4];};
static struct World {struct State s;unsigned calls;struct {s32 kind,arg[10];u32 hash;} events[12];} actual,expected,initial,*w;
static unsigned scenario,mutation,kind;
#define garage_D_801276A8 w->s.garage
#define ModeGroup w->s.row
#define ModeId w->s.position
#define ModeSubState w->s.selection
#define garage_D_801148F0 save_message
#define garage_D_80114900 load_message
static void effect(unsigned n){
 unsigned i;u32 v=scenario*0x9e3779b9U+n*47;
 w->s.garage.dirtyD3=(u8)v;w->s.garage.stateD2=(u8)(v+1);w->s.garage.flagsD1=(u8)(v+2);
 w->s.garage.fade00=(s32)v;w->s.garage.word04=(s32)~v;w->s.garage.phaseCC=v;
 for(i=0;i<10;i++)w->s.garage.reset08[i]=(s16)(v+i*53);
 for(i=0;i<2;i++){w->s.garage.buffers24[i]=buffers[(scenario+n+i)%4];w->s.garage.image38[i]=buffers[(scenario+n+i+1)%4];}
 w->s.row=(s16)(v+3);w->s.position=(s16)(v+4);w->s.selection=(s16)(v+5);
}
static void event(s32 service,const s32 *args,unsigned count){
 unsigned i,n=w->calls;u32 h=2166136261U;assert(n<12 && count<=10);
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 w->events[n].kind=service;for(i=0;i<count;i++)w->events[n].arg[i]=args[i];w->events[n].hash=h;
 w->calls++;if(mutation && service!=0)effect(n);
}
s32 DebugPrintStub(const char *format,...){
 assert(format==(kind?load_message:save_message));event(0,0,0);return 1;
}
void garage_func_8011A788(s32 mode){event(1,&mode,1);}
void garage_func_8011D278(s32 mode){assert(mode==-1);event(2,&mode,1);}
void garage_func_8011D120(void){event(3,0,0);}
void ClearScreenOverlayState(void){event(4,0,0);}
void InitBadgeRow(s32 i,s32 first,s16 x,s32 y,u16 width,u16 mode,u16 flags){s32 a[]={i,first,x,y,width,mode,flags};event(5,a,7);}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){s32 args[]={a,b,c,d,e,f,g,h,i,j};event(6,args,10);}
''' + bodies + r'''
static void reference(void){
 DebugPrintStub(kind?load_message:save_message);
 garage_func_8011A788((s32)kind);
 if(kind){garage_func_8011D278(-1);garage_func_8011D120();}
 w->s.garage.dirtyD3=0;ClearScreenOverlayState();
 InitBadgeRow(0,12,304,296,88,0,0);
 InitUiRow(0,140,136,28,32,0,0,1,0,104);
 InitUiRow(1,140,176,28,32,0,0,1,0,105);
 w->s.row=3;w->s.position=4;w->s.selection=0;
}
int main(void){unsigned i;
 assert((uintptr_t)buffers>UINT32_MAX);
 for(kind=0;kind<2;kind++)for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;effect(9);
  for(i=0;i<4;i++){w->s.guard[i]=0x12345678U+i;w->s.tail[i]=0x87654321U-i;}
  actual=initial;expected=initial;w=&actual;
  if(kind)garage_func_8011A988();else garage_func_8011A898();
  w=&expected;reference();assert(memcmp(&actual,&expected,sizeof(actual))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as directory:
            p = Path(directory)
            (p / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(p / 'test.c'), '-o', str(p / 'test')], check=True)
            subprocess.run([str(p / 'test')], check=True)


if __name__ == '__main__':
    unittest.main()
