#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define ImageRows ((E *)ImageRows)

#undef ImageRows

/* legacy typing of shared globals in this part of the unit */
#define ImageRows ((E_1 *)ImageRows)

#undef ImageRows

/* legacy typing of shared globals in this part of the unit */
#define ImageRows ((E_2 *)ImageRows)

#undef ImageRows

typedef struct {
    s32 prim;
} Scratchpad;

typedef struct {
    s16 x, y, w, h;
} RectS16;

typedef struct {
    u8 b0;
    u8 b1;
    u8 b2;
    u8 b3;
    s16 s4;
    s16 s6;
    s16 s8;
    s16 sA;
    s8 c0C;
    u8 pad0D[7];
    u8 b14;
    u8 b15;
    u8 b16;
    u8 b17;
    RectS16 rect18;
    s16 s20;
    u8 pad22[2];
} UiElem24;

typedef struct {
    u8 b0;
    u8 b1;
    u8 b2;
    u8 b3;
} FontGlyph4;

extern s32 AddTextSprites();

extern s32 AddDrawAreaPrim();

extern s32 AddDrawModePrim();

extern s32 AddTilePrim();

extern s32 AddSpritePrim();

extern void func_8003EA04();

extern s32 rsin();

extern s32 rcos();

extern char D_80010924[];

extern char D_80010928[];

extern char StrPleaseWait[];

extern FontGlyph4 UiFontGlyphs[];

extern FontGlyph4 D_8009FB80[];

extern FontGlyph4 D_8009FC0C[];

extern s8 D_8009FC14[][4];

extern s8 D_8009FC7C[][8];

extern FontGlyph4 D_8009FCDC[];

extern s8 D_8009FD00[];

extern s16 SelectedCarIndex;

extern s16 NumCars;

extern s32 CurrentDrawBuf;

extern s16 ReverseCourse;

extern s16 CourseListIndex;

extern u8 SaveProgress[];

extern s32 NumLaps;

extern UiElem24 *UiRows;

extern s32 D_80119754;

extern s32 D_80119758;

