#include "scratchpad.h"
#include "rr/gpu_scratch_types.h"
#include "rr_asm.h"
#include "rr/sprite_preset.h"
#include "game/effect/spfade.h"

struct Ot { int x; int next; };

struct Pk { char pad[3]; char len; int a, b, c, d; };

extern short D_8009E874[];

void RampValueToLimit(unsigned short *a0, short *a1, int a2);
void RampValuePairTo4096(unsigned short *a0, unsigned short *a1, short *a2);
void RampValueTo4096(unsigned short *a0, short *a1);
void func_8001B8B0(s32 intensity, s32 scale);
void EmitPresetSprite(s32 intensity,s32 index);
void EmitFadeTile(s32 intensity, s32 scale, u32 pos, s32 width);
void func_8001BC84(int a0, int a1);

void RampValueToLimit(unsigned short *a0, short *a1, int a2) {
    int v = *a1;
    unsigned short u = *(unsigned short *)a1;
    if (v > 0) {
        *a0 += u;
        if ((short)*a0 >= a2) {
            *a0 = a2;
            *a1 = 0;
        }
    } else if (v < 0) {
        *a0 += u;
        if ((short)*a0 <= 0) {
            *a0 = 0;
            *a1 = 0;
        }
    }
}

void RampValuePairTo4096(unsigned short *a0, unsigned short *a1, short *a2) {
    int v = *a2;
    unsigned short u = *(unsigned short *)a2;
    if (v > 0) {
        *a1 += u;
        if ((short)*a1 >= 0x1000) {
            *a1 = 0x1000;
            *a2 = 0;
        }
    } else if (v < 0) {
        *a0 += u;
        if ((short)*a0 <= 0) {
            *a0 = 0;
            *a2 = 0;
        }
    }
}

void RampValueTo4096(unsigned short *a0, short *a1) {
    int v = *a1;
    unsigned short u = *(unsigned short *)a1;
    if (v > 0) {
        *a0 += u;
        if ((short)*a0 >= 0x1000) {
            *a0 = 0x1000;
            *a1 = 0;
        }
    } else if (v < 0) {
        *a0 += u;
        if ((short)*a0 <= 0) {
            *a0 = 0;
            *a1 = 0;
        }
    }
}

