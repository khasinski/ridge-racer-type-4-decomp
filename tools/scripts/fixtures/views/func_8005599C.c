#include "rr/matrix_words.h"

#define addPrim(ot, p) \
    ((DrTPage *)(p))->addr = (ot)->addr; \
    (ot)->addr = (u32)(p)

#define P RaceContext.p

#define B(o) (*(unsigned char *)(P + (o)))

#define H(o) (*(short *)(P + (o)))

#define W(o) (*(int *)(P + (o)))

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(struct { char pad[0xDC]; s32 a; char pad2[4]; u8 b, c, d; } (*))&RaceContext)

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define SetDrawStp ((int (*)())SetDrawStp)

#define CurrentDrawBuf ((S *)CurrentDrawBuf)

#undef SetDrawStp

#undef CurrentDrawBuf

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(struct { char pad[0xD4]; s32 a, b, c, d; u8 e[4]; } (*))&RaceContext)

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(struct { char pad[0x3DC]; s32 a; } (*))&RaceContext)

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(struct { char pad[0xEC]; char buf[0x160]; char *p; } (*))&RaceContext)

#undef RaceContext

typedef struct { int w[4]; } Q16;

typedef RrMatrixWords32 M32;

typedef struct { short m[3][3]; short pad[7]; } S70;

extern Q16 ObjectLightScale;

extern short LightMatrix[][3];

extern int BuildRotMatrixZ();

extern int BuildRotMatrixY();

extern int BuildRotMatrixX();

extern int MulMatrix0();

extern int MulMatrix();

extern int MulMatrix2();

extern int ScaleMatrix();

extern int SetLightMatrix();

extern int SetColorMatrix();

extern int SetBackColor();

void SetupObjectLightMatrices(a0, a1, a2, a3) char *a0; char *a1; M32 *a2; M32 *a3; {
    M32 s10, s30, s50;
    S70 s70;
    Q16 s90;
    int n;
    s90 = ObjectLightScale;
    BuildRotMatrixY(&s50, -*(short *)(a0 + 0x12));
    MulMatrix2(0x1F800028, &s50);
    BuildRotMatrixX(&s10, -*(short *)(a0 + 0x10));
    BuildRotMatrixZ(&s30, -*(short *)(a0 + 0x14));
    MulMatrix(&s10, &s30);
    MulMatrix2(&s50, &s10);
    BuildRotMatrixY(&s50, *(short *)(a0 + 0x12));
    BuildRotMatrixX(&s30, *(short *)(a0 + 0x10));
    MulMatrix(&s50, &s30);
    BuildRotMatrixZ(&s30, *(short *)(a0 + 0x14));
    MulMatrix2(&s50, &s30);
    *a3 = s30;
    n = 0;
    do {
        LightMatrix[n][0] = *(unsigned short *)(a1 + n * 8 + 0xC);
        LightMatrix[n][1] = *(unsigned short *)(a1 + n * 8 + 0xE);
        LightMatrix[n][2] = *(unsigned short *)(a1 + n * 8 + 0x10);
        s70.m[0][n] = (unsigned char)a1[n * 4] * 16;
        s70.m[1][n] = (unsigned char)a1[n * 4 + 1] * 16;
        s70.m[2][n] = (unsigned char)a1[n * 4 + 2] * 16;
        n++;
    } while (n < 3);
    MulMatrix0(LightMatrix, &s10, &s30);
    SetLightMatrix(&s30);
    SetColorMatrix(&s70);
    SetBackColor((unsigned char)a1[0x24], (unsigned char)a1[0x25], (unsigned char)a1[0x26]);
    ScaleMatrix(&s10, &s90);
    *a2 = s10;
}
