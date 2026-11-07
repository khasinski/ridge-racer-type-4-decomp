#include "rr/race_types.h"
#include "rr/menu_choice.h"
#include "game/ui/uipart1.h"
#include "game/race/raceset.h"
#include "rr/overlay661_init.h"
#include "rr/audio_loop.h"
#include "rr/menu_transition.h"
#include "game/race/raceset.h"

typedef struct { int key; short idx; } W10_Q;

typedef struct { char p[0x184]; int a; int b; char q[0x62]; short n; } W10_O;

void SaveCarSnapshots(s32);
void *SoundQueueEnqueue(s32, s32);
void SoundSetMasterVolume(s16, s16);
extern u8 GpQualifyRankLimits[];
extern s16 MenuFadeTimer;
extern s32 CourseBestLap[2][8];
extern s16 ReverseCourse;
extern s32 TrackLength;
extern s32 CourseBestLapTimes[2][8][3];
extern s16 CourseIndex;
extern s32 NumLaps;
extern s32 CourseBestLapRecords[];
extern s32 LapFrameCounts[];
extern s32 TotalRaceTime;
extern s32 BestLapTime;
extern s32 LapTimes[];
extern s32 RaceState;
extern s32 BestSplitTimes[3];
extern s32 SplitTimes[3];
extern s32 NextSplitIndex;
extern s16 LapAnnouncePending;
extern s32 SplitDistances[3];
extern short NumCars;
extern W10_Q *RankTable[];
extern W10_O *CarTable[];
extern void SetupRace();
extern unsigned short GameTypeSceneIds[];

void UpdateLapTiming(RrRaceVehicle *arg0);
void UpdateRaceRanking(void);
void SetupRaceAndScene(void);

