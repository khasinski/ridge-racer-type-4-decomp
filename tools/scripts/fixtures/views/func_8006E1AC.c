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

extern short RearViewPitch;

extern unsigned short RearViewRotMatrix[];

extern char CameraViewMatrix[];

extern char CameraPitchRollMatrix[];

extern char CameraYawMatrix[];

extern void MulMatrix0();

extern void MulMatrix2();

extern void SetRotMatrix();

void BuildCameraMatrices(void) {
    char sp10[32];
    int *g = (int *)0x1F800000;
    BuildRotMatrixY(0x1F800028, g[7]);
    BuildRotMatrixY(CameraViewMatrix, g[7]);
    SetIdentityMatrix(CameraPitchRollMatrix);
    BuildRotMatrixY(CameraYawMatrix, g[7]);
    BuildRotMatrixX(sp10, g[6]);
    MulMatrix2(sp10, 0x1F800028);
    MulMatrix2(sp10, CameraViewMatrix);
    MulMatrix2(sp10, CameraPitchRollMatrix);
    BuildRotMatrixZ(sp10, g[8]);
    MulMatrix2(sp10, 0x1F800028);
    MulMatrix2(sp10, CameraViewMatrix);
    MulMatrix2(sp10, CameraPitchRollMatrix);
    BuildRotMatrixY(sp10, 0x800);
    MulMatrix0(sp10, 0x1F800028, RearViewRotMatrix);
    BuildRotMatrixX(sp10, RearViewPitch * 2);
    MulMatrix2(sp10, RearViewRotMatrix);
    RearViewRotMatrix[0] = -RearViewRotMatrix[0];
    RearViewRotMatrix[1] = -RearViewRotMatrix[1];
    RearViewRotMatrix[2] = -RearViewRotMatrix[2];
    SetRotMatrix(0x1F800028);
}