void func_8001B8B0(s32 intensity, s32 scale) {
    /* ASM_BARRIER: keep value/y1/uv2 in $a2 */
    register u32 r_a2 __asm__("$6") = intensity;
    s32 clamped;
    /* ASM_BARRIER: keep hi/y2 in $a0 */
    register u32 r_a0 __asm__("$4");
    u32 middle;
    /* ASM_BARRIER: keep GTE shift/color param in $t7 */
    register s32 r_t7 __asm__("$15");
    /* ASM_BARRIER: keep interpolated GTE IR1 result in $t6 */
    register s32 r_t6 __asm__("$14");
    /* ASM_BARRIER: keep 3*r_t6 in $t3 */
    register s32 r_t3 __asm__("$11");
    /* ASM_BARRIER: keep 0x70 constant in $t4 */
    register s32 r_t4 __asm__("$12");
    /* ASM_BARRIER: keep x offset in $t5 */
    register u32 r_t5 __asm__("$13");
    /* ASM_BARRIER: keep y offset / uv1 in $t0 */
    register u32 r_t0 __asm__("$8");
    /* ASM_BARRIER: keep size constant 0x6C0020 in $t2 */
    register u32 r_t2 __asm__("$10");
    /* ASM_BARRIER: keep scratch pointer in $a1 */
    register RrGpuScratchPointers *scratch __asm__("$5");
    /* ASM_BARRIER: keep sprite command word in $a3 */
    register u32 r_a3 __asm__("$7");
    /* ASM_BARRIER: keep packet cursor in $v0 */
    register u8 *packet __asm__("$2");
    /* ASM_BARRIER: keep tag size 4 in $t1 */
    register u32 r_t1 __asm__("$9");
    /* ASM_BARRIER: keep scratch temp in $v1 */
    register u32 r_v1 __asm__("$3");

    if ((s32)r_a2 != 0) {
        if ((s32)r_a2 >= 0) {
            clamped = (s32)r_a2 < 256;
            if (clamped) goto low;
            clamped = 255;
        } else {
low:
            clamped = r_a2;
            if ((s32)r_a2 < 0) clamped = 0;
        }
        r_a2 = clamped;
        r_a0 = r_a2 << 16;
        middle = r_a2 << 8;
        middle |= 0x66000000U;
        r_a0 |= middle;
        r_a2 |= r_a0;

        /* ASM_BARRIER: load GTE IR0 with scale */
        __asm__ volatile ("mtc2 %0, $8" : : "r"(scale));
        r_t7 = 0;
        /* ASM_BARRIER: load GTE IR1 with 0 */
        __asm__ volatile ("mtc2 %0, $9" : : "r"(r_t7));
        r_t7 = 0x10;
        /* ASM_BARRIER: load GTE RFC with 0x10 */
        __asm__ volatile ("ctc2 %0, $21" : : "r"(r_t7));
        RR_GTE_INTPL();
        /* ASM_BARRIER: read interpolated GTE IR1 result */
        __asm__ volatile ("mfc2 %0, $9" : "=r"(r_t6));
        /* ASM_BARRIER: mfc2 load delay slot before reading $t6 */
        RR_NOP_DEP_ON(r_t6);

        r_t3 = (r_t6 << 1) + r_t6;
        r_t4 = 0x70;
        r_t5 = r_t4 - r_t3;
        r_t0 = r_t6 + 0x88;
        r_t2 = 0x006C0020U;
        scratch = (RrGpuScratchPointers *)SCRATCHPAD;
        /* ASM_BARRIER: stage scratch pointer at 8001B938 */
        __asm__("" : "+r"(scratch));
        r_a3 = r_a2 | 0x64000000U;
        r_a2 = r_t0 << r_t7;
        r_v1 = r_a2 | r_t5;
        /* ASM_BARRIER: compute r_a2 | r_t5 into $v1 before loading cursor */
        __asm__ volatile("" : "+r"(r_v1));
        packet = scratch->cursor;
        r_t0 = 0x1BA40000U;

        *(u32 *)(packet + 4) = r_a3;
        /* ASM_BARRIER: keep packet+4 store before packet+8 store */
        __asm__ volatile("");
        *(u32 *)(packet + 8) = r_v1;
        *(u32 *)(packet + 12) = r_t0;
        *(u32 *)(packet + 16) = r_t2;
        r_v1 = (u32)(unsigned long)scratch->order;
        /* ASM_BARRIER: separate scratch->order load from order[1] load so delay slot stays nop */
        __asm__ volatile("" : "+r"(r_v1));
        r_v1 = ((u32 *)(unsigned long)r_v1)[1];
        r_t1 = 4;
        *(u32 *)packet = r_v1;
        /* ASM_BARRIER: schedule 0x90 load after packet[0] store */
        __asm__ volatile("" : : "r"(r_v1));
        r_v1 = 0x90;
        /* ASM_BARRIER: materialize 0x90 in $v1 before packet[3] store */
        __asm__("" : "+r"(r_v1));
        packet[3] = r_t1;
        r_a0 = (u32)(unsigned long)scratch->order;
        r_t5 = r_v1 - r_t6;
        ((u32 *)(unsigned long)r_a0)[1] = (u32)(unsigned long)packet;
        ((u8 *)&scratch->order[1])[3] = 0;
        packet += 0x14;

        *(u32 *)(packet + 4) = r_a3;
        *(u32 *)(packet + 8) = r_a2 | r_t5;
        /* ASM_BARRIER: keep r_t5 live so r_a2 | r_t5 writes to $v1 */
        __asm__("" : : "r"(r_t5));
        *(u32 *)(packet + 12) = r_t0;
        *(u32 *)(packet + 16) = r_t2;
        *(u32 *)packet = scratch->order[1];
        packet[3] = r_t1;
        scratch->order[1] = (u32)(unsigned long)packet;
        ((u8 *)&scratch->order[1])[3] = 0;
        packet += 0x14;

        r_t5 = r_t6 + 0xB0;
        r_a2 |= r_t5;
        r_t5 = r_t4 - r_t6;
        /* ASM_BARRIER: keep 0x88 load in $v1 after packet 2 order stores */
        __asm__ volatile("" : : "r"(r_t5));
        r_v1 = 0x88;
        /* ASM_BARRIER: stage 0x88 in $v1 before packet+12 store */
        __asm__("" : "+r"(r_v1));
        *(u32 *)(packet + 12) = r_t0;
        r_t0 = r_v1 - r_t6;
        /* ASM_BARRIER: compute 0x88 - r_t6 immediately after packet+12 store */
        __asm__("" : "+r"(r_t0));
        *(u32 *)(packet + 4) = r_a3;
        *(u32 *)(packet + 8) = r_a2;
        *(u32 *)(packet + 16) = r_t2;
        /* ASM_BARRIER: keep packet+4/8/16 stores before scratch->order load and 0x1BA40020 load */
        __asm__ volatile("");
        r_a0 = (u32)(unsigned long)scratch->order;
        r_v1 = ((u32 *)(unsigned long)r_a0)[1];
        r_a2 = 0x1BA40020U;
        *(u32 *)packet = r_v1;
        packet[3] = r_t1;
        r_a0 = r_t0 << r_t7;
        scratch->order[1] = (u32)(unsigned long)packet;
        ((u8 *)&scratch->order[1])[3] = 0;
        packet += 0x14;

        *(u32 *)(packet + 4) = r_a3;
        *(u32 *)(packet + 8) = r_a0 | r_t5;
        /* ASM_BARRIER: keep r_t5 live so r_a0 | r_t5 writes to $v1 */
        __asm__("" : : "r"(r_t5));
        *(u32 *)(packet + 12) = r_a2;
        *(u32 *)(packet + 16) = r_t2;
        *(u32 *)packet = scratch->order[1];
        packet[3] = r_t1;
        scratch->order[1] = (u32)(unsigned long)packet;
        ((u8 *)&scratch->order[1])[3] = 0;
        packet += 0x14;

        r_t5 = r_t6 + 0x90;
        *(u32 *)(packet + 4) = r_a3;
        *(u32 *)(packet + 8) = r_a0 | r_t5;
        /* ASM_BARRIER: keep r_t5 live so r_a0 | r_t5 writes to $v1 */
        __asm__("" : : "r"(r_t5));
        *(u32 *)(packet + 12) = r_a2;
        *(u32 *)(packet + 16) = r_t2;
        *(u32 *)packet = scratch->order[1];
        packet[3] = r_t1;
        scratch->order[1] = (u32)(unsigned long)packet;
        ((u8 *)&scratch->order[1])[3] = 0;
        packet += 0x14;

        r_t5 = r_t3 + 0xB0;
        r_a0 |= r_t5;
        *(u32 *)(packet + 4) = r_a3;
        *(u32 *)(packet + 8) = r_a0;
        *(u32 *)(packet + 12) = r_a2;
        *(u32 *)(packet + 16) = r_t2;
        *(u32 *)packet = scratch->order[1];
        packet[3] = r_t1;
        scratch->order[1] = (u32)(unsigned long)packet;
        ((u8 *)&scratch->order[1])[3] = 0;
        packet += 0x14;

        scratch->cursor = packet;
    }
}

