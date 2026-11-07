#include "rr/overlay660.h"

void ovl660_func_80114910(void) { SceneId = 0x131; }

void ovl660_func_80114920(void) {
    RrProjectionScratchPrefix *scratch;
    s32 selection;
    s16 *result;
    ModeFrame=0;
    ScreenFrame=0;
    MenuCursor=0;
    MenuItemCount=3;
    selection=DecodeCourseIndex(CourseMenuSelection);
    result=&CourseIndex;
    *result=selection;
    ResetScreenCounters();
    MenuArenas.second=CdDirectoryBuffer+0x2D000;
    InitMenuArenas();
    LoadCursor=MenuArenas.second;
    InitUiRow(0,12,64,68,32,0,0,1,0,83);
    InitUiRow(1,12,104,68,32,0,0,1,0,102);
    InitUiRow(2,12,144,68,32,0,0,1,0,85);
    InitUiRow(3,12,368,208,32,0,0,8,4,CourseMenuSelection&7);
    InitUiRow(4,24,88,68,32,0,0,1,0,84);
    InitUiRow(5,24,128,68,32,0,0,1,0,54);
    InitUiRow(6,24,168,68,32,0,0,1,0,85);
    InitUiRow(7,24,88,68,32,0,0,1,0,84);
    InitUiRow(8,24,128,68,32,0,0,1,0,53);
    InitUiRow(9,24,168,68,32,0,0,1,0,85);
    InitUiRow(10,116,88,28,32,0,0,1,0,55);
    InitUiRow(11,116,128,28,32,0,0,1,0,56);
    scratch=RR_PROJECTION_SCRATCH;
    scratch->word18=512;
    scratch->word0C=-416;
    scratch->word1C=0;
    scratch->word20=0;
    scratch->word08=0;
    scratch->word10=0;
    PlayerCars[0].pos20[0]=0;
    PlayerCars[0].pos20[1]=0;
    PlayerCars[0].pos20[2]=0;
}

void ovl660_func_80114C40(void)
{
    s32 val;

    SetDispMask(0);
    ClearScreenOverlayState();
    ovl660_preview_bank = 0;
    ovl660_preview_selection = 0;
    ovl660_preview_confirmation = 0;
    CopyStringClearTail(ovl660_record_name, SaveProgress.prefix.progress.prefix00.fields.name);
    SetupDrawDispEnvs(0, 0, 0, 2);
    val = SaveProgress.prefix.progress.prefix00.fields.configuration0C;
    CourseMenuSelection = 0;
    ovl660_available_course_count = 4;
    if (val >= 16) {
        ovl660_available_course_count = 8;
    }
    if (val & 0x100) {
        ovl660_available_course_count = 16;
    }
    ovl660_func_80114920();
    UnpackChoiceGlyph((s8*)&SelectionRows,CourseChoiceTable[0][0].glyph);
    ModeGroup = 1;
    ModeId = 50;
    D_800F4D8A = 0;
    DebugPrintStub(ovl660_title);
    SeqStartTrack(0);
}

