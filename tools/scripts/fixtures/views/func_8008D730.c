/* GCC_VERSION: 970318 */

/* MASPSX_FLAGS: --assembler-profile=gnu */

#include "types.h"

#define R(o) (*(unsigned short *)(CdSpuRegBase + (o)))

/* legacy typing of shared globals in this part of the unit */
#define CD_cbsync (*(void (*(*))(u8, u8 *))&CD_cbsync)

#define CD_cbready (*(void (*(*))(u8, u8 *))&CD_cbready)

#undef CD_cbsync

#undef CD_cbready

/* legacy typing of shared globals in this part of the unit */
#define CD_reg0 ((unsigned char *)CD_reg0)

#define CD_REG1 ((unsigned char *)CD_REG1)

#define CD_REG2 ((unsigned char *)CD_REG2)

#undef CD_reg0

#undef CD_REG1

#undef CD_REG2

/* legacy typing of shared globals in this part of the unit */
#define CD_reg0 ((signed char *)CD_reg0)

#define CD_REG2 ((signed char *)CD_REG2)

#undef CD_reg0

#undef CD_REG2

/* legacy typing of shared globals in this part of the unit */
#define CD_reg0 ((unsigned char *)CD_reg0)

#define CD_REG1 ((unsigned char *)CD_REG1)

#define CD_REG2 ((unsigned char *)CD_REG2)

#undef CD_reg0

#undef CD_REG1

#undef CD_REG2

extern volatile u8 *CD_reg0;

extern volatile u8 *CD_REG1;

extern volatile u8 *CD_REG2;

extern unsigned char *CD_reg3;

int CD_initvol(void) {
    unsigned char b[4];
    if (R(0x1B8) == 0 && R(0x1BA) == 0) {
        R(0x180) = 0x3FFF;
        R(0x182) = 0x3FFF;
    }
    R(0x1B0) = 0x3FFF;
    R(0x1B2) = 0x3FFF;
    R(0x1AA) = 0xC001;
    b[2] = 0x80;
    b[0] = 0x80;
    b[3] = 0;
    b[1] = 0;
    *CD_reg0 = 2;
    *CD_REG2 = b[0];
    *CD_reg3 = b[1];
    *CD_reg0 = 3;
    *CD_REG1 = b[2];
    *CD_REG2 = b[3];
    *CD_reg3 = 0x20;
    return 0;
}
