#ifndef RR_OVERLAY662_OPTIONS_H
#define RR_OVERLAY662_OPTIONS_H
#include "rr/overlay662_garage.h"
/* Eight genuine ROM grids:40-byte rows, ten destination button indices.
 * Exact array boundaries run268CC..26C14. First three grids select horizontal
 * navigation; the remaining five select vertical navigation. */
struct RrGarageOptionRecord {s32 destinations[10];};
extern RrGarageOptionRecord garage_D_801268CC[5];
extern RrGarageOptionRecord garage_D_80126994[2];
extern RrGarageOptionRecord garage_D_801269E4[2];
extern RrGarageOptionRecord garage_D_80126A34[2];
extern RrGarageOptionRecord garage_D_80126A84[3];
extern RrGarageOptionRecord garage_D_80126AFC[5];
extern RrGarageOptionRecord garage_D_80126BC4[1];
extern RrGarageOptionRecord garage_D_80126BEC[1];
/* Nonzero fullword mode selects first group0..2; zero selects second group0..4.
 * The corresponding stored row must fit that grid and column must be0..9.
 * Its destination selects an initialized rectangle table entry. */
void garage_func_8011E5C0(s32 mode);
RR_ASSERT_SIZE(RrGarageOptionRecord,40);
#endif
