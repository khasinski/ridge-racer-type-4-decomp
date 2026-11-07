#include "rr/overlay662_car_bars.h"

#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000U)
#endif

void garage_draw_car_status_bars(s32 selected)
{
    RrUiUvRect strip;
    void *result;
    u32 *head;
    void *initial;
    s32 x = 40, index, r, g, b;
    s32 y = 288;
    s16 *status;
    void *cursor;

    strip.v = 64;
    strip.w = 240;
    strip.h = 24;
    strip.u = 0;
    head = &CurrentDrawBuf->head70;
    initial = RR_GPU_SCRATCH->cursor;
    cursor = AddSpritePrim(head, initial, 40, y, &strip, 0x7FA4, 1);

    index = 0;
    status = garage_car_status;
    do {
        if (*status != 0) {
            r = 255;
            if (index == selected) {
                r = 240;
                g = 122;
                b = 20;
            } else {
                g = 189;
                b = 0;
            }
            cursor = AddTilePrim(&CurrentDrawBuf->head70, cursor,
                                  x + 1, y, 10, 22, r, g, b);
        }
        x += 12;
        index++;
        status++;
    } while (index < 20);

    result = AddDrawModePrim(&CurrentDrawBuf->head70, cursor, 0x39);
    RR_GPU_SCRATCH->cursor = result;
    if (garage_car_status[(SelectionRows.bytes[2] % 20) & 255] == 2) {
        result = AddSpritePrim(&CurrentDrawBuf->head70, result, 142, 256,
                              &garage_owned_car_marker_sprite, 0x7CBB, 0);
        RR_GPU_SCRATCH->cursor = AddDrawModePrim(&CurrentDrawBuf->head70, result, 0x38);
    }
}
