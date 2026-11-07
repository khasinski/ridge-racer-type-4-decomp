#ifndef RR_GARAGE_OPTIONS_H
#define RR_GARAGE_OPTIONS_H
#include "rr/garage_garage.h"
/* Eight genuine ROM grids:40-byte rows, ten destination button indices.
 * Exact array boundaries run268CC..26C14. First three grids select horizontal
 * navigation; the remaining five select vertical navigation. */
struct RrGarageOptionRecord {s32 destinations[10];};
extern RrGarageOptionRecord garage_nav_grid_h0[5];
extern RrGarageOptionRecord garage_nav_grid_h1[2];
extern RrGarageOptionRecord garage_nav_grid_h2[2];
extern RrGarageOptionRecord garage_nav_grid_v0[2];
extern RrGarageOptionRecord garage_nav_grid_v1[3];
extern RrGarageOptionRecord garage_nav_grid_v2[5];
extern RrGarageOptionRecord garage_nav_grid_v3[1];
extern RrGarageOptionRecord garage_nav_grid_v4[1];
/* Nonzero fullword mode selects first group0..2; zero selects second group0..4.
 * The corresponding stored row must fit that grid and column must be0..9.
 * Its destination selects an initialized rectangle table entry. */
void garage_navigate_to_option(s32 mode);
RR_ASSERT_SIZE(RrGarageOptionRecord,40);
#endif
