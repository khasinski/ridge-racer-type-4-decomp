#include "rr/overlay662_garage.h"
/* Overlay-local storage; the same RAM can have another meaning in another image. */
void garage_func_8011E7C0(s32 a, s32 b, s32 c, s32 d)
{
    garage_D_80127668.halves[24] = a;
    garage_D_80127668.halves[25] = b;
    garage_D_80127668.halves[26] = c;
    garage_D_80127668.halves[27] = d;
}
