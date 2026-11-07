/* GCC_VERSION: 2.7.2 */
#include "rr/channels.h"
s32 SsUtKeyOffV(s32 a0) {
    int r;
    int h = a0;
    int selected = a0;
    if (_snd_ev_flag == 1) { r = -1; goto out; }
    _snd_ev_flag = 1;
    if ((unsigned)(selected & 0xFFFF) >= 0x18U) { 
        /* ASM_BARRIER: Use the original assembler temporary for the failure cleanup. */
        register int *page __asm__("$1");
        r=-1;
        page=(int *)0x80110000U;
        /* ASM_BARRIER: Expose the cleanup store for the shared-return delay slot. */
        __asm__("" : "=r"(page) : "0"(page));
        page[-0xFA4/4]=0;
        goto out; }
    SvmVoiceIndex = h;
    /* ASM_BARRIER: Keep the selector in a1 until the reset call. */
    __asm__("" : : "r"(selected));
    _SsVmKeyOffNow(0);
    _snd_ev_flag = 0;
    r = 0;
out:
    return r;
}
