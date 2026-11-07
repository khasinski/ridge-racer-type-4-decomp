#include "rr/overlay667_init.h"
#ifndef RR_GPU_PROJECTION_SCRATCH
#define RR_GPU_PROJECTION_SCRATCH ((RrProjectionScratchPrefix *)0x1F800000)
#endif

void serial_link_func_80117EF8(void)
{
    s32 choice;
    ModeFrame=0;
    ScreenFrame=0;
    MenuCursor=0;
    MenuItemCount=3;
    LinkResultsExitFlag=0;
    choice=DecodeCourseIndex(CourseMenuSelection);
    CourseIndex=choice;
    ResetScreenCounters();
    RR_GPU_PROJECTION_SCRATCH->word18=512;
    RR_GPU_PROJECTION_SCRATCH->word0C=-416;
    RR_GPU_PROJECTION_SCRATCH->word1C=0;
    RR_GPU_PROJECTION_SCRATCH->word20=0;
    RR_GPU_PROJECTION_SCRATCH->word08=0;
    RR_GPU_PROJECTION_SCRATCH->word10=0;
    ComCars.words20[0]=0;
    ComCars.words20[1]=0;
    ComCars.words20[2]=0;
    SelectionPlayer=0;
    CourseMenuPhase=0;
    LoadCursor=MenuArenas.second;
}
