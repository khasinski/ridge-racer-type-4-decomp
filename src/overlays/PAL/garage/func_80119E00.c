#include "rr/overlay662_palette_controller.h"

void garage_update_color_select(void) {
    s32 badgePhase;
    s32 ready;
    void *palette;
    s32 paletteIndex;
    s32 index;
    s8 highlight;
    u16 previous;
    s32 rowPhase;
    s16 key;
    RrUiRow24 *acceptRow;
    RrUiRow24 *closingRow;
    RrUiRow24 *cancelRow;

    ModeFrame = (u32)ModeFrame + 1;
    ScreenFrame = (u32)ScreenFrame + 1;
    switch ((u16)ModeSubState) {
    case 0:
        if (garage_returned_from_logo_editor == 0) SetMenuFrameOptions(1,1,1);
        else SetMenuFrameOptions(1,1,0);
        previous = garage_clut_list_index;
        if (garage_image_fade_phase >= 0x11) {
            StepMenuValue(1, 2, (char *)&garage_clut_list_index, 0x19, 0);
        }
        if ((s16) previous != (s16) garage_clut_list_index) {
            key = (s8)garage_car_color_map[(s16)garage_clut_list_index];
            garage_clut_choice = key;
            if (key == 0) {
                palette = DefaultCarClut;
                paletteIndex = 0;
            } else {
                palette = CarClutData;
                paletteIndex = key - 1;
            }
            UploadCarClut(palette, paletteIndex, 0);
            garage_image_fade_phase = 0;
        }
        UiRows[(s16)MenuCursor].cells[0] = 0;
        StepMenuValue(0, 2, (char *)&MenuCursor, MenuItemCount, 0);
        highlight = 2;
        if (garage_clut_choice >= 0x19) {
            UiRows->half20 = 1;
            if ((s16) MenuCursor == 0) {
                highlight = 4;
            }
        } else {
            UiRows->half20 = 0;
        }
        UiRows[(s16)MenuCursor].cells[0] = highlight;
        rowPhase = UiRows->byte14;
        if (rowPhase == 2) {
            badgePhase = BadgeRows->state10;
            if (badgePhase == 3) {
                if (PlayerControllers[0].a != 0) {
                    UiRows[(s16)MenuCursor].cells[0] = 1;
                    if ((s16) MenuCursor == 0) {
                        if (garage_clut_choice < 0x19) {
                            PlaySfx(1);
                            index = 0;
                            garage_color_preview_state = (s16) rowPhase;
                            garage_image_fade_phase = 0;
                            ModeSubState = (u16) badgePhase;
                            do {
                                acceptRow = (RrUiRow24 *)(index * sizeof(RrUiRow24) + (unsigned long)UiRows);
                                if (acceptRow->byte14 == 2) {
                                    acceptRow->byte14 = 3U;
                                }
                                index += 1;
                            } while (index < 2);
                            BadgeRows->state10 = 4;
                        } else {
                            PlaySfx(4);
                        }
                    } else if ((s16) MenuCursor == 1) {
                        PlaySfx(1);
                        index = 0;
                        garage_image_fade_phase = 0;
                        garage_color_preview_state = (s16) rowPhase;
                        BadgeRows->state10 = 4;
                        do {
                            closingRow = (RrUiRow24 *)(index * sizeof(RrUiRow24) + (unsigned long)UiRows);
                            if (closingRow->byte14 == 2) {
                                closingRow->byte14 = 3U;
                            }
                            index += 1;
                        } while (index < 2);
                        goto closeMenu;
                    }
                } else if (PlayerControllers[0].b != 0) {
                    if (garage_returned_from_logo_editor == 0) {
                        PlaySfx(2);
                        index = 0;
                        garage_color_preview_state = (s16) rowPhase;
                        garage_image_fade_phase = 0;
                        BadgeRows->state10 = 4;
                        do {
                            cancelRow = (RrUiRow24 *)(index * sizeof(RrUiRow24) + (unsigned long)UiRows);
                            if (cancelRow->byte14 == 2) {
                                cancelRow->byte14 = 3U;
                            }
                            index += 1;
                        } while (index < 2);
closeMenu:
                        ModeSubState = 4;
                    }
                }
            }
        }
        goto drawMenu;
    case 3:
        if (garage_returned_from_logo_editor == 0) SetMenuFrameOptions(0,1,1);
        else SetMenuFrameOptions(0,1,0);
        if ((UiRows->byte14 == 5) && (BadgeRows->state10 == 11)) {
            PlayerCourseChoices[0].second = (u16)garage_clut_choice+1;
            if (garage_returned_from_logo_editor == 0) goto enterPalette;
            goto publishScene;
        }
        goto drawMenu;
    case 4:
        if (garage_returned_from_logo_editor == 0) SetMenuFrameOptions(0,1,1);
        else SetMenuFrameOptions(0,1,0);
        if ((UiRows->byte14 == 5) && (BadgeRows->state10 == 11)) {
            if (garage_returned_from_logo_editor != 0) goto publishScene;
enterPalette:
            garage_init_car_select();
            break;
publishScene:
            ModeGroup=3;ModeId=1;ModeSubState=0;
            break;
        }
        goto drawMenu;
    default:
drawMenu:
        if (garage_color_preview_state == 0) {
            ready = UpdateRect16Row(0);
            if (ready == 1) {
                garage_color_preview_state = ready;
            }
        } else {
            garage_draw_color_preview_image(garage_color_preview_state - 1);
            DrawMenuStepArrows(0xA0, 0xC0, 0x30, 0, (s32) (s16) garage_clut_list_index, 0, (s32) garage_clut_list_count);
        }
        index = 0;
        if (MenuItemCount > 0) {
            do {
                DrawUiRow(index);
                index += 1;
            } while (index < MenuItemCount);
        }
        UpdateBadgeRowAnim(0);
        break;
    }
    UpdateScreenFade(0x1004A);
}
