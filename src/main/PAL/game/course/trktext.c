#include "rr/gpu_address.h"
#include "game/system/pad.h"
#include "types.h"
#include "rr/scene_camera_state.h"
#include "rr/frame_storage_types.h"
#include "scratchpad.h"
#include "rr/track_lateral.h"
#include "rr/text_sprites.h"
#include "rr/gpu.h"
#include "rr/tables.h"
#include "game/course/trktext.h"
#include "rr/bios_devices.h"
#include "rr/controller_hud.h"
#include "rr/vehicle_drive.h"

typedef struct { short a[8], b[8], c[8]; } U;

typedef struct {
    u32 otIndex00;
    u32 shadeTex04;
    u32 semiTrans08;
    u16 tpageMode0C;
    u16 unk0E;
    u8 r10, g11, b12, unk13;
    u16 x14, y16, w18, h1A;
    u8 u1C, unk1D, v1E, unk1F;
    u8 uw20, unk21, vh22, unk23;
    u16 tpageX24, tpageY26;
    u16 clutX28, clutY2A;
} TexturedQuadDesc2C;

typedef struct {
    u16 x, y, w, h;
} RectU16;

typedef struct {
    u8 r, g, b;
} ColorRGB8;

extern U PadButtonMap[];
extern unsigned char FontGlyphMap[];

s32 GetTrackNodeHeading(s32 node, s32 lane);
void DrawFontString(s32 x, s32 y, const u8 *text, s32 color, s32 bank);
int DrawFontStringCentered(int a0, int a1, unsigned char *s, int a3, int a4);
int DrawFontStringCenteredOffset(int a0, int a1, unsigned char *s, int a3, int a4);
void DrawFontStringRight(int a0, int a1, unsigned char *s, int a3, int a4);
void *EmitTexturedQuadPrim(TexturedQuadDesc2C *a0);
void InitTexturedQuadDesc(TexturedQuadDesc2C *p);
void AddHGradientRectPrim(RectU16 *arg0, ColorRGB8 *arg1, ColorRGB8 *arg2, s32 arg3);
void AddSlantedGradientRect(const GpuRectangleFields *rectangle, s32 reverse, const GpuColorCommand *top, const GpuColorCommand *bottom, s32 depth);

int GetMappedPadButtons(int i) {
    ControllerState5C *t = &PlayerControllers[i];
    switch (t->input00.fields.type) {
    case 0x41:
    case 0x73:
    default:
        return PlayerControllers[i].edges08.fields.pressed & PadButtonMap[i].a[6];
    case 0x23:
        return t->edges08.fields.pressed & PadButtonMap[i].b[6];
    case 0xE3:
        return t->edges08.fields.pressed & PadButtonMap[i].c[6];
    }
}

s32 GetTrackNodeHeading(s32 node, s32 lane)
{
    int delta[6];
    int previous;
    TrackLaneOffset8 *currentOffset, *previousOffset;
    int previousOffsetBytes, currentOffsetBytes;
    TrackSample3C *previousSample, *currentSample;
    previous = (node - 1) % TrackNodeCount;
    if (previous < 0) previous += TrackNodeCount;
    switch (lane) {
    case 0:
        currentOffset = RaceContext.laneOffsets54C + node;
        previousOffset = RaceContext.laneOffsets54C + previous;
        previousOffsetBytes = previous * sizeof(TrackSample3C);
        currentOffsetBytes = node * sizeof(TrackSample3C);
        previousSample = (TrackSample3C *)(previousOffsetBytes + (unsigned long)TrackNodes);
        currentSample = (TrackSample3C *)(currentOffsetBytes + (unsigned long)TrackNodes);
        delta[2] = previousSample->road.x + previousOffset->x - (currentSample->road.x + currentOffset->x);
        delta[3] = previousSample->road.z + previousOffset->z - (currentSample->road.z + currentOffset->z);
        break;
    case 10:
        delta[2] = TrackNodes[previous].road.x - TrackNodes[node].road.x;
        delta[3] = TrackNodes[previous].road.z - TrackNodes[node].road.z;
        break;
    }
    return (s16)ratan2(delta[3], delta[2]);
}

