#ifndef RR_GAME_CAR_GRID_H
#define RR_GAME_CAR_GRID_H

#include "rr/layout.h"

/* Start grid positions. */
typedef struct {
    s32 trackNode00;
    s32 reserved04[3];
    struct {
        s32 x, z;
    } positions10[8];
    s16 headings50[8];
} RaceStartGrid60;

RR_ASSERT_SIZE(RaceStartGrid60, 0x60);
RR_ASSERT_OFFSET(RaceStartGrid60, positions10, 0x10);
RR_ASSERT_OFFSET(RaceStartGrid60, headings50, 0x50);

#endif
