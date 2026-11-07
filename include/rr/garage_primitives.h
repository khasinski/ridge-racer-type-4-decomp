#ifndef RR_GARAGE_PRIMITIVES_H
#define RR_GARAGE_PRIMITIVES_H
#include "rr/garage_render_step.h"
#include "rr/garage_garage.h"
#include "rr/gpu.h"
#include "rr/draw_environment.h"
#include "rr/garage_sprites.h"
/* Clip helper takes a packet cursor and preserves the low16 rectangle fields. */
u8 *garage_add_draw_area(u32 *ordering,u8 *cursor,u32 x,u32 y,u32 w,u32 h);
/* Unless work bit0 is set, emit the selected sprite then the full-screen clip.
 * The active work index must select an initialized sprite. */
void garage_draw_cursor_sprite(void);
/* Build draw-mode and null texture-window commands and prepend to ordering. */
RrGpuDrawModePacket *garage_add_draw_mode(u32 *ordering,
    RrGpuDrawModePacket *packet,u32 x,u32 y);
/* Capture the garage origin, write a three-word TILE, then prepend it to the
 * ordering link. Coordinates, sizes and colors preserve their low16/8 bits.
 * The supplied packet belongs to an array with room for the returned cursor. */
RrGpuTilePacket *garage_add_tile(u32 *ordering, RrGpuTilePacket *packet,
    u32 x,u32 y,u32 w,u32 h,u32 r,u32 g,u32 b);
/* Inputs wrap to the packet field widths. Capture both origin coordinates
 * before writing. Ordering may alias an actual u32 member (tag or terminator) of an output
 * packet, but not a packed pair of coordinates or four color bytes.
 * The packet cursor belongs to an array with room for the returned cursor. */
RrGpuPolyFT4Packet *garage_add_textured_rect(u32 *ordering,RrGpuPolyFT4Packet *packet,
    u32 x,u32 y,u32 w,u32 h,u32 u,u32 v,u32 uw,u32 vh,u32 clut,u32 tpage);
RrGpuTilePacket *garage_add_hline_double(u32 *ordering,RrGpuTilePacket *packet,
    u32 x0,u32 y,u32 x1,u32 r,u32 g,u32 b);
RrGpuLineF4Packet *garage_add_rect_outline(u32 *ordering,RrGpuLineF4Packet *packet,
    u32 x,u32 y,u32 w,u32 h,u32 r,u32 g,u32 b);
#endif
