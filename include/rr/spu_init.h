#ifndef RR_SPU_INIT_H
#define RR_SPU_INIT_H

#include "rr/spu.h"

/* DMA priority register and the software state reset by SPU initialization. */
extern volatile u32 *SPU_DPCR;
extern s32 _spu_addrMode;
extern volatile u32 D_800A4814;
extern const u16 D_800A4818[8];
extern u16 _spu_RQ[10];
extern const char SpuTimeoutFmt[], SpuWaitResetStr[];
s32 DebugPrintStub(const char *format, ...);
s32 _spu_init(s32 warm);

#endif
