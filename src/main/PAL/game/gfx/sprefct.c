#include "types.h"
#include "rr/sprite_effect.h"
#include "rr/textured_triangle.h"
#include "m2c_macros.h"
#include "rr/sprite_emit.h"
#include "psyq/libgpu/primatt2.h"
#include "psyq/libgpu/primatt3.h"
#include "psyq/libgpu/primatt6.h"
#include "psyq/libgpu/primattr.h"
#include "psyq/libgpu/settrans.h"

struct P { int tag; unsigned char r, g, b, code; short x, y, w, h; };

extern void SetSprt(void *);
extern void AddPrim(u32 *,u32 *);
extern s32 PsyqRand(void);
/* extern */
/* extern */
/* extern */

u8 *AddJitterSprites(u32 *ordering,u8 *cursor,RrSpriteEffectSource *descriptor_arg,s32 semi,s32 strength_arg,s32 count_arg);
struct S32 *EmitPolyFT3(u32 *arg0, struct S32 *arg1, void *arg2, void *arg3, void *arg4, void *arg5, u16 arg6, u16 arg7, u16 arg8, u16 arg9);
void *AddTileSemiTransPrim(s32 *arg0, void *arg1, s16 arg2, s16 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8);
struct P *GameQueueTileTrans(int a0, struct P *a1, short a2, short a3, int a4, int a5, int a6, int a7, int a8);
struct P *AddLineF2SemiTrans(int a0, struct P *a1, short a2, short a3, int a4, int a5, int a6, int a7, int a8);
RrPolyF3Packet *EmitPolyF3(u32 *ordering,RrPolyF3Packet *packet,s32 xy0,s32 xy1,s32 xy2,const u8 *rgb,s32 flags);

/* legacy typing of shared globals in this part of the unit */
#define AddDrawModePrim (*(u8 *(*)(u32 *,u8 *,u16))&AddDrawModePrim)

u8 *AddJitterSprites(u32 *ordering,u8 *cursor,RrSpriteEffectSource *descriptor_arg,s32 semi,s32 strength_arg,s32 count_arg) {
    /* ASM_BARRIER: retain the loop bound in callee-saved S5 across helpers. */
    register s32 count asm("$21")=count_arg;
    /* ASM_BARRIER: retain the full signed intensity in callee-saved S7. */
    register s32 strength asm("$23")=strength_arg;
    register RrSpritePacket *sprite;
    /* ASM_BARRIER: retain the source in callee-saved S2 across helpers. */
    register RrSpriteEffectSource *descriptor asm("$18");
    register s32 i;
    /* ASM_BARRIER: use A1 for jitter only until the transparency helper. */
    register s32 jitter asm("$5");
    s32 y;
    s32 level;
    /* ASM_BARRIER: capture the incoming ordering pointer in A0. */
    register u32 *incoming_order asm("$4")=ordering;
    /* ASM_BARRIER: capture the incoming cursor in A1 before the saved copy. */
    register u8 *incoming_cursor asm("$5")=cursor;
    /* ASM_BARRIER: capture the incoming source in A2 before the saved copy. */
    register RrSpriteEffectSource *incoming_descriptor asm("$6")=descriptor_arg;
    u32 *saved_ordering;
    /* ASM_BARRIER: capture stack arguments before copying the pointer arguments. */
    __asm__("" : "+r"(count), "+r"(strength), "+r"(incoming_order), "+r"(incoming_cursor), "+r"(incoming_descriptor));
    saved_ordering=incoming_order;
    /* ASM_BARRIER: finish saving the ordering pointer before the other copies. */
    __asm__("" : "+r"(incoming_cursor), "+r"(incoming_descriptor) : "r"(saved_ordering));
    sprite=(RrSpritePacket *)incoming_cursor;
    descriptor=incoming_descriptor;
    /* ASM_BARRIER: finish the source copy before initializing the loop index. */
    __asm__("" : "+r"(descriptor));
    i=0;
    while(i<count) {
        SetSprt(sprite);
        jitter=0;
        if(count!=1) {
            u32 random=(u32)PsyqRand()&7;
            register s32 three=3;
            jitter=three-random;
            /* ASM_BARRIER: keep the subtraction input distinct from its A1 result. */
            __asm__("" : "+r"(jitter) : "r"(three));
        }
        /* ASM_BARRIER: use V0 for the half-jitter value only during geometry setup. */
        { register s32 temp asm("$2")=jitter>>1;

        if(i==0)y=descriptor->y+temp;
        else if(i==1)y=descriptor->y+jitter*3;
        else if(i==2)y=descriptor->y+jitter; }
        {s16 x=descriptor->x;
        sprite->y=y;
        sprite->x=x;}
        sprite->w=descriptor->uv.w;
        sprite->h=descriptor->uv.h;
        sprite->u=descriptor->uv.u;
        {u8 v=((volatile RrSpriteEffectSource *)descriptor)->uv.v;
        s32 transparency=*(volatile s32 *)&semi;
        sprite->v=v;
        SetSemiTrans(sprite,transparency);}
        level=strength;
        if(count!=1) {
            if(i==1)level=(s32)((u32)strength*((u32)PsyqRand()&15))/24;
            else level=(s32)((u32)strength*(((u32)PsyqRand()&31)+8))/48;
        }
        /* ASM_BARRIER: move intensity to A1 after division and before the RGB stores. */
        { register s32 color asm("$5")=level;  sprite->r=color;sprite->g=color;sprite->b=color; }
        sprite->clut=descriptor->clut;
        AddPrim(saved_ordering,(u32 *)sprite++);
        {
            u32 page;
            /* ASM_BARRIER: supply the advanced cursor in A1 to the mode helper. */
            register u8 *out_cursor asm("$5");
            if(i>0)page=(u8)descriptor->page|0x60;
            else page=descriptor->page;
            out_cursor=(u8 *)sprite;
            i++;
            sprite=(RrSpritePacket *)AddDrawModePrim((u32 *)(CurrentDrawBuf+0x70),out_cursor,page);
        }
    }
    return (u8 *)sprite;
}

