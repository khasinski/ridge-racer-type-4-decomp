#include "rr/menu_grid.h"

/* legacy typing of shared globals in this part of the unit */
#define MenuGridRecords (*(int (*))&MenuGridRecords)

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
