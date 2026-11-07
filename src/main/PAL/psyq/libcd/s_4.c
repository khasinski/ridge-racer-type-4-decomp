/* GCC_VERSION: 2.7.2 */
#include "types.h"
#include "m2c_macros.h"
#include "psyq/libcd/s_4.h"
#include "rr/library.h"

extern int D_801143A8[];
extern int D_801143AC[];
extern int D_801143B0[];

int CdAltSyncCallback();
int CdAltReadCallback();

int CdAltSyncCallback(a0) int a0; { int *p = D_801143A8; int r = *p; *p = a0; return r; }

int CdAltReadyCallback(a0) int a0; { int *p = D_801143AC; int r = *p; *p = a0; return r; }

int CdAltReadCallback(a0) int a0; { int *p = D_801143B0; int r = *p; *p = a0; return r; }

void CdAltDataCallback(s32 arg0) {
    DMACallback(3, arg0);
}
