#ifndef RR_PSYQ_LIBCD_DSSTRM32_H
#define RR_PSYQ_LIBCD_DSSTRM32_H

#include "types.h"

extern volatile u8 *CD_REG0_STRM;
extern int D_801102EC;
extern int StRingWriteIndex;
extern int StRingStartIndex;
extern int StRingBase;
extern int StMaskEnabled;
void StCdInterrupt(void);

#endif /* RR_PSYQ_LIBCD_DSSTRM32_H */
