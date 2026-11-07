#ifndef RR_GPU_H
#define RR_GPU_H

#include "rr/layout.h"

/* Configured GPU data and status/control register addresses. */
extern volatile u32 *GPU_DATA, *GPU_STATUS;
s32 GPU_CheckVersion(s32 mode);

/* GPU DMA channel and queue timeout state. Queue tail is interrupt-visible. */
extern volatile u32 *DMA2_MADR, *DMA2_BCR, *DMA2_CHCR;
extern s32 _qin, SavedIntrMask, GpuTimeoutDeadline, GpuAlarmCount;
extern volatile s32 _qout;

/* Drawing limits and packed E3/E4 drawing-area coordinate commands. */
extern s16 D_800ABAC4, D_800ABAC6;
u32 get_cs(s32 x, s32 y);
u32 get_ce(s32 x, s32 y);

/* Ordering-table DMA on channel 6. Address/count arithmetic wraps at 32 bits on PAL.
 * The supplied address is recorded after adding signed32(count*4-4). */
extern u8 * volatile *DMA6_MADR;
extern volatile u32 *DMA6_BCR, *DMA6_CHCR, *DPCR;
s32 _otc(u8 *buffer, s32 count);
void set_alarm(void);
s32 get_alarm(void);

/* RECT has both signed-coordinate and packed-word views on little-endian PAL. */
typedef struct { s16 x,y,w,h; } RrGpuRectangleFields;
typedef union {
    RrGpuRectangleFields rect;
    u32 words[2];
} RrGpuRectangle;

typedef struct {
    u32 position, destination, size;
} RrGpuMovePayload;

/* The DMA packet header occupies the eight bytes preceding this payload. */
extern RrGpuMovePayload GpuMovePayload;
void checkRECT(const char *name, RrGpuRectangle *rectangle);
void GPU_DmaDoneRequeue();
void _cwc(unsigned long address);
s32 MoveImage2(RrGpuRectangle *rectangle, s32 x, s32 y);
/* Uploads ceil(width*height/2) packed pixel words, using a CPU tail and DMA
 * blocks of16 words. Dimensions clamp against the signed configured limits
 * (inclusive), storing their low16 bits. Returns-1 for a nonpositive word
 * count or timeout, otherwise0. The source must cover the complete transfer. */
s32 _dws(RrGpuRectangle *rectangle, const u32 *pixels);
/* Reverse transfer with the same dimension and buffer-capacity contract.
 * Polls command readiness, requests image data, then polls receive readiness
 * before reading the CPU tail and starting DMA. Returns -1 on either timeout
 * or a nonpositive word count, otherwise 0. */
s32 _drs(RrGpuRectangle *rectangle, u32 *pixels);
/* In-place dimensions clamp against signed limit-1. Aligned X/width use the
 * fast GPU fill; other rectangles use a linked packet that restores draw state.
 * The color's high bit contributes bit10 of the drawing-mode command. */
s32 _clr(RrGpuRectangle *rectangle, u32 color);
/* Query a GPU state selector and return its low24 bits. */
s32 _param(s32 selector);
RR_ASSERT_SIZE(RrGpuRectangle, 8);
RR_ASSERT_OFFSET(RrGpuRectangleFields, w, 4);
RR_ASSERT_OFFSET(RrGpuRectangleFields, h, 6);
RR_ASSERT_SIZE(RrGpuMovePayload, 12);
RR_ASSERT_OFFSET(RrGpuMovePayload, destination, 4);
RR_ASSERT_OFFSET(RrGpuMovePayload, size, 8);

/* GPU sprite packets with a 24-bit link and 8-bit command length.
 * CLUT signedness differs between original units; preserve both views. */

typedef struct RrSpritePacket {
    u32 addr:24;
    u32 len:8;
    u8 r, g, b, code;
    s16 x, y;
    u8 u, v;
    u16 clut;
    s16 w, h;
} RrSpritePacket;

typedef struct RrSpritePacketSignedClut {
    u32 addr:24;
    u32 len:8;
    u8 r, g, b, code;
    s16 x, y;
    u8 u, v;
    s16 clut, w, h;
} RrSpritePacketSignedClut;

RR_ASSERT_SIZE(RrSpritePacket, 0x14);
RR_ASSERT_OFFSET(RrSpritePacket, r, 0x4);
RR_ASSERT_OFFSET(RrSpritePacket, x, 0x8);
RR_ASSERT_OFFSET(RrSpritePacket, clut, 0xe);
RR_ASSERT_OFFSET(RrSpritePacket, w, 0x10);
RR_ASSERT_SIZE(RrSpritePacketSignedClut, 0x14);
RR_ASSERT_OFFSET(RrSpritePacketSignedClut, r, 0x4);
RR_ASSERT_OFFSET(RrSpritePacketSignedClut, x, 0x8);
RR_ASSERT_OFFSET(RrSpritePacketSignedClut, clut, 0xe);
RR_ASSERT_OFFSET(RrSpritePacketSignedClut, w, 0x10);

/* Serialized GPU DMA tags retain only the low24 address bits. The tag is
 * an encoded hardware word, never a C pointer used for memory access. */
