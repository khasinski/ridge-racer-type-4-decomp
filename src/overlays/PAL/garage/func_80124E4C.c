#include "rr/garage_pixel_dispatch.h"

void garage_fill_at_cursor(void)
{
    RrGaragePixelEdit4 operation;
    u8 *pixel;
    RrGarageSelectionPrefix *state;
    s32 x;
    register s32 y __asm__("$4");
    register u32 coordX __asm__("$4");
    register u32 offset __asm__("$2");
    register s32 dx __asm__("$5");
    s32 dy;
    register u32 packed __asm__("$3");

    if (garage_cursor_outside_rect(82, 71, 128, 256) == 0 &&
        (garage_logo_work.words[1] & 1)) {
        if (garage_logo_state.layer23 == 0) {
            pixel = (u8 *)garage_logo_state.image38[garage_logo_state.active21];
        } else {
            pixel = (u8 *)garage_logo_state.image40[garage_logo_state.active21];
        }
        state = &garage_logo_state;
        x = (u8)state->reset08[0];
        dx = (u8)state->reset08[6];
        y = (u8)state->reset08[1];
        dy = (u8)state->reset08[7];
        x += dx;
        y += dy;
        operation.y = y;
        /* Only the initialized coordinate byte is an input at each barrier. */
        __asm__("" : : "m"(operation.y));
        offset = (u8)y;
        operation.x = x;
        __asm__("" : : "m"(operation.x));
        coordX = (u8)x;
        offset *= 32;
        offset += coordX >> 1;
        pixel += offset;
        if (coordX & 1) {
            operation.previous = *pixel >> 4;
        } else {
            operation.previous = *pixel & 15;
        }
        if (PlayerControllers[0].type01 != 0x12 &&
            (PlayerControllers[0].buttons02 & 2) && (PlayerControllers[0].buttons02 & 1)) {
            operation.replacement = 0;
        } else {
            operation.replacement = garage_logo_state.config22;
        }
        if (operation.previous != operation.replacement) {
            garage_save_undo_copy();
            packed = (u32)operation.y << 8;
            packed = (u32)operation.x | packed;
            packed |= (u32)operation.previous << 16;
            FloodFill4bpp(pixel, packed | ((u32)operation.replacement << 24));
        }
    }
}
