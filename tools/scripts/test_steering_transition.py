"""Steering controller against an independent wrapping-word/state protocol model.

Curve services simulate MIPS arithmetic; no console hardware is executed.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import re, subprocess, unittest
ROOT = Path(__file__).resolve().parents[2]

class SteeringTransitionTest(unittest.TestCase):
 def test_modes_arithmetic_and_mutable_services(self):
  body = (ROOT/'tools/scripts/fixtures/views/func_80048DB0.c').read_text().split('#define CONTROL_VOL', 1)[1]
  body = '#define CONTROL_VOL'+body
  body = re.sub(r' __asm__\("\$[0-9]+"\)', '', body)
  source = r'''
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/steering.h"
typedef struct {u32 guard[4];RrRaceVehicle car;s32 trackCount;s16 mode,marker;u32 rng,tail[4];} State;
typedef struct {s32 kind,arg[4];State before;} Event;
typedef struct {State s;Event events[4];unsigned count;} World;
static World actual,reference,*w;
static unsigned scenario,mutate,seen[32],callback[4],modeSeen[65536];
static const s32 edge[16]={INT32_MIN,INT32_MIN+1,-61441,-61440,-4097,-4096,-1,0,1,3,4095,4096,61440,61441,INT32_MAX-1,INT32_MAX};
static const s32 phases[8]={INT32_MIN,-1,0,1,2,3,INT32_MAX-1,INT32_MAX};
#define TrackNodeCount (w->s.trackCount)
#define RaceType (w->s.mode)
#define CourseListIndex (w->s.marker)
static s32 word(u32 u){s32 n;memcpy(&n,&u,4);return n;}
static s16 half(u16 u){s16 n;memcpy(&n,&u,2);return n;}
static s32 add(s32 a,s32 b){return word((u32)a+(u32)b);}
static s32 sub(s32 a,s32 b){return word((u32)a-(u32)b);}
static s32 mul(s32 a,s32 b){return word((u32)a*(u32)b);}
static u32 next(void){u32 n=w->s.rng;n^=n<<13;n^=n>>17;n^=n<<5;return w->s.rng=n;}
static void fill(void){unsigned i;u8 *p=(u8*)&w->s.car;RrSteeringState *s=&w->s.car.steering288;
 for(i=0;i<sizeof(w->s.car);i++)p[i]=(u8)next();
 s->duration04=4+next()%1024;s->timer02=half((u16)next());
 s->phase28=phases[next()%8];s->mode00=half((u16)next());
 w->s.car.lateralMotion178=edge[next()%16];s->lateral24=edge[next()%16];s->angle0C=edge[next()%16];s->remaining08=edge[next()%16];
 w->s.car.steeringEnabled318=(next()%4)?half((u16)(next()|1)):0;
 w->s.car.steeringLock1B8=(next()%4)?0:half((u16)(next()|1));
 s->primary34.duration=(s32)(next()%64);s->secondary4C.duration=(s32)(next()%64);
 w->s.trackCount=1+next()%4096;w->s.mode=(next()%2)?6:half((u16)next());w->s.marker=next()%2;
}
static Event *event(s32 kind){Event *e;assert(w->count<4);e=&w->events[w->count++];memset(e,0,sizeof(*e));e->kind=kind;e->before=w->s;callback[kind]++;return e;}
/* Each expression is evaluated as a MIPS word, including products and shifts. */
s32 EvalRateCurve(const RrCurve24 *r,s32 time,s32 *out){RrCurve24 saved=*r;Event *e=event(1);s32 h,v;ptrdiff_t off=(u8*)r-(u8*)&w->s.car;
 assert(off==0x2BC||off==0x2D4);e->arg[0]=(s32)off;e->arg[1]=time;
 if(time>=saved.duration){v=-(saved.secondAcceleration/2);seen[24]++;}else{h=saved.duration/2;if(time<h){v=add(add(saved.initialRate,saved.firstAcceleration/2),mul(saved.firstAcceleration,time));seen[25]++;}else{v=add(add(saved.middleRate,saved.secondAcceleration/2),mul(saved.secondAcceleration,sub(time,h)));seen[26]++;}}
 if(mutate)fill();*out=v;return time>=saved.duration?-1:0;
}
void BuildRateCurve(RrCurve24 *record,s32 a,s32 duration,s32 b){s32 *r=(s32*)record;Event *e=event(2);s32 h,x,z;ptrdiff_t off=(u8*)r-(u8*)&w->s.car;
 assert(off==0x2D4&&duration>=3&&b==0);e->arg[0]=a;e->arg[1]=duration;e->arg[2]=b;
 h=duration/2;x=word((u32)a<<12);z=sub(x/h,word((u32)b<<11));
 if(mutate)fill();r[0]=x;r[1]=duration;r[2]=word((u32)b<<12);r[3]=z;r[4]=sub(z,r[2])/h;r[5]=word(0U-(u32)z)/h;
}
s32 AngleDiffAbs(s32 a,s32 b){Event *e=event(3);s32 d=(a&4095)-(b&4095);e->arg[0]=a;e->arg[1]=b;if(d<0)d=-d;if(d>2048)d=4096-d;if(mutate)fill();return d;}
'''+body+r'''
static void expected(RrRaceVehicle *v){RrSteeringState *s=&v->steering288;s32 output,mode,phase,duration,q,step,delta,reverse,offset,diff,gain;u32 unsignedReverse;
 if(!v->steeringEnabled318){v->rot54=v->heading1D4;seen[0]++;return;}
 mode=s->mode00;
 if(mode==0){v->rot54=v->heading1D4;seen[1]++;}
 else if(mode==1){seen[2]++;EvalRateCurve(&s->primary34,s->timer02,&output);
  s->angle0C=sub(s->angle0C,output);s->delta10=output;s->remaining08=sub(s->remaining08,output);v->rot54=s->angle0C/4096;
  if(!s->phase28){seen[3]++;v->lateralMotion178=s->delta10/8;duration=s->duration04;q=duration/4;
   if(s->timer02>=q){seen[4]++;BuildRateCurve(&s->secondary4C,(v->lateralMotion178/256)*2,duration-q,0);s->lateral24=word((u32)(v->lateralMotion178/256)<<12);s->phase28=mode;}else seen[5]++;
  }else seen[6]++;
  phase=s->phase28;if(phase>0){seen[7]++;EvalRateCurve(&s->secondary4C,phase-1,&output);s->lateral24=sub(s->lateral24,output);v->lateralMotion178=s->lateral24/16;s->phase28=add(s->phase28,1);}else seen[8]++;
  s->timer02=half((u16)((u16)s->timer02+1U));if(s->timer02>=s->duration04){s->mode00=2;seen[9]++;}else seen[10]++;
 }else if(mode==2||mode==3){
  if(mode==2){seen[11]++;s->lateral24/=16;delta=sub(v->trackNodeF0,s->targetNode18);reverse=sub(s->targetNode18,v->trackNodeF0);
   if(delta>=0)offset=delta/2;else{unsignedReverse=(u32)reverse;offset=word(unsignedReverse+(unsignedReverse>>31))>>1;}
   s->midpoint1C=sub(v->trackNodeF0,offset);s->mode00=3;
  }else seen[12]++;
  if(s->midpoint1C<v->trackNodeF0){v->lateralMotion178=s->lateral24;seen[13]++;}
  else{seen[14]++;q=mul(sub(s->lateral24,v->lateralMotion178),3)/4;s->lateral24=q;s->lateral24=add(q,v->lateralMotion178);v->lateralMotion178=s->lateral24;}
  assert(w->s.trackCount>0);offset=sub(s->targetNode18,v->trackNodeF0)%w->s.trackCount;if(offset>w->s.trackCount/2)offset-=w->s.trackCount;
  if(offset>=0){seen[15]++;diff=sub(v->rot54,v->heading1D4);step=diff&4095;if(diff&2048){step|=61440;seen[16]++;}
   if(!step||!(step/4)){s->mode00=0;seen[17]++;}else{v->rot54=sub(v->rot54,step/4);seen[18]++;}
  }else seen[19]++;
 }else seen[20]++;
 if(!v->steeringLock1B8){seen[21]++;diff=AngleDiffAbs(v->rot54,v->heading1D4);gain=(diff<513)?diff/8+64:127;s->gain30=mul(gain,96)/128;
  if(w->s.mode==6&&!w->s.marker){s->gain30=0;seen[27]++;}else seen[28]++;
  s->range2C=(diff<513)?diff*3+6144:7680;seen[29+(diff>=513)]++;
 }else{s->gain30=0;s->range2C=0;seen[22]++;}
 if(v->lateralMotion178<-61440){v->lateralMotion178=-61440;seen[31]++;}else if(v->lateralMotion178>61440){v->lateralMotion178=61440;seen[23]++;}
}
int main(void){unsigned i,mode;uint64_t cases=0;
 _Static_assert(sizeof(RrRaceVehicle)==0x320,"complete vehicle");_Static_assert(sizeof(RrRaceVehicleControl)==0x12C,"complete control");
 _Static_assert(sizeof(RrSteeringState)==0x64,"complete steering");_Static_assert(sizeof(RrCurve24)==24,"complete curve");
 _Static_assert(offsetof(RrRaceVehicle,steering288)==0x288,"steering offset");_Static_assert(offsetof(RrRaceVehicle,lateralMotion178)==0x178,"motion offset");
 _Static_assert(offsetof(RrRaceVehicle,steeringEnabled318)==0x318,"enabled offset");_Static_assert(offsetof(RrRaceVehicle,steeringLock1B8)==0x1B8,"lock offset");
 _Static_assert(offsetof(RrSteeringState,primary34)==0x34,"primary offset");_Static_assert(offsetof(RrSteeringState,secondary4C)==0x4C,"secondary offset");
 _Static_assert(offsetof(RrRaceVehicleControl,steering94)==0x94,"control steering offset");_Static_assert(offsetof(RrRaceVehicleControl,steeringEnabled124)==0x124,"control enabled offset");
 assert((uintptr_t)&actual.s.car>UINT32_MAX);
 for(scenario=0;scenario<131072;scenario++)for(mutate=0;mutate<2;mutate++){
  memset(&actual,0,sizeof(actual));memset(&reference,0,sizeof(reference));w=&actual;w->s.rng=0x48DB0U+scenario*73U;fill();
  if(scenario<65536){mode=scenario;w->s.car.steering288.mode00=half((u16)mode);modeSeen[mode]++;}
  else{mode=1+(scenario%3);w->s.car.steering288.mode00=mode;}
  w->s.car.steeringEnabled318=(scenario%17||mutate)?1:0;w->s.car.steeringLock1B8=(scenario%7)?0:1;
  w->s.car.steering288.phase28=phases[(scenario>>2)%8];
  w->s.car.steering288.timer02=(s16[]){INT16_MIN,-1,0,1,2,3,4,1024,INT16_MAX}[(scenario>>5)%9];
  if(scenario>=65536&&scenario%11==0){w->s.car.steering288.targetNode18=w->s.car.trackNodeF0;w->s.car.rot54=(s32)w->s.car.heading1D4+(s32[]){0,1,3,4,2047,2048,4095}[(scenario>>3)%7];}
  for(i=0;i<4;i++){w->s.guard[i]=next();w->s.tail[i]=next();}reference.s=actual.s;
  w=&actual;UpdateComCarLaneChange(&w->s.car);w=&reference;expected(&w->s.car);
  assert(!memcmp(&actual.s,&reference.s,sizeof(State)));assert(actual.count==reference.count);assert(!memcmp(actual.events,reference.events,actual.count*sizeof(Event)));cases++;
 }
 assert(cases==262144);for(i=0;i<32;i++){if(!seen[i])fprintf(stderr,"unvisited branch %u\n",i);assert(seen[i]);}for(i=1;i<4;i++)assert(callback[i]);for(i=0;i<65536;i++)assert(modeSeen[i]);return 0;
}
'''
  with TemporaryDirectory() as tmp:
   p=Path(tmp);(p/'test.c').write_text(source)
   r=subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(p/'test.c'),'-o',str(p/'test')],capture_output=True,text=True)
   self.assertEqual(r.returncode,0,r.stderr)
   subprocess.run([str(p/'test')],check=True)
if __name__=='__main__':unittest.main()