void DrawFontString(s32 x, s32 y, const u8 *text, s32 color, s32 bank) {
    SpritePacket *packet;
    u32 current_x;
    s32 glyph;
    s32 width;
    const u8 *width_entry;
    FrameStorage *context;
    u32 *head;

    /* ASM_BARRIER: retain the biased packet-field cursor in T0. */
    register u8 *fields asm("$8");
    s32 row;
    s32 v;
    /* ASM_BARRIER: retain Y in S1 while A1 becomes glyph and width address. */
    register s32 yy asm("$17");
    s32 u;
    const u8 *map, *widths;
    s32 missing, command;
    u32 low, high;
    const u8 *lookup;
    packet = (SpritePacket *)((GpuScratchPointers *)SCRATCHPAD)->cursor;
    current_x = x;
    yy = y;
if (*text != 0) {
        map = FontAtlasRowTable + 32;
        missing = 255;
        command = 0x66;
        widths = FontGlyphWidths;
        low = 0xFFFFFF;
        high = 0xFF000000;
        fields = (u8 *)packet + 14;
        do {
            lookup = (const u8 *)((unsigned long)*text + (unsigned long)map);
            glyph = lookup[-32];
            text++;
            if (glyph != missing) {
                fields[-11] = 4;
                fields[-7] = 0x64;
                row = glyph / 21;
                u = (glyph % 21) * 12;
                v = row * 12 + 188;
                if (color == missing) {
                    fields[-7] = 0x65;
                } else {
                    fields[-7] = command;
                    fields[-10] = color;
                    fields[-9] = color;
                    fields[-8] = color;
                }
                width_entry = (const u8 *)(glyph + (unsigned long)widths);
                ((s16 *)fields)[-3] = current_x;
                ((s16 *)fields)[-2] = yy;
                width = *width_entry;
                fields[-2] = u;
                fields[-1] = v;
                ((s16 *)fields)[2] = 12;
                *(s16 *)fields = 0x7F78;
                context = CurrentDrawBuf;
                head = (u32 *)((bank * 4) + (unsigned long)context + 0x70);
                ((s16 *)fields)[1] = width;
                fields += 20;
                *(u32 *)packet = (*(u32 *)packet & high) | (*head & low);
                *head = (*head & high) | ((unsigned long)packet & low);
                packet++;
                current_x = (current_x + *width_entry);
            } else {
                current_x = (current_x + 4U);
            }
        } while (*text != 0);
    }
    ((GpuScratchPointers *)SCRATCHPAD)->cursor = (u8 *)packet;
}

int DrawFontStringCentered(int a0, int a1, unsigned char *s, int a3, int a4) {
    int sum = 0;
    unsigned char *s0 = s;
    if (*s0 != 0) {
        do {
            unsigned char c = FontGlyphMap[*s - 0x20];
            s++;
            if (c != 0xFF) {
                sum += FontGlyphWidths[c];
            } else {
                sum += 4;
            }
        } while (*s != 0);
    }
    DrawFontString(0xA0 - sum / 2, a1, s0, a3, a4);
    return sum;
}

int DrawFontStringCenteredOffset(int a0, int a1, unsigned char *s, int a3, int a4) {
    int sum = 0;
    unsigned char *s0 = s;
    if (*s0 != 0) {
        do {
            unsigned char c = FontGlyphMap[*s - 0x20];
            s++;
            if (c != 0xFF) {
                sum += FontGlyphWidths[c];
            } else {
                sum += 4;
            }
        } while (*s != 0);
    }
    DrawFontString(a0 + (0xA0 - sum / 2), a1, s0, a3, a4);
    return sum;
}

void DrawFontStringRight(int a0, int a1, unsigned char *s, int a3, int a4) {
    int sum = 0;
    unsigned char *s0 = s;
    if (*s0 != 0) {
        do {
            unsigned char c = FontGlyphMap[*s - 0x20];
            s++;
            if (c != 0xFF) {
                sum += FontGlyphWidths[c];
            } else {
                sum += 4;
            }
        } while (*s != 0);
    }
    DrawFontString(a0 - sum, a1, s0, a3, a4);
}

