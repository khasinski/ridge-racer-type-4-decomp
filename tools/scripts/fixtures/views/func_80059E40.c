#include "rr/line_emit.h"

extern void DrawFontString(s32, s32, const char *, s32, s32);

extern s32 DrawFontStringCentered(s32, s32, char *, s32, s32);

extern s32 DrawFontStringCenteredOffset(s32, s32, char *, s32, s32);

extern void AddSetTexpagePrim(s32);

extern void DrawGlyphList(s16 *, s32, s32);

extern void DrawGlyphListSloped(s16 *, s32, s32);

extern void AddBrightnessSprite(u8 *, s32, s32, s32);

extern void EmitIconSpriteRow120(s32, u8);

extern void EmitIconSpriteRow128(s32, u8);

extern void PsyqSprintf(char *, const char *, s32);

extern const char StrInOpening[];

extern const char D_8001122C[];

extern const char StrApostropheOpening[];

extern const char StrFmtTwoDigitsOpening[];

extern const char StrRandomPlay[];

extern char CdTrackTitleStrings[];

extern s32 BgmNumberToCdTrack[];

extern char CourseNames[];

extern s16 *D_800A18F0[];

extern s16 *D_800A1910[];

extern s16 *D_800A1930[];

extern s16 *D_800A1970[];

extern s16 *D_800A19A4[];

extern u8 D_800A1A60[];

extern u8 D_800A1A68[];

extern u32 OpeningFadeInStartFrames[];

extern u32 OpeningFadeStartFrames[];

extern s16 CourseIndex;

extern u16 Pad0Buttons;

extern s32 MenuConfirmPressed;

extern struct {
    char pad[0x28A4];
    s32 w28a4;
    s32 w28a8;
    s32 w28ac;
} SoundState;

extern s32 OpeningFadeTimer;

extern s32 OpeningMode;

extern s32 D_8010E230;

extern s32 OpeningBgmSelectState;

extern s32 OpeningSlideTimer;

