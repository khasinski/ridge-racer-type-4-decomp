#include "types.h"

/* Drawing-environment descriptor prefix (clip..draw packet), 0x5C bytes. */
typedef struct {
    s16 clip_x, clip_y, clip_w, clip_h;
    s16 offset_x, offset_y;
    s16 window_x, window_y, window_w, window_h;
    u16 page;
    u8 dither, draw_display, rgb24, r, g, b;
    u32 draw_packet1C[16];
} RrStrDrawEnvironment;

/* Semi-transparent flat rectangle (GPU command 0x62). */
typedef struct {
    u32 tag;
    u8 r, g, b, code;
    s16 x, y;
    s16 w, h;
} RrStrFadeTile;

typedef struct {
    u8 pad00[0x70];
    u32 ordering70[704];
} RrStrFrameStorage;

extern RrStrFrameStorage *CurrentDrawBuf;
extern s32 DrawBufIndex;
void *SetDefDrawEnv(void *environment, s32 x, s32 y, s32 w, s32 h);
void SetDrawEnv(u32 *output, void *environment);
void *AddDrawModePrim(u32 *ordering, void *cursor, u16 mode);

typedef struct {
    u32 addr : 24;
    u32 len : 8;
    u8 r0, g0, b0, code;
} RrStrPrimTag;

#define RR_STR_SET_ADDR(p, a) (((RrStrPrimTag *)(p))->addr = (u32)(a))
#define RR_STR_GET_ADDR(p) ((u32)((RrStrPrimTag *)(p))->addr)

#define RR_STR_SCRATCH_CURSOR (*(u32 **)0x1F800000)

/* Queues the letterbox drawing-environment / fade packets for one pass. */
void movie_playback_func_80115BD0(s32 mode) {
    RrStrDrawEnvironment env;
    u32 *packet;
    u32 *ot;
    RrStrFadeTile *tile;

    switch (mode) {
    case 0:
        SetDefDrawEnv(&env, 0, (DrawBufIndex << 8) | 40, 320, 160);
        packet = RR_STR_SCRATCH_CURSOR;
        SetDrawEnv(packet, &env);
        ot = &CurrentDrawBuf->ordering70[703];
        *packet = (*packet & 0xFF000000) | (*ot & 0x00FFFFFF);
        *ot = (*ot & 0xFF000000) | ((u32)packet & 0x00FFFFFF);
        packet += 16;
        RR_STR_SCRATCH_CURSOR = packet;
        break;
    case 1:
        SetDefDrawEnv(&env, 0, DrawBufIndex << 8, 320, 256);
        packet = RR_STR_SCRATCH_CURSOR;
        SetDrawEnv(packet, &env);
        ot = &CurrentDrawBuf->ordering70[0];
        *packet = (*packet & 0xFF000000) | (*ot & 0x00FFFFFF);
        *ot = (*ot & 0xFF000000) | ((u32)packet & 0x00FFFFFF);
        packet += 16;
        RR_STR_SCRATCH_CURSOR = packet;
        break;
    case 2:
        tile = (RrStrFadeTile *)RR_STR_SCRATCH_CURSOR;
        ((RrStrPrimTag *)tile)->len = 3;
        tile->code = 0x62;
        tile->y = 40;
        tile->w = 320;
        tile->h = 160;
        tile->r = 24;
        tile->g = 24;
        tile->b = 12;
        tile->x = 0;
        ot = &CurrentDrawBuf->ordering70[0];
        RR_STR_SET_ADDR(tile, RR_STR_GET_ADDR(ot));
        *ot = (*ot & 0xFF000000) | ((u32)tile & 0x00FFFFFF);
        tile++;
        RR_STR_SCRATCH_CURSOR = (u32 *)tile;
        RR_STR_SCRATCH_CURSOR = AddDrawModePrim(ot, tile, 37);
        break;
    case 3:
        SetDefDrawEnv(&env, 0, (DrawBufIndex << 8) | 80, 320, 160);
        packet = RR_STR_SCRATCH_CURSOR;
        SetDrawEnv(packet, &env);
        ot = &CurrentDrawBuf->ordering70[0];
        *packet = (*packet & 0xFF000000) | (*ot & 0x00FFFFFF);
        *ot = (*ot & 0xFF000000) | ((u32)packet & 0x00FFFFFF);
        packet += 16;
        RR_STR_SCRATCH_CURSOR = packet;
        break;
    }
}
