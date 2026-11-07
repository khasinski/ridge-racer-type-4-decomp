/* RODATA 0x80010550 0x60 */
#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "rr/selection.h"
#include "game/data/ldscn1.h"
#include "game/gfx/gfxotag.h"
#include "game/menu/menutran.h"
#include "game/ui/barsel0.h"
#include "game/data/globals.h"
#include "game/gfx/drawbuf.h"

typedef struct {
    u8 pad0[0x16];
    s16 unk16;
    u8 pad18[0x4];
    s8 unk1C;
    s8 unk1D;
    s8 unk1E;
    u8 unk1F;
    s32 unk20;
} Struct800FF68C;

typedef struct {
    u8 pad0[2];
    s16 unk2;
    u8 pad4[0x18];
} Struct800F3AFC;

struct S { int a; short b; short c; };

struct E { int a; short b; short c; };

typedef struct { char b[0x17]; signed char b17; char r[0x2C - 0x18]; } w3g2S;

extern unsigned char D_800AD4F5;
extern signed char *Dialogue;
extern short MenuResultCode;
extern s32 AddDrawModePrim(s32, s32, s32);
extern s32 AddTilePrim(s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 AddJitterSprites(s32, s32, void *, s32, s32, s32);
extern s32 AddTileSemiTransPrim(s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern void InitRect16Row(s32, s32, s32, s32, s32, s32, s32);
extern s32 UpdateRect16Row(s32);
extern void LoadTimImage(s32);
extern u8 D_8009F3F8[];
extern u8 D_8009F404[];
extern int SetMenuFrameOptions();
extern int UpdateDialogue();
extern int SkipDialoguePage();
extern int UpdateBadgeRowAnim();
extern struct S RankEntries[];
extern struct S *RankTable[];
extern short NumCars;
extern int ClearScreenOverlayState();
extern int InitBadgeRow();
extern s8 ScreenFadeState;
extern s16 ModeId;
extern s16 ModeGroup;
extern s16 SaveSlotIndex;
extern s16 ModeSubState;
extern int AddScreenFadeTiles();
extern int UpdateScreenFade();
extern int SetMenuFrameOptions();
extern int StepMenuValue();
extern int UpdateDialogue();
extern int ResetScreenCounters();
extern int UpdateBadgeRowAnim();
extern int PlaySfx();
extern struct { char pad[4]; char b4; char pad5; signed char b6; char pad2[0xC - 7]; } *DialogueState;
extern short MenuFadeTimer;
extern short MenuScriptId;
extern int LoadCursor;
extern int LoadCursorBase;
extern w3g2S SaveSlotSelections[];
extern struct { char pad[0x28]; int w28, w2C; } PlayerControllers;
extern struct { char pad[8]; short s8, sA; char padC[4]; short s10; char pad12[0x2C - 0x12]; short s2C; } *BadgeRows;
extern char *UiRows;
extern unsigned short Pad0RepeatButtons;
extern int D_8010BFC0;
extern int UpdateScreenFade();
extern void InitUiRow(int,int,int,int,int,int,int,int,int,int,...);
extern s16 MenuCursor;
extern s32 ModeFrame;
extern s16 CardSlot;

void SelectDialogueScript(int a0, int a1);
void UpdateDialoguePanel(void);
void UpdateDialogueBox(void);
void func_80031DA0(void);
void func_80031DA8(void);
void ResetRankEntries();
void ResetRankEntriesForCars(void);
void SaveSlotSelectModeEnter(void);
void SaveSlotSelectModeUpdate(void);
void func_8003244C(void);
void func_800324F8(void);

/* legacy typing of shared globals in this part of the unit */
#define UpdateSelectionRank ((int (*)())UpdateSelectionRank)

void SelectDialogueScript(int a0, int a1) {
    int sw;
    int idx, a3, v, t, r;
    signed char *P;
    int u;
    int i, sum;

    sw = D_800AD4F2;
    Dialogue[7] = 1;
    *(short *)(Dialogue + 0x16) = 0;
    Dialogue[0x1E] = -2;
    switch (sw) {
    case 0:
        Dialogue[7] = 0;
    case 1:
        Dialogue[4] = 0;
        PendingModeId = 0x109;
        D_800AD4F2 = 1;
        break;
    case 2:
        Dialogue[4] = D_800AD4F5 + 0x18;
        PendingModeId = 0x10F;
        break;
    case 4:
        idx = SelectionState[0x17];
        if (idx < 4) {
            a3 = SelectionState[idx];
        } else {
            a3 = 3 - SelectionState[idx + 8];
            if (idx == 7) a3 = 0;
        }
        switch (idx) {
        case 0:
            a1 = SelectionState[idx + 8] < 3;
            break;
        case 1:
        case 3:
            a1 = 4;
            if (SelectionState[idx + 8] >= 3) a1 = SelectionState[idx - 1];
            break;
        case 2:
            a1 = 4;
            if (SelectionState[idx + 8] >= 3) a1 = (unsigned char)SelectionState[0x14];
            break;
        case 4:
            a1 = D_800AD4EC;
            break;
        case 5:
            a1 = 3 - SelectionState[idx + 7];
            break;
        case 6:
        case 7:
            a1 = 0;
            i = 0;
            sw = (int)SelectionState;
            do {
                a1 += *(signed char *)(i + sw + 0xC);
                i++;
            } while (i < 4);
            if (a1 == 0xC) {
                a1 = 0;
            } else if (idx == 7) {
                a1 = 1;
            } else {
                a1 = 4 - SelectionState[idx + 7];
            }
            break;
        }
        P = Dialogue;
        u = a3 * 5;
        t = a1 + 4;
        P[4] = u + t;
        v = D_800AD4EF;
        if (v == 3) goto L814;
        if (v < 4) {
            if (v == 1) goto L814;
            PendingModeId = 0x10F;
        } else {
            if (v == 6) {
L814:
                D_800AD4F2 = 6;
                UpdateSelectionRank(P, a1, sw, a3);
                PendingModeId = 0x12A;
            } else PendingModeId = 0x10F;
        }
        if (D_800AD4EF == 7) PendingModeId = 0x205;
        break;
    case 6:
        Dialogue[4] = 0x25 - D_800AD4EC;
        PendingModeId = 0x111;
        MenuResultCode = 3;
        break;
    }
}

#undef UpdateSelectionRank

/* legacy typing of shared globals in this part of the unit */
#define BadgeRows ((Struct800F3AFC *)BadgeRows)
#define Dialogue ((Struct800FF68C *)Dialogue)

void UpdateDialoguePanel(void) {
    s32 var_a1;
    s32 temp_v0;
    s32 temp_s2;

    temp_v0 = Dialogue->unk1E;
    temp_s2 = Dialogue->unk1F;
    switch (temp_v0) {
    case -2:
        Dialogue->unk1F = 0;
        Dialogue->unk1E = -4;
        Dialogue->unk1D = -1;
        Dialogue->unk1C = -1;
        break;
    case -4:
        InitRect16Row(0, 0x78, 0x60, 0x50, 0xB4, 0, 0);
        Dialogue->unk1E = -5;
        break;
    case -5:
        if (UpdateRect16Row(0) != 0) {
            Dialogue->unk1E = -1;
        }
        break;
    case -3:
        break;
    case -1:
        {
            s32 a1 = *(s32 *)SCRATCHPAD;
            *(s32 *)SCRATCHPAD = AddDrawModePrim(
                CurrentDrawBuf + 0x74,
                AddTileSemiTransPrim(CurrentDrawBuf + 0x74, a1, 0x78, 0x60, 0x50, 0xB4, (u16)temp_s2, (u16)temp_s2, (u16)temp_s2),
                0x45);
            if ((u16)temp_s2 >= 0x7C) {
                Dialogue->unk1E = 2;
                Dialogue->unk1F = 0;
                BadgeRows[Dialogue->unk16].unk2 = 1;
            } else {
                Dialogue->unk1F += 8;
            }
            UpdateRect16Row(0);
        }
        break;
    case 0:
        var_a1 = *(s32 *)SCRATCHPAD;
        if ((u16)temp_s2 >= 3) {
            var_a1 = AddJitterSprites(CurrentDrawBuf + 0x70, var_a1, D_8009F3F8, 1, 0xFF, 2);
        }
        *(s32 *)SCRATCHPAD = AddTilePrim(CurrentDrawBuf + 0x74, var_a1, 0x78, 0x60, 0x50, 0xB4, 8, 8, 8);
        if ((u16)temp_s2 >= 0x15) {
            Dialogue->unk1E = 1;
            Dialogue->unk1F = 0;
        } else {
            Dialogue->unk1F += 1;
        }
        if (temp_s2 == 1) {
            u8 temp_a0 = Dialogue->unk1C;
            if ((u32)(temp_a0 - 1) < 5U) {
                LoadTimImage(Dialogue->unk20 + (((s8)temp_a0 - 1) << 14));
            }
        }
        break;
    case 1:
        {
            s32 two = 2;
            *(s32 *)SCRATCHPAD = AddTilePrim(
                CurrentDrawBuf + 0x7C,
                AddJitterSprites(
                    CurrentDrawBuf + 0x70,
                    AddJitterSprites(CurrentDrawBuf + 0x70, *(s32 *)SCRATCHPAD, D_8009F404, 1, temp_s2, 1),
                    D_8009F3F8, 1, (0x80 - temp_s2) * 2, two),
                0x78, 0x60, 0x50, 0xB4, 8, 8, 8);
            if ((u32)temp_s2 >= 0x80U) {
                Dialogue->unk1F = 0;
                Dialogue->unk1E = two;
            } else {
                Dialogue->unk1F += 8;
            }
        }
        break;
    case 2:
    default:
        var_a1 = *(s32 *)SCRATCHPAD;
        if (Dialogue->unk1C >= 0) {
            var_a1 = AddJitterSprites(CurrentDrawBuf + 0x70, var_a1, D_8009F404, 0, 0x80, 1);
        }
        *(s32 *)SCRATCHPAD = AddTilePrim(CurrentDrawBuf + 0x74, var_a1, 0x78, 0x60, 0x50, 0xB4, 8, 8, 8);
        break;
    }
}

#undef BadgeRows
#undef Dialogue

/* legacy typing of shared globals in this part of the unit */
#define BadgeRows (*(s32 (*))&BadgeRows)
#define Dialogue ((void *)Dialogue)

void UpdateDialogueBox(void) {
    void *temp_v0;

    if (M2C_FIELD(((M2C_FIELD(Dialogue, s16 *, 0x16) * 0x1C) + BadgeRows), s16 *, 0x10) == 3) {
        SetMenuFrameOptions(1, 1, 0);
        SkipDialoguePage(Dialogue);
        temp_v0 = (M2C_FIELD(Dialogue, s16 *, 0x16) * 0x1C) + BadgeRows;
        UpdateDialogue(M2C_FIELD(temp_v0, s16 *, 8) + 0xA, M2C_FIELD(temp_v0, s16 *, 0xA) + 0x28, Dialogue);
    }
    UpdateBadgeRowAnim(M2C_FIELD(Dialogue, s16 *, 0x16));
}

#undef BadgeRows
#undef Dialogue

void func_80031DA0(void) {
}

void func_80031DA8(void) {
}

void ResetRankEntries() {
    int i = 0;
    struct S **q = RankTable;
    struct S *p = RankEntries;
    do { p->a = 0; p->b = i; *q++ = p; i++; p++; } while (i < 8);
}

/* legacy typing of shared globals in this part of the unit */
#define RankEntries (*(struct E (*)[])&RankEntries)
#define RankTable (*(struct E *(*)[])&RankTable)

void ResetRankEntriesForCars(void) {
    int i = 0;
    struct E *e;
    struct E **t;
    int n;
    if (NumCars > 0) {
        n = NumCars;
        t = RankTable;
        e = RankEntries;
        do {
            e->a = 0;
            e->b = i;
            *t = e;
            t++;
            i++;
            e++;
        } while (i < n);
    }
}

#undef RankEntries
#undef RankTable

void InitSelectionRows(void) {
    s32 a2 = 0;
    s32 a1;
    RrSelectionRow *a0;
    RrSelectionRow *v1;
    RrSelectionRows *b;
    unsigned char *page;
    const u8 *choices, *variants;
    const RrSelectionConfigView *config;
    u8 t;
    s32 x;
    unsigned char *stage;
    /* ASM_BARRIER: Preserve the indexed byte offset in the address temporary. */
    register u32 offset __asm__("$2");
    u32 attribute;
    page=(unsigned char *)0x800F0000U;
    /* ASM_BARRIER: Materialize the state page after the initial row index. */
    __asm__("" : "+r"(page) : "r"(a2));
    b=(RrSelectionRows *)(page+0x4D98);
    /* ASM_BARRIER: Retain the full frame pointer separately from its page. */
    __asm__("" : "+r"(b));
    stage=(unsigned char *)0x800A0000U;
    /* ASM_BARRIER: Expose the selection-table page before adding its signed offset. */
    __asm__("" : "+r"(stage));
    choices=stage-0xBDC;
    stage=(unsigned char *)0x800A0000U;
    /* ASM_BARRIER: Keep the separate variant-table page materialization. */
    __asm__("" : "+r"(stage));
    variants=stage-0xBE0;
    stage=(unsigned char *)0x800B0000U;
    /* ASM_BARRIER: Expose the configuration page before adding its signed offset. */
    __asm__("" : "+r"(stage));
    config=(const RrSelectionConfigView *)(stage-0x2B28);
    /* ASM_BARRIER: Form lookup bases before initializing the byte offset. */
    __asm__("" : : "r"(choices), "r"(variants), "r"(config) : "$5");
    a1=RR_FIELD_OFFSET(RrSelectionRows,rows);
    do {
        x = page[0x4D98];
        offset=x*8;
        offset=a2+offset;
        t=*(unsigned char *)((unsigned long)offset+(unsigned long)choices);
        a0 = (RrSelectionRow *)((unsigned long)a1+(unsigned long)b);
        a0->selection = t;
        if (t == page[0x4D98]) {
            a0->variant = b->variant;
        } else {
            a0->variant = *(unsigned char *)((unsigned long)a0->selection+(unsigned long)variants);
            /* ASM_BARRIER: Retain the original row-pointer recomputation after the variant store. */
            __asm__("" : "+r"(a0));
            a0 = (RrSelectionRow *)((unsigned long)a1+(unsigned long)b);
        }
        if (a0->selection != page[0x4D98]) {
            if (a0->variant == b->variant) {
                a0->variant = *(unsigned char *)((unsigned long)page[0x4D98]+(unsigned long)variants);
            }
        }
        v1 = (RrSelectionRow *)((unsigned long)a1+(unsigned long)b);
        /* ASM_BARRIER: Keep the current row and index before publishing its attribute. */
        __asm__("" : : "r"(v1), "r"(a2));
        attribute=config->parameter;
        /* ASM_BARRIER: Read the row attribute before advancing the loop index. */
        __asm__("" : "+r"(attribute) : "r"(a2));
        a2 += 1;
        v1->parameter = attribute;
        a1 += sizeof(RrSelectionRow);
    } while (a2 < 7);
}

/* legacy typing of shared globals in this part of the unit */
#define BadgeRows ((void *)BadgeRows)

void SaveSlotSelectModeEnter(void) {
    ClearScreenOverlayState();
    ScreenFadeState = 2;
    SaveSlotIndex = 0;
    InitBadgeRow(1, 0x80, 0x58, 0xA8, 0x80, 0, 0x701);
    M2C_FIELD(BadgeRows, s16 *, 0x1E) = 1;
    InitBadgeRow(0, 0xC, 0x130, 0x128, 0x58, 0, 0);
    M2C_FIELD(BadgeRows, s16 *, 2) = 1;
    SetMenuFrameOptions(0, 1, 1);
    ModeGroup = 1;
    ModeId = 0xE;
    ModeSubState = 0;
}

#undef BadgeRows

/* legacy typing of shared globals in this part of the unit */
#define SelectionState (*(w3g2S (*))&SelectionState)
#define ModeSubState (*(unsigned short (*))&ModeSubState)

void SaveSlotSelectModeUpdate(void) {
    int s0;
    switch (ModeSubState) {
    case 0:
        *(UiRows + SaveSlotIndex * 0x24 + 0xC) = 0;
        StepMenuValue(0, 2, &SaveSlotIndex, 3, 0);
        *(UiRows + SaveSlotIndex * 0x24 + 0xC) = 2;
        if (BadgeRows->s10 == 3) {
            s0 = DialogueState->b6;
            if (s0 == 4) {
                SetMenuFrameOptions(1, 1, 1);
                if (PlayerControllers.w28 != 0) {
                    if (SaveSlotSelections[SaveSlotIndex].b17 > 0) {
                        PlaySfx(1);
                        ModeSubState = 1;
                        MenuFadeTimer = 0x40;
                        ResetScreenCounters();
                        LoadCursor = LoadCursorBase;
                        BadgeRows->s10 = s0;
                        BadgeRows->s2C = s0;
                    } else {
                        PlaySfx(4);
                    }
                } else if (PlayerControllers.w2C != 0) {
                    PlaySfx(2);
                    ModeSubState = 2;
                    MenuFadeTimer = 0x40;
                    ResetScreenCounters();
                    LoadCursor = LoadCursorBase;
                    LoadTimListStep(0);
                    BadgeRows->s10 = s0;
                    BadgeRows->s2C = s0;
                }
            }
        }
        goto common;
    case 1:
        SetMenuFrameOptions(0, 1, 1);
        if (MenuFadeTimer > 0) MenuFadeTimer = MenuFadeTimer - 1;
        AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x40 - MenuFadeTimer) * 4, 0x45);
        if (MenuFadeTimer <= 0) {
            if (LoadTimListStep(1) == 1) {
                SelectionState = SaveSlotSelections[SaveSlotIndex];
                ModeGroup = 1;
                ModeId = 0xF;
                ModeSubState = 0;
                return;
            }
        }
        goto common;
    case 2:
        SetMenuFrameOptions(0, 1, 1);
        if (MenuFadeTimer > 0) MenuFadeTimer = MenuFadeTimer - 1;
        AddScreenFadeTiles(CurrentDrawBuf + 0x70, (0x40 - MenuFadeTimer) * 4, 0x45);
        if (MenuFadeTimer <= 0) {
            func_8002F688();
            MenuScriptId = 1;
            ModeGroup = 1;
            ModeId = 0x23;
            ModeSubState = 0;
            return;
        }
    default:
    common:
        UpdateBadgeRowAnim(0);
        UpdateBadgeRowAnim(1);
        if (BadgeRows->s10 == 3) {
            DialogueState->b4 = 0x1C;
            UpdateDialogue(BadgeRows->s8 + 0xA, BadgeRows->sA + 0x28, DialogueState);
        }
        UpdateScreenFade(0x10086);
    }
}

