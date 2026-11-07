"""Available-car initializer with actual glyph codecs and service snapshots."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess,unittest
class AvailableCarsTest(unittest.TestCase):
 def test_progress_and_live_records(self):
  root=Path(__file__).resolve().parents[2]
  body=(root/'src/overlays/PAL/garage/func_80116960.c').read_text()
  codecs=(root/'tools/scripts/fixtures/views/func_80037040.c').read_text().replace('UnpackChoiceGlyph','actual_decode')+'\n'+(root/'tools/scripts/fixtures/views/func_8003708C.c').read_text().replace('PackChoiceGlyph','actual_encode')
  code=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4||sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4||offsetof(t,f)==n,"PAL offset")
#include "rr/overlay662_available_cars.h"
static u8 resources[2][0x2D100];
static struct World {u32 guard[4];s32 frame,clock;s16 phase,pick,count,row,layout,scene,state;u16 enabled;u8 *resource;void *ui,*screen;RrMenuArenas arenas;RrMenuChoice4 choice;RrSaveProgressRecord7C save;RrCourseChoice4 courses[2][9];s16 status[20];RrGarageAvailableCar cars[20];RrMenuGridPoint grid[4];u32 tail[4];} world,expected;
static struct Event {unsigned id;u32 args[10];struct World before;} events[350];
static unsigned count,seen,serial,n,profile;static int reference_run;
#define ModeFrame world.frame
#define ScreenFrame world.clock
#define garage_D_801272D4 world.phase
#define garage_car_index world.pick
#define garage_car_list_count world.count
#define MenuCursor world.row
#define MenuItemCount world.layout
#define ModeId world.scene
#define ModeSubState world.state
#define ModeGroup world.enabled
#define CdDirectoryBuffer world.resource
#define LoadCursor world.ui
#define MenuArenaBase world.screen
#define MenuArenas world.arenas
#define SelectionRows world.choice
#define SaveProgress world.save
#define CourseChoiceTable world.courses
#define garage_car_status world.status
#define garage_collected_cars world.cars
#define PlayerCourseChoices world.grid
'''+codecs+body+r'''
static u32 seed=0x16960;static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 b){u16 u=(u16)b;s16 r;memcpy(&r,&u,2);return r;}
static void effect(struct World *w,unsigned id,unsigned order){
 w->resource=resources[(n+order)&1];w->screen=resources[(n+order+1)&1]+128;w->ui=resources[order&1]+256;
 w->arenas.first=resources[0]+512;w->arenas.second=resources[(n+id)&1]+64;
 w->choice.bytes[0]=(u8)((n+order)&3);w->choice.bytes[1]=(u8)(((n>>2)+order)&3);
 w->cars[(n+order)%20].value=half(n*73U+order);w->cars[(n+order)%20].reserved^=(u16)(order+id);
 w->courses[1][order%8].glyph=half(n*31U+order*17U);
 w->grid[0].first=half(order);w->grid[0].second=half(order+1);
 if(profile&8)w->save.prefix.progress.progress10[w->choice.bytes[0]][w->choice.bytes[1]]^=1U<<((n+order)%20);
}
static void service(unsigned id,const u32 *a){struct World *w=reference_run?&expected:&world;unsigned order=serial++;
 if(reference_run){assert(count<350);events[count].id=id;memcpy(events[count].args,a,40);events[count++].before=*w;}
 else{assert(seen<count);struct Event *e=&events[seen++];if(!(e->id==id&&memcmp(e->args,a,40)==0&&memcmp(w,&e->before,sizeof(*w))==0)){
 fprintf(stderr,"n%u profile%u event%u id%u expected%u args",n,profile,seen-1,id,e->id);for(unsigned k=0;k<10;k++)fprintf(stderr," %u/%u",a[k],e->args[k]);
 for(unsigned k=0;k<sizeof(*w);k++)if(((u8*)w)[k]!=((u8*)&e->before)[k])fprintf(stderr," offset%u:%u/%u",k,((u8*)w)[k],((u8*)&e->before)[k]);fprintf(stderr,"\n");assert(0);
 }}
 effect(w,id,order);
}
void ResetScreenCounters(void){u32 a[10]={0};service(0,a);}
void InitMenuArenas(void){u32 a[10]={0};service(1,a);}
void InitUiRow(s32 a0,s32 a1,s32 a2,s32 a3,s32 a4,s32 a5,s32 a6,s32 a7,s32 a8,s32 a9,...){u32 a[10]={(u32)a0,(u32)a1,(u32)a2,(u32)a3,(u32)a4,(u32)a5,(u32)a6,(u32)a7,(u32)a8,(u32)a9};service(2,a);}
void InitPreviewImageRows(void){u32 a[10]={0};service(3,a);}
void InitBadgeRow(s32 a0,s32 a1,s16 a2,s32 a3,u16 a4,u16 a5,u16 a6){u32 a[10]={(u32)a0,(u32)a1,(u32)(s32)a2,(u32)a3,a4,a5,a6};service(4,a);}
void SetupRenderer(s32 n){u32 a[10]={(u32)n};service(5,a);}
s32 PackChoiceGlyph(void *p){u8 bytes[3];memcpy(bytes,p,3);s32 result=actual_encode(bytes);u32 a[10]={bytes[0],bytes[1],bytes[2]};service(6,a);return result;}
void UnpackChoiceGlyph(s8 *p,s32 glyph){u32 a[10]={(u32)glyph};service(7,a);actual_decode(p,glyph);}
static s32 encode(const u8 *bytes){return bytes[0]*160+bytes[1]*40+bytes[2];}
static void decode(u8 *p,s32 g){s32 first=g/160;p[0]=(u8)first;p[1]=(u8)(g/40-first*4);p[2]=(u8)(g%40);}
static s32 get(const u8 *p){u32 a[10]={p[0],p[1],p[2]};s32 result=encode(p);service(6,a);return result;}
static void decode_call(u8 *p,s32 glyph){u32 a[10]={(u32)glyph};service(7,a);decode(p,glyph);}
static void reference(void){
 expected.frame=0;expected.clock=0;expected.phase=0;expected.pick=0;expected.row=0;expected.layout=2;u32 empty[10]={0};service(0,empty);
 expected.arenas.second=expected.resource+0x2D000;service(1,empty);expected.ui=expected.arenas.second;
 u32 row0[10]={0,24,88,68,32,0,0,1,0,83},row1[10]={1,24,128,68,32,0,0,1,0,85};service(2,row0);service(2,row1);service(3,empty);
 u32 label[10]={1,24,192,84,34,0,0x201},shift[10]={5};service(4,label);service(5,shift);
 unsigned entries=0;
 for(unsigned i=0;i<20;i++){
  expected.choice.bytes[2]=(u8)i;u32 mask=expected.save.prefix.progress.progress10[expected.choice.bytes[0]][expected.choice.bytes[1]];
  expected.status[i]=(s16)((mask>>i)&1U);
  if(expected.status[i]){
   s32 car=get(expected.choice.bytes);expected.cars[entries].value=half((u32)car);
   for(unsigned j=0;j<8;j++){u8 parts[3];s16 glyph=expected.courses[1][j].glyph;decode_call(parts,glyph);if(parts[2]<39)parts[2]%=20;
    s16 candidate=half((u32)get(parts));if(expected.cars[entries].value==candidate)expected.status[i]=2;
   }entries++;
  }
 }
 expected.count=(s16)entries;decode_call(expected.choice.bytes,expected.cars[0].value);
 expected.grid[0].first=0;expected.grid[0].second=0;expected.enabled=1;expected.scene=67;expected.state=0;expected.ui=expected.screen;expected.arenas.first=expected.screen;
}
int main(void){assert(sizeof(void*)==8&&(uintptr_t)resources>UINT32_MAX);memset(resources,0x69,sizeof(resources));
 for(n=0;n<4096;n++)for(profile=0;profile<16;profile++){
  memset(&world,0xA6,sizeof(world));world.resource=resources[0];world.screen=resources[1]+128;world.ui=resources[0]+32;world.arenas.first=resources[1]+64;world.arenas.second=resources[1]+256;
  world.choice.bytes[0]=(u8)(n&3);world.choice.bytes[1]=(u8)((n>>2)&3);
  for(unsigned g=0;g<4;g++)for(unsigned t=0;t<4;t++)world.save.prefix.progress.progress10[g][t]=profile%4==0?0:profile%4==1?0xFFFFFFFFU:profile%4==2?1U<<((n+g+t)%20):next();
  for(unsigned i=0;i<20;i++){world.cars[i].value=half(n*17U+i);world.cars[i].reserved=(u16)(n+i);world.status[i]=half(n+i);}
  for(unsigned g=0;g<2;g++)for(unsigned i=0;i<9;i++)world.courses[g][i].glyph=half(n*73U+i*17U+g);
  expected=world;count=seen=serial=0;reference_run=1;reference();reference_run=0;serial=0;garage_init_car_list();assert(count==seen&&memcmp(&world,&expected,sizeof(world))==0);
 }
 for(unsigned i=0;i<sizeof(resources);i++)assert(((u8*)resources)[i]==0x69);
 puts("65536 cases;actual glyph codecs,full native arenas,live masks/car records and complete service snapshots");return 0;
}
'''
  with TemporaryDirectory() as d:
   c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(code)
   subprocess.run(['cc','-std=gnu11','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-fsanitize=undefined','-fno-sanitize-recover=all','-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
   subprocess.run([str(exe)],check=True)
if __name__=='__main__':unittest.main()
