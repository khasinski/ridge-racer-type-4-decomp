#include "rr/controller_hud.h"

/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf ((struct O *)CurrentDrawBuf)

#undef CurrentDrawBuf

void DrawSteeringIndicator(u32 *ordering, s32 layout) {
    RrHudTriangle triangle;
    s16 *outputPoint;
    s32 status;

    void *modeCursor;
    void *spriteCursor;
    s32 parity;
    void *cursor;
    s32 shapeOffset;
    s32 i;
    s32 tickX;
    s32 player;
    s32 baselineY;
    s32 offsetX;
    s32 widthBase,heightBase,color;
    u16 *toggle;
    u8 *shapeY;
    u8 *shapeX;
    u8 encodedX;
    RrMenuItemState5C *input;
    RrMenuItemState5C *inputBase;
    u8 *shapeBase;

    player = 0;
    baselineY = 0x70;
    if (layout != 1) {
        baselineY = 0xF0;
        player = layout == 2;
    }
    inputBase = PlayerControllers;

    input = (RrMenuItemState5C *)((unsigned long)inputBase + player * 0x5C);
    status = input->status10;
    if (status == 2) {
        if (input->buttons08 & 0x100) {
            toggle = (u16 *)&SteeringGaugeVisible[player];
            *toggle ^= 1;
        }
        i = GetControllerWarning(player);
        if (i > 0) {
            if ((u32) (ScreenFrame & 3) < 2U) {
                if (i == status) {
                    goto drawWarning;
                }
            } else {
drawWarning:
                modeCursor = AddDrawModePrim(ordering, RR_HUD_SCRATCH->cursor, 0x25);
                RR_HUD_SCRATCH->cursor = modeCursor;
                spriteCursor = GameQueueSprite(ordering, (RrSpritePacketSignedClut *)RR_HUD_SCRATCH->cursor, 0x80, baselineY - 0xA, 0x44, 8, 0x80, 0x40, 0x1B16);
                RR_HUD_SCRATCH->cursor = spriteCursor;
                RR_HUD_SCRATCH->cursor = AddDrawModePrim(ordering, spriteCursor, 0x25);
            }
        }
        i = 0;
        if (SteeringGaugeVisible[player] != 0) {
            cursor = RR_HUD_SCRATCH->cursor;
            widthBase = 2;
            heightBase = 6;
            color = 255;
            tickX = 0x63;
            do {
                parity = i & 1;
                offsetX = widthBase - parity;
                cursor = AddTilePrim(ordering, cursor, tickX, baselineY | (parity * 2), offsetX, heightBase - (parity * 4), color, color, color);
                i += 1;
                tickX += 0xF;
            } while (i < 9);
            offsetX = (PlayerControllers[player].steering3C * 0x3C) / 127;
            if (offsetX >= 0x3D) {
                offsetX = 0x3C;
                goto drawNeedle;
            }
            i = 0;
            if (offsetX < -0x3C) {
                offsetX = -0x3C;
drawNeedle:
                i = 0;
            }
            shapeBase = SteeringNeedleShape;
            shapeOffset = 1;
            outputPoint = triangle.half;
            shapeX = shapeBase;
            do {
                shapeY = (u8 *)((unsigned long)shapeOffset + (unsigned long)shapeBase);
                shapeOffset += 2;
                encodedX = *shapeX;
                shapeX += 2;
                outputPoint[0] = (s16) (offsetX + (s16)((s8) encodedX + 0xA0));
                i += 1;
                outputPoint[1] = (s16) ((s8) *shapeY + baselineY + 5);
                outputPoint += 2;
            } while (i < 3);
            RR_HUD_SCRATCH->cursor = (u8 *)EmitPolyF3(ordering, cursor, triangle.word[0], triangle.word[1], triangle.word[2], D_8009EE64, 0);
        }
    }
}
