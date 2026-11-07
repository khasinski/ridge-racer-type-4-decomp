"""Closed progress initializer: real reset, loader/UI boundaries and 320 bits."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

class Overlay662ProgressInitializerTest(unittest.TestCase):
    def test_counts_and_fresh_resource_publication(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/garage/func_80114998.c').read_text()
        reset=re.sub(r'^extern[^\n]*;\n','',(root/'tools/scripts/fixtures/legacy/func_8003A198.c').read_text(),flags=re.M)
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
#include "rr/overlay662_progress_init.h"
typedef struct {u32 guard[4];s32 frame,counter;s16 current,limit;s8 flag1,flag2;RrMenuArenas arenas;u8 *base;void *published,*saved;RrSaveProgressRecord7C save;u16 counts[4][4],rows[4],total;u32 tail[4];} State;
typedef struct {s32 kind,args[10];State state;} Event;
static struct World {State s;u32 calls;Event events[5];u32 guard[4];} world,expected;
static u8 resources[3][0x2D100],original[3][0x2D100];
#define ModeFrame world.s.frame
#define ScreenFrame world.s.counter
#define MenuCursor world.s.current
#define MenuItemCount world.s.limit
#define LoadState world.s.flag1
#define MenuScriptStep world.s.flag2
#define MenuArenas world.s.arenas
#define CdDirectoryBuffer world.s.base
#define LoadCursor world.s.published
#define MenuArenaBase world.s.saved
#define SaveProgress world.s.save
#define garage_team_maker_car_counts world.s.counts
#define garage_team_car_counts world.s.rows
#define garage_total_car_count world.s.total
''' + reset + body + r'''
static u32 seed=0x14998,scenario;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void effect(struct World *v){
 unsigned step=v->calls,i,j;State *s=&v->s;
 s->base=resources[(scenario+step)%3];s->arenas.first=resources[(scenario+step+1)%3]+32+step;
 s->arenas.second=resources[(scenario+step+2)%3]+96+step;s->published=resources[(scenario+step)%3]+128+step;s->saved=resources[(scenario+step+1)%3]+256+step;
 s->frame=(s32)(scenario+step);s->counter=(s32)(scenario*17+step);s->current=(s16)step;s->limit=(s16)(step+20);s->flag1=(s8)step;s->flag2=(s8)(step+40);
 for(i=0;i<4;i++)for(j=0;j<4;j++){
  u32 bits=(scenario*0x9E3779B9U)^(i*0x7F4A7C15U)^(j*0xABCDEF01U)^(step*0x1234567U);
  if(scenario<320)bits=(scenario/20==i*4+j)?1U<<(scenario%20):0;
  else if(scenario<640)bits=(scenario%20==0)?0xFFF00000U:0xFFFFFFFFU;
  s->save.prefix.progress.progress10[i][j]=bits;s->counts[i][j]=(u16)(scenario*47+step+i+j);
 }
 for(i=0;i<4;i++)s->rows[i]=(u16)(scenario*83+step+i);
 s->total=(u16)(scenario*131+step);
}
static void observe(struct World *v,s32 kind,const s32 args[10]){
 Event *e;assert(v->calls<5);e=&v->events[v->calls++];memset(e,0,sizeof(*e));e->kind=kind;memcpy(e->args,args,sizeof(e->args));memcpy(&e->state,&v->s,sizeof(v->s));effect(v);
}
void InitMenuArenas(void){s32 a[10]={0};assert(world.calls==0 && world.s.frame==0 && world.s.counter==0 && world.s.current==0 && world.s.limit==3 && world.s.flag1==0 && world.s.flag2==0 && world.s.arenas.second==world.s.base+0x2D000);observe(&world,0,a);}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){s32 args[10]={a,b,c,d,e,f,g,h,i,j};assert(world.calls>=1 && world.calls<=3 && a==(s32)world.calls-1);if(world.calls==1)assert(world.s.published==world.s.arenas.second);observe(&world,1,args);}
void SetupRenderer(s32 mode){s32 a[10]={mode};assert(world.calls==4);observe(&world,2,a);}
static const s32 rows[3][10]={{0,118,176,68,32,0,0,1,0,54},{1,118,216,68,32,0,0,1,1,15},{2,118,256,68,32,0,0,1,0,85}};
static void reference(void){
 unsigned i,j;State *s=&expected.s;s32 a[10]={0};
 s->frame=0;s->counter=0;s->current=0;s->limit=3;s->flag1=0;s->flag2=0;s->arenas.second=s->base+0x2D000;
 observe(&expected,0,a);s->published=s->arenas.second;
 for(i=0;i<3;i++)observe(&expected,1,rows[i]);a[0]=5;observe(&expected,2,a);
 s->total=0;s->published=s->saved;s->arenas.first=s->saved;
 for(i=0;i<4;i++){
  unsigned row=0;
  for(j=0;j<4;j++){
   u32 bits=s->save.prefix.progress.progress10[i][j]&0xFFFFFU;unsigned count=0;
   while(bits){bits&=bits-1;count++;}
   s->counts[i][j]=(u16)count;row+=count;
  }
  s->rows[i]=(u16)row;s->total=(u16)(s->total+row);
 }
}
int main(void){unsigned k,i;
 for(i=0;i<sizeof(resources);i++)((u8*)resources)[i]=(u8)next();memcpy(original,resources,sizeof(original));
 for(k=0;k<65536;k++){
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();scenario=k;world.calls=0;
  world.s.base=resources[k%3];world.s.arenas.first=resources[(k+1)%3]+40;world.s.arenas.second=resources[(k+2)%3]+48;world.s.published=resources[k%3]+56;world.s.saved=resources[(k+1)%3]+64;
  assert((uintptr_t)world.s.base>UINT32_MAX);memcpy(&expected,&world,sizeof(world));reference();garage_init_main_menu();assert(memcmp(&world,&expected,sizeof(world))==0);
  assert(world.s.total<=320);for(i=0;i<4;i++)assert(world.s.rows[i]<=80);
 }
 assert(memcmp(resources,original,sizeof(resources))==0);return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
