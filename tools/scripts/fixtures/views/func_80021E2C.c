#include "rr/controller_hud.h"

/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf ((struct O *)CurrentDrawBuf)

#undef CurrentDrawBuf

void DrawRaceMinimap(s32 arg0) {
    s32 var_s2;
    s32 temp_hi;
    void *var_a1;
    s32 var_s0;
    s32 var_s0_2;
    u32 *var_s3;
    s32 var_s4;
    register s32 var_v0 __asm__("$2");
    RrBankRecord320 *temp_a0;

    var_s4 = 0;
    if (arg0 == 0) {
        var_s3 = ((RrFrameStorage *)(void *)CurrentDrawBuf)->ordering.ordering70;
        if (LinkRoleSlot >= 0) {
            var_s2 = (s16)((u16)LinkRoleSlot ^ 1);
        } else {
            var_s2 = 0;
        }
    } else {
        var_s3 = ((RrFrameStorage *)(void *)CurrentDrawBuf)->tables.secondB70;
        if (arg0 == 1) {
            var_s4 = -0x80;
        }
        if (LinkRoleSlot >= 0) {
            {s32 step=((s32)(((u16)LinkRoleSlot^1)<<16)>>15);step=step-1;var_s2=arg0+step;}
        } else {
            var_s2 = arg0 - 1;
        }
    }
    var_a1 = RR_HUD_SCRATCH->cursor;
    var_s0 = 0;
    if (NumCars > 0) {
        s32 yBase;RrUiUvRect *const glyph=&D_8009EE58;
        do {
            temp_hi = (s32) (var_s2 + var_s0) % (s16) NumCars;
            temp_a0 = CarTable[temp_hi];
            if ((u16)temp_a0->value1E4 & 0x10) {
                u16 clut=(temp_hi<<6)+0x1A16;
                s32 yNumerator=temp_a0->y28; s32 xNumerator=temp_a0->x20;
                register s32 yValue __asm__("$2");s32 xValue;xValue=xNumerator/1150+4;yBase=var_s4+242;yValue=yBase-yNumerator/1150;
                var_a1 = AddSpritePrim(var_s3, var_a1, xValue, yValue-4, glyph, clut, 0);
            }
            var_s0 += 1;
        } while (var_s0 < NumCars);
    }
    var_s0_2 = 0;
    if ((arg0 + 1) > 0) {
        s16 originX=8;
        do {
            __asm__("" : : "r"((s32)originX));
            var_v0 = var_s4 + 0x3E;
            if (var_s0_2 <= 0) {
                var_v0 = var_s4 + 0xBE;
            }
            var_a1 = GameQueueSprite(var_s3, var_a1, originX, var_v0, 0x34, 0x34, 0x4C, 0x6C, 0x1BD6);
            var_s0_2 += 1;
        } while (var_s0_2 < (arg0 + 1));
    }
    RR_HUD_SCRATCH->cursor = AddDrawModePrim(var_s3, var_a1, 0x25);
}