void AddSetTexpagePrim(int a0) {
    GpuCommandPacket *p = (GpuCommandPacket *)((GpuScratchPointers *)SCRATCHPAD)->cursor;
    int *q;
    p->tag.bytes.length = 1;
    p->command = 0xE100023E;
    q = (int *)(a0 * 4 + (int)CurrentDrawBuf);
    p->tag.word = (p->tag.word & 0xFF000000) | (q[0x70/4] & 0xFFFFFF);
    q[0x70/4] = (q[0x70/4] & 0xFF000000) | ((int)p & 0xFFFFFF);
    p++;
    ((GpuScratchPointers *)SCRATCHPAD)->cursor = (u8 *)p;
}

void *EmitTexturedQuadPrim(TexturedQuadDesc2C *a0)
{
    u16 *new_var;
    TexturedQuadDesc2C *new_var2;
    int new_var3;
    GpuPolyFT4Packet *p = *(GpuPolyFT4Packet **)SCRATCHPAD;
    u16 a1;
    char *d;
    int dd;
    p->tag.bytes.length = 9;
    p->code = 0x2C;
    if (a0->semiTrans08 != 0) {
        p->code |= 2;
    } else {
        p->code &= ~2;
    }
    if (a0->shadeTex04 != 0) {
        p->code |= 1;
    } else {
        p->code &= ~1;
    }
    p->r0 = a0->r10;
    p->g0 = a0->g11;
    p->b0 = a0->b12;
    p->x0 = a0->x14;
    new_var2 = a0;
    new_var3 = 1;
    p->y0 = a0->y16;
    p->x1 = (u16)((new_var2->x14 + new_var3) + a0->w18);
    p->y1 = a0->y16;
    p->x2 = a0->x14;
    a1 = 1;
    p->y2 = (u16)((a0->y16 + a1) + a0->h1A);
    p->x3 = (u16)(((short)a0->x14 + 1) + a0->w18);
    new_var = &a0->y16;
    p->y3 = (u16)((*new_var + a1) + a0->h1A);
    p->u0 = a0->u1C;
    p->v0 = a0->v1E;
    p->u1 = a0->u1C + a0->uw20;
    p->v1 = a0->v1E;
    p->u2 = a0->u1C;
    p->v2 = a0->v1E + a0->vh22;
    p->u3 = a0->u1C + a0->uw20;
    p->v3 = a0->v1E + a0->vh22;
    a1 = a0->tpageY26;
    p->tpage = (((a0->tpageMode0C & 3) << 7) | (((a1 & 0x100) << 16) >> 20) | ((a0->tpageX24 & 0x3FF) >> 6)) | ((a1 & 0x200) * 4);
    p->clut = (a0->clutY2A << 6) | ((a0->clutX28 >> 4) & 0x3F);
    dd = CurrentDrawBuf;
    p->tag.word = (p->tag.word & 0xFF000000) | (((u32 *)((a0->otIndex00 * 4) + dd))[0x70 / 4] & 0xFFFFFF);
    d = (char *)((a0->otIndex00 * 4) + dd);
    *(u32 *)(d + RR_FIELD_OFFSET(FrameStorage, buffer.ordering70)) = (((u32 *)d)[0x70 / 4] & 0xFF000000) | ((u32)p & 0xFFFFFF);
    p++;
    *(GpuPolyFT4Packet **)SCRATCHPAD = p;
    return p;
}

void InitTexturedQuadDesc(TexturedQuadDesc2C *p) {
    PsyqBzero((char *)p, sizeof(*p));
    p->shadeTex04 = 1;
    p->r10 = 0x80;
    p->g11 = 0x80;
    p->b12 = 0x80;
}

