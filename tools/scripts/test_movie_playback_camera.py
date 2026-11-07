"""Replay camera: wrapped arithmetic, live track roots and explicit GTE simulation."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class ReplayCameraTest(unittest.TestCase):
    def test_native_camera(self):
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/movie_playback/func_{a}.c').read_text()
                           for a in ('80115554', '801156FC', '80115850'))
        bodies = re.sub(r'__asm__\("\$3"\)', '', bodies)
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/movie_playback_camera.h"
typedef struct {
 u32 guard[4],rng;RrReplayCameraContextPrefix camera;RrRaceVehicle vehicles[2];
 RrTrackSample3C tables[2][9];s32 scratch[256],table,count;u32 tail[4];
} State;
typedef struct {s32 op,a[5];u32 hash;} Event;
typedef struct {State s;unsigned calls;Event events[6];} World;
static World actual,reference,initial,*w;static unsigned scenario,mutation;
RrTrackSample3C *TrackNodes;
#define RaceContext w->s.camera
#define PlayerCars w->s.vehicles
#define TrackNodeCount w->s.count
#undef RR_REPLAY_SCRATCH_WORDS
#define RR_REPLAY_SCRATCH_WORDS w->s.scratch
''' + bodies + r'''
static u32 next(void){u32 x=w->s.rng;x^=x<<13;x^=x>>17;x^=x<<5;return w->s.rng=x;}
static s32 word(u32 u){s32 v;memcpy(&v,&u,4);return v;}
static s32 wrap(int64_t v){return word((u32)(uint64_t)v);}
static s16 half(s32 a){u16 u=(u16)(u32)a;s16 b;memcpy(&b,&u,2);return b;}
static void change(void){unsigned i,j;RrReplayCameraContextPrefix *c=&w->s.camera;
 w->s.table=next()%2;w->s.count=4+next()%6;TrackNodes=w->s.tables[w->s.table];
 c->node498=next()%w->s.count;c->fraction49C=next()%1024;c->lateral49E=half(next());
 c->pitch460=half(next());c->yaw462=half(next());c->roll464=half(next());c->angle490=half(next());c->velocity4A0=word(next());
 for(i=0;i<3;i++){c->position450[i]=word(next());c->delta478[i]=word(next());}
 for(i=0;i<4;i++){c->target468[i]=word(next());c->offset488[i]=half(next());}
 for(i=0;i<2;i++)for(j=0;j<9;j++){w->s.tables[i][j].x08=word(next());w->s.tables[i][j].z0C=word(next());w->s.tables[i][j].y10=half(next());w->s.tables[i][j].heading12=half(next());}
}
static Event *event(s32 op){size_t i;Event *e;u32 h=2166136261U;assert(w->calls<6);
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 e=&w->events[w->calls++];memset(e,0,sizeof(*e));e->op=op;e->hash=h;return e;
}
static void effect(void){if(mutation)change();}
void InterpolateLanePointXYZ(const RrTrackSample3C *a,const RrTrackSample3C *b,s32 lateral,s32 fraction,s32 *out){
 Event *e=event(1);s32 ia=a-TrackNodes,ib=b-TrackNodes;unsigned i;
 assert(ia>=0&&ia<w->s.count&&ib>=0&&ib<w->s.count);
 e->a[0]=ia;e->a[1]=ib;e->a[2]=lateral;e->a[3]=fraction;
 /* Bounded simulated samples keep the sqrt input valid. Full camera position
  * and velocity words still exercise wrapping subtraction and smoothing. */
 out[0]=(s32)(((u32)a->x08^(u32)b->x08)&2047)-1024;
 out[1]=(s32)(((u32)a->y10+(u32)b->y10+(u32)lateral)&2047)-1024;
 out[2]=(s32)(((u32)a->z0C+(u32)b->z0C+(u32)fraction)&2047)-1024;
 out[3]=0;for(i=0;i<3;i++)e->a[4]^=out[i];effect();
}
s32 Atan2Table(s32 y,s32 x){Event *e=event(2);s32 result=(s32)(next()%4097)-1024;e->a[0]=y;e->a[1]=x;e->a[2]=result;effect();return result;}
s32 CordicSqrt(s32 squared){Event *e=event(3);s32 result=(s32)(next()&0x7fffffffU);assert(squared>=0);e->a[0]=squared;e->a[1]=result;effect();return result;}
u16 *RotMatrix(s16 *angles,u16 *matrix){Event *e=event(4);unsigned i;assert(((uintptr_t)matrix&3)==0);for(i=0;i<3;i++)e->a[i]=angles[i];for(i=0;i<16;i++)matrix[i]=(u16)next();effect();return matrix;}
void *ApplyMatrixSV(const RrMatrixWords32 *matrix,const s16 *offset,RrPreviewVector8 *output){
 Event *e=event(5);u32 words[8];unsigned i;assert(offset==w->s.camera.offset488);memcpy(words,matrix,32);
 for(i=0;i<8;i++)e->a[3]^=word(words[i]);for(i=0;i<3;i++)e->a[i]=offset[i];
 output->x=half((s32)offset[0]+(s32)(words[0]&65535));output->y=half((s32)offset[1]+(s32)(words[1]&65535));output->z=half((s32)offset[2]+(s32)(words[2]&65535));effect();return output;
}
void BuildCameraMatrices(void){event(6);effect();}
static void expected(unsigned kind){RrReplayCameraContextPrefix *c=&w->s.camera;
 if(kind==0){
  int fraction=c->fraction49C,a=c->node498-(3072-fraction)/1024,b;int old,newAngle,doubled,delta,velocity;int64_t product;
  if(a<0)a+=w->s.count;b=a-1;if(b<0)b+=w->s.count;
  InterpolateLanePointXYZ(&TrackNodes[a],&TrackNodes[b],0,(1024-fraction)&1023,c->target468);
  for(a=0;a<3;a++)c->delta478[a]=wrap((int64_t)c->target468[a]-c->position450[a]/4);
  old=c->yaw462;newAngle=half(1024-Atan2Table(c->delta478[0],c->delta478[2]));
  doubled=(newAngle-old)*2;delta=(int)((u32)doubled&4095);if(doubled&2048)delta-=4096;c->yaw462=newAngle;
  velocity=c->velocity4A0;product=(int64_t)wrap((int64_t)delta-velocity)*15;c->velocity4A0=wrap((int64_t)velocity+wrap(product)/16);
  c->roll464=half(wrap((int64_t)c->velocity4A0+c->roll464)/2);
 }else if(kind==1){
  s32 first[4],second[4],a=c->node498,b=a+1,dx,dz,height,root,angle;
  if(b>=w->s.count)b=0;InterpolateLanePointXYZ(TrackNodes+a,TrackNodes+b,c->lateral49E,c->fraction49C,first);
  b=c->node498;a=b-1;c->position450[1]=wrap((int64_t)first[1]-120);if(a<0)a+=w->s.count;
  InterpolateLanePointXYZ(TrackNodes+a,TrackNodes+b,c->lateral49E,c->fraction49C,second);
  dx=wrap(((int64_t)second[0]-first[0])*4);dz=wrap(((int64_t)second[2]-first[2])*4);height=wrap((int64_t)second[1]-first[1]);
  root=CordicSqrt(wrap((int64_t)dx*dx+(int64_t)dz*dz));angle=Atan2Table(height,root>>6);c->pitch460=half(1024-angle);
 }else{
  RrPreviewMatrixWorkspace matrix;RrPreviewVector8 vector={0};int i;
  w->s.scratch[6]=c->pitch460;w->s.scratch[7]=c->yaw462;w->s.scratch[8]=c->roll464;
  for(i=0;i<3;i++)w->s.scratch[2+i]=c->position450[i];
  vector.x=half(w->s.scratch[6]);vector.y=half(w->s.scratch[7]);vector.z=half(w->s.scratch[8]);
  RotMatrix(&vector.x,matrix.halves);ApplyMatrixSV(&matrix.matrix,c->offset488,&vector);
  w->s.vehicles[0].pos20[0]=c->position450[0]/4+vector.x;w->s.vehicles[0].pos20[1]=c->position450[1]/4+vector.y;w->s.vehicles[0].pos20[2]=c->position450[2]/4+vector.z;
  w->s.vehicles[0].rot50=(s32)c->pitch460+c->angle490;w->s.vehicles[0].rot54=c->yaw462;w->s.vehicles[0].rot58=c->roll464;BuildCameraMatrices();
 }
}
int main(void){unsigned kind,i;
 assert((uintptr_t)actual.s.tables>UINT32_MAX);_Static_assert(sizeof(RrRaceVehicle)==0x320,"whole vehicle");
 for(scenario=0;scenario<65536;scenario++)for(kind=0;kind<3;kind++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0x15554+scenario*23+kind;change();
  w->s.camera.yaw462=half(scenario);w->s.camera.fraction49C=scenario%1024;w->s.camera.node498=scenario%w->s.count;
  for(i=0;i<256;i++)w->s.scratch[i]=word(next());
  actual=reference=initial;w=&actual;TrackNodes=w->s.tables[w->s.table];
  if(kind==0)movie_playback_camera_steer();else if(kind==1)movie_playback_camera_update_pitch();else movie_playback_camera_apply();
  w=&reference;TrackNodes=w->s.tables[w->s.table];expected(kind);assert(!memcmp(&actual,&reference,sizeof(actual)));
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
