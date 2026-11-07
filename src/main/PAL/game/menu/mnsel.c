#include "rr/menu_grid.h"
#include "game/menu/mnsel.h"
#include "rr/bios_devices.h"
#include "game/data/selection.h"

typedef struct { char p[0x10]; short s10; } T3;

extern int ResetScreenCounters();
extern int PlaySfx();
extern int MenuStepDirection;
extern char SelectionState[];
extern short SelectedTeamIndex;
extern int ModeFrame;
extern short ModeId;
extern unsigned short Pad0RepeatButtons;
extern int MenuConfirmPressed;
extern T3 *BadgeRows;
extern short ModeGroup;
extern char *UiRows;
extern short ModeSubState;
extern signed char *GlyphRows;
extern char *Dialogue;

int AllocMenuGridRecords(int a, int n);
void InitMenuGrid(s32 index, s32 layout, s32 count, s32 x, s32 y);

void UpdateDialogueChoiceCursor()
{
    int s0;
    char *q; char *r;
    int t = SelectionRows[0];
    SelectedTeamIndex = t;
    UiRows[t + 0xC] = 0;
    if (Pad0RepeatButtons & 0x2000) {
        if (SelectedTeamIndex < 3) {
            SelectedTeamIndex = (unsigned short)SelectedTeamIndex + 1;
            MenuStepDirection = 2;
            goto L_a8;
        }
    }
    if (Pad0RepeatButtons & 0x8000) {
        if (SelectedTeamIndex > 0) {
            SelectedTeamIndex = (unsigned short)SelectedTeamIndex - 1;
            MenuStepDirection = 1;
            goto L_a8;
        }
    }
    MenuStepDirection = 0;
    goto L_54;
L_a8:
    PlaySfx(0);
    if (SelectedTeamIndex < 0) {
        SelectedTeamIndex = 0;
    } else if (SelectedTeamIndex >= 4) {
        SelectedTeamIndex = 3;
    }
    SelectionRows[0] = SelectedTeamIndex;
    Dialogue[4] = (unsigned char)SelectedTeamIndex + 0x22;
    GlyphRows[SelectedTeamIndex * 0x1C + 2] = -6;
    GlyphRows[SelectedTeamIndex * 0x1C] = 0;
    UiRows[0x16] = SelectedTeamIndex;
L_54:
    q = UiRows + 0xC;
    q[SelectedTeamIndex] = 2;
    if (MenuConfirmPressed != 0) {
        s0 = BadgeRows->s10;
        if (s0 == 3) {
            PlaySfx(1);
            ModeFrame = 0;
            ResetScreenCounters();
            r = SelectionState;
            *(short *)(r + 0x1A) = 1;
            r[0x1C] = SelectionRows[0];
            BadgeRows->s10 = 4;
            UiRows[0x14] = s0;
            ModeGroup = 1;
            ModeId = 0xB;
            ModeSubState = 0;
        }
    }
}

/* legacy typing of shared globals in this part of the unit */
#define MenuGridRecords (*(int (*))&MenuGridRecords)

int AllocMenuGridRecords(int a, int n) {
    int sz = n * 56;
    MenuGridRecords = a;
    PsyqBzero(a, sz);
    return sz;
}

#undef MenuGridRecords

void InitMenuGrid(s32 index, s32 layout, s32 count, s32 x, s32 y) {
    register s32 six = 6;
    register s32 quotient = six / count;
    register s32 offset;
    register u8 *page;
    /* ASM_BARRIER: retain the captured record in V1 through the copy. */
    register RrMenuGrid38 *entry asm("$3");
    register RrMenuGrid38 *next;
    register RrMenuGrid38 *last;
    const RrGridRow8 *row, *table;
    register s32 step_y;
    s32 step_x;
    register s32 layout_offset;
    /* ASM_BARRIER: retain both captured packed words in T4/T5 until stores. */
    register u32 first asm("$12"), second asm("$13");
    /* PAL RAM page owns the full-width record-pointer slot at offset 0x3A28. */
    page = (u8 *)0x800F0000U;
    offset = index * 0x38;
    step_y = y;
    /* ASM_BARRIER: capture the full stack argument before its signed16 narrowing. */
    __asm__("" : "+r"(step_y));
    entry = (RrMenuGrid38 *)((unsigned long)offset + (unsigned long)(*(RrMenuGrid38 **)(page + 0x3A28)));
    entry->columns2E = quotient * 3;
    ((RrMenuGrid38 *)((unsigned long)offset + (unsigned long)(*(RrMenuGrid38 **)(page + 0x3A28))))->rows2F = six;
    entry = (RrMenuGrid38 *)((unsigned long)offset + (unsigned long)(*(RrMenuGrid38 **)(page + 0x3A28)));
    step_y = (s16)step_y / (s32)entry->rows2F;
    step_x = (s16)x / (s32)((u32)count * (u32)entry->columns2E);
    layout_offset = layout * 8;
    entry->count2C = count;
    entry->width04 = 64;
    entry->height06 = 8;
    table = MenuGridLayoutRows;
    entry->step_y0A = step_y;
    /* ASM_BARRIER: reload the record-pointer slot after publishing the dimensions. */
    __asm__("" : : : "memory");
    next = (RrMenuGrid38 *)((unsigned long)offset + (unsigned long)(*(RrMenuGrid38 **)(page + 0x3A28)));
    row = (const RrGridRow8 *)((u8 *)table + layout_offset);
    entry->step_x08 = step_x;
    first = row->first;
    second = row->second;
    /* ASM_BARRIER: stage the captured first word before the packed destination writes. */
    __asm__("" : : "r"(first));
    entry->layout30.first = first;
    entry->layout30.second = second;
    ((u8 *)&next->layout30)[2] = (u32)((u8 *)&next->layout30)[2] / next->columns2E;
    last = (RrMenuGrid38 *)((unsigned long)offset + (unsigned long)(*(RrMenuGrid38 **)(page + 0x3A28)));
    ((u8 *)&last->layout30)[3] = (u32)((u8 *)&last->layout30)[3] / last->rows2F;
}
