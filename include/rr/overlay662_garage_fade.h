#ifndef RR_OVERLAY662_GARAGE_FADE_H
#define RR_OVERLAY662_GARAGE_FADE_H
#include "rr/overlay662_input.h"
void PlaySfx(s32 sound);
void garage_update_cursor_glide(void);
/* GTE INTPL protocol: two packed signed-halfword points supply IR1/IR2 and
 * far-colour1/2; IR0 receives factor. IR3/far-colour3 keep their previous
 * inputs. INTPL also changes other GTE result registers/FLAG; this macro is
 * not a pure two-coordinate arithmetic operation. Only resulting IR1/IR2
 * are written to the destination word. Native tests override the hardware
 * boundary and do not claim to emulate hardware timing or saturation. */
#ifndef RR_GARAGE_GTE_LERP
#define RR_GARAGE_GTE_LERP(factor,source,destination) __asm__ volatile( \
        "lw $8, 0(%1)\n\t" \
        "lw $9, 4(%1)\n\t" \
        "sra $10, $8, 16\n\t" \
        "sll $8, $8, 16\n\t" \
        "sra $8, $8, 16\n\t" \
        "mtc2 $8, $9\n\t" \
        "mtc2 $10, $10\n\t" \
        "sra $10, $9, 16\n\t" \
        "sll $9, $9, 16\n\t" \
        "sra $9, $9, 16\n\t" \
        "ctc2 $9, $21\n\t" \
        "ctc2 $10, $22\n\t" \
        "mtc2 %0, $8\n\t" \
        "nop\n\t" \
        ".word 0x4A980011\n\t" \
        "mfc2 $8, $9\n\t" \
        "mfc2 $9, $10\n\t" \
        "sh $8, 0(%2)\n\t" \
        "sh $9, 2(%2)" \
        : \
        : "r"(factor), "r"(source), "r"(destination) \
        : "$8", "$9", "$10", "memory")
#endif
#endif