void UpdateLapTiming(RrRaceVehicle *arg0) {
    RrRaceVehicleControl *s0;
    s16 v1;
    s32 msg;
    s32 target;

    s0 = (RrRaceVehicleControl *)((u8 *)arg0 + 0x1F4);
    v1 = s0->steering94.count22;
    if (v1 > 0) {
        if (v1 <= NumLaps) {
            LapFrameCounts[v1 - 1] += 1;
            if (LapFrameCounts[s0->steering94.count22 - 1] > 0xFFFF) {
                LapFrameCounts[s0->steering94.count22 - 1] = 0x10000;
            }
            LapTimes[s0->steering94.count22 - 1] = FramesToMilliseconds(
                LapFrameCounts[s0->steering94.count22 - 1],
                PsyqRand() % 40
            );
            if (LapTimes[s0->steering94.count22 - 1] > 0x927BE) {
                LapTimes[s0->steering94.count22 - 1] = 0x927BF;
            }
            if (RaceType == 0) {
                SplitTimes[NextSplitIndex] = LapTimes[s0->steering94.count22 - 1];
            }
        }
    }
    if (s0->steering94.count22 * TrackLength <= arg0->second + arg0->first) {
        if (s0->steering94.count22 <= NumLaps) {
            s0->steering94.count22 = (u16)s0->steering94.count22 + 1;
            if (s0->steering94.count22 == 1) {
                SplitDeltaTimer = 0;
            } else {
                SplitDeltaTimer = 0x3C;
            }
            ShownSplitTime = SplitTimes[NextSplitIndex];
            LatchedBestSplitTime = BestSplitTimes[NextSplitIndex];
            SplitDeltaIndex = NextSplitIndex;
            NextSplitIndex = 0;
            LapAnnouncePending = 0;
            if (BestLapTime > LapTimes[s0->steering94.count22 - 2]) {
                if (s0->steering94.count22 != 1) {
                    SoundQueueEnqueue(9, 0);
                    BestLapTime = LapTimes[s0->steering94.count22 - 2];
                    s0->steering94.finish20 = (u16)s0->steering94.count22 - 2;
                    if (RaceType == 0) {
                        BestSplitTimes[0] = SplitTimes[0];
                        BestSplitTimes[1] = SplitTimes[1];
                        BestSplitTimes[2] = SplitTimes[2];
                    }
                }
            }
            if ((u16)(CourseIndex - 6) < 2U) {
                target = NumLaps - 1;
            } else {
                target = NumLaps;
            }
            if (s0->steering94.count22 == target) {
                SaveCarSnapshots(0);
            } else if (s0->steering94.count22 > NumLaps) {
                if (RaceState < 4) {
                    LapAnnouncePending = -1;
                    RaceState = 4;
                    MenuFadeTimer = 0x4B;
                    SoundSetMasterVolume(0, 0x47);
                    msg = 0x13;
                    if (RaceType == 1) {
                        /* ASM_BARRIER: clobber msg on entry to mode-1 branch so CSE retains msg = 0x13 and frees $a0 for phase17 */
                        __asm__ volatile("" : "=r"(msg));
                        if (arg0->half1EE <= GpQualifyRankLimits[SelectionState.phase17 / 2]) {
                            msg = 0x13;
                        } else {
                            if (SelectionState.retries08[SelectionState.phase17] > 0) {
                                msg = 0x14;
                            } else {
                                msg = -1;
                            }
                        }
                    }
                    if (msg >= 0) {
                        SoundQueueEnqueue(msg, 1);
                    }
                    if (arg0->half1EE < 8 && RaceType < 2) {
                        msg = 0;
                        TotalRaceTime = 0;
                        if (NumLaps > 0) {
                            do {
                                TotalRaceTime += LapTimes[msg];
                                msg++;
                            } while (msg < NumLaps);
                        }
                        if (TotalRaceTime > 0x927BE) {
                            TotalRaceTime = 0x927BF;
                        }
                        if (RaceType == 0) {
                            if (CourseBestTotal[ReverseCourse][CourseIndex] > TotalRaceTime) {
                                CourseBestTotal[ReverseCourse][CourseIndex] = TotalRaceTime;
                            }
                            if (CourseBestLap[ReverseCourse][CourseIndex] > BestLapTime) {
                                CourseBestLap[ReverseCourse][CourseIndex] = BestLapTime;
                            }
                            CourseBestLapTimes[ReverseCourse][CourseIndex][0] = BestSplitTimes[0];
                            CourseBestLapTimes[ReverseCourse][CourseIndex][1] = BestSplitTimes[1];
                            CourseBestLapTimes[ReverseCourse][CourseIndex][2] = BestSplitTimes[2];
                        } else if (RaceType == 1) {
                            if (CourseBestTotalTimes[CourseIndex] > TotalRaceTime) {
                                CourseBestTotalTimes[CourseIndex] = TotalRaceTime;
                            }
                            if (CourseBestLapRecords[CourseIndex] > BestLapTime) {
                                CourseBestLapRecords[CourseIndex] = BestLapTime;
                            }
                        }
                    }
                }
            }
        } else {
            goto check_split;
        }
    } else {
check_split:
        if (SplitDistances[NextSplitIndex] + TrackLength * (s0->steering94.count22 - 1) <= arg0->second + arg0->first) {
            if (RaceType == 0) {
                SplitDeltaTimer = 0x3C;
                SplitDeltaIndex = NextSplitIndex;
                ShownSplitTime = SplitTimes[NextSplitIndex];
                LatchedBestSplitTime = BestSplitTimes[NextSplitIndex];
                NextSplitIndex += 1;
            }
        }
    }
    if (LapAnnouncePending == 0) {
        msg = 9 - (NumLaps - s0->steering94.count22);
        if (msg == 9) {
            msg = 0xF;
        }
        if (msg >= 6) {
            SoundQueueEnqueue(msg, 0);
        }
        LapAnnouncePending = -1;
    }
}

void UpdateRaceRanking(void)
{
  int i;
  int j;
  if (RaceState < 4)
  {
    for (i = 0; i < NumCars; i++)
    {
      W10_Q *q = RankTable[i];
      W10_O **o = &CarTable[q->idx];
      q->key = (*o)->a + (*o)->b;
      (*o)->n = i;
    }
    for (i = 0; i < NumCars - 1; i++)
    {
      for (j = NumCars - 1; i < j; j--)
      {
        W10_Q *a2 = RankTable[j - 1];
        W10_Q *a1 = RankTable[j];
        int a0 = a2->key;
        int v1 = a1->key;
        if (a0 < v1 || (a0 == v1 && a1->idx == 0))
        {
          RankTable[j] = a2;
          RankTable[j - 1] = a1;
        }
      }
    }
  }
}

void SetupRaceAndScene(void) {
    SetupRace();
    SceneId = GameTypeSceneIds[RaceType];
}
