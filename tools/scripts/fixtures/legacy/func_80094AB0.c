/* GCC_VERSION: 2.7.2 */
#include "types.h"
extern u32 *GPU_STATUS;
void _ctl(u32 command) {
    /* ASM_BARRIER: reuse v0 for the port pointer and then the command index. */
    register u32 index __asm__("$2");
    /* ASM_BARRIER: retain the original assembler temporary for the history address. */
    register u8 *base __asm__("$1");
    {
        /* ASM_BARRIER: keep the command-port pointer in v0 until the word is written. */
        register u32 *port __asm__("$2") = GPU_STATUS;
        *port = command;
    }
    index = command >> 24;
    /* ASM_BARRIER: finish the port write before binding the history address. */
    __asm__ volatile("" : "=r"(index) : "0"(index) : "$1", "memory");
    /* PAL RAM mapping: GpuCtlBuf is base plus 0x27D0. */
    base = (u8 *)0x80110000U;
    /* ASM_BARRIER: retain the base-plus-index byte store in the return delay slot. */
    __asm__("" : "=r"(base) : "0"(base));
    base += index;
    base[0x27D0] = command;
}
