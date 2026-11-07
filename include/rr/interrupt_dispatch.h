#ifndef RR_INTERRUPT_DISPATCH_H
#define RR_INTERRUPT_DISPATCH_H
#include "rr/library.h"
#include "rr/diagnostics.h"
extern RrLibSavedState IntrEnv;
extern volatile u16 *I_STAT,*I_MASK;
extern s32 IntrStuckCount;
extern const char UnexpectedInterruptFmt[],IntrTimeoutFmt[];
void ReturnFromException(void);
void trapIntr(void);
/* I_STAT acknowledgements write zero to the serviced bits. The native protocol
 * fixture supplies the register's write-zero-to-clear behavior here. */
#ifndef RR_IRQ_ACK16
#define RR_IRQ_ACK16(port,value) (*(port)=(u16)(value))
#endif
#endif
