#ifndef RR_OVERLAY661_RACE_CONTROLLER_H
#define RR_OVERLAY661_RACE_CONTROLLER_H
#include "rr/overlay660_race_controller.h"
#include "rr/race_checkpoint_types.h"
typedef struct {s32 x,y,z,w;} Rr661FrameVector16;
typedef struct {RrGpuScratchPointers pointers;Rr661FrameVector16 position08,rotation18;u8 reserved28[0x5E - 0x28];s16 mode5E;} Rr661FrameScratch;
#ifndef RR_661_FRAME_SCRATCH
#define RR_661_FRAME_SCRATCH ((Rr661FrameScratch *)0x1F800000U)
#endif
#ifndef RR_661_FRAME_MODE
#define RR_661_FRAME_MODE (*(s16 *)0x1F80005EU)
#endif
extern Rr661FrameVector16 D_801183D0[2],D_801183F0[2];
extern s32 TotalRaceTime;
extern s16 versus_select_D_80118448[2];
void DrawHudNumber5(s32,s32);void UpdateHudRankDisplay(s32,s32);void DrawLapCounter(s32,s32);void UpdateFinalLapIndicator(s32);void DrawSplitScreenDivider(s32);void DrawTimeRemainingForPlayer(s32,s32);void SoundSetMasterVolume(s16,s16);
void versus_select_func_80114924(RrRaceCheckpointVehicle *);
/* The hardware view has two full CPU pointers followed by two four-word
 * vectors. Native builds grow the pointer prefix; vector and tail offsets
 * follow that prefix rather than retaining PAL pointer widths.
 * Services may replace frame roots and mutate both complete vector tables,
 * vehicle/input owners and the later-read counters. Frame/tick arithmetic
 * wraps32; rendering visits both players and publishes current scratch state. */
void versus_select_func_80114B00(void);
RR_ASSERT_SIZE(Rr661FrameVector16,0x10);
RR_ASSERT_OFFSET(Rr661FrameScratch,pointers,0);
RR_ASSERT_OFFSET(Rr661FrameScratch,position08,sizeof(RrGpuScratchPointers));
RR_ASSERT_OFFSET(Rr661FrameScratch,rotation18,sizeof(RrGpuScratchPointers)+0x10);
RR_ASSERT_OFFSET(Rr661FrameScratch,mode5E,sizeof(RrGpuScratchPointers)+0x56);
RR_ASSERT_SIZE(Rr661FrameScratch,sizeof(RrGpuScratchPointers)+0x58);
#endif