void AddHGradientRectPrim(RectU16 *arg0, ColorRGB8 *arg1, ColorRGB8 *arg2, s32 arg3) {
    s32 *temp_a3;
    PolyG4Packet *temp_t0;

    temp_t0 = *(PolyG4Packet **)SCRATCHPAD;
    temp_t0->len = 8;
    temp_t0->code = 0x38;
    temp_t0->x0 = arg0->x;
    temp_t0->y0 = arg0->y;
    temp_t0->x1 = arg0->x + arg0->w;
    temp_t0->y1 = arg0->y;
    temp_t0->x2 = arg0->x;
    temp_t0->y2 = arg0->y + arg0->h;
    temp_t0->x3 = arg0->x + arg0->w;
    temp_t0->y3 = arg0->y + arg0->h;
    temp_t0->r0 = arg1->r;
    temp_t0->g0 = arg1->g;
    temp_t0->b0 = arg1->b;
    temp_t0->r1 = arg2->r;
    temp_t0->g1 = arg2->g;
    temp_t0->b1 = arg2->b;
    temp_t0->r2 = arg1->r;
    temp_t0->g2 = arg1->g;
    temp_t0->b2 = arg1->b;
    temp_t0->r3 = arg2->r;
    temp_t0->g3 = arg2->g;
    temp_t0->b3 = arg2->b;
    temp_a3 = (void *)((arg3 * 4) + (s32)CurrentDrawBuf);
    *(s32 *)temp_t0 = (*(s32 *)temp_t0 & 0xFF000000) | (temp_a3[28] & 0xFFFFFF);
    *(s32 *)((s8 *)temp_a3 + RR_FIELD_OFFSET(FrameStorage, buffer.ordering70)) = (temp_a3[28] & 0xFF000000) | ((s32)temp_t0 & 0xFFFFFF);
    *(PolyG4Packet **)SCRATCHPAD = temp_t0 + 1;
}

void AddSlantedGradientRect(const GpuRectangleFields *rectangle, s32 reverse, const GpuColorCommand *top, const GpuColorCommand *bottom, s32 depth) {
    PolyG4Packet *p = *(PolyG4Packet **)SCRATCHPAD;
    int *a;
    p->len = 8;
    p->code = 0x3A;
    if (reverse == 0) {
        p->x0 = (u16)rectangle->x;
        p->y0 = (u16)rectangle->y;
        p->x1 = (u16)rectangle->x + (u16)rectangle->w;
        p->y1 = (u16)rectangle->y + (short)(u16)rectangle->h / 2;
        p->x2 = (u16)rectangle->x;
        p->y2 = (u16)rectangle->y + (u16)rectangle->h;
        p->x3 = (u16)rectangle->x + (u16)rectangle->w;
        p->y3 = (u16)rectangle->y + (u16)rectangle->h;
    } else {
        p->x0 = (u16)rectangle->x;
        p->y0 = (u16)rectangle->y;
        p->x1 = (u16)rectangle->x + (u16)rectangle->w;
        p->y1 = (u16)rectangle->y;
        p->x2 = (u16)rectangle->x;
        p->y2 = (u16)rectangle->y + (u16)rectangle->h;
        p->x3 = (u16)rectangle->x + (u16)rectangle->w;
        p->y3 = (u16)rectangle->y + (short)(u16)rectangle->h / 2;
    }
    p->r0 = top->r;
    p->g0 = top->g;
    p->b0 = top->b;
    p->r1 = top->r;
    p->g1 = top->g;
    p->b1 = top->b;
    p->r2 = bottom->r;
    p->g2 = bottom->g;
    p->b2 = bottom->b;
    p->r3 = bottom->r;
    p->g3 = bottom->g;
    p->b3 = bottom->b;
    a = (void *)(depth * 4 + (unsigned long)CurrentDrawBuf);
    *(int *)p = (*(int *)p & 0xFF000000) | (a[0x70 / 4] & 0xFFFFFF);
    *(int *)((char *)a + RR_FIELD_OFFSET(FrameStorage, buffer.ordering70)) = (a[0x70 / 4] & 0xFF000000) | (GpuDmaEncodeAddress(p) & 0xFFFFFF);
    p++;
    *(PolyG4Packet **)SCRATCHPAD = p;
}
