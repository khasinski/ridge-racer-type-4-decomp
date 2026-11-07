"""Race checkpoint notifications against the actual25-row message-table contract."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess, unittest
ROOT=Path(__file__).resolve().parents[2]
class CheckpointTest(unittest.TestCase):
 def test_counters_valid_messages_and_latch_reload_after_callbacks(self):
  body=(ROOT/'src/overlays/PAL/versus_select/func_80114924.c').read_text()
  body='\n'.join(line for line in body.splitlines() if not line.startswith('#include') and not line.startswith('extern '))
  code=r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "rr/overlay661_checkpoint.h"
typedef struct {u32 prefix[4];RrRaceCheckpointVehicle vehicle;u32 suffix[4];
 s32 length,target,state,flag,latch;s16 timer,pending;u32 tail[4];} Environment;
typedef struct {Environment pre;unsigned id;s32 a,b;} Event;
static Environment actual,expected;static Event events[4];
static unsigned count,index_,mutation,scenario;static unsigned paths[12],messages[25];
static u32 seed=0x66114924;
static u32 next(void){u32 x=seed;x^=x<<13;x^=x>>17;x^=x<<5;return seed=x;}
static s32 sign32(u32 x){s32 result;memcpy(&result,&x,4);return result;}
static s16 sign16(u16 x){s16 result;memcpy(&result,&x,2);return result;}
static void mutate(Environment *e,unsigned id){
 if(mutation&1){e->vehicle.first=sign32((u32)e->vehicle.first+0xfedcba98U);
  e->vehicle.control.steering94.count22=sign16((u16)(scenario*17U+id));}
 /* All callback profiles preserve the documented valid-message domain. */
 if(mutation&7){int message=6+(int)((scenario+id)%19);
  e->target=e->vehicle.control.steering94.count22+9-message;}
 if(mutation&2){e->length=sign32(0xdeadbeefU);e->state=-12345;e->flag=77;e->timer=-123;}
 if(mutation&4)e->pending=0;
 if(mutation&8){e->pending=1;e->target=INT_MIN;}
 e->latch=sign32((u32)e->latch^(0x9e3779b9U+id));
}
static void expect_event(unsigned id,s32 a,s32 b){
 assert(count<4);events[count].pre=expected;events[count].id=id;
 events[count].a=a;events[count].b=b;count++;mutate(&expected,id);
}
static void service(unsigned id,s32 a,s32 b){Event *event;
 assert(index_<count);event=&events[index_++];assert(event->id==id&&event->a==a&&event->b==b);
 assert(!memcmp(&actual,&event->pre,sizeof(actual)));
 if(id==2)assert(a>=0&&a<RR_SEQUENCE_TABLE_COUNT);
 mutate(&actual,id);
}
static void SaveCarSnapshots(s32 unused){service(0,unused,0);}
static void SoundSetMasterVolume(s16 a,s16 b){service(1,a,b);}
static void *sequence(s32 a,s32 b){service(2,a,b);return 0;}
#define SoundQueueEnqueue sequence
#define TrackLength (actual.length)
#define NumLaps (actual.target)
#define RaceState (actual.state)
#define TotalRaceTime (actual.flag)
#define MenuFadeTimer (actual.timer)
#define LapAnnouncePending (actual.pending)
#define RaceVoiceFlags (actual.latch)
'''+body+r'''
static void model(void){
 int lap=expected.vehicle.control.steering94.count22;s32 target,id;
 s32 sum=sign32((u32)expected.vehicle.first+(u32)expected.vehicle.second);
 s32 threshold=sign32((u32)((int64_t)lap*expected.length));u32 bit;
 if(sum>=threshold && lap<=expected.target){
  paths[0]++;target=expected.target;lap=sign16((u16)(lap+1));
  expected.vehicle.control.steering94.count22=(s16)lap;expected.pending=0;
  if(lap==target){paths[1]++;expect_event(0,0,0);}
  else if(lap>target&&expected.state<4){
   paths[2]++;expected.pending=-1;expected.state=4;expected.timer=75;
   expect_event(1,0,33);expected.flag=0;expect_event(2,19,1);
  }else paths[3]++;
 }else paths[4]++;
 if(expected.pending==0){
  paths[5]++;
  id=sign32(9U-((u32)expected.target-(u32)(s32)expected.vehicle.control.steering94.count22));
  if(id>=6){
   if(id==9){id=15;paths[6]++;}
   assert(id<RR_SEQUENCE_TABLE_COUNT);messages[id]++;
   bit=512U*(1U<<(unsigned)(id-6));
   if(((u32)expected.latch&bit)==0){
    paths[7]++;expect_event(2,id,0);
    /* Reload the current latch after the callback's mutation. */
    expected.latch=sign32((u32)expected.latch|bit);
   }else paths[8]++;
  }else paths[9]++;
  expected.pending=-1;
 }else paths[10]++;
 if(count&&mutation)paths[11]++;
}
int main(void){unsigned raw,phase,i;uint64_t cases=0;
 for(raw=0;raw<65536;raw++)for(phase=0;phase<48;phase++){
  unsigned char *bytes=(unsigned char *)&actual;int lap=sign16((u16)raw);s32 threshold;
  for(i=0;i<sizeof(actual);i++)bytes[i]=(unsigned char)(i*37U+raw+phase);
  scenario=raw+phase;actual.vehicle.control.steering94.count22=(s16)lap;
  actual.length=sign32(next());actual.target=lap+(int)(phase%4)-1;
  actual.state=(phase/4)&1?3:4;actual.flag=sign32(next());actual.timer=sign16((u16)next());
  actual.pending=phase%3==0?0:phase%3==1?-1:1;
  actual.latch=sign32((raw&65535U)|(next()&0xffff0000U));
  threshold=sign32((u32)((int64_t)lap*actual.length));
  actual.vehicle.first=sign32(next());
  actual.vehicle.second=sign32((u32)threshold-(u32)actual.vehicle.first);
  if(phase<19){
   actual.length=0;actual.target=lap+9-(int)(6+phase);actual.pending=0;
   actual.vehicle.second=sign32(0xffffffffU-(u32)actual.vehicle.first);
  }else if(phase>=40){
   actual.length=0;actual.target=phase&1?INT_MIN:INT_MAX;actual.pending=-1;
   actual.vehicle.second=sign32(0xffffffffU-(u32)actual.vehicle.first);
  }
  mutation=(raw+phase)&15;expected=actual;count=index_=0;
  model();versus_select_update_lap_progress(&actual.vehicle);
  assert(index_==count);assert(!memcmp(&actual,&expected,sizeof(actual)));cases++;
 }
 for(i=0;i<12;i++)assert(paths[i]);
 for(i=6;i<RR_SEQUENCE_TABLE_COUNT;i++)if(i!=9)assert(messages[i]);
 printf("%llu cases; every counter encoding and valid mapped notification ID, complete snapshots and latch mutations\n",(unsigned long long)cases);
}
'''
  with TemporaryDirectory() as temp:
   path=Path(temp);(path/'test.c').write_text(code)
   subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-Wno-unused-function','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(path/'test.c'),'-o',str(path/'test')],check=True)
   subprocess.run([str(path/'test')],check=True)
if __name__=='__main__':unittest.main()
