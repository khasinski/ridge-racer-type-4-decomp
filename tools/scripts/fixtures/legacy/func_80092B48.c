/* CC1_FLAGS: -mno-split-addresses */
#include "types.h"

extern u8 GpuDebugLevel[];
extern void (*GPU_printf)(const char *, s32, s32, s32);

s32 SetGraphDebug(s32 enabled) {
    u8 *state = GpuDebugLevel;
    s32 previous;
    previous = state[0];
    state[0] = enabled;
    if (enabled & 0xFF) {
        const char *format;
        void (*callback)(const char *, s32, s32, s32) = GPU_printf;
        s32 first = state[0], second = state[-2], third = state[1];
        /* ASM_BARRIER: read the callback and state bytes before materializing the format page. */
        __asm__ volatile("" : : "r"(callback), "r"(first), "r"(second), "r"(third) : "$4");
        /* PAL read-only mapping: format + 0x1E24 is SetGraphDebugFmt. */
        format = (const char *)0x80010000U;
        /* ASM_BARRIER: expose the page-relative addition in the indirect-call delay slot. */
        __asm__("" : "=r"(format) : "0"(format));
        callback(format + 0x1E24, first, second, third);
        return previous;
    }
    return previous;
}
