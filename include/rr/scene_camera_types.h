#ifndef RR_SCENE_CAMERA_TYPES_H
#define RR_SCENE_CAMERA_TYPES_H

#include "rr/geometry.h"
#include "rr/lighting_types.h"
#include "rr/track_lookup_types.h"
#include "rr/race_spec_types.h"
#include "rr/track_lane_types.h"
#include "game/car/grid.h"
#include "rr/demo_fade_types.h"

struct CamNode44;
struct TrackLightPreset8;
struct RaceStandingsTable;

/* A word-component position followed by the SDK short-angle vector. */
typedef struct {
    VectorWords16 position00;
    Vec3sFields8 rotation10;
} SceneCameraPose18;

typedef struct {
    SceneCameraPose18 pose00;
    s32 absolute18, distance1C, projection20;
} SceneCameraView24;

typedef struct {
    SceneCameraView24 view00;
    s32 time24, restart28;
} SceneCameraKeyframe2C;

typedef SceneCameraKeyframe2C SceneCameraTrack24[24];

/* Shared storage initialized by InitCameraState and consumed by both the
 * showcase controller and the reflection renderer. Unknown words stay
 * explicit; they are not separate, incompatible views of the same camera. */
typedef struct {
    SceneCameraPose18 pose00;
    SceneCameraPose18 parts18[4];
    PreviewLighting40 lighting78;
    s32 extentA0, mirrorA4;
    s32 reservedA8;
    s32 offsetAC, offsetB0;
    s32 reservedB4[4];
    s32 offsetC4, offsetC8;
    s32 reservedCC[13];
    s32 distance100;
    s32 frame104, phase108, segment10C;
    VectorWords16 position110;
    Vec3s8 rotation120;
    s32 reserved128, value12C, value130, reserved134;
    SceneCameraTrack24 *tracks138;
    s32 mode13C, mode140;
    s32 reserved144[7];
} SceneCameraState160;

/* Written by race-audio setup, then consumed by the staged bank loader and
 * slot starter. All four words are configuration, not independent globals. */
typedef struct {
    s32 sfxVabId;
    s32 carVariants[2];
    s32 comBank;
} RaceSoundConfig;
RR_ASSERT_SIZE(RaceSoundConfig, 0x10);
RR_ASSERT_OFFSET(RaceSoundConfig, carVariants, 4);
RR_ASSERT_OFFSET(RaceSoundConfig, comBank, 0xC);

typedef struct {
    s16 startOrder00[8];
    s32 comCarCount10;
    s32 playerCount14;
    s32 reserved18;
    s32 cameraNodeChanged1C;
    VectorWords16 cameraFollowPosition20;
    Vec3s8 cameraFollowOffset30;
    s32 cameraDistanceSpan38, cameraNearDistance3C;
    s32 cameraPreviousDistance40, cameraLimitReached44;
    s32 cameraNodeFrames48, cameraNodeIndex4C;
    u32 reserved50[4];
    SceneCameraView24 view60;
    s32 reserved84[(0xB0 - 0x84) / 4];
    s32 sceneStateB0;
    s32 reservedB4[(0xD4 - 0xB4) / 4];
    s32 overlayEnabledD4, overlayStyleD8, overlayModeDC, overlayBlendE0;
    u8 overlayColorE4[4];
    struct RaceStandingsTable *standings;
    SceneCameraState160 cameraEC;
    SceneCameraState160 *camera24C;
    s32 reserved250[99];
    s32 overlayToggle3DC;
    DemoFadeState64 fade3E0;
    s32 sceneStep444;
    u8 reserved448[0x53C - 0x448];
    RaceStartGrid60 *startGrid53C;
    RaceSpecEntry30 *carSpecs540;
    RaceSpecEntry30 *specTargets544[2];
    TrackLaneOffset8 *laneOffsets54C;
    struct CamNode44 *cameraNodes550;
    u8 *workspace554;
    struct TrackLightPreset8 *lightPresets558;
    u8 *workspace55C;
    /* The legacy mesh API still accepts this slot as an integer. Its pointer
     * migration belongs with DrawMeshPrimList and the reflection packet path. */
    s32 model560;
    char *soundPayload564;
    TrackEventDirectory *lookupTables;
    RaceSoundConfig sound;
} SceneRaceContext;

