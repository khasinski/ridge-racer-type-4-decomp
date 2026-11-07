/* GCC_VERSION: 2.7.2 */
#include "types.h"
s32 _getctl(s32 index) {
    /* PAL RAM mapping: GpuCtlBuf is 0x27D0 bytes into this page. */
    u8 *base = (u8 *)0x80110000U;
    /* ASM_BARRIER: retain the base register through the indexed byte load. */
    __asm__("" : "=r"(base) : "0"(base));
    base += index;
    return base[0x27D0];
}
