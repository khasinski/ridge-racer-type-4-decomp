"""Garage dispatch: defined fade arithmetic and state changes at typed ABI calls."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

class GarageUpdateTest(unittest.TestCase):
    def test_mutating_dispatch_and_full_word_fades(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/garage/func_8011CF8C.c').read_text()
        # Compiler-only empty scheduling metadata has no executable C behavior.
        body=body.replace('__asm__ volatile("" : : : "memory");','')
        source=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/garage_update.h"
typedef struct {s32 kind,args[3];const void *pointer;RrGarageSelectionPrefix garage;RrGpuRectangle *context;RrMenuTextState *text;} Event;
static struct World {u32 guard[4];RrGarageSelectionPrefix garage;RrGpuRectangle *context;RrMenuTextState *text;RrMenuTextState texts[3];u32 calls;Event events[8];u32 tail[4];} world,expected;
static RrFrameOrderingPrefix frames[3],original[3];
#define garage_logo_state world.garage
#define CurrentDrawBuf world.context
#define DialogueState world.text
''' + body + r'''
static u32 seed=0x1CF8C,scenario,mutate;
static const s32 fades[]={INT32_MIN,INT32_MIN+1,-1000000,-65,-2,-1,0,1,2,63,64,65,66,536870975,536870976,536870977,1073741888,INT32_MAX-1,INT32_MAX};
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 word(u32 bits){s32 v;memcpy(&v,&bits,4);return v;}
static void effect(struct World *v,s32 kind,const void *pointer){
 unsigned step=v->calls,id=(scenario+step)%3;
 if(kind==3 || kind==4){unsigned t;for(t=0;t<3;t++)if(pointer==&world.texts[t]){v->texts[t].i18=(s32)(scenario+step);v->texts[t].s12=(s16)step;break;}assert(t<3);}
 if(!mutate)return;
 v->garage.fade00=fades[(scenario+step)%19];v->garage.flagsD1=(u8)(scenario*73U+step*29U);
 v->garage.word04=word(scenario^0xABCDEF01U^step);v->garage.current60=(s32)step;v->garage.optionIndices98.flat[step%8]=(s32)(scenario*step);
 v->context=&frames[id].rectangle;v->text=&world.texts[(id+1)%3];
}
static void observe(struct World *v,s32 kind,s32 a,s32 b,s32 c,const void *pointer){
 Event *e;assert(v->calls<8);e=&v->events[v->calls++];memset(e,0,sizeof(*e));e->kind=kind;e->args[0]=a;e->args[1]=b;e->args[2]=c;e->pointer=pointer;memcpy(&e->garage,&v->garage,sizeof(v->garage));e->context=v->context;e->text=v->text;effect(v,kind,pointer);
}
void AddScreenFadeTiles(u32 *ordering,s32 intensity,u32 mode){assert(mode==69);observe(&world,0,intensity,(s32)mode,0,ordering);}
void UpdateBadgeRowAnim(s32 index){assert(index==0);observe(&world,1,index,0,0,0);}
void DrawUiRow(s32 index){assert(index==0 || index==1);observe(&world,2,index,0,0,0);}
void SkipDialoguePage(void *state){observe(&world,3,0,0,0,state);}
void UpdateDialogue(s32 x,s32 y,RrMenuTextState *state){assert(x==22 && y==320);observe(&world,4,x,y,0,state);}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){observe(&world,5,a,b,c,0);}
void UpdateScreenFade(s32 flags){assert(flags==0x1004E);observe(&world,6,flags,0,0,0);}
static void reference(void){
 RrGarageSelectionPrefix *g=&expected.garage;s32 a,b;
 if(g->fade00>=0){
  int64_t value;if(g->fade00>0)g->fade00-=1;
  value=4*(64-(int64_t)g->fade00);
  observe(&expected,0,word((u32)value),69,0,((RrFrameOrderingPrefix *)expected.context)->ordering70);
 }
 if(g->flagsD1&1)observe(&expected,1,0,0,0,0);
 if(g->flagsD1&2){observe(&expected,2,0,0,0,0);observe(&expected,2,1,0,0,0);}
 if(g->flagsD1&4){observe(&expected,3,0,0,0,expected.text);observe(&expected,4,22,320,0,expected.text);}
 a=g->fade00<0 && ((g->flagsD1&3)==0 || (g->flagsD1&4)!=0);
 b=g->fade00<0 && (g->flagsD1&3)!=0 && (g->flagsD1&4)!=0;
 observe(&expected,5,a,b,0,0);observe(&expected,6,0x1004E,0,0,0);
}
int main(void){unsigned k,m,i,j;
 for(i=0;i<sizeof(frames);i++)((u8*)frames)[i]=(u8)next();memcpy(original,frames,sizeof(frames));
 for(k=0;k<65536;k++)for(m=0;m<2;m++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();scenario=k;mutate=m;world.calls=0;
  world.garage.fade00=k<256*19?fades[k/256]:word(next());world.garage.flagsD1=(u8)k;
  for(i=0;i<2;i++){world.garage.image38[i]=frames[0].ordering70;world.garage.image40[i]=frames[1].ordering70;world.garage.palette48[i]=frames[2].ordering70;}
  for(i=0;i<8;i++)world.garage.options78.flat[i]=0;
  for(i=0;i<3;i++)world.texts[i].p=(u16*)&frames[i].ordering70[0];
  world.context=&frames[k%3].rectangle;world.text=&world.texts[(k+1)%3];assert((uintptr_t)world.context>UINT32_MAX && (uintptr_t)world.text>UINT32_MAX);
  memcpy(&expected,&world,sizeof(world));reference();garage_update_editor_overlays();assert(memcmp(&world,&expected,sizeof(world))==0);
  for(j=0;j<world.calls;j++)assert(world.events[j].kind>=0 && world.events[j].kind<=6);
 }
 assert(memcmp(frames,original,sizeof(frames))==0);return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