void ovl660_func_80114D38(void) {
    s32 loopCount;
    s16 cancelTimer;
    s16 settingsTimer;
    s32 row;
    s32 previewIndex;
    s32 settingsIndex;
    s32 cancelIndex;
    s32 backIndex;
    s32 cell;
    u16 previousCourse;
    u8 activeState;
    RrUiRow24 *preview;
    RrUiRow24 *settings;
    RrUiRow24 *cancel;
    RrUiRow24 *back;

    ModeFrame = (s32)((u32)ModeFrame + 1U);
    ScreenFrame = (s32)((u32)ScreenFrame + 1U);
    if (LoadTrackNodesStep() == 1) {
        switch (D_800F4D8A) {
        case 0:
            SetMenuFrameOptions(1, 1, 1);
            previousCourse = CourseMenuSelection;
            StepMenuValue(3, 2, (char *)&CourseMenuSelection, ovl660_available_course_count, 0);
            if ((s16) previousCourse != (s16) CourseMenuSelection) {
                ResetScreenCounters();
            } else {
                DrawCourseMapPreview((s16) previousCourse, 0);
            }
            CourseIndex = DecodeCourseIndex((s16) CourseMenuSelection);
            UiRows[MenuCursor].cells[0] = 0;
            StepMenuValue(0, 2, (char *)&MenuCursor, (s32) MenuItemCount, 0);
            UiRows[MenuCursor].cells[0] = 2;
            cell = 0;
            if (UiRows[3].byte02 != 0) {
                do {
                    if (cell == (CourseMenuSelection & 7)) {
                        UiRows[3].cells[(s32)cell] = (s8) (((u8 *)&ReverseCourse)[0] + 2);
                        UiRows[3].byte16 = cell;
                    } else if (cell >= ovl660_available_course_count) {
                        UiRows[3].cells[(s32)cell] = 1;
                    } else {
                        UiRows[3].cells[(s32)cell] = (u8) ((u8 *)&ReverseCourse)[0];
                    }
                    cell += 1;
                } while (cell < (s32) UiRows[3].byte02);
            }
            if (MenuConfirmPressed != 0) {
                activeState = UiRows[0].byte14;
                if (activeState == 2) {
                    PlaySfx(1);
                    SetMenuFrameOptions(0, 1, 1);
                    if ((s16) MenuCursor == 0) {
                        row = 0;
                        previewIndex = 0;
                        UiRows[0].cells[0] = 1;
                        do {
                            preview = (RrUiRow24 *)((unsigned long)(previewIndex*sizeof(RrUiRow24))+(unsigned long)UiRows);
                            if (preview->byte14 == 2) {
                                preview->byte14 = 3U;
                            }
                            row += 1;
                            previewIndex++;
                        } while (row < 4);
                        D_800F4D8A = 3;
                    } else if ((s16) MenuCursor == 1) {
                        row = 0;
                        if (UiRowCount > 0) {
                            settingsIndex = 0;
                            do {
                                settings = (RrUiRow24 *)((unsigned long)(settingsIndex*sizeof(RrUiRow24))+(unsigned long)UiRows);
                                if (settings->byte14 == 2) {
                                    settings->byte14 = 3U;
                                }
                                loopCount=UiRowCount;
                                row += 1;
                                settingsIndex++;
                            } while (row < loopCount);
                        }
                        MenuFadeTimer = 0x40;
                        D_800F4D8A = 0xA;
                    } else if ((s16) MenuCursor == activeState) {
                        row = 0;
                        if (UiRowCount > 0) {
                            cancelIndex = 0;
                            do {
                                cancel = (RrUiRow24 *)((unsigned long)(cancelIndex*sizeof(RrUiRow24))+(unsigned long)UiRows);
                                if (cancel->byte14 == 2) {
                                    cancel->byte14 = 3U;
                                }
                                loopCount=UiRowCount;
                                row += 1;
                                cancelIndex++;
                            } while (row < loopCount);
                        }
                        MenuFadeTimer = 0x40;
                        D_800F4D8A = 8;
                        SeqFadeVolume(0, 0x3C);
                    }
                } else {
                    goto check_back;
                }
            } else {
check_back:
                if ((D_800F3A64 != 0) && (UiRows[0].byte14 == 2)) {
                    PlaySfx(2);
                    SetMenuFrameOptions(0, 1, 1);
                    row = 0;
                    if (UiRowCount > 0) {
                        backIndex = 0;
                        do {
                            back = (RrUiRow24 *)((unsigned long)(backIndex*sizeof(RrUiRow24))+(unsigned long)UiRows);
                            if (back->byte14 == 2) {
                                back->byte14 = 3U;
                            }
                                loopCount=UiRowCount;
                                row += 1;
                            backIndex++;
                        } while (row < loopCount);
                    }
                    MenuFadeTimer = 0x40;
                    D_800F4D8A = 8;
                }
            default:;
            }
            goto draw_course;
        case 2:
            if (MenuConfirmPressed != 0) {
                if (UiRows[0].byte14 == 5) {
                    D_800F4D8A = 4;
                    InitUiRow(3, 0x78, 0xDC, 0x44, 0x20, 0, 0, 1, 0, 0, 0, 0);
                    InitUiRow(4, 0x78, 0x104, 0x44, 0x20, 0, 0, 1, 0, 0, 0, 0);
                }
            }
            goto draw_course;
        case 4:
            if (MenuConfirmPressed != 0) {
                if (UiRows[3].byte14 == 2) {
                    D_800F4D8A = 5;
                    ImageRows[1] = 3;
                    UiRows[3].byte14 = 3U;
                    UiRows[4].byte14 = 3;
                }
            } else {
            case 5:;
            }
            if (UiRows[3].byte14 == 5) {
                UiRows[0].byte14 = 0U;
                UiRows[1].byte14 = 0;
                UiRows[2].byte14 = 0;
                D_800F4D8A = 0;
            }
            goto draw_course;
        case 3:
            if (UiRows[0].byte14 == 5) {
                ovl660_func_801163C4();
                ModeGroup = 1;
                ModeId = 0x33;
                D_800F4D8A=0;
                goto draw_rows;
            }
            break;
        case 8:
            cancelTimer = (u16)MenuFadeTimer - 1;
            MenuFadeTimer = (u16) cancelTimer;
            AddScreenFadeTiles(((RrFrameOrderingPrefix *)(void *)CurrentDrawBuf)->ordering70, (0x40 - cancelTimer) * 4, 0x45);
            if ((s16) MenuFadeTimer <= 0) {
                SeqMuteVolume();
                ModeGroup = 1;
                ModeId = 3;
                D_800F4D8A=0;
                goto draw_rows;
            }
            break;
        case 10:
            settingsTimer = (u16)MenuFadeTimer - 1;
            MenuFadeTimer = (u16) settingsTimer;
            AddScreenFadeTiles(((RrFrameOrderingPrefix *)(void *)CurrentDrawBuf)->ordering70, (0x40 - settingsTimer) * 4, 0x45);
            if ((s16) MenuFadeTimer <= 0) {
                MenuResultCode = 1;
                D_800F3A1E = 0x132;
                ModeGroup = 1;
                ModeId = 0x47;
                D_800F4D8A=0;
                goto draw_rows;
            }
            break;
        }
    }
    else goto draw_course;
    goto draw_rows;

draw_course:
    DrawCourseInfoPanel((s16)CourseMenuSelection,0);
draw_rows:
    DrawUiRow(0);
    DrawUiRow(1);
    DrawUiRow(2);
    DrawUiRow(3);
    if (UiRows[0].byte14 == 2) {
        DrawMenuStepArrows(0xCE, 0xD0, 0x5E, 0, (s32) (s16) CourseMenuSelection, -1, (s32)((u32)ovl660_available_course_count + 1U));
    }
    UpdateScreenFade(0x10026);
}
