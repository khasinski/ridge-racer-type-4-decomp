#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define rsin ((int (*)())rsin)

#define rcos ((int (*)())rcos)

#undef rsin

#undef rcos

/* legacy typing of shared globals in this part of the unit */
#define rsin ((int (*)())rsin)

#define rcos ((int (*)())rcos)

#undef rsin

#undef rcos

extern short D_800AC0A4;

extern unsigned short D_800AD598[];

extern char CameraViewMatrix[];

extern char CameraPitchRollMatrix[];

extern char D_800FF6D8[];

extern void MulMatrix0();

extern void MulMatrix2();

extern void SetRotMatrix();

void BuildCameraMatrices(void) {
    char sp10[32];
    int *g = (int *)0x1F800000;
    BuildRotMatrixY(0x1F800028, g[7]);
    BuildRotMatrixY(CameraViewMatrix, g[7]);
    SetIdentityMatrix(CameraPitchRollMatrix);
    BuildRotMatrixY(D_800FF6D8, g[7]);
    BuildRotMatrixX(sp10, g[6]);
    MulMatrix2(sp10, 0x1F800028);
    MulMatrix2(sp10, CameraViewMatrix);
    MulMatrix2(sp10, CameraPitchRollMatrix);
    BuildRotMatrixZ(sp10, g[8]);
    MulMatrix2(sp10, 0x1F800028);
    MulMatrix2(sp10, CameraViewMatrix);
    MulMatrix2(sp10, CameraPitchRollMatrix);
    BuildRotMatrixY(sp10, 0x800);
    MulMatrix0(sp10, 0x1F800028, D_800AD598);
    BuildRotMatrixX(sp10, D_800AC0A4 * 2);
    MulMatrix2(sp10, D_800AD598);
    D_800AD598[0] = -D_800AD598[0];
    D_800AD598[1] = -D_800AD598[1];
    D_800AD598[2] = -D_800AD598[2];
    SetRotMatrix(0x1F800028);
}
