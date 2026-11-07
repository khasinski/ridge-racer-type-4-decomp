#include "rr/garage_name_controller.h"

void garage_update_name_entry(void) {
    s16 fadeStep;
    s32 cancel;
    s32 phase;
    u8 normalizedRow;

    phase = (u16)ModeSubState;
    ModeFrame=(u32)ModeFrame+1;
    cancel = 0;
    ScreenFrame=(u32)ScreenFrame+1;
    if (phase == 0) {
        SetMenuFrameOptions(1, 1, 1);
        if (PlayerControllers[0].b != 0) {cancel=1;goto normalizeColumn;}
        if (PlayerControllers[0].buttons0A & 0x1000) {
            PlaySfx(0);garage_name_key_row--;goto normalizeRow;
        }else if (PlayerControllers[0].buttons0A & 0x4000) {
            PlaySfx(0);garage_name_key_row++;goto normalizeRow;
        }else if (PlayerControllers[0].buttons0A & 0x8000) {
            PlaySfx(0);garage_name_key_column--;goto normalizeColumn;
        }else if (PlayerControllers[0].buttons0A & 0x2000) {
            PlaySfx(0);garage_name_key_column++;goto normalizeColumn;
        }else goto normalizeColumn;
normalizeRow:
        normalizedRow=((s8)garage_name_key_row+5)%5;
        garage_name_key_row=normalizedRow;
        if((s8)garage_name_key_column>=3){if((s8)normalizedRow==4)garage_name_key_column=2;}
normalizeColumn:
        if((s8)garage_name_key_row>=4)garage_name_key_column=((s8)garage_name_key_column+3)%3;
        else garage_name_key_column=((s8)garage_name_key_column+10)%10;
        garage_name_key_index = (s8) garage_name_key_column + ((s8) garage_name_key_row * 0xA);
        if (MenuConfirmPressed != 0) {
            PlaySfx(1);
            if(garage_name_key_index<41){
                if (garage_name_cursor < 8) {
                    PlaySfx(1);
                    if (garage_name_key_index == 0x28) {
                        garage_name_buffer[garage_name_cursor] = NameCharset[0];
                    } else {
                        garage_name_buffer[garage_name_cursor] = NameCharset[garage_name_key_index+1];
                    }
                    if (garage_name_cursor < 7) {
                        garage_name_cursor = (u8) garage_name_cursor + 1;
                    }
                }
}else if(garage_name_key_index==41){
                if (garage_name_cursor > 0) {
                    garage_name_cursor = (u8) garage_name_cursor - 1;
                }
}else if(garage_name_key_index==42){
                MenuFadeTimer = 0x40;
                ModeSubState = 1;
                CopyStringClearTail(SaveProgress.prefix.progress.prefix00.fields.name,(char*)garage_name_buffer);
            }
        } else if (cancel == 1) {
            PlaySfx(2);
            garage_name_buffer[garage_name_cursor] = NameCharset[0];
            if (garage_name_cursor > 0) {
                garage_name_cursor = (u8) garage_name_cursor - 1;
            }
        }
    } else if (phase == 1) {
        SetMenuFrameOptions(0, 1, 1);
        fadeStep = (u16)MenuFadeTimer - 1;
        MenuFadeTimer = (u16) fadeStep;
        AddScreenFadeTiles(&CurrentDrawBuf->head70, (0x40 - fadeStep) * 4, 0x45);
        if ((s16) MenuFadeTimer <= 0) {
            MenuResultCode = 5;
            ModeGroup = phase;
            ModeId = 0x47;
            ModeSubState = 0;
        }
    }
    UpdateBadgeRowAnim(0);
    UpdateBadgeRowAnim(1);
    UpdateScreenFade(0x1008B);
}
