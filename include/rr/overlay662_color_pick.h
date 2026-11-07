#ifndef RR_OVERLAY662_COLOR_PICK_H
#define RR_OVERLAY662_COLOR_PICK_H
#include "rr/overlay662_garage.h"
#include "rr/menu_choice.h"
/* Complete64x64 four-bit image: each row has32 bytes. The shared garage
 * fields publish its first word; byte reads use a character view of this owner. */
typedef u32 RrGarageImage512[512];
/* An in-bounds hit with button bit2 set requires active21=0/1, zoom
 *2/4/8/16, and each scroll origin in
 *0..64-128/zoom. Both selected images cover512 words. The first nonzero
 * nibble across image38 then image40 wins; two transparent pixels reject. */
void garage_func_80125370(void);
RR_ASSERT_SIZE(RrGarageImage512,2048);
#endif
