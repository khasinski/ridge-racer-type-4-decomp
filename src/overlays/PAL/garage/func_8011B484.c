#include "rr/garage_primitives.h"
#include "rr/garage_garage_fade.h"
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

extern RrGaragePanelElement garage_panel_anim_bar0, garage_panel_anim_bar1, garage_panel_anim_box0,
    garage_panel_anim_box1, garage_panel_anim_box2;
extern RrGaragePanelTrack *garage_panel_anim_tracks[5];

void garage_draw_animated_panels(void)
{
    RrGpuScratchPointers *scratch = RR_GPU_SCRATCH;
    u32 *ordering = scratch->order + 128;
    RrGaragePoint point;
    s32 i;
    s32 t;
    s32 range;

    if (garage_logo_state.flagsD1 != 0) {
        return;
    }
    for (i = 0; i < 5; i++) {
        t = garage_logo_state.word04 - garage_panel_anim_tracks[i]->start;
        range = garage_panel_anim_tracks[i]->end - garage_panel_anim_tracks[i]->start;
        if (t < 0) {
            continue;
        }
        if (range < t) {
            t = 4096;
        } else {
            t = (t << 12) / range;
        }
        RR_GARAGE_GTE_LERP(t, &garage_panel_anim_tracks[i]->from, &point);
        switch (i) {
        case 0:
            scratch->cursor = (u8 *)garage_add_hline_double(ordering + garage_panel_anim_bar0.order, (RrGpuTilePacket *)scratch->cursor,
                garage_panel_anim_bar0.x, garage_panel_anim_bar0.y, point.x, garage_panel_anim_bar0.r, garage_panel_anim_bar0.g, garage_panel_anim_bar0.b);
            break;
        case 1:
            scratch->cursor = (u8 *)garage_add_hline_double(ordering + garage_panel_anim_bar1.order, (RrGpuTilePacket *)scratch->cursor,
                garage_panel_anim_bar1.x, garage_panel_anim_bar1.y, point.x, garage_panel_anim_bar1.r, garage_panel_anim_bar1.g, garage_panel_anim_bar1.b);
            break;
        case 2:
            scratch->cursor = (u8 *)garage_add_tile(ordering + garage_panel_anim_box0.order, (RrGpuTilePacket *)scratch->cursor,
                point.x, point.y, garage_panel_anim_box0.w, garage_panel_anim_box0.h, garage_panel_anim_box0.r, garage_panel_anim_box0.g, garage_panel_anim_box0.b);
            scratch->cursor = (u8 *)garage_add_rect_outline(ordering + garage_panel_anim_box0.order, (RrGpuLineF4Packet *)scratch->cursor,
                (s16)(point.x - 1), (s16)(point.y - 2), (s16)(garage_panel_anim_box0.w + 2), (s16)(garage_panel_anim_box0.h + 4), 255, 255, 255);
            break;
        case 3:
            if (garage_logo_state.layer23) {
                point.y += 80;
            }
            scratch->cursor = (u8 *)garage_add_tile(ordering + garage_panel_anim_box1.order, (RrGpuTilePacket *)scratch->cursor,
                point.x, point.y, garage_panel_anim_box1.w, garage_panel_anim_box1.h, garage_panel_anim_box1.r, garage_panel_anim_box1.g, garage_panel_anim_box1.b);
            scratch->cursor = (u8 *)garage_add_rect_outline(ordering + garage_panel_anim_box1.order, (RrGpuLineF4Packet *)scratch->cursor,
                point.x, (s16)(point.y - 2), (s16)(garage_panel_anim_box1.w + 1), (s16)(garage_panel_anim_box1.h + 4), 255, 255, 255);
            break;
        case 4:
            scratch->cursor = (u8 *)garage_add_tile(ordering + garage_panel_anim_box2.order, (RrGpuTilePacket *)scratch->cursor,
                point.x, point.y, garage_panel_anim_box2.w, garage_panel_anim_box2.h, garage_panel_anim_box2.r, garage_panel_anim_box2.g, garage_panel_anim_box2.b);
            scratch->cursor = (u8 *)garage_add_rect_outline(ordering + garage_panel_anim_box2.order, (RrGpuLineF4Packet *)scratch->cursor,
                (s16)(point.x - 1), (s16)(point.y - 2), (s16)(garage_panel_anim_box2.w + 2), (s16)(garage_panel_anim_box2.h + 4), 255, 255, 255);
            break;
        }
    }
}
