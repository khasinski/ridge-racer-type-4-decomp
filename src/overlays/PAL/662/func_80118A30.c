#include "rr/overlay662_name_controller.h"

void ovl662_func_80118A30(void) {
    s16 fadeStep;
    s32 cancel;
    s32 phase;
    u8 normalizedRow;

    phase = (u16)D_800F4D8A;
    ModeFrame=(u32)ModeFrame+1;
    cancel = 0;
    ScreenFrame=(u32)ScreenFrame+1;
    if (phase == 0) {
        SetMenuFrameOptions(1, 1, 1);
        if (PlayerControllers[0].b != 0) {cancel=1;goto normalizeColumn;}
        if (PlayerControllers[0].buttons0A & 0x1000) {
            PlaySfx(0);ovl662_D_801273A3--;goto normalizeRow;
        }else if (PlayerControllers[0].buttons0A & 0x4000) {
            PlaySfx(0);ovl662_D_801273A3++;goto normalizeRow;
        }else if (PlayerControllers[0].buttons0A & 0x8000) {
            PlaySfx(0);ovl662_D_801273A2--;goto normalizeColumn;
        }else if (PlayerControllers[0].buttons0A & 0x2000) {
            PlaySfx(0);ovl662_D_801273A2++;goto normalizeColumn;
        }else goto normalizeColumn;
normalizeRow:
        normalizedRow=((s8)ovl662_D_801273A3+5)%5;
        ovl662_D_801273A3=normalizedRow;
        if((s8)ovl662_D_801273A2>=3){if((s8)normalizedRow==4)ovl662_D_801273A2=2;}
normalizeColumn:
        if((s8)ovl662_D_801273A3>=4)ovl662_D_801273A2=((s8)ovl662_D_801273A2+3)%3;
        else ovl662_D_801273A2=((s8)ovl662_D_801273A2+10)%10;
        ovl662_D_801273B4 = (s8) ovl662_D_801273A2 + ((s8) ovl662_D_801273A3 * 0xA);
        if (MenuConfirmPressed != 0) {
            PlaySfx(1);
            if(ovl662_D_801273B4<41){
                if (ovl662_D_801273A0 < 8) {
                    PlaySfx(1);
                    if (ovl662_D_801273B4 == 0x28) {
                        ovl662_D_801273A8[ovl662_D_801273A0] = NameCharset[0];
                    } else {
                        ovl662_D_801273A8[ovl662_D_801273A0] = NameCharset[ovl662_D_801273B4+1];
                    }
                    if (ovl662_D_801273A0 < 7) {
                        ovl662_D_801273A0 = (u8) ovl662_D_801273A0 + 1;
                    }
                }
}else if(ovl662_D_801273B4==41){
                if (ovl662_D_801273A0 > 0) {
                    ovl662_D_801273A0 = (u8) ovl662_D_801273A0 - 1;
                }
}else if(ovl662_D_801273B4==42){
                MenuFadeTimer = 0x40;
                D_800F4D8A = 1;
                CopyStringClearTail(SaveProgress.prefix.progress.prefix00.fields.name,(char*)ovl662_D_801273A8);
            }
        } else if (cancel == 1) {
            PlaySfx(2);
            ovl662_D_801273A8[ovl662_D_801273A0] = NameCharset[0];
            if (ovl662_D_801273A0 > 0) {
                ovl662_D_801273A0 = (u8) ovl662_D_801273A0 - 1;
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
            D_800F4D8A = 0;
        }
    }
    UpdateBadgeRowAnim(0);
    UpdateBadgeRowAnim(1);
    UpdateScreenFade(0x1008B);
}
