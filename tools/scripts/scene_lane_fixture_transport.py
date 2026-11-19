"""Bind the legacy lane fixture to a complete native scene owner.

Its literal World, model, cases and PRNG stay unchanged. Entry and service name
adapters synchronize the old lane root with its real context field. No production
calculation is rewritten and no host pointer is reconstructed from address bits.
"""
from c_functions import definition_source


def transport(source, bodies):
    entries=('ComputeCarTrackSurfaceProj','UpdateComCarTargetPoint')
    services={
        'ProjectToTrackSegment':(
            'void ProjectToTrackSegment(TrackOffsetArgument x,TrackOffsetArgument z,s32 node,s16 *fraction,s16 *lateral,s32 *remaining)',
            'x,z,node,fraction,lateral,remaining'),
        'InterpolateLanePointXZ':(
            'void InterpolateLanePointXZ(const TrackSample3C *first,const TrackSample3C *second,s32 lateral,s32 fraction,s32 *output)',
            'first,second,lateral,fraction,output'),
    }
    support=r'''
static void rr_fixture_scene_load(void){
    rr_fixture_scene.value.laneOffsets54C=w->lanes;
    rr_fixture_scene_expected.value.laneOffsets54C=w->lanes;
}
static void rr_fixture_scene_begin(void){
    memset(&rr_fixture_scene,0xA5,sizeof(rr_fixture_scene));
    SceneRaceContext *p=&rr_fixture_scene.value;
    p->standings=NULL;p->cameraEC.tracks138=NULL;p->camera24C=NULL;
    p->startGrid53C=NULL;p->carSpecs540=NULL;p->specTargets544[0]=NULL;p->specTargets544[1]=NULL;
    p->cameraNodes550=NULL;p->workspace554=NULL;p->lightPresets558=NULL;p->workspace55C=NULL;p->soundPayload564=NULL;p->lookupTables=NULL;
    memcpy(&rr_fixture_scene_expected,&rr_fixture_scene,sizeof(rr_fixture_scene));
    rr_fixture_scene_load();assert((uintptr_t)p->laneOffsets54C>UINT32_MAX);
}
'''
    assert source.count(bodies)==1
    source=source.replace(bodies,support+bodies)
    for name,(signature,args)in services.items():
        body=definition_source(source,name).rstrip()
        assert source.count(body)==1
        source=source.replace(body,'#define '+name+' rr_fixture_service_'+name+'\n'+body+
                              '\n#undef '+name+'\n'+signature+'{rr_fixture_service_'+name+'('+args+');rr_fixture_scene_load();}\n')
    for name in entries:
        body=definition_source(bodies,name).rstrip()
        assert source.count(body)==1
        source=source.replace(body,'#define '+name+' rr_fixture_entry_'+name+'\n'+body+
                              '\n#undef '+name+'\nvoid '+name+'(RaceVehicle *vehicle){rr_fixture_scene_begin();rr_fixture_entry_'+name+'(vehicle);assert(!memcmp(&rr_fixture_scene,&rr_fixture_scene_expected,sizeof(rr_fixture_scene)));}\n')
    prefix='''#include "rr/scene_camera_state.h"
typedef struct {u32 before[4];SceneRaceContext value;u32 after[4];} TestLaneSceneOwner;
static TestLaneSceneOwner rr_fixture_scene,rr_fixture_scene_expected;
#define RaceContext rr_fixture_scene.value
static void rr_fixture_scene_load(void);
'''
    for name in entries:
        assert source.count(definition_source(bodies,name).rstrip())==1
    return prefix+source
