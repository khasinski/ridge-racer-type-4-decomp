/* MASPSX_FLAGS: --assembler-profile=gnu */

/* CC1_FLAGS: -mno-split-addresses */

#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define GetClut ((int (*)())GetClut)

#undef GetClut

/* legacy typing of shared globals in this part of the unit */
#define GpuDebugLevel (*(u8 (*)[])&GpuDebugLevel)

#define GPU_printf (*(void (*(*))(const char *, s32, s32, s32))&GPU_printf)

#undef GpuDebugLevel

#undef GPU_printf

/* legacy typing of shared globals in this part of the unit */
#define GPU_printf (*(int (*(*))())&GPU_printf)

#define GpuFuncTable ((int *)GpuFuncTable)

#define DMACallback ((int (*)())DMACallback)

#undef GPU_printf

#undef GpuFuncTable

#undef DMACallback

/* legacy typing of shared globals in this part of the unit */
#define GpuDebugLevel (*(u8 (*))&GpuDebugLevel)

#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuDebugLevel (*(unsigned char (*)[])&GpuDebugLevel)

#define GPU_printf (*(void (*(*))())&GPU_printf)

#undef GpuDebugLevel

#undef GPU_printf

/* legacy typing of shared globals in this part of the unit */
#define GpuDebugLevel (*(unsigned char (*)[])&GpuDebugLevel)

#define GPU_printf (*(void (*(*))())&GPU_printf)

#define GpuFuncTable (*(struct { char pad[0x10]; int (*f)(); } *(*))&GpuFuncTable)

#define GPU_memset ((int (*)())GPU_memset)

#undef GpuDebugLevel

#undef GPU_printf

#undef GpuFuncTable

#undef GPU_memset

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((void *)GpuFuncTable)

#define GPU_printf (*(M2C_UNK (*(*))(M2C_UNK *, s32))&GPU_printf)

#define GpuDebugLevel (*(u8 (*))&GpuDebugLevel)

#undef GpuFuncTable

#undef GPU_printf

#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((void *)GpuFuncTable)

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuMovePayload (*(B (*))&GpuMovePayload)

#define GpuFuncTable ((int *)GpuFuncTable)

#undef GpuMovePayload

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GPU_printf (*(int (*(*))())&GPU_printf)

#define GpuDebugLevel (*(unsigned char (*))&GpuDebugLevel)

#undef GPU_printf

#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

#define GPU_printf (*(int (*(*))())&GPU_printf)

#define GpuDebugLevel (*(unsigned char (*))&GpuDebugLevel)

#undef GpuFuncTable

#undef GPU_printf

#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((RrLibGraphicsDriverPrefix *)GpuFuncTable)

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((int *)GpuFuncTable)

#define GPU_printf (*(int (*(*))())&GPU_printf)

#define GpuDebugLevel (*(unsigned char (*))&GpuDebugLevel)

#undef GpuFuncTable

#undef GPU_printf

#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

#define GPU_printf (*(int (*(*))())&GPU_printf)

#define GpuDebugLevel (*(unsigned char (*)[])&GpuDebugLevel)

#undef GpuFuncTable

#undef GPU_printf

#undef GpuDebugLevel

typedef struct { short a,b,c,d,e,f,g,h; char i,j,k,l; } W3L2_A;

W3L2_A *SetDefDispEnv(W3L2_A *p, int a, int b, int c, int d) {
    p->a = a; p->b = b; p->c = c;
    p->e = 0; p->f = 0; p->g = 0; p->h = 0;
    p->j = 0; p->i = 0; p->l = 0; p->k = 0;
    p->d = d;
    return p;
}
