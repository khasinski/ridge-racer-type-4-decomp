#ifndef RR_TEST_SCENE_CONTEXT_WIRE_H
#define RR_TEST_SCENE_CONTEXT_WIRE_H
/* Frozen PAL tokens bind to real native resource owners. No token becomes a
 * host pointer; the camera fixtures do not access these resource payloads. */
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/scene_camera_types.h"
#include "rr/race_types.h"
#include "game/gfx/dispenv.h"
/* E8 is a real pointer into complete standings storage. Frozen camera inputs
 * carry only an opaque PAL token; keep it separate from the native pointer. */
typedef struct {
    u32 address;
    u32 before[4];
    RaceStandingsTable table;
    u32 after[4];
} TestSceneStandingsBinding;
static TestSceneStandingsBinding rr_scene_standings;
static inline void rr_scene_context_prefix_load(SceneRaceContext *context,const void *input){
    const u8 *prefix=input;
    memcpy(context,prefix,0xE8);
    memset(&rr_scene_standings,0xA5,sizeof(rr_scene_standings));
    memcpy(&rr_scene_standings.address,prefix+0xE8,4);
    context->standings=rr_scene_standings.address?&rr_scene_standings.table:NULL;
    assert((uintptr_t)&rr_scene_standings.table>UINT32_MAX);
}
static inline void rr_scene_context_prefix_store(u8 output[0xEC],const SceneRaceContext *context){
    assert(context->standings==(rr_scene_standings.address?&rr_scene_standings.table:NULL));
    memcpy(output,context,0xE8);
    memcpy(output+0xE8,&rr_scene_standings.address,4);
    const u8 *p=(const u8*)&rr_scene_standings.before;
    for(size_t i=0;i<sizeof(rr_scene_standings)-offsetof(TestSceneStandingsBinding,before);i++)assert(p[i]==0xA5);
}
typedef struct {
    u32 addresses[4];
    u32 before[4];CamNode44 nodes[2];u32 afterNodes[4];
    u8 primary[0xA00];u32 afterPrimary[4];
    TrackLightPreset8 lightPresets[4];u32 afterSecondary[4];
    u8 tertiary[0x200];u32 after[4];
} TestSceneResourceBindings;
static TestSceneResourceBindings rr_scene_resources;
/* Five PAL labels at +53C..54C are pointer cells in the same owner.
 * Old camera inputs do not dereference them; preserve their opaque tokens while
 * binding non-null cells to complete, guarded native allocations. */
typedef struct {
    u32 addresses[5];
    u32 before[4];RaceStartGrid60 grid;u32 afterGrid[4];
    RaceSpecEntry30 specs[3];u32 afterSpecs[4];
    TrackLaneOffset8 lanes[2];u32 after[4];
} TestSceneGridBindings;
static TestSceneGridBindings rr_scene_grid;
static inline void rr_scene_grid_load(SceneRaceContext *context,const u8 input[20]) {
    memset(&rr_scene_grid,0xA5,sizeof(rr_scene_grid));
    memcpy(rr_scene_grid.addresses,input,20);
    context->startGrid53C=rr_scene_grid.addresses[0]?&rr_scene_grid.grid:NULL;
    RaceSpecEntry30 **cells[3]={&context->carSpecs540,&context->specTargets544[0],&context->specTargets544[1]};
    for(unsigned i=0;i<3;i++) {
        unsigned owner=i;
        for(unsigned j=0;j<i;j++)if(rr_scene_grid.addresses[j+1]==rr_scene_grid.addresses[i+1]){owner=j;break;}
        *cells[i]=rr_scene_grid.addresses[i+1]?&rr_scene_grid.specs[owner]:NULL;
    }
    context->laneOffsets54C=rr_scene_grid.addresses[4]?rr_scene_grid.lanes:NULL;
    assert((uintptr_t)&rr_scene_grid.grid>UINT32_MAX);
}
static inline void rr_scene_grid_store(u8 output[20],const SceneRaceContext *context) {
    assert(context->startGrid53C==(rr_scene_grid.addresses[0]?&rr_scene_grid.grid:NULL));
    const RaceSpecEntry30 *cells[3]={context->carSpecs540,context->specTargets544[0],context->specTargets544[1]};
    for(unsigned i=0;i<3;i++) {
        unsigned owner=i;
        for(unsigned j=0;j<i;j++)if(rr_scene_grid.addresses[j+1]==rr_scene_grid.addresses[i+1]){owner=j;break;}
        assert(cells[i]==(rr_scene_grid.addresses[i+1]?&rr_scene_grid.specs[owner]:NULL));
    }
    assert(context->laneOffsets54C==(rr_scene_grid.addresses[4]?rr_scene_grid.lanes:NULL));
    memcpy(output,rr_scene_grid.addresses,20);
    const u8 *p=(const u8*)&rr_scene_grid.before;
    for(size_t i=0;i<sizeof(rr_scene_grid)-offsetof(TestSceneGridBindings,before);i++)assert(p[i]==0xA5);
}