s32 DrawUiRow(s32 arg0) {
    RectS16 sp28;
    s32 sp30;
    s32 sp34;
    s32 sp38;
    s32 s0;
    s32 s1;
    s32 s2;
    /* ASM_BARRIER: Keep s3 in $s3 while s3_init has single assignment in prologue */
    register s32 s3 asm("s3");
    /* ASM_BARRIER: Use $0 register operand so reload_cse_regs does not replace 0 with $fp */
    register s32 zero asm("$0");
    s32 s3_init;
    s32 s4;
    s32 s5_1;
    s32 s5_2;
    s32 s6;
    s32 s7;
    s32 fp;
    u32 a0_byte;
    s32 a0_arg;
    FontGlyph4 *glyph;

    fp = 0;
    s3_init = ((Scratchpad *)0x1F800000)->prim;
    sp38 = UiRows[arg0].s4;
    s2 = zero;
    sp30 = UiRows[arg0].b14;
    s4 = sp38;
    a0_byte = UiRows[arg0].b0;
    s7 = UiRows[arg0].s6;
    s6 = UiRows[arg0].b15;
    sp34 = UiRows[arg0].b16;

    switch (sp30) {
    case 0:
        UiRows[arg0].b14 = 1;
        UiRows[arg0].rect18.w = 0;
        UiRows[arg0].rect18.x = UiRows[arg0].s4;
        UiRows[arg0].rect18.y = UiRows[arg0].s6;
        UiRows[arg0].rect18.h = UiRows[arg0].sA;
        UiRows[arg0].b0 = 0;
        UiRows[arg0].c0C = 0;
        break;
    case 1:
        UiRows[arg0].rect18.w = (rsin((u16)a0_byte << 6) * UiRows[arg0].s8) / 4096;
        if ((u16)a0_byte >= 16) {
            UiRows[arg0].b0 = 0;
            UiRows[arg0].b14 = 2;
        } else {
            UiRows[arg0].b0++;
        }
        break;
    case 2:
        if (UiRows[arg0].b2 == 1) {
            if (UiRows[arg0].c0C & 2) {
                s2 = rcos((UiRows[arg0].b17 << 4) - 0x2AA);
                fp = (u32)s2 >> 31;
                if (s2 >= 0) {
                    s2 = ((s2 >> 6) + 100) << 1;
                } else {
                    s2 = 0;
                }
            }
            if (UiRows[arg0].s20 == 1) {
                fp = 2;
            }
        }
        break;
    case 3:
        UiRows[arg0].rect18.w = (rcos((u16)a0_byte << 6) * UiRows[arg0].s8) / 4096;
        if ((u16)a0_byte >= 16) {
            UiRows[arg0].b0 = 0;
            UiRows[arg0].b14 = 4;
        } else {
            UiRows[arg0].b0++;
        }
        break;
    case 4:
    case 7:
        if (a0_byte >= 2) {
            UiRows[arg0].b0 = 0;
            if (sp30 == 7) {
                UiRows[arg0].b14 = 8;
            } else if (UiRows[arg0].b1 == 0) {
                UiRows[arg0].b14 = 5;
            } else {
                UiRows[arg0].b14 = 6;
            }
        } else {
            UiRows[arg0].b0++;
        }
        break;
    case 5:
    case 6:
        break;
    case 8:
        if (a0_byte >= 16) {
            UiRows[arg0].b0 = 0;
            UiRows[arg0].b14 = 9;
        } else {
            UiRows[arg0].b0++;
        }
        /* fallthrough */
    case 9:
        if (UiRows[arg0].b2 == 1) {
            fp = (UiRows[arg0].c0C != 1) << 1;
        }
        break;
    }

    sp28 = UiRows[arg0].rect18;
    s3 = AddDrawAreaPrim(CurrentDrawBuf + 0x70, s3_init, 0, 0, 0x140, 0x200);
    s4 += 7;

    switch (s6) {
    case 5:
    case 6:
    case 7:
        if (s6 == 5) {
            s0 = NumLaps;
        }
        if (s6 == 6) {
            s0 = NumCars - 2;
        }
        fp = 0;
        if (s6 == 7) {
            s0 = SelectedCarIndex + 1;
        }
        s2 = 0;
        /* fallthrough */
    case 0:
        {
            FontGlyph4 *base_0 = UiFontGlyphs;
            s3 = AddSpritePrim(CurrentDrawBuf + 0x70, s3, s4, s7 + 8, &base_0[sp34], fp | 0x7FF8, s2);
            if (s6 != 0) {
                for (s1 = 0; s1 < 2; s1++) {
                    s3 = AddSpritePrim(CurrentDrawBuf + 0x70, s3, s4 + 0x1E + s1 * 0x16, s7 + 8, &D_8009FC0C[s1], 0x7FF8, 0);
                    if (s1 == 0) {
                        D_8009FD00[0] = s0 / 10;
                    } else {
                        D_8009FD00[0] = (s0 % 10) * 8;
                    }
                    s3 = AddSpritePrim(CurrentDrawBuf + 0x70, s3, s4 + 0x26 + s1 * 6, s7 + 8, D_8009FD00, 0x7FF8, 0);
                }
                /* ASM_BARRIER: Keep base_0 (UiFontGlyphs) live across case 0 loop so reload rematerializes it into $t0 */
                __asm__("" : : "ri"(base_0));
            }
        }
        break;
    case 1:
    case 4:
    case 8:
    case 9:
        for (s1 = 0; s1 < 4; s1++) {
            s0 = D_8009FC14[sp34][s1];
            if (s0 == -1) {
                break;
            }
            s5_1 = fp | 0x7FF8;
            s3 = AddSpritePrim(CurrentDrawBuf + 0x70, s3, s4, s7 + 8, &UiFontGlyphs[s0], s5_1, s2);
            s4 += 2 + UiFontGlyphs[s0].b2;
        }
        if (s6 == 4) {
            if (ReverseCourse == 1) {
                s0 = 0xD;
            } else {
                s0 = 0xC;
            }
            s3 = AddSpritePrim(CurrentDrawBuf + 0x70, s3, s4, s7 + 8, &UiFontGlyphs[s0], 0x7FF8, 0);
        } else if ((u32)(s6 - 8) < 2U) {
            s3 = AddTextSprites(CurrentDrawBuf + 0x70, s3, s4, s7 + 8, (s6 == 8) ? D_80010924 : D_80010928, fp | 0x7FF8, 0, s2);
        }
        break;
    case 2:
        for (s1 = 0; s1 < 7; s1++) {
            s0 = D_8009FC7C[sp34][s1];
            if (s0 == -1) {
                break;
            }
            a0_arg = CurrentDrawBuf + 0x70;
            glyph = &UiFontGlyphs[s0];
            s5_2 = fp | 0x7FF8;
            s3 = AddSpritePrim(a0_arg, s3, s4, s7 + 8, glyph, s5_2, 0);
            s4 += 2 + UiFontGlyphs[s0].b2;
        }
        s3 = AddTilePrim(CurrentDrawBuf + 0x70, s3, sp38 + 0xA9, s7 + 6, 1, 0x14, 0xFF, 0xFF, 0xFF);
        s4 = sp38 + 0xB0;
        for (s1 = 0; s1 < 2; s1++) {
            if (s1 == 0) {
                s0 = 0x45;
            } else {
                s0 = D_8009FC7C[sp34][7];
            }
            s3 = AddSpritePrim(CurrentDrawBuf + 0x70, s3, s4, s7 + 8, &UiFontGlyphs[s0], 0x7FF8, 0);
            s4 += 2 + UiFontGlyphs[s0].b2;
        }
        break;
    case 3:
        for (s1 = 0; s1 < 7; s1++) {
            s0 = D_8009FC7C[sp34][s1];
            if (s0 == -1) {
                break;
            }
            s3 = AddSpritePrim(CurrentDrawBuf + 0x70, s3, s4, s7 + 8, &UiFontGlyphs[s0], 0x7FF8, 0);
            s4 += 2 + UiFontGlyphs[s0].b2;
            if (s1 == 3) {
                s3 = AddTilePrim(CurrentDrawBuf + 0x70, s3, sp38 + 0x7E, s7 + 6, 1, 0x14, 0xFF, 0xFF, 0xFF);
                s4 = sp38 + 0x86;
            }
        }
        break;
    case 10:
    case 11:
        s3 = AddSpritePrim(CurrentDrawBuf + 0x70, s3, s4, s7 + 8, &D_8009FB80[0], 0x7FF8, 0);
        s3 = AddSpritePrim(CurrentDrawBuf + 0x70, s3, s4 + 0x44, s7 + 8, &D_8009FB80[1], 0x7FF8, 0);
        if (s6 == 10) {
            s0 = D_80119754;
        } else {
            s0 = D_80119758;
        }
        for (s1 = 0; s1 < s0; s1++) {
            s3 = AddTilePrim(CurrentDrawBuf + 0x70, s3, s4 + 0x12 + s1 * 3, s7 + 8, 2, 0x10, 0xFF, 0xFF, 0xFF);
        }
        break;
    case 12:
        if (SaveProgress[CourseListIndex + 0x5C] == 1) {
            s2 = 8;
        } else {
            s2 = 7;
        }
        if (s2 != 0) {
            s1 = 0;
            do {
                s0 = D_8009FC7C[sp34][s1];
                s3 = AddSpritePrim(CurrentDrawBuf + 0x70, s3, s4, s7 + 8, &UiFontGlyphs[s0], 0x7FF8, 0);
                s4 += 2 + UiFontGlyphs[s0].b2;
                s1++;
            } while (s1 < s2);
        }
        break;
    case 0x7E:
        s3 = AddTextSprites(CurrentDrawBuf + 0x70, s3, s4, s7 + 8, StrPleaseWait, 0x7FF8, 2, 0);
        break;
    case 0x7F:
        s3 = AddSpritePrim(CurrentDrawBuf + 0x70, s3, s4, s7 + 8, &D_8009FCDC[sp34], fp | 0x4820, s2);
        break;
    }

    ((Scratchpad *)0x1F800000)->prim = AddDrawModePrim(CurrentDrawBuf + 0x70, s3, 0x38);
    DrawUiBox(sp38, s7, UiRows[arg0].s8, UiRows[arg0].sA, 0x4A, 0x4A, 0x4A);
    ((Scratchpad *)0x1F800000)->prim = AddDrawAreaPrim(CurrentDrawBuf + 0x70, ((Scratchpad *)0x1F800000)->prim, sp28.x, sp28.y, sp28.w, sp28.h);
    sp38 += UiRows[arg0].rect18.w;
    if ((u32)(sp30 - 4) >= 3U) {
        func_8003EA04(arg0, sp38, s7);
    }
    return (u32)~sp30 >> 31;
}
