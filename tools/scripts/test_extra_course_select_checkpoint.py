"""Overlay666 checkpoint, mode adjustment and deferred notification under UBSan."""
from overlay_units import read_c_source
from pathlib import Path
from tempfile import TemporaryDirectory
import re, subprocess, unittest
ROOT=Path(__file__).resolve().parents[2]
class CheckpointTest(unittest.TestCase):
 def test_counters_modes_and_mutating_services(self):
  body=read_c_source(ROOT/'src/overlays/PAL/extra_course_select/func_8011656C.c')
  body='\n'.join(line for line in body.splitlines() if not line.startswith('#include') and not line.startswith('extern '))
  body=re.sub(r' __asm__\("\$3"\)','',body)
  code=r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "rr/overlay666_checkpoint.h"
typedef struct {u32 prefix[4];RrRaceCheckpointVehicle vehicle;u32 suffix[4];
 s32 length,target,state,flag;s16 timer,pending;u16 mode,pad;u32 tail[4];} Environment;
typedef struct {Environment pre;unsigned id;s32 a,b;} Event;
static Environment actual,expected;static Event events[4];
static unsigned count,index_,mutation;static unsigned paths[12];
static u32 seed=0x6661656c;
static u32 next(void){u32 x=seed;x^=x<<13;x^=x>>17;x^=x<<5;return seed=x;}
static s32 sign32(u32 x){s32 result;memcpy(&result,&x,4);return result;}
static s16 sign16(u16 x){s16 result;memcpy(&result,&x,2);return result;}
static void mutate(Environment *e,unsigned id){
 if(mutation&1){e->vehicle.first=sign32((u32)e->vehicle.first+0xfedcba98U);
  e->vehicle.control.steering94.count22=sign16((u16)(0x7fffU+id));}
 if(mutation&2){e->length=sign32(0xdeadbeefU);e->target=INT_MIN;e->state=-12345;
  e->flag=77;e->timer=-123;}
 if(mutation&4)e->pending=0;
 if(mutation&8){e->pending=1;e->target=INT_MAX;}
}
static void expect_event(unsigned id,s32 a,s32 b){
 assert(count<4);events[count].pre=expected;events[count].id=id;
 events[count].a=a;events[count].b=b;count++;mutate(&expected,id);
}
static void service(unsigned id,s32 a,s32 b){Event *event;
 assert(index_<count);event=&events[index_++];
 assert(event->id==id&&event->a==a&&event->b==b);
 assert(!memcmp(&actual,&event->pre,sizeof(actual)));mutate(&actual,id);
}
void SaveCarSnapshots(s32 unused){service(0,unused,0);}
void SoundSetMasterVolume(s16 a,s16 b){service(1,a,b);}
void *SoundQueueEnqueue(s32 a,s32 b){service(2,a,b);return 0;}
#define TrackLength (actual.length)
#define NumLaps (actual.target)
#define RaceState (actual.state)
#define TotalRaceTime (actual.flag)
#define MenuFadeTimer (actual.timer)
#define LapAnnouncePending (actual.pending)
#define CourseIndex (actual.mode)
'''+body+r'''
static void model(void){
 int lap=expected.vehicle.control.steering94.count22;
 s32 sum=sign32((u32)expected.vehicle.first+(u32)expected.vehicle.second);
 s32 threshold=sign32((u32)((int64_t)lap*expected.length));s32 adjusted,id;
 if(sum>=threshold && lap<=expected.target){
  paths[0]++;lap=sign16((u16)(lap+1));
  expected.vehicle.control.steering94.count22=(s16)lap;expected.pending=0;
  adjusted=expected.target;
  if(expected.mode==6||expected.mode==7){adjusted=sign32((u32)adjusted-1U);paths[1]++;}
  if(lap==adjusted){paths[2]++;expect_event(0,0,0);}
  else if(lap>expected.target&&expected.state<4){
   paths[3]++;expected.pending=-1;expected.state=4;expected.timer=75;
   expect_event(1,0,33);expected.flag=0;expect_event(2,19,1);
  }else paths[4]++;
 }else paths[5]++;
 /* Helper mutations are visible to both of these reloads. */
 if(expected.pending==0){
  paths[6]++;
  id=sign32(9U-((u32)expected.target-(u32)(s32)expected.vehicle.control.steering94.count22));
  if(id==9){id=15;paths[7]++;}
  if(id>=6){paths[8]++;expect_event(2,id,0);}else paths[9]++;
  expected.pending=-1;
 }else paths[10]++;
 if(count&&mutation)paths[11]++;
}
static void run(unsigned raw,unsigned phase,unsigned mode,int exhaustive){
 unsigned char *bytes=(unsigned char *)&actual;unsigned i;
 int lap=sign16((u16)raw);s32 threshold;
 for(i=0;i<sizeof(actual);i++)bytes[i]=(unsigned char)(i*37U+raw+phase);
 actual.vehicle.control.steering94.count22=(s16)lap;
 actual.length=sign32(next());
 actual.target=phase<24?lap+(int)(phase%4)-1:sign32(next());
 actual.state=(phase/4)&1?3:4;actual.flag=sign32(next());actual.timer=sign16((u16)next());
 actual.mode=(u16)mode;actual.pending=phase%3==0?0:phase%3==1?-1:1;
 threshold=sign32((u32)((int64_t)lap*actual.length));
 actual.vehicle.first=sign32(next());
 actual.vehicle.second=sign32((u32)threshold-(u32)actual.vehicle.first+(phase>=24?0xffffffffU:0U));
 if(exhaustive){actual.target=lap+1;actual.pending=0;}
 mutation=(raw+phase)&15;expected=actual;count=index_=0;
 model();extra_course_select_update_checkpoint(&actual.vehicle);
 assert(index_==count);assert(!memcmp(&actual,&expected,sizeof(actual)));
}
int main(void){unsigned raw,phase,i;uint64_t cases=0;
 static const u16 modes[6]={0,5,6,7,8,65535};
 for(raw=0;raw<65536;raw++)for(phase=0;phase<32;phase++){
  run(raw,phase,modes[(phase/4)%6],0);cases++;
 }
 /* Cover every unsigned mode word, in addition to every signed counter word. */
 for(raw=0;raw<65536;raw++){run(next()&65535U,raw%32,raw,1);cases++;}
 for(i=0;i<12;i++)assert(paths[i]);
 printf("%llu cases; complete guarded vehicle/global snapshots and service arguments\n",(unsigned long long)cases);
}
'''
  with TemporaryDirectory() as temp:
   path=Path(temp);(path/'test.c').write_text(code)
   subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-Wno-unused-function','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(path/'test.c'),'-o',str(path/'test')],check=True)
   subprocess.run([str(path/'test')],check=True)
if __name__=='__main__':unittest.main()
