/* GCC_VERSION: 2.7.2 */
#include "types.h"
#include "m2c_macros.h"
#include "psyq/libcd/s_4.h"
#include "rr/library.h"

extern int CdAltSyncCallbackPtr[];
extern int CdAltReadyCallbackFunc[];
extern int CdAltReadCallbackFunc[];

int CdAltSyncCallback();
int CdAltReadCallback();

int CdAltSyncCallback(a0) int a0; { int *p = CdAltSyncCallbackPtr; int r = *p; *p = a0; return r; }

int CdAltReadyCallback(a0) int a0; { int *p = CdAltReadyCallbackFunc; int r = *p; *p = a0; return r; }

int CdAltReadCallback(a0) int a0; { int *p = CdAltReadCallbackFunc; int r = *p; *p = a0; return r; }

void CdAltDataCallback(s32 arg0) {
    DMACallback(3, arg0);
}
