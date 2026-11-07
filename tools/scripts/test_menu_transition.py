"""Actual menu transition and real four-way rectangle wrapper, native pointers."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT = Path(__file__).resolve().parents[2]


class MenuTransitionTest(unittest.TestCase):
    def test_actual_body_and_rectangle_wrapper(self):
        source = (ROOT / 'tools/scripts/fixtures/views/func_8003A544.c').read_text()
        source = re.sub(r'^#include.*\n', '', source, flags=re.M)
        # The sole literal is the existing scratchpad cursor owner.
        self.assertEqual(source.count('*(void **)0x1F800000'), 2)
        source = source.replace('*(void **)0x1F800000', 'w->cursor')
        helper = (ROOT / 'tools/scripts/fixtures/views/func_8001FDD4.c').read_text()
        helper = re.sub(r'^#include.*\n', '', helper, flags=re.M)
        prefix = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,s)
#define RR_ASSERT_OFFSET(t,f,o)
#include "rr/menu_transition.h"
typedef struct {
 s32 frame;u16 mode,scene,phase,transition; s16 ui_mode,timer,next_timer;
 s8 step,operation;u8 context;u8 canary[17];void *cursor;s8 scripts[12][256];
} World;
typedef struct {int kind;intptr_t args[9];World snapshot;} Entry;
static World initial,actual,expected,*w;
static Entry entries[12],saved[12];
static int count,mutation,status,op_status;
static unsigned seen[12];
static _Alignas(16) u8 contexts[2][0xc00],packets[4096];
static s8 *scripts[12];
#define CurrentDrawBuf ((RrIconSpriteContext*)contexts[w->context])
#define SceneId (w->transition)
#define ModeFrame (w->frame)
#define ModeId (w->next_timer)
#define MenuScriptStep (w->step)
#define MenuScriptId (w->scene)
#define ModeGroup (w->phase)
#define RaceType (w->ui_mode)
#define LoadState (w->operation)
#define ModeSubState (w->timer)
#define MenuDispMode (w->mode)
#define MenuScripts scripts
static void record(int kind,intptr_t a,intptr_t b,intptr_t c,intptr_t d,intptr_t e,intptr_t f,intptr_t g,intptr_t h,intptr_t j){
 assert(count<12);Entry *p=&entries[count++];memset(p,0,sizeof(*p));p->kind=kind;
 intptr_t args[9]={a,b,c,d,e,f,g,h,j};memcpy(p->args,args,sizeof(args));p->snapshot=*w;seen[kind]++;
 if(mutation){
  w->mode=(w->mode+1U)%4U;w->scene=(w->scene+5U)%12U;
  w->step=(s8)(u8)((u8)w->step+37U);w->frame=(s32)((u32)w->frame+0x80000009U);
  w->ui_mode=(w->ui_mode==4)?0:4;w->operation^=0x7f;
  w->transition=(u16)(w->transition+0x2719U);w->phase=(w->phase+1U)%3U;w->context^=1;
  for(int i=0;i<12;i++)for(int k=0;k<256;k++)w->scripts[i][k]=(s8)((k%12U));
  /* Next script cell can terminate or continue after the operation callback. */
  w->scripts[w->scene][(unsigned)(u8)((u8)w->step+1U)^128U]=(count&1)?-7:2;
 }
}
void SetDispMask(s32 arg){record(0,arg,0,0,0,0,0,0,0,0);}
int UpdateLinkHandshake(int arg){record(1,arg,0,0,0,0,0,0,0,0);return -77;}
s32 OpeningScriptStep(s32 frame){record(2,frame,0,0,0,0,0,0,0,0);return status;}
s32 IntroScriptStep(void){record(3,0,0,0,0,0,0,0,0,0);return status;}
void StartSceneFadeOut(void){record(4,0,0,0,0,0,0,0,0,0);}
static s32 op0(void){record(5,0,0,0,0,0,0,0,0,0);return op_status;}
static s32 op1(void){record(5,1,0,0,0,0,0,0,0,0);return op_status;}
static s32 op2(void){record(5,2,0,0,0,0,0,0,0,0);return op_status;}
static s32 op3(void){record(5,3,0,0,0,0,0,0,0,0);return op_status;}
static void enter0(void){record(6,0,0,0,0,0,0,0,0,0);}
static void enter1(void){record(6,1,0,0,0,0,0,0,0,0);}
static void enter2(void){record(6,2,0,0,0,0,0,0,0,0);}
static void enter3(void){record(6,3,0,0,0,0,0,0,0,0);}
RrMenuScriptOperation MenuScriptOps[]={op0,op1,op2,op3,op0,op1,op2,op3,op0,op1,op2,op3};
RrMenuSceneEntry MenuSceneEntries[]={enter0,enter1,enter2,enter3,enter0,enter1,enter2,enter3,enter0,enter1,enter2,enter3};
void *AddTilePrim(u32 *order,void *cursor,s32 x,s32 y,s32 width,s32 height,s32 r,s32 g,s32 b){
 record(7,(intptr_t)order,(intptr_t)cursor,x,y,width,height,r,g,b);
 return (u8*)cursor+16;
}
static int narrow8(unsigned v){v&=255;return v<128?(int)v:(int)v-256;}
static void reference(void){
 w->frame=(s32)((u32)w->frame+1U);
 if(w->frame==2)record(0,1,0,0,0,0,0,0,0,0);
 if(w->ui_mode==4)record(1,1,0,0,0,0,0,0,0,0);
 int result=1;
 if(w->mode==1){s32 frame=w->frame;record(2,frame,0,0,0,0,0,0,0,0);result=status;}
 else if(w->mode==0){record(3,0,0,0,0,0,0,0,0,0);result=status;}
 if(w->step>=0){
  int op=w->scripts[w->scene][128+(int)w->step];assert(op>=0&&op<12);
  record(5,op%4,0,0,0,0,0,0,0,0);
  if(op_status==1){
   w->operation=0;int step=narrow8((u8)w->step+1U);w->step=(s8)step;
   int next=w->scripts[w->scene][128+step];
   if(next<0){w->scene=(u16)-next;w->step=-1;if(!w->mode)record(4,0,0,0,0,0,0,0,0,0);}
  }
 }else{
  int enter=(w->mode==0)?result==2:result==1;
  if(enter){int scene=w->scene;record(6,scene%4,0,0,0,0,0,0,0,0);w->timer=0;w->phase=w->transition/256;w->next_timer=w->transition%256;}
 }
 if(w->phase!=1){
  u32 *order=(u32*)(contexts[w->context]+0xB6C);void *cursor=w->cursor;
  for(int i=0;i<4;i++){
   int x=(i%2)*160,y=(i/2)*256;
   record(7,(intptr_t)order,(intptr_t)cursor,x,y,160,256,0,0,0);cursor=(u8*)cursor+16;
  }
  w->cursor=cursor;
 }
}
static unsigned seed=0xF2A47619;
static unsigned draw(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void setup(World *state){w=state;count=0;memset(entries,0,sizeof(entries));for(int i=0;i<12;i++)scripts[i]=w->scripts[i]+128;}
int main(void){
 assert((uintptr_t)packets>UINT32_MAX);assert((uintptr_t)contexts>UINT32_MAX);
 const u16 modes[]={0,1,2,3,127,128,255,256,32767,32768,65535};
 const int results[]={0,1,2,3,-1,INT32_MIN,INT32_MAX};
 const u32 frames[]={0,1,2,0x7fffffff,0x80000000,0xffffffff};
 for(unsigned k=0;k<65536;k++){
  unsigned char *bytes=(unsigned char*)&initial;for(size_t j=0;j<sizeof(initial);j++)bytes[j]=(unsigned char)draw();
  initial.frame=(s32)frames[k%6];initial.mode=modes[k%11];initial.scene=k%12;initial.phase=k%3;
  initial.ui_mode=(k&4)?4:0;initial.step=(s8)(u8)k;initial.context=k&1;initial.cursor=packets+(k%16)*16;
  for(int i=0;i<12;i++)for(int j=0;j<256;j++)initial.scripts[i][j]=(s8)(j%12);
  /* Exercise signed wrap127 -> -128 and negative scene markers. */
  int next=narrow8((u8)initial.step+1U);
  initial.scripts[initial.scene][128+next]=(k&1)?-7:3;
  mutation=(k>>8)&1;status=results[k%7];op_status=(int)((k>>2)%3);
  actual=expected=initial;setup(&actual);UpdateMenuScript();memcpy(saved,entries,sizeof(saved));int n=count;
  setup(&expected);reference();assert(n==count);assert(!memcmp(saved,entries,sizeof(saved)));assert(!memcmp(&actual,&expected,sizeof(actual)));
 }
 for(int i=0;i<=7;i++)assert(seen[i]);puts("65536 native transition and real rectangle-wrapper cases passed");
}
'''
        with TemporaryDirectory() as directory:
            c = Path(directory) / 'transition.c'
            exe = Path(directory) / 'transition'
            c.write_text(prefix + helper + source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(ROOT / 'include'),
                            str(c), '-o', str(exe)], check=True, capture_output=True, text=True)
            result = subprocess.run([str(exe)], check=True, capture_output=True, text=True)
            self.assertIn('65536 native transition and real rectangle-wrapper cases passed', result.stdout)
