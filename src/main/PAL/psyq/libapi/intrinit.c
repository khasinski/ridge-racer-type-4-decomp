/* CC1_FLAGS: -mno-split-addresses */
#include "psyq/libapi/intrdisp.h"
#include "psyq/libapi/intrdisp.h"
#include "psyq/libapi/vsyncini.h"
extern int trapIntr();
extern int startIntrDMA();
extern int PsyqSetjmp();
extern int _96_remove();
extern int HookEntryInt();
extern int ExitCriticalSection();
extern unsigned short D_800A52C8;
extern char IntrStackArea[];
extern int *IntrDriverTablePtr;
extern unsigned short *I_STAT;
extern unsigned short *I_MASK;
char *startIntr()
{
    char *p, *r;
    unsigned short *q = &D_800A52C8;
    unsigned short *a, *b;
    if (*q == 0) {
        a = I_STAT;
        b = I_MASK;
        *b = 0;
        *a = *(volatile unsigned short *)b;
        *DPCR_Intr = 0x33333333;
        memclr(q, 0x41A);
        if (PsyqSetjmp((char *)q + 0x38)) trapIntr();
        p = IntrStackArea;
        *(char **)p = p + 0xFDC;
        HookEntryInt(p - 4);
        *(short *)(p - 0x3C) = 1;
        IntrDriverTablePtr[5] = startIntrVSync();
        IntrDriverTablePtr[1] = startIntrDMA();
        _96_remove(IntrDriverTablePtr);
        r = p - 0x3C;
        ExitCriticalSection();
        return r;
    }
    return 0;
}
