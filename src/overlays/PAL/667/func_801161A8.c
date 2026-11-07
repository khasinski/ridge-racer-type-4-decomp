#include "rr/selection_initializer.h"
void ovl667_func_801161A8(s32 arg0) {
    RrRaceVehicle *var_s0;
    s16 *temp_s0;
    s16 *temp_t0_2;
    s32 temp_a0;
    s32 temp_a2;
    s32 temp_a2_2;
    s32 var_s1;
    s32 placeY,placeZ;
    /* ASM_BARRIER: preserve the original rotation publication register. */
    register s32 rotation __asm__("$2");
    RrMenuGridPoint *temp_a3_2;

    ModeFrame = 0;
    ScreenFrame = 0;
    MenuCursor = 0;
    MenuItemCount = 3;
    ResetScreenCounters();
    MenuArenas.second = CdDirectoryBuffer + 0x2D000;
    InitMenuArenas();
    SelectionPlayer = arg0;
    LoadCursor = MenuArenas.second;
    InitUiRow(0, 0x18, 0x58, 0x44, 0x20, 0, 0, 1, 0, 0x53);
    /* ASM_BARRIER: keep the published arena cell across the first UI service.
     * Read/write memory annotation retains its value and original address lifetime. */
    __asm__("" : "+m"(LoadCursor));
    InitUiRow(1, 0x18, 0x80, 0x44, 0x20, 0, 0, 1, 0, 0x36);
    InitUiRow(2, 0x18, 0xA8, 0x44, 0x20, 0, 0, 1, 0, 0x55);
    InitUiRow(3, 0x18, 0x58, 0x44, 0x20, 0, 0, 1, 0, 0x53);
    InitUiRow(4, 0x18, 0x80, 0x44, 0x20, 0, 0, 1, 0, 0x65);
    InitUiRow(5, 0x18, 0xA8, 0x44, 0x20, 0, 0, 1, 0, 0x35);
    InitUiRow(6, 0x18, 0xD0, 0x44, 0x20, 0, 0, 1, 0, 0x55);
    InitUiRow(7, 0x74, 0x58, 0x1C, 0x20, 0, 0, 1, 0, 0x37);
    InitUiRow(8, 0x74, 0x80, 0x1C, 0x20, 0, 0, 1, 0, 0x38);
    InitUiRow(9, 0x18, 0x58, 0x44, 0x20, 0, 0, 1, 0, 0x53);
    InitUiRow(0xA, 0x18, 0x80, 0x44, 0x20, 0, 0, 1, 0, 0x36);
    InitUiRow(0xB, 0x18, 0xA8, 0x44, 0x20, 0, 0, 1, 0, 0x55);
    InitUiRow(0xC, 0x18, 0x58, 0x44, 0x20, 0, 0, 1, 0, 0x53);
    InitUiRow(0xD, 0x18, 0x80, 0x44, 0x20, 0, 0, 1, 0, 0x65);
    InitUiRow(0xE, 0x18, 0xA8, 0x44, 0x20, 0, 0, 1, 0, 0x35);
    InitUiRow(0xF, 0x18, 0xD0, 0x44, 0x20, 0, 0, 1, 0, 0x55);
    InitUiRow(0x10, 0x74, 0x58, 0x1C, 0x20, 0, 0, 1, 0, 0x37);
    InitUiRow(0x11, 0x74, 0x80, 0x1C, 0x20, 0, 0, 1, 0, 0x38);
    InitUiRow(0x12, 0xC, 0x40, 0x44, 0x20, 0, 0, 1, 0, 0x54);
    InitUiRow(0x13, 0xC, 0x68, 0x44, 0x20, 0, 0, 1, 0, 3);
    InitUiRow(0x14, 0xC, 0x90, 0x44, 0x20, 0, 0, 1, 0, 0x55);
    InitUiRow(0x15, 0xC, 0x170, 0xD0, 0x20, 0, 0, 8, 4, (s32) CourseMenuSelection);
    InitUiRow(0x17, 0x68, 0x68, 0x44, 0x20, 0, 0, 1, 5, 3);
    InitUiRow(0x18, 0x68, 0x40, 0x44, 0x20, 0, 0, 0, 0x7E, 0);
    SetupRenderer(5);
    MenuCursor = 0;
    MenuItemCount = 3;
    ResetScreenCounters();
    InitPreviewImageRows();
    CourseMenuPhase = 0;
    LoadCursor = MenuArenaBase;
    if (!(SelectionPlayer & 1)) {
        MenuArenas.first = MenuArenaBase;
    } else {
        LoadCursor = MenuArenas.first;
    }
    var_s1 = 0;
    placeY=10;placeZ=308;
    var_s0 = PlayerCars;
    RR_PROJECTION_SCRATCH->word18 = 0x78;
    RR_PROJECTION_SCRATCH->word08 = -0x10;
    RR_PROJECTION_SCRATCH->word0C = -0xF8;
    RR_PROJECTION_SCRATCH->word1C = 0;
    RR_PROJECTION_SCRATCH->word20 = 0;
    RR_PROJECTION_SCRATCH->word10 = -0x60;
    do {
        /* ASM_BARRIER: retain the current vehicle across its reset callback. */
        __asm__("" : "+r"(var_s0));
        ResetCarState(var_s0);
        var_s0->pos20[1] = placeY;
        var_s0->pos20[2] = placeZ;
        rotation=0x800;
        var_s0->rot54 = rotation;
        /* ASM_BARRIER: publish vehicle fields before advancing the loop phase. */
        __asm__("" : : "r"(var_s1) : "memory");
        var_s1 += 1;
        var_s0++;
    } while (var_s1 < 2);
    ChoiceGlyphCounts[0][0] = CountChoiceGlyphs(CourseMenuChoices[0][0]);
    ChoiceGlyphCounts[0][1] = CountChoiceGlyphs(CourseMenuChoices[0][1]);
    ChoiceGlyphCounts[1][0] = CountChoiceGlyphs(CourseMenuChoices[1][0]);
    ChoiceGlyphCounts[1][1] = CountChoiceGlyphs(CourseMenuChoices[1][1]);
    temp_a0 = arg0 & 1;
    if (ChoiceGlyphCounts[temp_a0][1] == 0) {
        CourseMenuPage[temp_a0] = 0;
    }
    temp_a2 = SelectionPlayer & 1;
    UnpackChoiceGlyph((s8 *)&SelectionRows + SelectionPlayer * 4,
        CourseMenuChoices[temp_a2][CourseMenuPage[temp_a2]][CourseMenuCursor[temp_a2]].glyph);
    temp_a2_2 = SelectionPlayer & 1;
    temp_s0 = &CourseMenuPage[temp_a2_2];
    temp_t0_2 = &CourseMenuCursor[temp_a2_2];
    temp_a3_2 = &PlayerCourseChoices[SelectionPlayer];
    temp_a3_2->first = CourseMenuChoices[temp_a2_2][*temp_s0][*temp_t0_2].first;
    temp_a3_2->second = CourseMenuChoices[temp_a2_2][*temp_s0][*temp_t0_2].second;
}
