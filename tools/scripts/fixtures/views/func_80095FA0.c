#include "rr/memory.h"

extern void StopCARD2(void),_copy_memcard_patch_end(void);

void _copy_memcard_patch(void) {
    /* ASM_BARRIER: Retain the kernel destination in the original cursor register. */
    register unsigned long destination __asm__("$2")=0xDF80U;
    /* ASM_BARRIER: Retain the source code cursor in the original register. */
    register unsigned long source __asm__("$10")=(unsigned long)StopCARD2+0x10;
    /* ASM_BARRIER: Keep the source endpoint in its original register. */
    register unsigned long limit __asm__("$9")=(unsigned long)_copy_memcard_patch_end;
    do {
        *(u32 *)destination=*(const u32 *)source;
        /* ASM_BARRIER: Publish each code word before advancing the source cursor. */
        __asm__ volatile("" : : "r"(source) : "memory");
        source+=sizeof(u32);
        destination+=sizeof(u32);
    } while(source!=limit);
}