RR_ASSERT_SIZE(SceneCameraPose18, 0x18);
RR_ASSERT_OFFSET(SceneCameraPose18, rotation10, 0x10);
RR_ASSERT_SIZE(SceneCameraView24, 0x24);
RR_ASSERT_OFFSET(SceneCameraView24, projection20, 0x20);
RR_ASSERT_SIZE(SceneCameraKeyframe2C, 0x2C);
RR_ASSERT_OFFSET(SceneCameraKeyframe2C, time24, 0x24);
RR_ASSERT_SIZE(SceneCameraTrack24, 0x420);
RR_ASSERT_OFFSET(SceneCameraState160, parts18, 0x18);
RR_ASSERT_OFFSET(SceneCameraState160, lighting78, 0x78);
RR_ASSERT_OFFSET(SceneCameraState160, extentA0, 0xA0);
RR_ASSERT_OFFSET(SceneCameraState160, offsetAC, 0xAC);
RR_ASSERT_OFFSET(SceneCameraState160, distance100, 0x100);
RR_ASSERT_OFFSET(SceneCameraState160, position110, 0x110);
RR_ASSERT_OFFSET(SceneCameraState160, rotation120, 0x120);
RR_ASSERT_OFFSET(SceneCameraState160, tracks138, 0x138);
RR_ASSERT_OFFSET(SceneRaceContext, startOrder00, 0);
RR_ASSERT_OFFSET(SceneRaceContext, comCarCount10, 0x10);
RR_ASSERT_OFFSET(SceneRaceContext, playerCount14, 0x14);
RR_ASSERT_OFFSET(SceneRaceContext, standings, 0xE8);
RR_ASSERT_OFFSET(SceneRaceContext, cameraNodeChanged1C, 0x1C);
RR_ASSERT_OFFSET(SceneRaceContext, cameraFollowPosition20, 0x20);
RR_ASSERT_OFFSET(SceneRaceContext, cameraFollowOffset30, 0x30);
RR_ASSERT_OFFSET(SceneRaceContext, cameraDistanceSpan38, 0x38);
RR_ASSERT_OFFSET(SceneRaceContext, cameraNodeIndex4C, 0x4C);
RR_ASSERT_OFFSET(SceneRaceContext, view60, 0x60);
RR_ASSERT_OFFSET(SceneRaceContext, sceneStateB0, 0xB0);
#ifdef __mips__
RR_ASSERT_SIZE(SceneCameraState160, 0x160);
RR_ASSERT_OFFSET(SceneCameraState160, mode13C, 0x13C);
RR_ASSERT_OFFSET(SceneRaceContext, cameraEC, 0xEC);
RR_ASSERT_SIZE(SceneRaceContext, 0x57C);
RR_ASSERT_OFFSET(SceneRaceContext, camera24C, 0x24C);
RR_ASSERT_OFFSET(SceneRaceContext, overlayToggle3DC, 0x3DC);
RR_ASSERT_OFFSET(SceneRaceContext, fade3E0, 0x3E0);
RR_ASSERT_OFFSET(SceneRaceContext, sceneStep444, 0x444);
RR_ASSERT_OFFSET(SceneRaceContext, startGrid53C, 0x53C);
RR_ASSERT_OFFSET(SceneRaceContext, carSpecs540, 0x540);
RR_ASSERT_OFFSET(SceneRaceContext, specTargets544, 0x544);
RR_ASSERT_OFFSET(SceneRaceContext, laneOffsets54C, 0x54C);
RR_ASSERT_OFFSET(SceneRaceContext, cameraNodes550, 0x550);
RR_ASSERT_OFFSET(SceneRaceContext, workspace554, 0x554);
RR_ASSERT_OFFSET(SceneRaceContext, lightPresets558, 0x558);
RR_ASSERT_OFFSET(SceneRaceContext, workspace55C, 0x55C);
RR_ASSERT_OFFSET(SceneRaceContext, model560, 0x560);
RR_ASSERT_OFFSET(SceneRaceContext, soundPayload564, 0x564);
RR_ASSERT_OFFSET(SceneRaceContext, lookupTables, 0x568);
RR_ASSERT_OFFSET(SceneRaceContext, sound, 0x56C);
#endif

#endif
