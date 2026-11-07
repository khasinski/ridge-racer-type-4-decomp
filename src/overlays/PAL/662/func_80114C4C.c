#include "rr/overlay662_progress_controller.h"
#include "rr/course_choice_types.h"

extern RrCourseChoice4 CourseChoiceTable[2][9];
extern s16 MenuCursor;
extern u8 D_800F4368[];
void SetModelTexturePage(s32 mode);
void StepMenuValue(s32 mode, s32 width, char *selection, s32 limit, s32 flags);
void SeqFadeVolume(s32, s32);
void SeqMuteVolume(void);
void InitTimTemplate(void *);
void UploadCarClut(void *, s32, s32);

void ovl662_func_80114C4C(void)
{
    s32 i;
    s32 state;
    s32 unused[2]; /* never referenced; original frame reserves 8 local bytes */

    ModeFrame++;
    switch ((u16)ModeSubState) {
    case 0:
        state = UiRows->byte14;
        if (state != 2) {
            break;
        }
        SetMenuFrameOptions(1, 1, 1);
        UiRows[MenuCursor].cells[0] = 0;
        StepMenuValue(0, 2, (char *)&MenuCursor, 3, 0);
        UiRows[MenuCursor].cells[0] = state;
        if (PlayerControllers[0].a) {
            SetMenuFrameOptions(0, 1, 1);
            UiRows[MenuCursor].cells[0] = 1;
            if (MenuCursor == 0) {
                if (CourseChoiceTable[1][0].glyph != -1) {
                    SetModelTexturePage(-1);
                    PlaySfx(1);
                    ModeSubState = 3;
                    MenuFadeTimer = 64;
                    for (i = 0; i < 3; i++) {
                        if (UiRows[i].byte14 == 2) {
                            UiRows[i].byte14 = 7;
                        }
                    }
                } else {
                    PlaySfx(4);
                }
            } else if (MenuCursor == 1) {
                PlaySfx(1);
                SetModelTexturePage(-1);
                ModeSubState = 4;
                for (i = 0; i < 3; i++) {
                    if (UiRows[i].byte14 == 2) {
                        UiRows[i].byte14 = 7;
                    }
                }
                MenuFadeTimer = 64;
            } else if (MenuCursor == state) {
                PlaySfx(1);
                SetModelTexturePage(-1);
                ModeSubState = 8;
                for (i = 0; i < 3; i++) {
                    if (UiRows[i].byte14 == 2) {
                        UiRows[i].byte14 = 3;
                    }
                }
                MenuFadeTimer = 64;
                SeqFadeVolume(0, 60);
            }
        } else if (PlayerControllers[0].b) {
            PlaySfx(2);
            SetModelTexturePage(-1);
            ModeSubState = 8;
            SetMenuFrameOptions(0, 1, 1);
            for (i = 0; i < 3; i++) {
                if (UiRows[i].byte14 == 2) {
                    UiRows[i].byte14 = 3;
                }
            }
            MenuFadeTimer = 64;
            SeqFadeVolume(0, 60);
        }
        break;
    case 3:
        MenuFadeTimer--;
        AddScreenFadeTiles(((RrFrameOrderingPrefix *)CurrentDrawBuf)->ordering70,
                      (64 - MenuFadeTimer) * 4, 69);
        if (MenuFadeTimer <= 0) {
            ModeGroup = 1;
            ModeId = 63;
            ModeSubState = 0;
        }
        break;
    case 4:
        MenuFadeTimer--;
        AddScreenFadeTiles(((RrFrameOrderingPrefix *)CurrentDrawBuf)->ordering70,
                      (64 - MenuFadeTimer) * 4, 69);
        if (MenuFadeTimer <= 0) {
            InitTimTemplate(D_800F4368);
            UploadCarClut(D_800F4368, 0, 0);
            ModeGroup = 3;
            ModeId = 1;
            ModeSubState = 0;
        }
        break;
    case 8:
        MenuFadeTimer--;
        AddScreenFadeTiles(((RrFrameOrderingPrefix *)CurrentDrawBuf)->ordering70,
                      (64 - MenuFadeTimer) * 4, 69);
        if (MenuFadeTimer <= 0) {
            ModeGroup = 1;
            ModeId = 3;
            ModeSubState = 0;
            SeqMuteVolume();
        }
        break;
    }
    for (i = 0; i < 3; i++) {
        DrawUiRow(i);
    }
    UpdateScreenFade(0x10043);
}
