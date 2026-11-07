#ifndef RR_OVERLAY_RACE_STATE_H
#define RR_OVERLAY_RACE_STATE_H
#include "rr/race.h"

/* Two genuine views of the same vehicle owner. The control subrecord begins
 * at1F4; flat vehicle fields and subrecord fields retain their PAL offsets. */
typedef struct {u8 pad00[0x1F4];RrRaceVehicleControl control;} RrRaceVehicleControlView;
typedef union {
    RrRaceVehicle vehicle;
    RrRaceVehicleControlView parts;
} RrRaceVehicleViews;
extern s32 RaceState,TrackLength;
extern s16 ReverseCourse,MenuFadeTimer;
void SoundSetMasterVolume(s16 first,s16 second);
void time_attack_course_select_func_80117084(RrRaceVehicleViews *player);
RR_ASSERT_SIZE(RrRaceVehicleViews,0x320);
RR_ASSERT_OFFSET(RrRaceVehicleViews,parts,0);
RR_ASSERT_OFFSET(RrRaceVehicleControlView,control,0x1F4);
RR_ASSERT_OFFSET(RrRaceVehicle,half1EA,0x1EA);
RR_ASSERT_OFFSET(RrRaceVehicleControl,steering94,0x94);
#endif
