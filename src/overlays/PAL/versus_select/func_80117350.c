#include "rr/overlay661_init.h"
#ifndef RR_GPU_PROJECTION_SCRATCH
#define RR_GPU_PROJECTION_SCRATCH ((RrProjectionScratchPrefix *)0x1F800000)
#endif

void versus_select_init_course_select(void)
{
    ModeFrame=0;ScreenFrame=0;MenuCursor=0;MenuItemCount=4;
    PlayerCourseChoices[2].first=(PsyqRand()&3)+10;
    PlayerCourseChoices[3].first=(PsyqRand()&3)+12;
    CourseIndex=DecodeCourseIndex(CourseMenuSelection);
    ResetScreenCounters();
    RR_GPU_PROJECTION_SCRATCH->word18=512;
    RR_GPU_PROJECTION_SCRATCH->word0C=-416;
    RR_GPU_PROJECTION_SCRATCH->word1C=0;
    RR_GPU_PROJECTION_SCRATCH->word20=0;
    RR_GPU_PROJECTION_SCRATCH->word08=0;
    RR_GPU_PROJECTION_SCRATCH->word10=0;
    ComCars.words20[0]=0;ComCars.words20[1]=0;ComCars.words20[2]=0;
    SelectionPlayer=0;
    LoadCursor=MenuArenas.second;
    SetMenuFrameOptions(0,1,1);
}
