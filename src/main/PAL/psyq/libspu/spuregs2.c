/* CC1_FLAGS: -mno-split-addresses */
#include "psyq/libspu/spuregs2.h"
#include "psyq/libspu/spuregs2.h"
extern int _spu_init();
extern int _spu_FsetRXX();
extern int SpuStart();
extern int ResetCallback();
extern int _spu_transMode;
extern int _spu_keystat;
extern int _spu_rev_offsetaddr;
extern struct { int a; short b, c; int d, e; } _spu_rev_attr;
extern int _spu_RQmask; extern int _spu_RQvoice;
extern unsigned short SpuVoiceCenterNoteLast;
extern int _spu_env; extern int _spu_AllocBlockNum;
extern int _spu_AllocLastNum;
extern int _spu_memList;
extern int _spu_rev_startaddr;
void _SpuInit(a0) int a0; {
    unsigned short *p;
    int i; unsigned c; struct { int a; short b, c; int d, e; } *q;
    ResetCallback();
    _spu_init(a0);
    c = 0xC000;
    if (a0 == 0) {
        i = 0x17;
        p = &SpuVoiceCenterNoteLast;
        do {
            *p = c;
            i--;
            p--;
        } while (i >= 0);
    }
    SpuStart(c);
    _spu_rev_flag = 0;
    _spu_rev_reserve_wa = 0;
    q = &_spu_rev_attr;
    q->a = 0;
    q->b = 0;
    q->c = 0;
    q->d = 0;
    q->e = 0;
    _spu_rev_offsetaddr = _spu_rev_startaddr;
    _spu_FsetRXX(0xD1, _spu_rev_startaddr, 0);
    _spu_AllocBlockNum = 0;
    _spu_AllocLastNum = 0;
    _spu_memList = 0;
    _spu_trans_mode = 0;
    _spu_transMode = 0;
    _spu_keystat = 0;
    _spu_RQmask = 0;
    _spu_RQvoice = 0;
    _spu_env = 0;
    /* ASM_BARRIER: keeps epilogue stack loads (lw ra/s*) after the final global stores; sched2 otherwise hoists/sinks them */
    __asm__ volatile ("" : : : "memory");
}
