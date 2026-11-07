#include "rr/cd.h"
#include "rr/movie_playback_stream.h"

extern s32 PsyqStrncmp(const void *,const void *,u32);
extern s32 DebugPrintStub(const char *,...);
extern char CdCommandErrorStr[];
extern char StrIso9660Id[];

int CdDiskReady();

int CdDiskReady(a0)
int a0;
{
    unsigned char sp10[8];
    int r, i;
    CdControlB(1, 0, sp10);
    if (sp10[0] & 0x10) return 0x10;
    r = CdControlB(0x13, 0, sp10);
    if (a0 == 1) {
        if (sp10[0] == 2 && r != 0) return 2;
        return 5;
    }
    for (i = 0; i < 10; i++) {
        if (sp10[0] & 2) {
            if (sp10[0] == 2 && r != 0) return 2;
            while (1) {
                VSync(0x1E);
                r = CdControlB(0x13, 0, sp10);
                if (sp10[0] & 0x10) return 0x10;
                if (sp10[0] == 2 && r != 0) return 2;
            }
        }
        VSync(0x1E);
        r = CdControlB(0x13, 0, sp10);
        if (sp10[0] & 0x10) return 0x10;
    }
    return 5;
}

int CdGetDiskType(void) {
    struct { u32 a[2]; u8 buf[0x800],st[8]; } L;
    int i, r, v, c;
    s32 sector;
    CdControl(1, 0, L.st);
    if (L.st[0] & 0x10) return 0x10;
    sector=0x10;
    /* ASM_BARRIER: Keep the sector number independent of the equal stack-buffer offset. */
    __asm__("" : "+r"(sector));
    CdIntToPos(sector, (u8 *)L.a);
    CdControl(0x1B, (u8 *)L.a, 0);
    i = 0;
    c = 1;
    do {
        r = CdReady(0, L.st);
        v = r;
        i++;
        if (v == c) break;
        if (i >= 10) break;
        CdControl(0x1B, (u8 *)L.a, 0);
    } while (1);
    if (v != 1) {
        v = L.st[0];
        if (v & 0x10) return 0x10;
        if (v & 1) {
            if (L.st[1] & 0x40) {
                /* ASM_BARRIER: Begin message address formation after the status-bit gate. */
                __asm__("" : : : "$4");
                {
                const char *page=(const char *)0x80010000U;
                /* ASM_BARRIER: Expose the real PAL message page before forming its call argument. */
                __asm__("" : "+r"(page));
                DebugPrintStub(page+0x1D78);
                }
                return 1;
            }
        }
        if (L.st[0] & 2) return 1;
        return 0;
    }
    {
        const u8 *parameters;
        /* ASM_BARRIER: Keep the two null command pointers independent. */
        __asm__("" : "=r"(parameters) : "0"(0));
        CdControl(9, parameters, 0);
    }
    CdGetSector(L.buf, 0x200);
    v = PsyqStrncmp(L.buf + 1, StrIso9660Id, 5);
    if (v != 0) return 1;
    return 2;
}
