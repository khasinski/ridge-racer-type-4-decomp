#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "psyq/libgte/gtemat7.h"
#include "rr/matrix_workspace.h"
#include "rr/menu_overlay_runtime.h"

extern short D_800AC0A4;
extern unsigned short D_800AD598[];
extern char CameraViewMatrix[];
extern char CameraPitchRollMatrix[];
extern char D_800FF6D8[];
extern void MulMatrix0();
extern short AtanTable[];

void BuildRotMatrixZ(short *a0, int a1);
void BuildRotMatrixY(void *arg0, M2C_UNK arg1);
void BuildRotMatrixX(void *arg0, M2C_UNK arg1);
void SetIdentityMatrix(void *arg0);
void BuildRotMatrixAngles(int *a0);
s32 Atan2Table(int a0, int a1);

void BuildRotMatrixZ(short *a0, int a1) {
    short s = rsin(a1);
    short c = rcos(a1);
    a0[1] = -s;
    a0[0] = c;
    a0[4] = c;
    a0[2] = 0;
    a0[3] = s;
    a0[5] = 0;
    a0[6] = 0;
    a0[7] = 0;
    a0[8] = 0x1000;
}

/* legacy typing of shared globals in this part of the unit */
#define rsin ((int (*)())rsin)
#define rcos ((int (*)())rcos)

void BuildRotMatrixY(void *arg0, M2C_UNK arg1) {
    s16 temp_s1;
    s16 temp_v0;

    temp_s1 = rsin(arg1);
    temp_v0 = rcos(arg1);
    M2C_FIELD(arg0, s16 *, 4) = (s16) -temp_s1;
    M2C_FIELD(arg0, s16 *, 0) = temp_v0;
    M2C_FIELD(arg0, s16 *, 2) = 0;
    M2C_FIELD(arg0, s16 *, 6) = 0;
    M2C_FIELD(arg0, s16 *, 8) = 0x1000;
    M2C_FIELD(arg0, s16 *, 0xA) = 0;
    M2C_FIELD(arg0, s16 *, 0xC) = temp_s1;
    M2C_FIELD(arg0, s16 *, 0xE) = 0;
    M2C_FIELD(arg0, s16 *, 0x10) = temp_v0;
}

#undef rsin
#undef rcos

/* legacy typing of shared globals in this part of the unit */
#define rsin ((int (*)())rsin)
#define rcos ((int (*)())rcos)

void BuildRotMatrixX(void *arg0, M2C_UNK arg1) {
    s16 temp_s1;
    s16 temp_v0;

    temp_s1 = rsin(arg1);
    temp_v0 = rcos(arg1);
    M2C_FIELD(arg0, s16 *, 0) = 0x1000;
    M2C_FIELD(arg0, s16 *, 2) = 0;
    M2C_FIELD(arg0, s16 *, 4) = 0;
    M2C_FIELD(arg0, s16 *, 6) = 0;
    M2C_FIELD(arg0, s16 *, 8) = temp_v0;
    M2C_FIELD(arg0, s16 *, 0xA) = (s16) -temp_s1;
    M2C_FIELD(arg0, s16 *, 0xC) = 0;
    M2C_FIELD(arg0, s16 *, 0xE) = temp_s1;
    M2C_FIELD(arg0, s16 *, 0x10) = temp_v0;
}

#undef rsin
#undef rcos

void SetIdentityMatrix(void *arg0) {
    M2C_FIELD(arg0, s16 *, 0) = 0x1000;
    M2C_FIELD(arg0, s16 *, 2) = 0;
    M2C_FIELD(arg0, s16 *, 4) = 0;
    M2C_FIELD(arg0, s16 *, 6) = 0;
    M2C_FIELD(arg0, s16 *, 8) = 0x1000;
    M2C_FIELD(arg0, s16 *, 0xA) = 0;
    M2C_FIELD(arg0, s16 *, 0xC) = 0;
    M2C_FIELD(arg0, s16 *, 0xE) = 0;
    M2C_FIELD(arg0, s16 *, 0x10) = 0x1000;
    M2C_FIELD(arg0, s32 *, 0x14) = 0;
    M2C_FIELD(arg0, s32 *, 0x18) = 0;
    M2C_FIELD(arg0, s32 *, 0x1C) = 0;
}

void BuildCameraMatrices(void) {
    char sp10[32];
    int *g = (int *)SCRATCHPAD;
    BuildRotMatrixY(SCRATCH_ROT_MATRIX, g[7]);
    BuildRotMatrixY(CameraViewMatrix, g[7]);
    SetIdentityMatrix(CameraPitchRollMatrix);
    BuildRotMatrixY(D_800FF6D8, g[7]);
    BuildRotMatrixX(sp10, g[6]);
    MulMatrix2(sp10, SCRATCH_ROT_MATRIX);
    MulMatrix2(sp10, CameraViewMatrix);
    MulMatrix2(sp10, CameraPitchRollMatrix);
    BuildRotMatrixZ(sp10, g[8]);
    MulMatrix2(sp10, SCRATCH_ROT_MATRIX);
    MulMatrix2(sp10, CameraViewMatrix);
    MulMatrix2(sp10, CameraPitchRollMatrix);
    BuildRotMatrixY(sp10, 0x800);
    MulMatrix0(sp10, SCRATCH_ROT_MATRIX, D_800AD598);
    BuildRotMatrixX(sp10, D_800AC0A4 * 2);
    MulMatrix2(sp10, D_800AD598);
    D_800AD598[0] = -D_800AD598[0];
    D_800AD598[1] = -D_800AD598[1];
    D_800AD598[2] = -D_800AD598[2];
    SetRotMatrix(SCRATCH_ROT_MATRIX);
}

void BuildRotMatrixAngles(int *a0) {
    char sp10[32];
    BuildRotMatrixY((SCRATCHPAD + 0x108), a0[1]);
    BuildRotMatrixX(sp10, a0[0]);
    MulMatrix2(sp10, (SCRATCHPAD + 0x108));
    BuildRotMatrixZ(sp10, a0[2]);
    MulMatrix2(sp10, (SCRATCHPAD + 0x108));
}

int Atan2Table(int a0, int a1)
{
    int r;
    if (a0 == 0) {
        r = 0;
        if (a1 != 0) {
            r = -0x400;
            if (a1 > 0) {
                return 0x400;
            }
            goto ret;
        }
        goto ret;
    }
    if (a0 > 0) {
        if (a1 >= 0) {
            if (a0 < a1) {
                return 0x400 - AtanTable[(a0 << 10) / a1];
            }
            r = AtanTable[(a1 << 10) / a0];
ret:
            return r;
        }
        a1 = -a1;
        if (a0 < a1) {
            return AtanTable[(a0 << 10) / a1] - 0x400;
        }
        return -AtanTable[(a1 << 10) / a0];
    }
    a0 = -a0;
    if (a1 >= 0) {
        if (a0 < a1) {
            return AtanTable[(a0 << 10) / a1] + 0x400;
        }
        return 0x800 - AtanTable[(a1 << 10) / a0];
    }
    a1 = -a1;
    if (a0 < a1) {
        return 0xC00 - AtanTable[(a0 << 10) / a1];
    }
    return AtanTable[(a1 << 10) / a0] + 0x800;
}
