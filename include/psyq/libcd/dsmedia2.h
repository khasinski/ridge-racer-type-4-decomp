#ifndef RR_PSYQ_LIBCD_DSMEDIA2_H
#define RR_PSYQ_LIBCD_DSMEDIA2_H

#include "types.h"

extern int StCurFrameNo;
extern short StNextSectorOffset;
extern int D_80110348;
void StClearRing(void);
void data_ready_callback(void);

#endif /* RR_PSYQ_LIBCD_DSMEDIA2_H */
