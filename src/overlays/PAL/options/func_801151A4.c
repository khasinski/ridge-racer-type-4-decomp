#include "rr/overlay665_audio_options.h"

void options_update_audio_menu(void) {
    s16 openingTimer;
    s32 signedTimer;
    s16 closingTimer;
    s32 closeIndex;
    s32 clearIndex;
    register s32 row __asm__("$16");
    s32 detailIndex;
    u32 layout;
    u8 monoState;
    u8 musicState;
    u8 effectsState;
    u8 detailState;
    RrUiRow24 *detail;
    RrUiRow24 *closingRow;
    RrUiRow24 *clearingRow;

    layout = (u32)options_menu_state.layout0C;
    switch (layout) {
    case 0:
        row = 0;
        options_init_audio_rows();
        MenuFadeTimer = 0x40;
        options_menu_state.layout0C = (s32) ((u32)options_menu_state.layout0C + 1U);
        break;
    case 1:
        signedTimer=MenuFadeTimer;
        __asm__("" : "+r"(signedTimer));
        openingTimer = (u16) MenuFadeTimer - 1;
        if (signedTimer > 0) {
            MenuFadeTimer = openingTimer;
            AddScreenFadeTiles(&((RrFrameOrderingPrefix *)(void *)CurrentDrawBuf)->ordering70[702], (s32)openingTimer*4, 0x45);
        }
        SetMotionControllerVisible(1);
        if (UiRows[0].byte14 == 2) {
            row = 0;
            if (MenuFadeTimer == 0) {
                options_menu_state.layout0C = (s32) ((u32)options_menu_state.layout0C + 1U);
            }
        } else {
            goto reset_rows;
        }
        break;
    case 2:
        UiRows[options_menu_state.exit34.word].cells[0] = 0;
        StepMenuValue(0, 2, (char *)&options_menu_state.exit34.halves[0], 4, 0);
        UiRows[options_menu_state.exit34.word].cells[0] = 2;
        if (PlayerControllers[0].a != 0) {
            PlaySfx(1);
            UiRows[options_menu_state.exit34.word].cells[0] = 1;
            options_menu_state.layout0C = (s32) (options_menu_state.exit34.word + 3);
            SetMenuFrameOptions(1, 1, 0);
            row = 0;
        } else {
            if (PlayerControllers[0].b != 0) {
                PlaySfx(2);
                MenuFadeTimer = 0x40;
                options_menu_state.layout0C = 6;
            }
            goto reset_rows;
        }
        break;
    case 3:
        musicState = UiRows[4].byte14;
        if (musicState == 2) {
            UiRows[4].cells[0] = musicState;
            StepMenuValue(1, -1, (char *)&Options.bytes[0x30], 0x11, 0);
            options_music_volume_setting = (s32) Options.bytes[0x30];
            SeqSetVolume(MusicSeqVolume);
            if (MenuConfirmPressed != 0) {
                PlaySfx(1);
                UiRows[4].cells[0] = 1U;
                options_menu_state.layout0C = 7U;
                SetMenuFrameOptions(1, 1, 1);
            }
        }
        DrawUiRow(4);
        row = 0;
        break;
    case 4:
        effectsState = UiRows[5].byte14;
        if (effectsState == 2) {
            UiRows[5].cells[0] = effectsState;
            StepMenuValue(1, -1, (char *)&Options.bytes[0x31], 0x11, 0);
            options_sfx_volume_setting = (s32) Options.bytes[0x31];
            if (MenuConfirmPressed != 0) {
                PlaySfx(1);
                UiRows[5].cells[0] = 1U;
                options_menu_state.layout0C = 7U;
                SetMenuFrameOptions(1, 1, 1);
            }
        }
        DrawUiRow(5);
        row = 0;
        break;
    case 5:
        monoState = UiRows[6].byte14;
        if (monoState == 2) {
            UiRows[Options.audio.mono32+6].cells[0] = 0;
            StepMenuValue(0, 2, (char *)&Options.audio.mono32, 2, 0);
            UiRows[Options.audio.mono32+6].cells[0] = monoState;
            if (MenuConfirmPressed != 0) {
                PlaySfx(1);
                UiRows[Options.audio.mono32+6].cells[0] = 1;
                options_menu_state.layout0C = 7U;
                SetMenuFrameOptions(1, 1, 1);
                SetCdMixMode(Options.audio.mono32);
            }
        }
        DrawUiRow(6);
        DrawUiRow(7);
        row = 0;
        break;
    case 6:
        row = 0;
        closeIndex = 0;
        do {
            closingRow = (RrUiRow24 *)((unsigned long)(closeIndex*sizeof(RrUiRow24))+(unsigned long)UiRows);
            if (closingRow->byte14 == 2) {
                closingRow->byte14 = 3U;
            }
            row += 1;
            closeIndex++;
        } while (row < 4);
        MenuFadeTimer = 0x40;
        options_menu_state.layout0C = 9U;
        goto reset_rows;
    case 7:
        row = 4;
        detailIndex = 4;
        do {
            detail = (RrUiRow24 *)((unsigned long)(detailIndex*sizeof(RrUiRow24))+(unsigned long)UiRows);
            detailState = detail->byte14;
            if (detailState == 2) {
                detail->byte14 = 3U;
            } else if (detailState == 6) {
                options_menu_state.layout0C = 8;
            }
            if ((u8) UiRows[detailIndex].byte14 >= 2U) {
                DrawUiRow(row);
            }
            row += 1;
            detailIndex++;
        } while (row < 8);
        row = 0;
        break;
    case 8:
        row = 4;
        clearIndex = 4;
        do {
            clearingRow = (RrUiRow24 *)((unsigned long)(clearIndex*sizeof(RrUiRow24))+(unsigned long)UiRows);
            if (clearingRow->byte14 != 0) {
                clearingRow->byte14 = 0U;
            }
            row += 1;
            clearIndex++;
        } while (row < 8);
        options_menu_state.layout0C = 1U;
        goto reset_rows;
    case 9:
        closingTimer = (u16) MenuFadeTimer - 1;
        MenuFadeTimer = closingTimer;
        AddScreenFadeTiles(&((RrFrameOrderingPrefix *)(void *)CurrentDrawBuf)->ordering70[0], (0x40 - closingTimer) * 4, 0x45);
        row = 0;
        if (MenuFadeTimer <= 0) {
            options_init_main_menu();
            options_menu_state.layout0C = 0;
            options_menu_state.scene00 = (s32) ((u32)options_menu_state.scene00 - 1U);
            goto reset_rows;
        }
        break;
    default: goto reset_rows;
    }
    goto draw_rows;
reset_rows:
    row=0;
draw_rows:
    do {
        DrawUiRow(row);
        row += 1;
    } while (row < 4);
    UpdateScreenFade(0x1006A);
}
