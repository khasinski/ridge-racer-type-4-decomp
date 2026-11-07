#include "types.h"

#define G ((Scratch_8006EF8C *)0x1F800000)

/* legacy typing of shared globals in this part of the unit */
#define LightMatrix (*(M2C_UNK (*))&LightMatrix)

#undef LightMatrix

/* legacy typing of shared globals in this part of the unit */
#define RunCourseDrawChain ((int (*)())RunCourseDrawChain)

#define BuildViewSegmentObjectList ((int (*)())BuildViewSegmentObjectList)

#define ApplySegmentOctantValue ((int (*)())ApplySegmentOctantValue)

#undef RunCourseDrawChain

#undef BuildViewSegmentObjectList

#undef ApplySegmentOctantValue

/* legacy typing of shared globals in this part of the unit */
#define BuildViewSegmentObjectList ((int (*)())BuildViewSegmentObjectList)

#define RunCourseDrawChain ((int (*)())RunCourseDrawChain)

#undef BuildViewSegmentObjectList

#undef RunCourseDrawChain

extern int SetTransMatrix();

extern int SetRotMatrix();

extern int ApplyMatrix();

void SetGteObjectTransform(short *a0, short *a1, int *a2) {
    short *g = (short *)0x1F800000;
    a0[0] = a1[0] - g[4];
    a0[1] = a1[2] - g[6];
    a0[2] = a1[4] - g[8];
    ApplyMatrix(0x1F800028, a0, a0 + 4);
    *(int *)(a0 + 0x16) = *(int *)(a0 + 4);
    *(int *)(a0 + 0x18) = *(int *)(a0 + 6);
    *(int *)(a0 + 0x1A) = *(int *)(a0 + 8);
    SetRotMatrix(a2);
    SetTransMatrix(a0 + 12);
}
