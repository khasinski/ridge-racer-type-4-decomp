#include "rr/fade_sprite.h"

extern RrSpritePacket *AddSpritePrim(u32 *, RrSpritePacket *, s32, s32,
                                  RrUiUvRect *, u16, s32);

void EmitFadeSprite(s32 index, s32 x, s32 y, s32 intensity) {
    register s32 xstage = x;
    register s32 ystage = y;
    register s32 istage = intensity;
    register s32 phase;
    /* ASM_BARRIER: capture the phase in v1, then end its hard binding before
     * the arithmetic temporaries reuse this register. */
    register s32 captured __asm__("$3");
    register s32 selector;
    register s32 scale;
    /* ASM_BARRIER: keep the scratchpad pointer in s0 through the sprite helper. */
    register RrGpuScratchPointers *scratch __asm__("$16");
    register RrFadeSpriteContext **slot;
    /* ASM_BARRIER: the input cursor dies at the helper call; its returned
     * packet starts a separate t0 lifetime after that call. */
    register RrGpuSingleCommandPacket *p __asm__("$8");
    register RrSpritePacket *cursor __asm__("$8");
    register RrFadeSpriteContext *context;
    register u32 command;
    /* ASM_BARRIER: use a1 for the low-address mask only after the helper. */
    register u32 low __asm__("$5");
    register u32 high;
    register u32 *modes;
    register u32 mode;
    register u32 length;
    register u32 tag;
    register s32 product;
    /* ASM_BARRIER: prepare the outgoing head address in a0. */
    register u32 *ordering __asm__("$4");
    register u32 compared;

    /* ASM_BARRIER: preserve coordinates/intensity before phase capture. */
    __asm__("" : "+r"(xstage), "+r"(ystage), "+r"(istage));
    compared = (u32)index ^ 4U;
    captured = FadeSpritePhase;
    phase = captured;
    selector = compared == 0;
    if (phase >= 128) {
        selector += 2;
        scale = (s32)((u32)phase - 127U);
    } else {
        product = 128;
        scale = (s32)((u32)product - (u32)phase);
    }
    scale = (s32)((u32)scale * (u32)istage) / 256;
    product = (s32)((u32)scale * (u32)FadeSpriteIntensityScale);
    scale = product >> 8;
    if (product < 0) {
        scale = (s32)((u32)product + 255U) >> 8;
    }
    scratch = (RrGpuScratchPointers *)0x1F800000U;
    if (scale > 0) {
        cursor = (RrSpritePacket *)scratch->cursor;
        slot = &CurrentDrawBuf;
        p = (RrGpuSingleCommandPacket *)AddSpritePrim(
            ({
                ordering = &(*slot)->head98;
                /* ASM_BARRIER: finish a0 within argument evaluation, leaving
                 * the independent cursor move for the call delay slot. */
                __asm__("" : "+r"(ordering));
                ordering;
            }),
            cursor, xstage, ystage, &FadeSpriteRects[index], 0x7F38,
            (s32)(((u32)scale << 1) | 1U));
        command = 0xE1000200U;
        low = 0xFFFFFFU;
        modes = FadeSpriteTpageModes;
        mode = (u32)selector * 4U;
        mode = *(u32 *)((u8 *)modes + mode);
        /* ASM_BARRIER: capture the mode word before publishing the packet. */
        __asm__("" : "+r"(mode));
        length = 1;
        scratch->cursor = (u8 *)p;
        ((u8 *)p)[3] = length;
        tag = p->tag;
        /* ASM_BARRIER: capture the tag before storing the command word. */
        __asm__("" : "+r"(tag), "+r"(mode));
        mode &= 0x9FFU;
        mode |= command;
        context = *slot;
        high = 0xFF000000U;
        p->command = mode;
        tag = (tag & high) | (context->head98 & low);
        p->tag = tag;
        context->head98 = (context->head98 & high) |
                          ((u32)(unsigned long)p & low);
        scratch->cursor = (u8 *)p + sizeof(*p);
        /* ASM_BARRIER: keep the returned packet live until cursor publication. */
        __asm__("" : : "r"(p));
    }
}
