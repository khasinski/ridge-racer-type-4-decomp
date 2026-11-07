#ifndef RR_OVERLAY668_CAMERA_H
#define RR_OVERLAY668_CAMERA_H
#include "rr/track_sample_types.h"
#include "rr/track_lateral.h"
#include "rr/race_types.h"
#include "rr/overlay665_matrix.h"
#include "psyq/libgte/gtecord.h"
typedef struct {
 u8 pad00[0xB0];s32 flagB0;u8 padB4[0x390];s32 counter444,step448,state44C;
 s32 position450[3],reserved45C;s16 pitch460,yaw462,roll464,reserved466;
 s32 target468[4],delta478[3];u8 reserved484[4];
 s16 offset488[4],angle490,timer492,timer494,reserved496;
 s32 node498;s16 fraction49C,lateral49E;s32 velocity4A0;
 s32 counter4A4,mode4A8,phase4AC,reserved4B0,counter4B4;
} RrReplayCameraContextPrefix;
extern RrReplayCameraContextPrefix RaceContext;
extern RrTrackSample3C *TrackNodes;
extern s32 TrackNodeCount;
s32 Atan2Table();
/* Node and its single-wrapped neighbours select accessible track samples.
 * Position and interpolation results use wrapping32 arithmetic. Fraction is
 *0..1023; sample count is at least4. Services may replace the sample table and
 * camera state. The angle service returns an angle in-1024..3072. */
void ovl668_func_80115554(void);
void ovl668_func_801156FC(void);
extern RrRaceVehicle PlayerCars[2];
u16 *RotMatrix(s16 *rotation,u16 *matrix);
void *ApplyMatrixSV(const RrMatrixWords32 *matrix,const s16 *vector,RrPreviewVector8 *output);
void BuildCameraMatrices(void);
void ovl668_func_80115850(void);
/* Explicit legacy position argument, ignored by the current body. */
void ovl668_func_80115460(s32 *position);
/* The track root after setup services covers at least321 samples. Services
 * may change roots and state; the final wrapped selection index must address
 * one of the20 lookup entries. Initializes from captured sample320, advances
 * the camera sixteen times, then publishes the post-service lookup value. */
void ovl668_func_80115DFC(void);
extern s32 VSyncThreshold,ViewUsesCarTrackNode,MaskBitSetting,D_80127E90,D_80127E94,D_800AD564;
extern s16 D_800AD520;
extern s32 ovl668_D_80116CC0[20];
void SetDispMask(s32);
void SetupDrawDispEnvs(s32,s32,s16,u32);
void InitViewState(void);
void SetupRenderer(s32);
void func_80055300(void);
void func_80055330(s32,u8,u8,u8);
void func_8005534C(s32);
#ifndef RR_REPLAY_SCRATCH_WORDS
#define RR_REPLAY_SCRATCH_WORDS ((s32 *)0x1F800000)
#endif
RR_ASSERT_SIZE(RrReplayCameraContextPrefix,0x4B8);
RR_ASSERT_OFFSET(RrReplayCameraContextPrefix,position450,0x450);
RR_ASSERT_OFFSET(RrReplayCameraContextPrefix,target468,0x468);
RR_ASSERT_OFFSET(RrReplayCameraContextPrefix,node498,0x498);
#endif
