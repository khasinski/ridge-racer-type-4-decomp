/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "types.h"
#include "rr/trig.h"
#include "psyq/libcd/dsstrm32.h"

extern const char DmaStatusErrorFmt[];
extern volatile s32 *CdDpcrReg;
extern volatile s32 *DICR;
extern void DebugPrintStub(const char *, ...);

void mem2mem(int *d, int *s, unsigned n);
void dma_execute(s32 ch, s32 madr, s32 bcr_hi, s32 bcr_lo, volatile s32 chcr, u8 flag_in);
s32 rsin(s32 a);

void mem2mem(int *d, int *s, unsigned n) {
    unsigned i;
    for (i = 0; i < n; i++) *d++ = *s++;
}

void dma_execute(s32 ch, s32 madr, s32 bcr_hi, s32 bcr_lo, volatile s32 chcr, u8 flag_in) {
    volatile s32 dummy;
    s32 i = 0;
    /* ASM_BARRIER: pin p to a1 */
    register volatile s32 *p __asm__("$5");
    /* ASM_BARRIER: pin cond to v0 */
    register s32 cond __asm__("$2");
    /* ASM_BARRIER: pin bit24 to v1 */
    register s32 bit24 __asm__("$3") = 0x01000000;
    s32 flag = flag_in;

    p = (volatile s32 *)(ch << 4);
    if ((cond = *(volatile s32 *)((s32)p + 0x1F801088) & bit24)) {
        do {
            /* ASM_BARRIER: keep v0 live at loop entry so beqz does not steal target insn */
            __asm__ volatile("" : : "r"(cond));
            if (i == 0x10000) {
                DebugPrintStub(DmaStatusErrorFmt, *(volatile s32 *)((s32)p + 0x1F801088));
                goto after_loop;
            }
            i++;
        } while ((cond = *(volatile s32 *)((s32)p + 0x1F801088) & bit24));
        {
            /* ASM_BARRIER: mark a2 unused before join label so reorg puts 0x10000 in beqz delay slot */
            register s32 dead_a2 __asm__("$6");
            /* ASM_BARRIER: materialize dead_a2 on normal loop exit */
            __asm__ volatile("" : "=r"(dead_a2));
        }
    }
after_loop:
    {
        /* ASM_BARRIER: pin mask to v0 */
        register s32 mask __asm__("$2") = 1;
        /* ASM_BARRIER: pin dicr to v1 */
        register u8 *dicr __asm__("$3");
        if (flag == mask) {
            dicr = (u8 *)DICR;
            mask = dicr[2] | (mask << ch);
        } else {
            dicr = (u8 *)DICR;
            mask = dicr[2] & ~(mask << ch);
        }
        dicr[2] = mask;
    }
    dummy = *DICR;
    /* ASM_BARRIER: prevent scheduling across dummy store */
    __asm__ volatile("" : : : "memory");
    {
        /* ASM_BARRIER: pin shift to a2 */
        register s32 shift __asm__("$6") = ch << 2;
        /* ASM_BARRIER: pin mask3 to v1 */
        register s32 mask3 __asm__("$3");
        /* ASM_BARRIER: pin bcr to v0 */
        register s32 bcr __asm__("$2");
        /* ASM_BARRIER: pin d88 to a0 */
        register volatile s32 *d88 __asm__("$4");
        /* ASM_BARRIER: prevent combining sll and addiu into ori */
        __asm__ volatile("" : "=r"(shift) : "0"(shift));
        shift += 3;
        mask3 = 1;
        /* ASM_BARRIER: order addiu a2 before addiu v1 */
        __asm__ volatile("" : "=r"(shift), "=r"(mask3) : "0"(shift), "1"(mask3));
        mask3 <<= shift;
        p = (volatile s32 *)0x1F801080;
        bcr = ch << 4;
        /* ASM_BARRIER: order lui/ori a1 before sll v0 */
        __asm__ volatile("" : "=r"(p), "=r"(bcr), "=r"(mask3) : "0"(p), "1"(bcr), "2"(mask3));
        p = (volatile s32 *)(bcr + (s32)p);
        /* ASM_BARRIER: prevent constant folding of p + 1 */
        __asm__ volatile("" : "=r"(p), "=r"(mask3) : "0"(p), "1"(mask3));
        d88 = CdDpcrReg;
        /* ASM_BARRIER: allocate madr to s2 before bcr_hi/bcr_lo to s3/s4 */
        __asm__ volatile("" : : "r"(madr));
        bcr = (bcr_hi << 16) | bcr_lo;
        *d88 |= mask3;
        *p = madr;
        p++;
        *p = bcr;
    }
    p++;
    while ((*CD_REG0_STRM & 0x40) == 0) {
    }
    *p = chcr;
    dummy = *p;
}

s32 rsin(s32 a) {
    int r;
    if (a >= 0) {
        r = sin_1(a & 0xFFF);
    } else {
        r = -sin_1((0U - (u32)a) & 0xFFFU);
    }
    return r;
}

s32 sin_1(s32 a) {
    if (a < 0x801) {
        if (a < 0x401) return SinTable[a];
        return SinTable[0x800 - a];
    }
    if (a < 0xC01) return -D_800A58C0[a];
    return -SinTable[0x1000 - a];
}
