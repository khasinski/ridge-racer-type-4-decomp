#include "rr/overlay662_car_layout.h"
void ovl662_func_8011512C(void) {
    s32 i;
    RrRaceVehicle *vehicle;
    RrProjectionScratchPrefix *scratch;
    /* Keep the per-vehicle rotation temporary in the call-result register. */
    register s32 rotation __asm__("$2");
    void *arena;
    ClearScreenOverlayState();
    ModeFrame=0;
    ScreenFrame=0;
    MenuCursor=0;
    MenuItemCount=4;
    ResetScreenCounters();
    MenuArenas.second=CdDirectoryBuffer+0x2D000;
    InitMenuArenas();
    LoadCursor=MenuArenas.second;
    InitUiRow(0,24,88,68,32,0,0,1,0,21);
    InitUiRow(1,24,128,68,32,0,0,1,0,102);
    InitUiRow(2,24,168,68,32,0,0,1,0,22);
    InitUiRow(3,24,208,68,32,0,0,1,0,85);
    SetupRenderer(5);
    ResetScreenCounters();
    InitPreviewImageRows();
    i=0;
    vehicle=PlayerCars;
    ovl662_D_801272D4=0;
    scratch=RR_PROJECTION_SCRATCH;
    scratch->word18=120;
    scratch->word08=-16;
    scratch->word0C=-248;
    scratch->word1C=0;
    scratch->word20=0;
    scratch->word10=-96;
    arena=MenuArenaBase;
    LoadCursor=arena;
    MenuArenas.first=arena;
    do {
        /* Keep one full vehicle cursor through both setup calls. */
        __asm__("" : "+r"(vehicle));
        ResetCarState(vehicle);
        vehicle->pos20[1]=10;
        vehicle->pos20[2]=308;
        rotation=2048;
        vehicle->rot54=rotation;
        /* Publish this vehicle before advancing the loop counter. */
        __asm__("" : "+r"(i) : : "memory");
        i++;
        vehicle++;
    } while(i<2);
    UnpackChoiceGlyph((s8*)SelectionRows.bytes,CourseChoiceTable[1][ovl662_D_801272E4].glyph);
    PlayerCourseChoices[0].first=CourseChoiceTable[1][ovl662_D_801272E4].first;
    PlayerCourseChoices[0].second=CourseChoiceTable[1][ovl662_D_801272E4].second;
    ovl662_D_801272E0=CountChoiceGlyphs(CourseChoiceTable[1]);
    SetMenuFrameOptions(0,1,1);
}
