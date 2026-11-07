#ifndef RR_HUD_ROTATING_QUAD_H
#define RR_HUD_ROTATING_QUAD_H
#include "rr/hud_number.h"
/* RGB occupies three bytes of a GPU color word. Its fourth byte is union
 * padding: copying the union preserves an unspecified, unused byte. */
typedef union {struct {u8 r,g,b;} rgb;u32 word;} RrHudQuadColor;
typedef struct {s16 x,y;RrHudQuadColor color;} RrHudQuadVertex;
/* Four XY/color pairs follow the first color. The final RGB write is at36,
 * in the next packet's tag storage, so the accessible emission spans40 bytes. */
typedef struct {
    RrGpuDmaTag tag;
    u8 r,g,b,code;
    RrHudQuadVertex vertices[4];
} RrHudQuadEmission;
typedef struct {s8 x,y;} RrHudQuadCorner;
extern RrHudQuadCorner HudQuadCorners[4];
#ifndef RR_HUD_QUAD_SCRATCH
#define RR_HUD_QUAD_SCRATCH ((RrGpuScratchPointers *)0x1F800000U)
#endif
/* Selected target and bank0..3 are accessible; cursor is4-byte aligned with
 *40 writable bytes. The live ordering head may alias this packet's tag.
 * Position/head/cursor are captured after both trig services. Advances36 bytes;
 * only low24 pointer bits are serialized into the GPU link. */
void DrawHudRotatingQuad(s32 phase,s32 index);
RR_ASSERT_SIZE(RrHudQuadColor,4);
RR_ASSERT_SIZE(RrHudQuadVertex,8);
RR_ASSERT_OFFSET(RrHudQuadVertex,color,4);
RR_ASSERT_SIZE(RrHudQuadEmission,40);
RR_ASSERT_OFFSET(RrHudQuadEmission,vertices,8);
RR_ASSERT_SIZE(RrHudQuadCorner,2);
#endif
