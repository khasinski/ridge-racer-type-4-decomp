#include "rr/overlay662_garage.h"
/* Overlay-local storage; the same RAM can have another meaning in another image. */
void garage_set_cursor_bounds(s32 a, s32 b, s32 c, s32 d)
{
    garage_logo_work.halves[24] = a;
    garage_logo_work.halves[25] = b;
    garage_logo_work.halves[26] = c;
    garage_logo_work.halves[27] = d;
}