void EmitPresetSprite(s32 intensity,s32 index) {
    s32 value=intensity;
    s32 selected=index;
    s32 clamped;
    /* ASM_BARRIER: captured packet cursor, independent of scratch pointer reloads. */
    register u8 *packet __asm__("$5");
    /* ASM_BARRIER: table address reuses the dead offset temporary. */
    register RrSpritePreset *preset __asm__("$2");
    /* ASM_BARRIER: keep the packed color in the original word temporary. */
    register u32 hi __asm__("$3");
    u32 middle,offset;
    /* ASM_BARRIER: retain the scratch page across packet and ordering writes. */
    register RrGpuScratchPointers *scratch __asm__("$6");
    if(value!=0) {
        if(value>=0) {
            clamped=value<256;
            if(clamped)goto low;
            clamped=255;
        } else {
low:
            clamped=value;
            if(value<0)clamped=0;
        }
        value=clamped;
        scratch=(RrGpuScratchPointers*)SCRATCHPAD;
        /* ASM_BARRIER: stage the scratch page before packing the RGB command. */
        __asm__("" : "+r"(scratch));
        hi=(u32)value<<16;
        middle=(u32)value<<8;
        middle|=0x66000000U;
        hi|=middle;
        hi|=value;
        offset=(u32)selected*sizeof(RrSpritePreset);
        packet=scratch->cursor;
        *(u32*)(packet+4)=hi;
        preset=(RrSpritePreset*)((u8*)D_8009E5E0+offset);
        *(u32*)(packet+8)=preset->position;
        *(u32*)(packet+16)=preset->size;
        *(u32*)(packet+12)=preset->uv;
        *(u32*)packet=scratch->order[1];
        packet[3]=4;
        scratch->order[1]=(u32)(unsigned long)packet;
        ((u8*)&scratch->order[1])[3]=0;
        scratch->cursor=packet+sizeof(RrSpritePacket);
    }
}

