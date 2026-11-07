#include "rr/overlay662_cursor_control.h"

void garage_func_8011E070(void) {
    RrGaragePoint movement;
    RrGaragePoint destination;
    u32 events;
    s32 flags;
    u16 ySum;
    u16 xSum;

    PsyqMemset(&movement, 0, 4);
    events = 0;
    if (!(garage_D_80127668.words[0] & 1)) {
        if (garage_D_80127668.words[0] & 2) {
            garage_func_8011DA90();
            if ((s32)garage_D_80127668.points.word28 >= 0x1000) {
                garage_D_80127668.points.cursor18 = garage_D_80127668.points.cursor14;
                garage_D_80127668.words[0] &= ~2U;
                garage_D_80127668.words[0] &= ~4U;
                goto processInput;
            }
        } else {
processInput:
            if (garage_D_80127668.words[0] & 4) {
                if (!(garage_D_80127668.words[0] & 8)) {
                    garage_func_8011DA90();
                    if ((s32)garage_D_80127668.points.word28 >= 0x1000) {
                        garage_func_8011D730(&events);
                        garage_func_8011D8E4(&events);
                        garage_func_8011DCDC(events);
                        goto finishInput;
                    }
                } else {
                    goto finishInput;
                }
            } else {
                garage_func_8011D338(&movement);
                garage_func_8011D58C(&movement);
                garage_func_8011D664(&movement);
                xSum = garage_D_80127668.points.cursor18.x + movement.x;
                ySum = garage_D_80127668.points.cursor18.y + movement.y;
                garage_D_80127668.points.cursor18.x = xSum;
                garage_D_80127668.points.cursor18.y = ySum;
                if ((s16) xSum < garage_D_80127668.halves[24]) {
                    garage_D_80127668.points.cursor18.x = (u16) garage_D_80127668.halves[24];
                }
                if (((garage_D_80127668.halves[24] + garage_D_80127668.halves[26]) - 1) < (s16) garage_D_80127668.points.cursor18.x) {
                    garage_D_80127668.points.cursor18.x = (u16) (((u16) garage_D_80127668.halves[24] + (u16) garage_D_80127668.halves[26]) - 1);
                }
                if ((s16) ySum < garage_D_80127668.halves[25]) {
                    garage_D_80127668.points.cursor18.y = (u16) garage_D_80127668.halves[25];
                }
                if (((garage_D_80127668.halves[25] + garage_D_80127668.halves[27]) - 1) < (s16) garage_D_80127668.points.cursor18.y) {
                    garage_D_80127668.points.cursor18.y = (u16) (((u16) garage_D_80127668.halves[25] + (u16) garage_D_80127668.halves[27]) - 1);
                }
                garage_D_80127668.points.cursor14 = garage_D_80127668.points.cursor18;
                flags = garage_D_80127668.words[0];
                if (!(flags & 8) && (PlayerControllers[0].type01 != 0x12) && (PlayerControllers[0].buttons08 & 4)) {
                    garage_D_80127668.words[0] = (s32) (flags | 4);
                    garage_D_80127668.words[1] = 0x30;
                    garage_D_80127668.points.first20 = garage_D_80127668.points.cursor18;
                    destination=garage_func_8011DC74(garage_D_801276A8.saved64[0]);
                    garage_D_80127668.points.second24 = destination;
                    garage_D_80127668.points.word28 = 0;
                    garage_D_80127668.points.distance2C = garage_func_8011DBD8(&garage_D_80127668.points.first20,&garage_D_80127668.points.second24);
                    garage_D_801276A8.current60 = (s32) garage_D_801276A8.saved64[0];
                    garage_D_801276A8.selectorsB8[0] = (s32) garage_D_801276A8.saved64[1];
                    garage_D_801276A8.selectorsB8[1] = (s32) garage_D_801276A8.saved64[2];
                    garage_D_801276A8.selectorsB8[2] = (s32) garage_D_801276A8.saved64[3];
                    garage_D_801276A8.selectorsB8[3] = (s32) garage_D_801276A8.saved64[4];
                    garage_D_80127470[((RrGarageButton *)garage_D_801273B8[garage_D_801276A8.saved64[0]])->action0A]->callback(-1);
                    PlaySfx(3);
                    return;
                }
finishInput:
                garage_func_8011DE70();
            }
        }
    }
}
