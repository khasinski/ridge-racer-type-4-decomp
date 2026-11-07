/* GCC_VERSION: 2.7.2 */
#include "psyq/libsnd/vmreset.h"
#include "psyq/libspu/sinit.h"
#include "rr/channels.h"
#include "rr/library.h"
extern unsigned short SsVoiceInitRegs[];
extern unsigned short D_800A4914[];
extern int _snd_openflag;
extern int VBLANK_MINUS;
void _SsInit(void) {
    unsigned short *d, *s, *p; int i, j;
    d = (unsigned short *)0x1F801C00;
    i = 0;
    p = SsVoiceInitRegs;
    do {
        j = 0;
        s = p;
        do { *d++ = *s++; j++; } while (j < 8);
        i++;
    } while (i < 0x18);
    d = (unsigned short *)0x1F801D80;
    i = 0;
    s = D_800A4914;
    do { *d++ = *s++; i++; } while (i < 0x10);
    _SsVmInit(0x18, j, d, p);
    for (j = 0; j < 0x20; j++)
        for (i = 0; i < 16; i++) _SsMarkCallback[j][i] = 0;
    VBLANK_MINUS = 0x3C;
    _snd_openflag = 0;
    _snd_ev_flag = 0;
}
