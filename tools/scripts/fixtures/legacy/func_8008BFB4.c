#include "types.h"
#include "m2c_macros.h"

extern u8 CD_com;

u8 CdLastCom(void) {
    return CD_com;
}
