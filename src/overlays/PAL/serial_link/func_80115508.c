#include "rr/overlay667_checkpoint.h"

extern s32 TrackLength;
extern s32 NumLaps;
extern s32 RaceState;
extern s32 TotalRaceTime;
extern s16 MenuFadeTimer;
extern void SaveCarSnapshots(s32);
extern void SoundSetMasterVolume(s16, s16);
extern void *SoundQueueEnqueue(s32, s32);

void serial_link_update_checkpoint(RrRaceCheckpointVehicle *arg0) {
    RrRaceVehicleControl *sub = &arg0->control;
    /* MIPS sums and products retain their low 32 bits. */
    if ((s32)((u32)arg0->second + (u32)arg0->first) >= (s32)((u32)(s32)sub->steering94.count22 * (u32)TrackLength)) {
        if (sub->steering94.count22 <= NumLaps) {
            sub->steering94.count22=(s16)((u16)sub->steering94.count22+1U);
            if (sub->steering94.count22 == NumLaps) {
                SaveCarSnapshots(0);
            } else if (sub->steering94.count22 > NumLaps) {
                if (RaceState < 4) {
                    TotalRaceTime = 1;
                    RaceState = 4;
                    MenuFadeTimer = 75;
                    SoundSetMasterVolume(0, 71);
                    SoundQueueEnqueue(19, 1);
                }
            }
        }
    }
}