typedef union {
    u32 word;
    struct {u8 link[3],length;} bytes;
} RrGpuDmaTag;
typedef struct {RrGpuDmaTag tag;u32 command[2];} RrGpuDrawModePacket;
RR_ASSERT_SIZE(RrGpuDrawModePacket,12);
RR_ASSERT_OFFSET(RrGpuDrawModePacket,command,4);
typedef struct {
    RrGpuDmaTag tag;
    u8 r,g,b,code;
    s16 x,y,w,h;
} RrGpuTilePacket;
RR_ASSERT_SIZE(RrGpuDmaTag,4);
RR_ASSERT_SIZE(RrGpuTilePacket,16);
RR_ASSERT_OFFSET(RrGpuTilePacket,r,4);
RR_ASSERT_OFFSET(RrGpuTilePacket,x,8);
RR_ASSERT_OFFSET(RrGpuTilePacket,w,12);

/* Flat triangle: three packed signed16 X/Y pairs in wire words. */
typedef struct {
    RrGpuDmaTag tag;
    u8 r, g, b, code;
    s32 xy0, xy1, xy2;
} RrPolyF3Packet;
RR_ASSERT_SIZE(RrPolyF3Packet,20);
RR_ASSERT_OFFSET(RrPolyF3Packet,xy0,8);

/* Textured quad and four-vertex polyline DMA packets. */
typedef struct {
    RrGpuDmaTag tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad2;
} RrGpuPolyFT4Packet;
typedef struct {
    RrGpuDmaTag tag;
    u8 r, g, b, code;
    s16 x0, y0, x1, y1, x2, y2, x3, y3;
    u32 pad;
} RrGpuLineF4Packet;
RR_ASSERT_SIZE(RrGpuPolyFT4Packet,40);
RR_ASSERT_OFFSET(RrGpuPolyFT4Packet,x0,8);
RR_ASSERT_OFFSET(RrGpuPolyFT4Packet,u0,12);
RR_ASSERT_OFFSET(RrGpuPolyFT4Packet,tpage,22);
RR_ASSERT_OFFSET(RrGpuPolyFT4Packet,x3,32);
RR_ASSERT_SIZE(RrGpuLineF4Packet,28);
RR_ASSERT_OFFSET(RrGpuLineF4Packet,x0,8);
RR_ASSERT_OFFSET(RrGpuLineF4Packet,pad,24);

/* Quad primitive GPU packets (POLY_F4, POLY_G4, POLY_GT4) and mesh face records. */
typedef struct {
    u8 link[3], len;
    s32 colorCode04;
    s32 xy0, xy1, xy2, xy3;
} RrPolyF4Packet;

typedef struct {
    u8 link[3], len;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad0F;
    s16 x1, y1;
    u8 r2, g2, b2, pad17;
    s16 x2, y2;
    u8 r3, g3, b3, pad1F;
    s16 x3, y3;
} RrPolyG4Packet;

typedef struct {
    u8 link[3], len;
    s32 color04, xy0, color0C, xy1, color14, xy2, color1C, xy3;
} RrPolyG4WordPacket;

typedef struct {
    u8 link[3], len;
    s32 color04, xy0, uvClut0C, color10, xy1, uvTpage18, color1C, xy2;
    u16 uv2;
    u8 pad26[2];
    s32 color28, xy3;
    u16 uv3;
    u8 pad32[2];
} RrPolyGT4WordPacket;

typedef struct {
    u16 v0, v1, v2, v3;
    s32 color08;
    u8 pad0C, otBias0D, pad0E[2];
} RrMeshQuadF4;

typedef struct {
    u16 v0, v1, v2, v3;
    s32 color08, color0C, color10, color14;
    u8 pad18, otBias19, pad1A[2];
} RrMeshQuadG4;

typedef struct {
    u16 v0, v1, v2, v3;
    s32 color08, color0C, color10, color14, uvClut18, uvTpage1C;
    u16 uv2, uv3;
    u8 pad24, otBias25, pad26[2];
} RrMeshQuadGT4;

RR_ASSERT_SIZE(RrPolyF4Packet, 0x18);
RR_ASSERT_SIZE(RrPolyG4Packet, 0x24);
RR_ASSERT_OFFSET(RrPolyG4Packet, x0, 0x8);
RR_ASSERT_OFFSET(RrPolyG4Packet, x1, 0x10);
RR_ASSERT_OFFSET(RrPolyG4Packet, x2, 0x18);
RR_ASSERT_OFFSET(RrPolyG4Packet, x3, 0x20);
RR_ASSERT_SIZE(RrPolyG4WordPacket, 0x24);
RR_ASSERT_SIZE(RrPolyGT4WordPacket, 0x34);
RR_ASSERT_OFFSET(RrPolyGT4WordPacket, uv2, 0x24);
RR_ASSERT_OFFSET(RrPolyGT4WordPacket, uv3, 0x30);
RR_ASSERT_SIZE(RrMeshQuadF4, 0x10);
RR_ASSERT_SIZE(RrMeshQuadG4, 0x1C);
RR_ASSERT_SIZE(RrMeshQuadGT4, 0x28);

#endif
