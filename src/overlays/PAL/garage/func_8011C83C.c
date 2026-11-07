#include "rr/overlay662_garage.h"
#include "rr/animated_sprite_grid.h"

#define RR_GARAGE_SCRATCH ((RrGpuScratchPointers *)0x1F800000U)

/* Local views of the overlay primitive helpers: this unit passes the
 * rectangle coordinates as signed 16-bit values. */
u8 *garage_func_8011ABB4(u32 *ordering, u8 *cursor, u32 x, u32 y, u32 w, u32 h);
u8 *garage_func_8011AEC4(u32 *ordering, u8 *cursor, s16 x, s16 y, s16 w, s16 h,
    u32 r, u32 g, u32 b);
u8 *garage_func_8011B1CC(u32 *ordering, u8 *cursor, s16 x, s16 y, s16 w, s16 h,
    s16 u, s16 v, u32 uw, u32 vh, u32 clut, u32 tpage);

/* Zoomed preview panel: clip, optional selection outlines, then the preview
 * image at the current zoom (config1C) and its frame. */
void garage_func_8011C83C(s32 renderArgument)
{
    RrGpuScratchPointers *scratch;
    RrGarageSelectionPrefix *garage;
    u32 *order0, *order1, *order;
    u16 x, y, w, h;
    s16 size, size2;
    s32 clut, tpage;
    u16 u, v;

    scratch = RR_GARAGE_SCRATCH;
    order0 = scratch->order + 134;
    order1 = scratch->order + 135;
    scratch->cursor = garage_func_8011ABB4(order0, scratch->cursor, 12, 61, 296, 352);
    /* ASM_BARRIER: keeps the clip cursor store ahead of the selection-size load. */
    __asm__("" : : : "memory");
    if (garage_D_801276A8.reset08[4] > 0 && !(garage_D_80127668.words[0] & 6)) {
        if (garage_D_801276A8.config1E < 2 || garage_D_801276A8.config1E == 3
            || garage_D_801276A8.config1F == 1) {
            x = garage_D_801276A8.reset08[2];
            y = garage_D_801276A8.reset08[3];
            w = garage_D_801276A8.reset08[4];
            h = garage_D_801276A8.reset08[5];
            scratch->cursor = garage_func_8011AEC4(order0, scratch->cursor,
                (s16)x * garage_D_801276A8.config1C + 82,
                (s16)y * garage_D_801276A8.config1C * 2 + 71,
                (s16)w * garage_D_801276A8.config1C,
                (s16)h * garage_D_801276A8.config1C * 2,
                0, garage_D_801276A8.shadeD0, 0);
        }
        if (garage_D_801276A8.config1E == 2 || garage_D_801276A8.config1F == 0) {
            x = garage_D_801276A8.reset08[6];
            y = garage_D_801276A8.reset08[7];
            w = garage_D_801276A8.reset08[8];
            h = garage_D_801276A8.reset08[9];
            scratch->cursor = garage_func_8011AEC4(order0, scratch->cursor,
                (s16)x * garage_D_801276A8.config1C + 82,
                (s16)y * garage_D_801276A8.config1C * 2 + 71,
                (s16)w * garage_D_801276A8.config1C,
                (s16)h * garage_D_801276A8.config1C * 2,
                0, garage_D_801276A8.shadeD0, 0);
        }
    }
    switch (garage_D_801276A8.config1C) {
    case 2:
        x = 81; y = 69; w = 133; h = 265; size = 67; size2 = 67;
        break;
    case 4:
        x = 80; y = 67; w = 137; h = 273; size = 35; size2 = 35;
        break;
    case 8:
        x = 78; y = 63; w = 145; h = 289; size = 19; size2 = 19;
        break;
    default:
        x = 74; y = 55; w = 161; h = 321; size = 11; size2 = 11;
        break;
    }
    garage = &garage_D_801276A8;
    order = order0;
    u = garage->reset08[0] + 139;
    v = garage->reset08[1] + 1;
    if (garage->layer23) order = order1;
    clut = 4272;
    tpage = 12;
    scratch->cursor = garage_func_8011B1CC(order, scratch->cursor,
        x, y, w, h, u, v, size2, size, clut, tpage);
    scratch->cursor = garage_func_8011B1CC(order0, scratch->cursor,
        x, y, w, h, garage->reset08[0] + 71, garage->reset08[1] + 1, size2, size, clut, tpage);
    scratch->cursor = garage_func_8011B1CC(order1, scratch->cursor,
        x, y, w, h, garage->reset08[0] + 3, garage->reset08[1] + 1, size2, size, 4273, tpage);
    scratch->cursor = garage_func_8011ABB4(order1, scratch->cursor, 82, 71, 128, 256);
    scratch->cursor = garage_func_8011AEC4(order1, scratch->cursor, 81, 69, 130, 260, 255, 255, 255);
}