#undef AddDrawModePrim

/* legacy typing of shared globals in this part of the unit */
#define AddPrim ((void (*)(u32 *, u32 *))AddPrim)

/* extern */
struct S32 *EmitPolyFT3(u32 *arg0, struct S32 *arg1, void *arg2, void *arg3, void *arg4, void *arg5, u16 arg6, u16 arg7, u16 arg8, u16 arg9) {
    SetPolyFT3(arg1);
    M2C_FIELD(arg1, u16 *, 8) = (u16) M2C_FIELD(arg2, u16 *, 0);
    M2C_FIELD(arg1, u16 *, 0xA) = (u16) M2C_FIELD(arg3, u16 *, 0);
    M2C_FIELD(arg1, u16 *, 0x10) = (u16) M2C_FIELD(arg2, u16 *, 4);
    M2C_FIELD(arg1, u16 *, 0x12) = (u16) M2C_FIELD(arg3, u16 *, 4);
    M2C_FIELD(arg1, u16 *, 0x18) = (u16) M2C_FIELD(arg2, u16 *, 8);
    M2C_FIELD(arg1, u16 *, 0x1A) = (u16) M2C_FIELD(arg3, u16 *, 8);
    M2C_FIELD(arg1, u8 *, 0xC) = (u8) M2C_FIELD(arg4, u8 *, 0);
    M2C_FIELD(arg1, u8 *, 0xD) = (u8) M2C_FIELD(arg5, u8 *, 0);
    M2C_FIELD(arg1, u8 *, 0x14) = (u8) M2C_FIELD(arg4, u8 *, 4);
    M2C_FIELD(arg1, u8 *, 0x15) = (u8) M2C_FIELD(arg5, u8 *, 4);
    M2C_FIELD(arg1, u8 *, 0x1C) = (u8) M2C_FIELD(arg4, u8 *, 8);
    M2C_FIELD(arg1, u8 *, 0x1D) = (u8) M2C_FIELD(arg5, u8 *, 8);
    SetSemiTrans(arg1, arg8 & 0xFFFF);
    if ((arg9 & 0xFFFF) != 0x80) {
        SetShadeTex(arg1, 0);
        M2C_FIELD(arg1, s8 *, 4) = (s8) arg9;
        M2C_FIELD(arg1, s8 *, 5) = (s8) arg9;
        M2C_FIELD(arg1, s8 *, 6) = (s8) arg9;
    } else {
        SetShadeTex(arg1, 1);
    }
    M2C_FIELD(arg1, u16 *, 0x16) = arg6;
    M2C_FIELD(arg1, u16 *, 0xE) = arg7;
    AddPrim(arg0, (u32 *)arg1++);
    return arg1;
}