void EmitFadeTile(s32 intensity, s32 scale, u32 pos, s32 width) {
    /* ASM_BARRIER: keep intensity in $t0 across GTE interpolation and clamping */
    register s32 value __asm__("$8") = intensity;
    /* ASM_BARRIER: keep pos in $t2 until packet write */
    register u32 position __asm__("$10") = pos;
    /* ASM_BARRIER: keep interpolated width in $t1 */
    register u32 interp __asm__("$9");
    s32 clamped;
    /* ASM_BARRIER: keep packet pointer in $a1 */
    register u8 *packet __asm__("$5");
    /* ASM_BARRIER: keep hi shift in $a0 */
    register u32 hi __asm__("$4");
    u32 middle;
    /* ASM_BARRIER: keep scratch base in $a2 */
    register RrGpuScratchPointers *scratch __asm__("$6");

    if (value != 0 && scale != 0) {
        /* ASM_BARRIER: load GTE IR0 with scale */
        __asm__ volatile ("mtc2 %0, $8" : : "r"(scale));
        /* ASM_BARRIER: load GTE IR1 with width */
        __asm__ volatile ("mtc2 %0, $9" : : "r"(width));
        /* ASM_BARRIER: GTE delay slots and GPF12 interpolation */
        __asm__ volatile ("nop\n\tnop\n\t.word 0x4B98003D");
        /* ASM_BARRIER: read interpolated GTE IR1 result */
        __asm__ volatile ("mfc2 %0, $9" : "=r"(interp));
        if (value >= 0) {
            clamped = value < 256;
            if (clamped) goto low;
            clamped = 255;
        } else {
low:
            clamped = value;
            if (value < 0) clamped = 0;
        }
        value = clamped;
        scratch = (RrGpuScratchPointers *)SCRATCHPAD;
        /* ASM_BARRIER: stage the scratch page before packing the RGB command */
        __asm__("" : "+r"(scratch));
        hi = (u32)value << 16;
        middle = (u32)value << 8;
        middle |= 0x62000000U;
        hi |= middle;
        value |= hi;
        packet = scratch->cursor;
        *(u32 *)(packet + 4) = value;
        *(u32 *)(packet + 8) = position;
        *(u32 *)(packet + 12) = interp | 0x20000U;
        /* ASM_BARRIER: keep interp ($t1) live so (interp | 0x20000) writes to $v0 */
        __asm__("" : : "r"(interp));
        *(u32 *)packet = scratch->order[1];
        packet[3] = 3;
        scratch->order[1] = (u32)(unsigned long)packet;
        ((u8 *)&scratch->order[1])[3] = 0;
        scratch->cursor = packet + 0x10;
    }
}

void func_8001BC84(int a0, int a1)
{
    struct Pk *p = *(struct Pk **)SCRATCHPAD;
    int col = (((-a1) >> 3) & 3) | 0x1BA4F0D0;
    short *t = D_8009E874;
    int i = 11;
    int *g;
    do {
        if (a0 & 1) {
            p->a = 0x65000000; p->b = *t | 0xA80000; p->d = 0x100004; p->c = col;
            g = (int *)SCRATCHPAD;
            *(int *)p = ((struct Ot *)g[1])->next;
            p->len = 4;
            ((struct Ot *)g[1])->next = (int)p;
            p++;
            ((char *)g[1])[7] = 0;
        }
        a0 >>= 1;
        i--;
        t++;
    } while (i > 0);
    *(struct Pk **)SCRATCHPAD = p;
}
