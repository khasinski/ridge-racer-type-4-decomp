#include "rr/overlay662_progress_controller.h"

void garage_update_collection_select(void)
{
    s32 ready, changed, state, i, flags;

    ModeFrame = (s32)((u32)ModeFrame + 1U);
    ScreenFrame = (s32)((u32)ScreenFrame + 1U);
    switch ((u16)ModeSubState) {
    case 0:
        ready = LoadGlyphRowsStep();
        if (ready == 1) {
            SetMenuFrameOptions(1, 1, 1);
            ModeSubState = ready;
        }
        flags = 0x10002;
        break;
    case 1:
        changed = garage_draw_select_arrows(garage_select_arrows_unused_arg, 0);
        state = BadgeRows->state10;
        if (state == 3) {
            if (changed) {
                garage_step_team_cursor();
            }
            if (PlayerControllers[0].a) {
                if ((s16)garage_team_car_counts[SelectionRows.bytes[0]] > 0) {
                    PlaySfx(1);
                    ModeFrame = 0;
                    ModeSubState = 2;
                    UiRows->byte14 = state;
                    BadgeRows->state10 = 4;
                } else {
                    PlaySfx(4);
                }
            } else if (PlayerControllers[0].b) {
                PlaySfx(2);
                ModeSubState = 8;
                MenuFadeTimer = 64;
                UiRows->byte14 = state;
                BadgeRows->state10 = 4;
            }
        }
        flags = 0x10002;
        break;
    case 2:
        if (BadgeRows->state10 == 11 && UiRows->byte14 == 5) {
            garage_init_maker_select(0);
            ModeSubState = 3;
        }
        flags = 0x10006;
        break;
    case 3:
        changed = garage_draw_select_arrows(garage_select_arrows_unused_arg, 1);
        state = BadgeRows->state10;
        if (state == 3) {
            if (changed) {
                garage_step_maker_cursor();
            }
            if (PlayerControllers[0].a) {
                if ((s16)garage_team_maker_car_counts[SelectionRows.bytes[0]][SelectionRows.bytes[1]] > 0) {
                    PlaySfx(1);
                    ModeFrame = 0;
                    ModeSubState = 4;
                    UiRows->byte14 = state;
                    BadgeRows->state10 = 4;
                } else {
                    PlaySfx(4);
                }
            } else if (PlayerControllers[0].b) {
                PlaySfx(2);
                garage_color_preview_state = 1;
                ModeSubState = 4;
                UiRows->byte14 = state;
                BadgeRows->state10 = 4;
            }
        }
        flags = 0x10006;
        break;
    case 4:
        if (BadgeRows->state10 == 11 && UiRows->byte14 == 5) {
            i = 0;
            if (garage_color_preview_state == 0) {
                garage_init_car_list();
                return;
            }
            for (; i < 4; i++) {
                GlyphRows[i].phase18 = (s32)((u32)GlyphRows[i].phase18 - 0x30000U);
            }
            garage_init_team_select();
            ModeSubState = 1;
            garage_color_preview_state = 0;
        }
        flags = 0x10006;
        break;
    case 8:
        MenuFadeTimer--;
        SetMenuFrameOptions(0, 1, 1);
        AddScreenFadeTiles(((RrFrameOrderingPrefix *)CurrentDrawBuf)->ordering70,
                     (64 - MenuFadeTimer) * 4, 69);
        if (MenuFadeTimer <= 0) {
            garage_init_car_info();
            ModeGroup = 1;
            ModeId = 64;
            ModeSubState = 0;
        }
        flags = 0x10002;
        break;
    }
    DrawUiRow(0);
    UpdateBadgeRowAnim(0);
    UpdateScreenFade(flags);
}