#undef SelectionState
#undef ModeSubState

void func_8003244C(void) {
    if (Pad0RepeatButtons & 0x1000) {
        D_8010BFC0 = D_8010BFC0 - 1;
    } else if (Pad0RepeatButtons & 0x4000) {
        D_8010BFC0 = D_8010BFC0 + 1;
    }
    if (D_8010BFC0 < 0) {
        D_8010BFC0 = 1;
    } else if (D_8010BFC0 >= 2) {
        D_8010BFC0 = 0;
    }
    if (Pad0Buttons & 0x800) {
        ModeGroup = 1;
        ModeId = 0xF;
        ModeSubState = 0;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define BadgeRows ((void *)BadgeRows)

void func_800324F8(void) {
    ModeFrame = 0;
    CardSlot = 0;
    MenuResultCode = 3;
    MenuCursor = 0;
    InitUiRow(0, 0x8C, 0x88, 0x1C, 0x20, 0, 0, 1, 0, 0x68);
    InitUiRow(1, 0x8C, 0xB0, 0x1C, 0x20, 0, 0, 1, 0, 0x69);
    InitBadgeRow(0, 0xC, 0x130, 0x128, 0x58, 0, 0);
    ModeGroup = 1;
    ModeId = 0x2B;
    ModeSubState = 0;
    M2C_FIELD(BadgeRows, s16 *, 2) = 1;
    UpdateScreenFade(0x10086);
}

#undef BadgeRows
