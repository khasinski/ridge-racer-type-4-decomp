#ifndef RR_MENU_GRID_H
#define RR_MENU_GRID_H

#include "rr/layout.h"

/* Two words copied with unaligned loads/stores; both are captured first. */
typedef struct {
    u32 first, second;
} __attribute__((packed)) RrGridRow8;

typedef struct {
    u8 pad00[4];
    s16 width04, height06, step_x08, step_y0A;
    u8 pad0C[0x20];
    s16 count2C;
    u8 columns2E, rows2F;
    RrGridRow8 layout30;
} RrMenuGrid38;

extern RrMenuGrid38 *MenuGridRecords;
extern RrGridRow8 MenuGridLayoutRows[];

/* Index and layout select accessible records/rows; their byte offsets must fit
 * signed32. Count is nonzero and abs(count)<=6, so all original divisions are
 * defined. X/Y narrow to signed16 before division. Copies all eight layout bytes
 * after publishing the dimensions and steps, then scales bytes 0x32/0x33 in place.
 * Pointer slots retain full native widths; the PAL slot is at 0x800F3A28. */
void InitMenuGrid(s32 index, s32 layout, s32 count, s32 x, s32 y);

RR_ASSERT_SIZE(RrGridRow8, 8);
RR_ASSERT_OFFSET(RrGridRow8, second, 4);
RR_ASSERT_SIZE(RrMenuGrid38, 0x38);
RR_ASSERT_OFFSET(RrMenuGrid38, width04, 4);
RR_ASSERT_OFFSET(RrMenuGrid38, height06, 6);
RR_ASSERT_OFFSET(RrMenuGrid38, step_x08, 8);
RR_ASSERT_OFFSET(RrMenuGrid38, step_y0A, 0xA);
RR_ASSERT_OFFSET(RrMenuGrid38, count2C, 0x2C);
RR_ASSERT_OFFSET(RrMenuGrid38, columns2E, 0x2E);
RR_ASSERT_OFFSET(RrMenuGrid38, rows2F, 0x2F);
RR_ASSERT_OFFSET(RrMenuGrid38, layout30, 0x30);

#endif
