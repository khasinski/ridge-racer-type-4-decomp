#include "rr/garage_distance.h"

s32 garage_calc_glide_step(const RrGaragePoint *a, const RrGaragePoint *b) {
    u32 dx = (u32)(b->x - a->x) * 2U;
    u32 dy = (u32)(b->y - a->y);

    return 4096 / (((CordicSqrt((s32)((dx * dx) + (dy * dy))) >> 6) / 64) + 1);
}
