#include "rr/overlay668_playback_init.h"

void movie_playback_func_801149E8(void)
{
    RrStrCdFileInfo file;
    u32 sectors;
    RrStrPlaybackState *state;

    SetDispMask(0);
    SetupDrawDispEnvs(0, 0, 0, 5);
    state = MenuArenaBase;
    LoadCursor = state;
    RaceContext.activeB8 = state;
    LoadCursor = (u8 *)state + sizeof(*state);
    PsyqBzero(state, sizeof(*state));
    ResetGraph(3);
    CdSync(0, 0);
    CdControl(9, 0, 0);
    RaceContext.phaseB4 = 0;
    RaceContext.stopD0 = 0;
    movie_playback_func_80114910(RaceContext.resourceB0, &file, &sectors);
    ModeGroup = 2;
    ModeId = 4;
    ModeSubState = 0;
}
