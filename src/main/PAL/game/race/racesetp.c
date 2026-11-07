#include "rr/race_setup.h"

void SetupRace(void)
{
    u8 *vectors;
    register u8 *countRoot __asm__("$4");
    u8 *otherRoot,*byteRoot;
    s32 courseIndex;
    s32 minusOne;
    register s32 offset0 __asm__("$2");
    s32 offset1;

    RrRaceVehicle *vehicle;
    s32 *rank;
    s32 *selectedCount;
    s32 *result;
    s32 third;
    s32 vehicleIndex;
    register s32 resetIndex __asm__("$17");
    u8 *participant;
    u32 course;
    s32 vehicleFlags;
    s32 *vector;
    if (RaceType == 4)
    {
        serial_link_open_sio();
        serial_link_init_connection(0x7E900);
    }
    DrawSync(0);
    ApplyOptionsPadButtonMaps();
    InitRaceCars(RaceType);
    if (((RaceType < 2) || (RaceType == 6)) || ((RaceType == 4) && (PlayerCount == 2)))
    {
        TrackEventSfxEnabled = 1;
    }
    else
    {
        TrackEventSfxEnabled = 0;
    }
    if (RaceType != 5)
    {
        vehicleIndex = 0;
        vehicle = PlayerCars;
        do
        {
            /* Keep the complete owner address through this traversal. */
            __asm__("" : "+r"(vehicle));
            vehicleFlags = (s16) vehicle->flags1E4;

            if (vehicleFlags != 0)
            {
                PadRumbleStart(vehicleIndex);
                vehicle->flags1E4 = (s16) (((u16) vehicle->flags1E4) ^ 0x100);
            }
            vehicleIndex += 1;
            vehicle++;
        }
        while (vehicleIndex < 2);
    }
    InitViewState();
    SetupRenderer(5);
    UnusedSplitWord = 0;
    NextSplitIndex = 2;
    SplitTimes[1] = 0;
    SplitTimes[0] = 0;
    SplitTimes[2] = -1;
    SplitDistances[2] = (s32) TrackLength;
    third = TrackLength / 3;
    SplitDistances[0] = third;
    vectors = (u8 *) (&CourseBestLapTimes);
    SplitDistances[1] = (s32) (third * 2);
    vector = (s32 *) (((CourseIndex * 12) + (ReverseCourse * 96)) + vectors);
    BestSplitTimes[0] = (s32) vector[0];
    BestSplitTimes[1] = (s32) vector[1];
    RaceState = SavedRaceState;
    BestSplitTimes[2] = (s32) vector[2];
    if (RaceType == 0)
    {
        countRoot = (u8 *)&CourseBestLap;

        offset0 = CourseIndex * 4;
        offset1 = ReverseCourse * 32;
        offset0 += offset1;
        selectedCount = (s32 *)(offset0+countRoot);
        goto publish_count;
    }
    if (RaceType == 1)
    {
        otherRoot = (u8 *)&CourseBestLapRecords;
        /* Retain the full table address before forming its byte offset. */
        __asm__("" : : "r"(otherRoot));

        offset0 = CourseIndex * 4;
        selectedCount = (s32 *)(offset0+otherRoot);
    publish_count:
        BestLapTime = *selectedCount;

    }
    LapAnnouncePending = -1;

    if (RaceType >= 2)
    {
        resetIndex = 0;
        if (RaceType == 6)
        {
            goto publish_participants;
        }
    }
    else
    {
    publish_participants:
        courseIndex = CourseIndex;
        byteRoot = CourseLapCounts;

        if (RaceType == 1)
        {
            courseIndex += 8;
        }
        participant = courseIndex + byteRoot;
        NumLaps = (s32) (*participant);
        /* Initialized-value metadata retains the original store order. */
        __asm__("" : : "m"(NumLaps));
        resetIndex = 0;
    }
    minusOne = -1;

    rank = LapTimes;
    result = LapFrameCounts;
    do
    {
        *result = 0;
        *rank = minusOne;
        rank++;
        resetIndex += 1;
        result++;
    }
    while (resetIndex < 0xA);
    TotalRaceTime = -1;
    course = (u16) CourseIndex;
    if (((course - 2U) < 3U) || (((s16) course) == 7))
    {
        TrackLightFlagExtra = 1;
    }
    else
    {
        TrackLightFlagExtra = 0;
    }
    ResetRaceSceneFadeState();
    if ((RaceType == 0) || (RaceType == 5))
    {
        RearViewMirrorEnabled = 0;
        RearViewMirrorHidden = 0;
        DrawSubOtFlag = 0;
    }
    else
    {
        ResetRearViewMirror();
        RearViewMirrorEnabled = 1;
        RearViewMirrorHidden = 0;
        DrawSubOtFlag = 1;
    }
    BuildCameraMatrices();
    InitRaceHudPackets(RaceType);
    InitHudNumberTargets(RaceType);
    ScreenFrame = 0;
    ModeFrame = 0;
    MenuFadeTimer = 0;
    RacePaused = 0;
    PauseCursor = 0;
    RaceIntroWipeY = 0x80;
    RaceTimeRemaining = 0x3A98;
    if ((RaceType != 1) && (RaceType != 6))
    {
        RaceTimeRemaining = 0xAFC8;
    }
    VSyncThreshold = 0x180;
    ClearTrackObjectStates(0);
    EngineRevDisplay.first = 0;
    EngineRevDisplay.second = 0;
    EngineThrottleOn.first = 0;
    EngineThrottleOn.second = 0;
    RaceVoiceFlags = 0;
    StopAllSoundAndReset();
    SelectCourseBgmTrack();
    SetModelTexturePage(0xA);

}