#undef AddPrim

void *AddTileSemiTransPrim(s32 *arg0, void *arg1, s16 arg2, s16 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8) {
    M2C_FIELD(arg1, s8 *, 3) = 3;
    M2C_FIELD(arg1, s16 *, 8) = arg2;
    M2C_FIELD(arg1, s16 *, 0xC) = (s16) arg4;
    M2C_FIELD(arg1, s8 *, 7) = 0x62;
    M2C_FIELD(arg1, s16 *, 0xA) = arg3;
    M2C_FIELD(arg1, s16 *, 0xE) = (s16) arg5;
    M2C_FIELD(arg1, s8 *, 4) = (s8) arg6;
    M2C_FIELD(arg1, s8 *, 5) = (s8) arg7;
    M2C_FIELD(arg1, s8 *, 6) = (s8) arg8;
    M2C_FIELD(arg1, s32 *, 0) = (s32) ((M2C_FIELD(arg1, s32 *, 0) & 0xFF000000) | (*arg0 & 0xFFFFFF));
    *arg0 = (*arg0 & 0xFF000000) | ((s32) arg1 & 0xFFFFFF);
    return arg1 + 0x10;
}

/* legacy typing of shared globals in this part of the unit */
#define AddPrim ((int (*)())AddPrim)
#define SetShadeTex ((int (*)())SetShadeTex)

struct P *GameQueueTileTrans(int a0, struct P *a1, short a2, short a3, int a4, int a5, int a6, int a7, int a8) {
    SetLineF2(a1);
    SetShadeTex(a1, 1);
    a1->x = a2;
    a1->y = a3;
    a1->w = a4;
    a1->h = a5;
    a1->r = a6;
    a1->g = a7;
    a1->b = a8;
    AddPrim(a0, a1++);
    return a1;
}

#undef AddPrim
#undef SetShadeTex

/* legacy typing of shared globals in this part of the unit */
#define AddPrim ((int (*)())AddPrim)
#define SetShadeTex ((int (*)())SetShadeTex)

struct P *AddLineF2SemiTrans(int a0, struct P *a1, short a2, short a3, int a4, int a5, int a6, int a7, int a8) {
    SetLineF2(a1);
    SetShadeTex(a1, 1);
    a1->x = a2;
    a1->y = a3;
    a1->code |= 2;
    a1->w = a4;
    a1->h = a5;
    a1->r = a6;
    a1->g = a7;
    a1->b = a8;
    AddPrim(a0, a1++);
    return a1;
}

#undef AddPrim
#undef SetShadeTex

/* legacy typing of shared globals in this part of the unit */
#define AddPrim ((void (*)(u32 *,u32 *))AddPrim)

RrPolyF3Packet *EmitPolyF3(u32 *ordering,RrPolyF3Packet *packet,s32 xy0,s32 xy1,s32 xy2,const u8 *rgb,s32 flags){
 SetPolyF3(packet);
 SetSemiTrans(packet,flags);
 packet->xy0=xy0;
 packet->xy1=xy1;
 packet->xy2=xy2;
 packet->r=rgb[0];
 packet->g=rgb[1];
 packet->b=rgb[2];
 AddPrim(ordering,(u32*)packet++);
 return packet;
}

#undef AddPrim