s32 OpeningScriptStep(s32 arg0) {
    s32 sp18[6];
    char sp30[16];
    s16 sp40[4];
    u8 sp48[8];
    s16 sp50[4];
    u8 sp58[8];
    s32 i;
    s32 s2;
    s32 s4;
    s32 s0_1;
    s32 s0_2;
    s32 s0_3;

    s2 = OpeningMode;
    s4 = 0;
    if (arg0 < 0) {
        arg0 = 0;
    }
    if (arg0 >= 0x1C3) {
        s4 = ++OpeningSlideTimer >= 0x4C;
        if (arg0 >= 0xA8D) {
            s4 = 1;
        }
        arg0 = 0x1C2;
    }

    if (s2 < 8) {
        for (i = 0; i < 6; i++) {
            if (OpeningFadeInStartFrames[i] < (u32)arg0) {
                sp18[i] = arg0 - OpeningFadeInStartFrames[i];
                if (sp18[i] < 0x19) {
                    sp18[i] = (sp18[i] * 0x80) / 25;
                } else {
                    sp18[i] = 0x80;
                }
            } else {
                sp18[i] = 0;
            }
            if (OpeningFadeTimer >= 0) {
                sp18[i] = (OpeningFadeTimer * 0x80) / 100;
                if (OpeningFadeTimer > 0) {
                    OpeningFadeTimer--;
                } else {
                    return 1;
                }
            }
        }

        sp40[0] = 0x48;
        sp40[1] = 0x48;
        sp40[3] = 0x48;
        if (OpeningFadeTimer < 0) {
            sp40[2] = (sp18[0] * 0xB0) / 128 + 0x47;
            sp48[2] = 0x80;
            sp48[1] = 0x80;
            sp48[0] = 0x80;
        } else {
            sp40[2] = 0xF7;
            sp48[0] = sp48[1] = sp48[2] = sp18[0];
        }
        if (sp40[2] > sp40[0]) {
            AddLineF2Prim(sp40, sp48, 1, 0);
            sp40[1] = 0x47;
            sp40[3] = 0x47;
            sp48[0] = sp48[1] = sp48[2] = sp48[0] >> 1;
            AddLineF2Prim(sp40, sp48, 1, 0);
            sp40[1] = 0x49;
            sp40[3] = 0x49;
            AddLineF2Prim(sp40, sp48, 1, 0);
        }

        DrawGlyphList(D_800A18F0[s2], sp18[1], 0);
        DrawGlyphList(D_800A1910[s2], sp18[1], 0);
        DrawGlyphList(D_800A1930[s2], sp18[2], 0);
        DrawGlyphListSloped(D_800A1970[D_8010E230], sp18[2], 0);
        EmitIconSpriteRow128(0, sp18[3]);
        DrawFontString(0x9A, 0xA0, StrInOpening, sp18[4], 0);
        s0_1 = DrawFontStringCentered(0x60, 0xB0, CourseNames + CourseIndex * 0x18, sp18[4], 0) / 2;
        DrawFontString(0x9A - s0_1, 0xB0, D_8001122C, sp18[4], 0);
        DrawFontString(0xA2 + s0_1, 0xB0, StrApostropheOpening, sp18[4], 0);
        AddSetTexpagePrim(0);

        if (sp18[5] >= 0x80) {
            if (Pad0Buttons & 0x8000) {
                if (OpeningBgmSelectState == 0) {
                    SoundState.w28a4 = 0;
                    OpeningBgmSelectState = 1;
                } else {
                    SoundState.w28a4--;
                    if (SoundState.w28a4 < 0) {
                        SoundState.w28a4 = SoundState.w28a8 - 1;
                    }
                }
                OpeningSlideTimer = 0;
            }
            if (Pad0Buttons & 0x2000) {
                if (OpeningBgmSelectState == 0) {
                    SoundState.w28a4 = 0;
                    OpeningBgmSelectState = 1;
                } else {
                    SoundState.w28a4++;
                    if (SoundState.w28a4 >= SoundState.w28a8) {
                        SoundState.w28a4 = 0;
                    }
                }
                OpeningSlideTimer = 0;
            }
            if (MenuConfirmPressed != 0) {
                s4 = 1;
            }
        }

        AddBrightnessSprite(D_800A1A60, 0x3C, 0xC8, sp18[5]);
        AddBrightnessSprite(D_800A1A60 + 4, 0xFC, 0xC8, sp18[5]);
        AddBrightnessSprite(D_800A1A60 + 8, 0x4C, 0xC8, sp18[5]);
        PsyqSprintf(sp30, StrFmtTwoDigitsOpening, SoundState.w28a4);
        DrawFontString(0x5A, 0xC8, sp30, sp18[5], 0);
        AddBrightnessSprite(D_800A1A60 + 0xC, 0x6C, 0xC8, sp18[5]);
        if (SoundState.w28a4 <= 0) {
            DrawFontString(0x78, 0xC8, StrRandomPlay, sp18[5], 0);
        } else {
            DrawFontString(0x78, 0xC8, CdTrackTitleStrings + (BgmNumberToCdTrack[SoundState.w28a4 - 1] << 5), sp18[5], 0);
        }
        AddSetTexpagePrim(0);
        return s4;
    } else {
        for (i = 0; i < 6; i++) {
            if (OpeningFadeStartFrames[i] < (u32)arg0) {
                sp18[i] = arg0 - OpeningFadeStartFrames[i];
                if (i != 0) {
                    if (sp18[i] < 0x19) {
                        sp18[i] = (sp18[i] * 0x80) / 25;
                    } else {
                        sp18[i] = 0x80;
                    }
                } else {
                    if (sp18[0] < 0xC) {
                        sp18[0] = (sp18[0] * 0x80) / 12;
                    } else {
                        sp18[0] = 0x80;
                    }
                }
            } else {
                sp18[i] = 0;
            }
            if (OpeningFadeTimer >= 0) {
                sp18[i] = (OpeningFadeTimer * 0x80) / 100;
                if (OpeningFadeTimer > 0) {
                    OpeningFadeTimer--;
                } else {
                    return 1;
                }
            }
        }

        s2 -= 8;
        DrawGlyphList(D_800A19A4[s2], sp18[1], 0);

        sp50[0] = 0x48;
        sp50[1] = 0x38;
        sp50[3] = 0x38;
        if (OpeningFadeTimer < 0) {
            sp50[2] = (sp18[0] * 0xB0) / 128 + 0x47;
            sp58[2] = 0x80;
            sp58[1] = 0x80;
            sp58[0] = 0x80;
        } else {
            sp50[2] = 0xF7;
            sp58[0] = sp58[1] = sp58[2] = sp18[0];
        }
        if (sp50[2] > sp50[0]) {
            AddLineF2Prim(sp50, sp58, 1, 0);
            sp50[1] = 0x37;
            sp50[3] = 0x37;
            sp58[0] = sp58[1] = sp58[2] = sp58[0] >> 1;
            AddLineF2Prim(sp50, sp58, 1, 0);
            sp50[1] = 0x39;
            sp50[3] = 0x39;
            AddLineF2Prim(sp50, sp58, 1, 0);
        }

        DrawFontString(0x9A, 0x48, StrInOpening, sp18[2], 0);
        s0_2 = DrawFontStringCentered(0x60, 0x58, CourseNames + CourseIndex * 0x18, sp18[2], 0) / 2;
        DrawFontString(0x9A - s0_2, 0x58, D_8001122C, sp18[2], 0);
        DrawFontString(0xA2 + s0_2, 0x58, StrApostropheOpening, sp18[2], 0);
        AddSetTexpagePrim(0);

        if (s2 < 3) {
            EmitIconSpriteRow120(1, sp18[3]);
        } else {
            EmitIconSpriteRow120(s2 - 1, sp18[3]);
        }

        if (OpeningBgmSelectState >= 0) {
            if (sp18[5] >= 0x80) {
                if (Pad0Buttons & 0x8000) {
                    SoundState.w28a4--;
                    if (SoundState.w28a4 < 0) {
                        SoundState.w28a4 = SoundState.w28a8 - 1;
                    }
                    OpeningSlideTimer = 0;
                }
                if (Pad0Buttons & 0x2000) {
                    SoundState.w28a4++;
                    if (SoundState.w28a4 >= SoundState.w28a8) {
                        SoundState.w28a4 = 0;
                    }
                    OpeningSlideTimer = 0;
                }
                if (MenuConfirmPressed != 0) {
                    s4 = 1;
                }
            }
            SoundState.w28ac = SoundState.w28a4;
            AddBrightnessSprite(D_800A1A60, 0x3C, 0xC8, sp18[5]);
            AddBrightnessSprite(D_800A1A60 + 4, 0xFC, 0xC8, sp18[5]);
            AddBrightnessSprite(D_800A1A60 + 8, 0x4C, 0xC8, sp18[5]);
            PsyqSprintf(sp30, StrFmtTwoDigitsOpening, SoundState.w28a4);
            DrawFontString(0x5A, 0xC8, sp30, sp18[5], 0);
            AddBrightnessSprite(D_800A1A60 + 0xC, 0x6C, 0xC8, sp18[5]);
            if (SoundState.w28a4 > 0) {
                DrawFontString(0x78, 0xC8, CdTrackTitleStrings + (BgmNumberToCdTrack[SoundState.w28a4 - 1] << 5), sp18[5], 0);
            } else {
                DrawFontString(0x78, 0xC8, StrRandomPlay, sp18[5], 0);
            }
        } else {
            s0_3 = 0x88 - DrawFontStringCenteredOffset(0x14, 0xC8, CdTrackTitleStrings + (BgmNumberToCdTrack[SoundState.w28a4 - 1] << 5), sp18[5], 0) / 2;
            AddBrightnessSprite(D_800A1A68, s0_3, 0xC8, sp18[5]);
            PsyqSprintf(sp30, StrFmtTwoDigitsOpening, SoundState.w28a4);
            DrawFontString(s0_3 + 0xE, 0xC8, sp30, sp18[5], 0);
            AddBrightnessSprite(D_800A1A68 + 4, s0_3 + 0x20, 0xC8, sp18[5]);
            if (sp18[5] >= 0x80 && MenuConfirmPressed != 0) {
                s4 = 1;
            }
        }
    }

    AddSetTexpagePrim(0);
    return s4;
}