static inline void rr_scene_resources_unchanged(void){
    const u8 *p=(const u8*)&rr_scene_resources.before;
    for(size_t i=0;i<sizeof(rr_scene_resources)-offsetof(TestSceneResourceBindings,before);i++)assert(p[i]==0xA5);
}
static inline void rr_scene_context_tail_load(SceneRaceContext *context,const void *input){
    const u8 *tail=input;
    memset(&rr_scene_resources,0xA5,sizeof(rr_scene_resources));
    memcpy((u8*)context+offsetof(SceneRaceContext,reserved250),tail,0x2EC);
    rr_scene_grid_load(context,tail+0x2EC);
    memcpy(rr_scene_resources.addresses,tail+0x300,16);
    context->cameraNodes550=rr_scene_resources.addresses[0]?rr_scene_resources.nodes:NULL;
    context->workspace554=rr_scene_resources.addresses[1]?rr_scene_resources.primary:NULL;
    context->lightPresets558=rr_scene_resources.addresses[2]?rr_scene_resources.lightPresets:NULL;
    context->workspace55C=rr_scene_resources.addresses[3]?rr_scene_resources.tertiary:NULL;
    memcpy(&context->model560,tail+0x310,4);
    context->soundPayload564=NULL;
    context->lookupTables=NULL;
    assert((uintptr_t)rr_scene_resources.nodes>UINT32_MAX);
    assert((uintptr_t)rr_scene_resources.primary>UINT32_MAX);
}
static inline void rr_scene_context_tail_store(u8 tail[0x314],const SceneRaceContext *context){
    assert(context->cameraNodes550==(rr_scene_resources.addresses[0]?rr_scene_resources.nodes:NULL));
    assert(context->workspace554==(rr_scene_resources.addresses[1]?rr_scene_resources.primary:NULL));
    assert(context->lightPresets558==(rr_scene_resources.addresses[2]?rr_scene_resources.lightPresets:NULL));
    assert(context->workspace55C==(rr_scene_resources.addresses[3]?rr_scene_resources.tertiary:NULL));
    memcpy(tail,(const u8*)context+offsetof(SceneRaceContext,reserved250),0x2EC);
    rr_scene_grid_store(tail+0x2EC,context);
    memcpy(tail+0x300,rr_scene_resources.addresses,16);memcpy(tail+0x310,&context->model560,4);
    rr_scene_resources_unchanged();
}
/* Original LP64 input format, independent of the growing production owner. */
typedef union {u64 words[0x578/8];u8 bytes[0x578];} FrozenNativeSceneInput578;
typedef struct {u32 before[4];FrozenNativeSceneInput578 value;u32 after[4];} FrozenNativeSceneOwner598;
static inline void rr_scene_context_load_native578(SceneRaceContext *context,const FrozenNativeSceneInput578 *input){
    memset(context,0xA5,sizeof(*context));
    rr_scene_context_prefix_load(context,input->bytes);
    memcpy((u8*)context+0xF0,input->bytes+0xF0,0x170);
    rr_scene_context_tail_load(context,input->bytes+0x260);
}
_Static_assert(sizeof(FrozenNativeSceneInput578)==0x578,"frozen LP64 scene input");
_Static_assert(sizeof(FrozenNativeSceneOwner598)==0x598,"frozen input owner");
_Static_assert(offsetof(SceneRaceContext,cameraEC)==0xF0,"native inline camera follows full standings pointer");
_Static_assert(offsetof(SceneRaceContext,reserved250)==0x260,"native scalar suffix start");
_Static_assert(sizeof(SceneRaceContext)==0x5C0,"complete native scene owner");
_Static_assert(offsetof(SceneRaceContext,startGrid53C)==0x550,"aligned native grid root");
_Static_assert(offsetof(SceneRaceContext,cameraNodes550)==0x578,"native resource pointer boundary");
_Static_assert(offsetof(SceneRaceContext,lookupTables)==0x5A8,"native resource-table pointer");
#endif
