/* GCC_VERSION: 2.7.2 */

#include "types.h"

extern int CdAltReadyCallbackFunc[];

int CdAltReadyCallback(a0) int a0; { int *p = CdAltReadyCallbackFunc; int r = *p; *p = a0; return r; }
