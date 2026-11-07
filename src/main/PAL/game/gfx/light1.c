/* CC1_FLAGS: -fno-schedule-insns */
#include "scratchpad.h"
#include "rr_asm.h"
#include "rr/menu_overlay_runtime.h"
void SetSineLightColors(a0) int a0;
{
    int *p; 
    register int x asm("$17"); /* ASM_BARRIER: pin to s-reg like the original saves */
    register int s2 asm("$18"); /* ASM_BARRIER: pin to s-reg like the original saves */
    register int a asm("$19"); /* ASM_BARRIER: pin to s-reg like the original saves */
    int v0, v1, z;
    x = a0;
    p = (int *)(SCRATCHPAD + 0x7C);
    if (x < 0) a0 = x + 3;
    a = a0 >> 2;
    s2 = rsin(a - 0x400) + 0x1000;
    v0 = rsin(a);
    v1 = (s2 << 16) | (s2 & 0xFFFF);
    p[0] = v1;
    p[1] = v1;
    p[2] = v1;
    p[3] = (v0 << 16) | (v0 & 0xFFFF);
    p[4] = v0;
    RR_GTE_SETLCM(p);
    z = 0x40;
    RR_GTE_SETBK(z, z, z);
    asm volatile("" : : "r"(s2)); /* ASM_BARRIER: keep s2 live so mask goes through a0 */
}
