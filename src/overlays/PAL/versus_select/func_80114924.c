#include "rr/overlay661_checkpoint.h"

extern s32 TrackLength;
extern s32 NumLaps;
extern s16 LapAnnouncePending;
extern s32 RaceState;
extern s16 MenuFadeTimer;
extern s32 TotalRaceTime;
extern s32 RaceVoiceFlags;

extern void SaveCarSnapshots(s32);
extern void SoundSetMasterVolume(s16, s16);

void versus_select_func_80114924(RrRaceCheckpointVehicle *vehicle) {
    RrRaceVehicleControl *control = &vehicle->control;
    s16 counter;
    s32 target;
    s32 message;

    counter = control->steering94.count22;
    if (((s32)((u32)vehicle->second + (u32)vehicle->first)) >= ((s32)((u32)(s32)counter * (u32)TrackLength))) {
        target = NumLaps;
        if (target >= counter) {
            control->steering94.count22=(s16)((u16)control->steering94.count22+1U);
            LapAnnouncePending = 0;
            if (control->steering94.count22 == target) {
                SaveCarSnapshots(0);
            } else if (target < control->steering94.count22) {
                if (RaceState < 4) {
                    LapAnnouncePending = -1;
                    RaceState = 4;
                    MenuFadeTimer = 75;
                    SoundSetMasterVolume(0, 33);
                    TotalRaceTime = 0;
                    SoundQueueEnqueue(19, 1);
                }
            }
        }
    }
    if (LapAnnouncePending == 0) {
        message = (s32)(9U - ((u32)NumLaps - (u32)(s32)control->steering94.count22));
        if (message >= 6) {
            if (message == 9) {
                message = 15;
            }
            if (!(RaceVoiceFlags & (0x200U << ((u32)message - 6U)))) {
                SoundQueueEnqueue(message, 0);
                RaceVoiceFlags |= 0x200U << ((u32)message - 6U);
            }
        }
        LapAnnouncePending = -1;
    }
}
