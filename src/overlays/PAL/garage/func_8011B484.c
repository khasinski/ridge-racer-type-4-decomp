#include "rr/overlay662_primitives.h"
#include "rr/overlay662_garage_fade.h"
#include "rr/animated_sprite_grid.h"
#ifndef RR_GPU_SCRATCH
#define RR_GPU_SCRATCH ((RrGpuScratchPointers *)0x1F800000)
#endif

/* Animated panel element: ordering slot, position/size and colour. */
typedef struct {
    s32 order;
    s16 x, y, w, h;
    u8 r, g, b, pad;
} RrGaragePanelElement;

/* Animation track: time window [start,end] and packed endpoints for GTE lerp. */
typedef struct {
    s16 start, end;
    u32 from, to;
} RrGaragePanelTrack;

extern RrGaragePanelElement garage_D_80126800, garage_D_80126810, garage_D_80126820,
    garage_D_80126830, garage_D_80126840;
extern RrGaragePanelTrack *garage_D_8012688C[5];

void garage_func_8011B484(void)
{
    RrGpuScratchPointers *scratch = RR_GPU_SCRATCH;
    u32 *ordering = scratch->order + 128;
    RrGaragePoint point;
    s32 i;
    s32 t;
    s32 range;

    if (garage_D_801276A8.flagsD1 != 0) {
        return;
    }
    for (i = 0; i < 5; i++) {
        t = garage_D_801276A8.word04 - garage_D_8012688C[i]->start;
        range = garage_D_8012688C[i]->end - garage_D_8012688C[i]->start;
        if (t < 0) {
            continue;
        }
        if (range < t) {
            t = 4096;
        } else {
            t = (t << 12) / range;
        }
        RR_GARAGE_GTE_LERP(t, &garage_D_8012688C[i]->from, &point);
        switch (i) {
        case 0:
            scratch->cursor = (u8 *)garage_func_8011AD3C(ordering + garage_D_80126800.order, (RrGpuTilePacket *)scratch->cursor,
                garage_D_80126800.x, garage_D_80126800.y, point.x, garage_D_80126800.r, garage_D_80126800.g, garage_D_80126800.b);
            break;
        case 1:
            scratch->cursor = (u8 *)garage_func_8011AD3C(ordering + garage_D_80126810.order, (RrGpuTilePacket *)scratch->cursor,
                garage_D_80126810.x, garage_D_80126810.y, point.x, garage_D_80126810.r, garage_D_80126810.g, garage_D_80126810.b);
            break;
        case 2:
            scratch->cursor = (u8 *)garage_func_8011B034(ordering + garage_D_80126820.order, (RrGpuTilePacket *)scratch->cursor,
                point.x, point.y, garage_D_80126820.w, garage_D_80126820.h, garage_D_80126820.r, garage_D_80126820.g, garage_D_80126820.b);
            scratch->cursor = (u8 *)garage_func_8011AEC4(ordering + garage_D_80126820.order, (RrGpuLineF4Packet *)scratch->cursor,
                (s16)(point.x - 1), (s16)(point.y - 2), (s16)(garage_D_80126820.w + 2), (s16)(garage_D_80126820.h + 4), 255, 255, 255);
            break;
        case 3:
            if (garage_D_801276A8.layer23) {
                point.y += 80;
            }
            scratch->cursor = (u8 *)garage_func_8011B034(ordering + garage_D_80126830.order, (RrGpuTilePacket *)scratch->cursor,
                point.x, point.y, garage_D_80126830.w, garage_D_80126830.h, garage_D_80126830.r, garage_D_80126830.g, garage_D_80126830.b);
            scratch->cursor = (u8 *)garage_func_8011AEC4(ordering + garage_D_80126830.order, (RrGpuLineF4Packet *)scratch->cursor,
                point.x, (s16)(point.y - 2), (s16)(garage_D_80126830.w + 1), (s16)(garage_D_80126830.h + 4), 255, 255, 255);
            break;
        case 4:
            scratch->cursor = (u8 *)garage_func_8011B034(ordering + garage_D_80126840.order, (RrGpuTilePacket *)scratch->cursor,
                point.x, point.y, garage_D_80126840.w, garage_D_80126840.h, garage_D_80126840.r, garage_D_80126840.g, garage_D_80126840.b);
            scratch->cursor = (u8 *)garage_func_8011AEC4(ordering + garage_D_80126840.order, (RrGpuLineF4Packet *)scratch->cursor,
                (s16)(point.x - 1), (s16)(point.y - 2), (s16)(garage_D_80126840.w + 2), (s16)(garage_D_80126840.h + 4), 255, 255, 255);
            break;
        }
    }
}
