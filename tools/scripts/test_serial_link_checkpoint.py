"""Complete vehicle checkpoint state and callback protocol against a wrapped oracle."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess, unittest
ROOT=Path(__file__).resolve().parents[2]
class CheckpointTest(unittest.TestCase):
 def test_all_counter_encodings_and_callback_mutations(self):
  body=(ROOT/'src/overlays/PAL/serial_link/func_80115508.c').read_text()
  body='\n'.join(line for line in body.splitlines() if not line.startswith('#include') and not line.startswith('extern '))
  code=r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "rr/overlay667_checkpoint.h"
typedef struct {u32 prefix[4];RrRaceCheckpointVehicle vehicle;u32 suffix[4];
 s32 length,target,state,flag;s16 timer;u16 pad;u32 tail[4];} Environment;
static Environment actual,expected,pre[3];
static unsigned events[3],eventCount,eventIndex,mutation;
static u32 seed=0x66715508;static unsigned paths[8];
static u32 next(void){u32 x=seed;x^=x<<13;x^=x>>17;x^=x<<5;return seed=x;}
static s32 sign32(u32 x){s32 result;memcpy(&result,&x,4);return result;}
static s16 sign16(u16 x){s16 result;memcpy(&result,&x,2);return result;}
static void mutate(Environment *e,unsigned event){
 if(mutation&1){e->vehicle.first=sign32((u32)e->vehicle.first+0xfedcba98U);
 e->vehicle.control.steering94.count22=sign16((u16)(0x8000U+event));}
 if(mutation&2){e->length=sign32(0xdeadbeefU);e->target=INT_MIN;e->state=-12345;
 e->flag=77;e->timer=-123;}
}
static void expect_event(unsigned id){assert(eventCount<3);pre[eventCount]=expected;events[eventCount++]=id;mutate(&expected,id);}
static void service(unsigned id,s32 a,s32 b){
 assert(eventIndex<eventCount&&events[eventIndex]==id);
 assert(!memcmp(&actual,&pre[eventIndex],sizeof(actual)));
 if(id==0)assert(a==0);else if(id==1)assert(a==0&&b==71);else assert(a==19&&b==1);
 ++eventIndex;mutate(&actual,id);
}
static void SaveCarSnapshots(s32 unused){service(0,unused,0);}
static void SoundSetMasterVolume(s16 a,s16 b){service(1,a,b);}
static void *SoundQueueEnqueue(s32 a,s32 b){service(2,a,b);return 0;}
#define TrackLength (actual.length)
#define NumLaps (actual.target)
#define RaceState (actual.state)
#define TotalRaceTime (actual.flag)
#define MenuFadeTimer (actual.timer)
'''+body+r'''
static void model(void){
 int old=expected.vehicle.control.steering94.count22;
 s32 sum=sign32((u32)expected.vehicle.first+(u32)expected.vehicle.second);
 s32 threshold=sign32((u32)((int64_t)old*expected.length));
 if(sum<threshold){paths[0]++;return;}
 if(old>expected.target){paths[1]++;return;}
 old=sign16((u16)(old+1));expected.vehicle.control.steering94.count22=(s16)old;
 if(old==expected.target){paths[2]++;expect_event(0);return;}
 if(old<=expected.target){paths[3]++;return;}
 if(expected.state>=4){paths[4]++;return;}
 paths[5]++;expected.flag=1;expected.state=4;expected.timer=75;
 expect_event(1);expect_event(2);
}
int main(void){unsigned raw,phase,i;uint64_t cases=0;
 for(raw=0;raw<65536;raw++)for(phase=0;phase<12;phase++){
  unsigned char *bytes=(unsigned char *)&actual;
  int lap=sign16((u16)raw);s32 threshold;
  /* Every byte, including reserved fields and guards, participates in comparison. */
  for(i=0;i<sizeof(actual);i++)bytes[i]=(unsigned char)(i*37U+raw+phase);
  actual.vehicle.control.steering94.count22=(s16)lap;
  actual.length=sign32(next());actual.target=phase<8?lap+(int)(phase%4)-1:sign32(next());
  actual.state=(phase/4)&1?3:4;actual.flag=sign32(next());actual.timer=sign16((u16)next());
  threshold=sign32((u32)((int64_t)lap*actual.length));
  actual.vehicle.first=sign32(next());
  actual.vehicle.second=sign32((u32)threshold-(u32)actual.vehicle.first+(phase>=8?0xffffffffU:0U));
  mutation=(raw+phase)&3;expected=actual;eventCount=eventIndex=0;
  model();serial_link_func_80115508(&actual.vehicle);
  assert(eventIndex==eventCount);assert(!memcmp(&actual,&expected,sizeof(actual)));
  if(lap==32767&&actual.vehicle.control.steering94.count22==-32768)paths[6]++;
  if(mutation&&eventCount)paths[7]++;cases++;
 }
 for(i=0;i<8;i++)assert(paths[i]);
 printf("%llu cases, all counter encodings, guarded complete owners and callback snapshots\n",(unsigned long long)cases);
}
'''
  with TemporaryDirectory() as temp:
   path=Path(temp);(path/'test.c').write_text(code)
   subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-Wno-unused-function','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(path/'test.c'),'-o',str(path/'test')],check=True)
   subprocess.run([str(path/'test')],check=True)
if __name__=='__main__':unittest.main()
