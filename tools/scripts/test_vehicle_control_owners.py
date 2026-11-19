from scene_lane_fixture_transport import transport as transport_scene_lanes
"""Track/AI paths use the actual pointer-bearing native vehicle control owner.

Coordinates/products stay in s32 range and sample indices stay inside eight rows.
Inactive lanes have timer0/10; active lanes cover signed convergence and clamping.
Typed projection/interpolation stubs mutate live owners; no GTE is simulated.
"""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

from c_functions import definition_source, source_owners

ROOT = Path(__file__).resolve().parents[2]


class VehicleControlOwnersTest(unittest.TestCase):
    def test_projection_and_lane_target_with_mutable_services(self):
        owners=source_owners(ROOT/'src/main/PAL')
        bodies='\n'.join(definition_source(owners[name].read_text(),name)
                         for name in ('ComputeCarTrackSurfaceProj','UpdateComCarTargetPoint'))
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "rr/track_projection.h"
#include "rr/track_lateral.h"
static TrackSample3C samples[2][8];
static TrackLaneOffset8 offsets[2][8];
typedef struct {
    u32 before[4];RaceVehicle vehicle;
    TrackSample3C *nodes;TrackLaneOffset8 *lanes;s32 count;
    u32 calls;s32 arguments[5];u32 after[4];
} World;
static World actual,expected,*w;
static unsigned scenario,mutation;
#define TrackNodes w->nodes
#define TrackLaneOffsets w->lanes
#define TrackNodeCount w->count
static void mutate(void) {
    if(!mutation)return;
    w->nodes=samples[1];w->lanes=offsets[1];
    w->vehicle.control.laneTimer38=(scenario&1)?0:10;
    w->vehicle.control.laneTarget3C=(s16)(scenario%2001)-1000;
    w->vehicle.position20.words[0]=(s32)(scenario%4001)-2000;
    w->vehicle.position20.words[2]=(s32)(scenario%6001)-3000;
}
void ProjectToTrackSegment(TrackOffsetArgument x,TrackOffsetArgument z,s32 node,
                          s16 *fraction,s16 *lateral,s32 *remaining) {
    assert(w->calls++==0&&lateral==&w->vehicle.control.laneCurrent3E);
    assert((uintptr_t)fraction>UINT32_MAX&&(uintptr_t)remaining>UINT32_MAX);
    w->arguments[0]=1;w->arguments[1]=x.value;w->arguments[2]=z.value;w->arguments[3]=node;
    mutate();*fraction=123;*lateral=(s16)(scenario%1601)-800;*remaining=-123;
}
void InterpolateLanePointXZ(const TrackSample3C *first,const TrackSample3C *second,
                            s32 lateral,s32 fraction,s32 *output) {
    assert(w->calls++==0);
    assert(first>=samples[0]&&first<samples[0]+8&&second>=samples[0]&&second<samples[0]+8);
    assert((uintptr_t)output>UINT32_MAX&&fraction>=0&&fraction<1024);
    w->arguments[0]=2;w->arguments[1]=(s32)(first-samples[0]);
    w->arguments[2]=(s32)(second-samples[0]);w->arguments[3]=lateral;w->arguments[4]=fraction;
    output[0]=first->road.x+(second->road.x-first->road.x)*fraction/1024+lateral;
    output[2]=first->road.z+(second->road.z-first->road.z)*fraction/1024-lateral;
    mutate();
}
'''+bodies+r'''
static void reference(unsigned routine) {
    RaceVehicle *v=&w->vehicle;RaceVehicleControl *c=&v->control;
    s32 distance=1024-v->trackFraction170;
    s32 first=v->trackNodeF0-distance/1024,second,fraction,x,z,difference;
    if(first<0)first+=8;second=(first+7)%8;
    fraction=distance%1024;if(fraction<0)fraction+=1024;
    if(routine==0){
        TrackOffsetArgument dx,dz;s16 along;s32 remaining;
        dx.value=offsets[0][first].x+(offsets[0][second].x-offsets[0][first].x)*fraction/1024;
        dz.value=offsets[0][first].z+(offsets[0][second].z-offsets[0][first].z)*fraction/1024;
        ProjectToTrackSegment(dx,dz,v->trackNodeF0,&along,&c->laneCurrent3E,&remaining);
        if(!c->laneTimer38)c->laneTarget3C=c->laneCurrent3E;
    }else{
        if(!c->laneActive6A&&c->laneTimer38==0){
            s32 ax=samples[0][first].road.x+offsets[0][first].x;
            s32 az=samples[0][first].road.z+offsets[0][first].z;
            s32 bx=samples[0][second].road.x+offsets[0][second].x;
            s32 bz=samples[0][second].road.z+offsets[0][second].z;
            x=ax+(bx-ax)*fraction/1024;z=az+(bz-az)*fraction/1024;
        }else{
            s32 out[3];
            if(c->laneActive6A){
                difference=c->laneDelta6C;
                if(difference>-16&&difference<16){
                    c->laneActive6A=0;c->laneStart3A=c->laneTarget3C;
                    if(c->laneMode88==3)c->laneMode88=0;
                }else{
                    if(difference>0){difference-=14;if(difference<0)difference=0;}
                    else{difference+=14;if(difference>0)difference=0;}
                    x=c->laneTarget3C-difference;
                    if(x<-800)x=-800;if(x>800)x=800;
                    c->laneStart3A=x;c->laneDelta6C=c->laneTarget3C-x;
                }
            }
            InterpolateLanePointXZ(samples[0]+first,samples[0]+second,c->laneStart3A,fraction,out);
            x=out[0];z=out[2];
        }
        c->pathDeltaX30=x-v->position20.words[0];c->pathDeltaZ34=z-v->position20.words[2];
    }
}
int main(void) {
    unsigned routine,i,bank;uint64_t count=0;
    const s16 fractions[]={-1024,-1,0,1,1023,1024,1025,2048};
    const s16 deltas[]={-1024,-16,-15,-1,0,1,15,16,1024};
    assert(sizeof(void*)==8&&offsetof(RaceVehicle,control)==0x1F8);
    assert((uintptr_t)&actual.vehicle>UINT32_MAX);
    for(bank=0;bank<2;bank++)for(i=0;i<8;i++){
        samples[bank][i].road.x=(s32)i*1103-3000+(s32)bank*100;
        samples[bank][i].road.z=(s32)i*-991+7000-(s32)bank*100;
        offsets[bank][i].x=(s16)(i*211)-700;offsets[bank][i].z=900-(s16)(i*193);
    }
    for(scenario=0;scenario<65536;scenario++)for(routine=0;routine<2;routine++)for(mutation=0;mutation<2;mutation++){
        memset(&actual,0xA6,sizeof(actual));
        actual.nodes=samples[0];actual.lanes=offsets[0];actual.count=8;actual.calls=0;
        for(i=0;i<sizeof(actual.vehicle);i++)((u8*)&actual.vehicle)[i]=(u8)(scenario*17+i*29);
        actual.vehicle.trackFraction170=fractions[scenario%8];actual.vehicle.trackNodeF0=2+(scenario/8)%4;
        actual.vehicle.position20.words[0]=(s32)(scenario%10001)-5000;
        actual.vehicle.position20.words[2]=(s32)(scenario%8001)-4000;
        actual.vehicle.control.laneActive6A=(scenario/32)%2;
        actual.vehicle.control.laneTimer38=((scenario/64)%2)?10:0;
        actual.vehicle.control.laneStart3A=(s16)(scenario%1601)-800;
        actual.vehicle.control.laneTarget3C=(s16)(scenario%4001)-2000;
        actual.vehicle.control.laneDelta6C=deltas[(scenario/128)%9];
        actual.vehicle.control.laneMode88=(scenario/1024)%4;
        memcpy(&expected,&actual,sizeof(actual));w=&actual;
        if(!routine)ComputeCarTrackSurfaceProj(&w->vehicle);else UpdateComCarTargetPoint(&w->vehicle);
        w=&expected;reference(routine);assert(!memcmp(&actual,&expected,sizeof(actual)));count++;
    }
    printf("PASS %llu guarded native cases; real control alignment, both track paths, live services\n",(unsigned long long)count);
    return 0;
}
'''
        with TemporaryDirectory() as temporary:
            directory=Path(temporary);c=directory/'test.c';exe=directory/'test';c.write_text(transport_scene_lanes(source,bodies))
            built=subprocess.run(['cc','-std=gnu11','-O2','-Wall','-Wextra','-Werror',
                                  '-fsanitize=undefined,bounds','-fno-sanitize-recover=all',
                                  '-I',str(ROOT/'include'),str(c),'-o',str(exe)],capture_output=True,text=True)
            self.assertEqual(built.returncode,0,built.stderr)
            subprocess.run([str(exe)],check=True)


if __name__=='__main__':
    unittest.main()
