/* MASPSX_FLAGS: --assembler-profile=gnu */
/* CC1_FLAGS: -mno-split-addresses */
#include "types.h"
#include "rr/gpu_reset.h"
#include "m2c_macros.h"
#include "rr/gpu.h"
#include "rr/library.h"
#include "rr/draw_environment.h"
#include "psyq/libgpu/addprim.h"
#include "psyq/libgpu/gpubase2.h"
#include "psyq/libgpu/gpuenv.h"
#include "rr/spu_tick.h"
#include "rr/driver_control.h"

typedef struct { short a,b,c,d,e,f,g,h; char i,j,k,l; } W3L2_A;

typedef struct { int w; int s4; } A;

typedef struct { int a, b, c; } B;

typedef struct { char pad[0x1C]; unsigned w; } W3L2_P;

extern unsigned char GpuSysQueueMode[];
extern int SetGraphQueFmt[];
extern char DrawSyncCallbackFmt[];
extern char StrSetDispMaskFmt[];
extern M2C_UNK DrawSyncFmt;
extern char StrBadRectFmt[];
extern char StrRectFmt[];
extern char GpuLabelColonFmt[];
extern char StrClearImage[];
extern char ClearImage2Name[];
/* extern */
extern M2C_UNK LoadImageName;
extern char ClearOTagFmt[];
extern char GpuOtTerminator[];
extern int OTagTerminator;
extern char StrClearOTagRFmt[];
extern int PsyqMemcpy();
extern char PutDrawEnvFmt[];
extern char StrDrawOTagEnvFmt[];
extern char CurrentDrawEnv[];

int LoadTPage(int *pix, int tp, int abr, int x, int y, int w, int h);
int LoadClut(int a, int b, int c);
int LoadClut2(int a, int b, int c);
char *SetDefDrawEnv(char *p, int a, int b, int c, int d);
W3L2_A *SetDefDispEnv(W3L2_A *p, int a, int b, int c, int d);
s32 SetGraphDebug(s32 enabled);
int SetGraphQueue();
u8 GetGraphDebug(void);
int DrawSyncCallback();
void DrawSync(s32 arg0);
void checkRECT(const char *name,RrGpuRectangle *rectangle);
void ClearImage(int a, int b, int c, int d);
void ClearImage2(int a, int b, int c, int d);
void LoadImage(s32 arg0, M2C_UNK arg1);
void StoreImage(int a, int b);
int MoveImage();
int *ClearOTag();
unsigned *ClearOTagR(unsigned *a, int b);
void DrawPrim(char *a0);
void DrawOTag(s32 ordering_table);
int PutDrawEnv(int *a0);
void DrawOTagEnv(int a, W3L2_P *b);
int GetDrawEnv(int a);

int LoadTPage(int *pix, int tp, int abr, int x, int y, int w, int h) {
    short sp[4];
    int r;
    sp[0] = x;
    sp[1] = y;
    sp[3] = h;
    switch (tp) {
    case 0:
        sp[2] = w / 4;
        break;
    case 1:
        sp[2] = w / 2;
        break;
    case 2:
        sp[2] = w;
        break;
    }
    LoadImage(sp, pix);
    /* ASM_BARRIER: boost x and abr reference counts so global-alloc assigns $s1 to x, $s2 to abr, and $s3 to y */
    __asm__ volatile("" : : "r"(x), "r"(abr));
    r = GetTPage(tp, abr, x, y);
    /* ASM_BARRIER: keep andi right after the call, before the epilogue loads */
    __asm__ volatile("" : "=r"(r) : "0"(r));
    return r;
}

int LoadClut(int a, int b, int c) {
    short sp[4];
    int r;
    sp[2] = 0x100;
    sp[0] = b;
    sp[1] = c;
    sp[3] = 1;
    LoadImage(sp, a);
    r = GetClut(b, c);
    /* ASM_BARRIER: keep andi right after the call, before the epilogue loads */
    __asm__ volatile("" : "=r"(r) : "0"(r));
    return r;
}

/* legacy typing of shared globals in this part of the unit */
#define GetClut ((int (*)())GetClut)

int LoadClut2(int a, int b, int c) {
    short sp[4];
    int r;
    sp[2] = 0x10;
    sp[0] = b;
    sp[1] = c;
    sp[3] = 1;
    LoadImage(sp, a);
    /* ASM_BARRIER: keeps the u16 mask (andi) scheduled right after the call, before the epilogue loads */
    r = GetClut(b, c) & 0xFFFF; __asm__ volatile("" : "=r"(r) : "0"(r)); return r;
}

#undef GetClut

char *SetDefDrawEnv(char *p, int a, int b, int c, int d) {
    int t = GetVideoMode();
    *(short *)p = a;
    *(short *)(p + 2) = b;
    *(short *)(p + 4) = c;
    *(short *)(p + 0xC) = 0;
    *(short *)(p + 0xE) = 0;
    *(short *)(p + 0x10) = 0;
    *(short *)(p + 0x12) = 0;
    p[0x19] = 0;
    p[0x1A] = 0;
    p[0x1B] = 0;
    p[0x16] = 1;
    *(short *)(p + 6) = d;
    if (t) p[0x17] = d < 0x121; else p[0x17] = d < 0x101;
    *(short *)(p + 8) = a;
    *(short *)(p + 0xA) = b;
    *(short *)(p + 0x14) = 0xA;
    p[0x18] = 0;
    return p;
}

W3L2_A *SetDefDispEnv(W3L2_A *p, int a, int b, int c, int d) {
    p->a = a; p->b = b; p->c = c;
    p->e = 0; p->f = 0; p->g = 0; p->h = 0;
    p->j = 0; p->i = 0; p->l = 0; p->k = 0;
    p->d = d;
    return p;
}

s32 ResetGraph(s32 mode)
{
    RrGpuResetState80 *state;
    switch(mode & 7) {
    case 0:
    case 3:
        DebugPrintStub(ResetGraphFmt,&GpuDriverTable,&GpuSysVersion);
    case 5:
        state=&GpuSysVersion;
        GPU_memset((s8 *)state,0,128);
        ResetCallback();
        GPU_cw((unsigned long)GpuFuncTable & 0xFFFFFF);
        state->kind00=_reset(mode);
        state->enabled01=1;
        /* Preserve the explicit byte reload after the reset service. */
        state->debug02.width=GpuVramWidthTable[*(volatile u8 *)&state->kind00].value;
        state->debug02.height=GpuVramHeightTable.dimensions[state->kind00].value;
        GPU_memset((s8 *)state->queue10,-1,0x5C);
        GPU_memset((s8 *)state->tail6C,-1,0x14);
        return state->kind00;
    default:
        if(GpuDebugLevel.level>=2) GPU_printf(StrResetGraphFmt,mode);
        return GpuFuncTable->reset34(1);
    }
}

/* legacy typing of shared globals in this part of the unit */
#define GpuDebugLevel (*(u8 (*)[])&GpuDebugLevel)
#define GPU_printf (*(void (*(*))(const char *, s32, s32, s32))&GPU_printf)

s32 SetGraphDebug(s32 enabled) {
    u8 *state = GpuDebugLevel;
    s32 previous;
    previous = state[0];
    state[0] = enabled;
    if (enabled & 0xFF) {
        const char *format;
        void (*callback)(const char *, s32, s32, s32) = GPU_printf;
        s32 first = state[0], second = state[-2], third = state[1];
        /* ASM_BARRIER: read the callback and state bytes before materializing the format page. */
        __asm__ volatile("" : : "r"(callback), "r"(first), "r"(second), "r"(third) : "$4");
        /* PAL read-only mapping: format + 0x1E24 is SetGraphDebugFmt. */
        format = (const char *)0x80010000U;
        /* ASM_BARRIER: expose the page-relative addition in the indirect-call delay slot. */
        __asm__("" : "=r"(format) : "0"(format));
        callback(format + 0x1E24, first, second, third);
        return previous;
    }
    return previous;
}

#undef GpuDebugLevel
#undef GPU_printf

/* legacy typing of shared globals in this part of the unit */
#define GPU_printf (*(int (*(*))())&GPU_printf)
#define GpuFuncTable ((int *)GpuFuncTable)
#define DMACallback ((int (*)())DMACallback)

int SetGraphQueue(a0) int a0; {
    register unsigned char *p __asm__("$17"); /* ASM_BARRIER: original keeps the pointer in s1 and a0 in s0 */
    int old;
    p = GpuSysQueueMode;
    old = p[0];
    if (p[1] >= 2) {
        GPU_printf(SetGraphQueFmt, a0);
    }
    if (a0 != p[0]) {
        ((int (*)())GpuFuncTable[0x34 / 4])(1);
        p[0] = a0;
        DMACallback(2, 0);
    }
    return old;
}

#undef GPU_printf
#undef GpuFuncTable
#undef DMACallback

/* legacy typing of shared globals in this part of the unit */
#define GpuDebugLevel (*(u8 (*))&GpuDebugLevel)

u8 GetGraphDebug(void) {
    return GpuDebugLevel;
}

#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuDebugLevel (*(unsigned char (*)[])&GpuDebugLevel)
#define GPU_printf (*(void (*(*))())&GPU_printf)

int DrawSyncCallback(a0) int a0; {
 int o;
 if (GpuDebugLevel[0] >= 2) GPU_printf(DrawSyncCallbackFmt, a0);
 o = *(int *)(GpuDebugLevel + 0xA);
 *(int *)(GpuDebugLevel + 0xA) = a0;
 return o; }

#undef GpuDebugLevel
#undef GPU_printf

/* legacy typing of shared globals in this part of the unit */
#define GpuDebugLevel (*(unsigned char (*)[])&GpuDebugLevel)
#define GPU_printf (*(void (*(*))())&GPU_printf)
#define GpuFuncTable (*(struct { char pad[0x10]; int (*f)(); } *(*))&GpuFuncTable)
#define GPU_memset ((int (*)())GPU_memset)

void SetDispMask(a0) int a0; {
 unsigned char *p = GpuDebugLevel;
 if (*p >= 2) GPU_printf(StrSetDispMaskFmt, a0);
 if (a0 == 0) GPU_memset(p + 0x6A, -1, 0x14);
 GpuFuncTable->f(a0 ? 0x3000000 : 0x3000001); }

#undef GpuDebugLevel
#undef GPU_printf
#undef GpuFuncTable
#undef GPU_memset

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((void *)GpuFuncTable)
#define GPU_printf (*(M2C_UNK (*(*))(M2C_UNK *, s32))&GPU_printf)
#define GpuDebugLevel (*(u8 (*))&GpuDebugLevel)

void DrawSync(s32 arg0) {
    if ((u8) GpuDebugLevel >= 2U) {
        GPU_printf(&DrawSyncFmt, arg0);
    }
    M2C_FIELD(GpuFuncTable, M2C_UNK (**)(s32), 0x3C)(arg0);
}

#undef GpuFuncTable
#undef GPU_printf
#undef GpuDebugLevel

void checkRECT(const char *name,RrGpuRectangle *rectangle) {
    const char *label=name;
    RrGpuRectangle *rect=rectangle;
    RrGpuDebugSettings *settings=&GpuDebugLevel;
    const char *format;
    switch(settings->level) {
    case 1: {
        s32 w=rect->rect.w;
        s32 maxw=settings->width;
        s32 x;
        s32 y;
        /* ASM_BARRIER: Preserve the original height temporary through the final validity tests. */
        register s32 h __asm__("$6");
        s32 maxh;
        s32 comparison;
        if(maxw<w) goto invalid;
        x=rect->rect.x;
        comparison=maxw<w+x;
        if(comparison) goto invalid;
        y=rect->rect.y;
        maxh=settings->height;
        if(maxh<y) goto invalid;
        h=rect->rect.h;
        comparison=maxh<y+h;
        if(comparison) goto invalid;
        if(w<=0) goto invalid;
        if(x<0) goto invalid;
        if(y<0) goto invalid;
        if(h>0) return;
    }
invalid:
        format=(const char *)0x80010000U;
        /* ASM_BARRIER: Expose the mapped invalid-message address for the jump delay slot. */
        __asm__("" : "+r"(format));
        format+=0x1EA8;
        goto report;
    case 2:
        format=GpuLabelColonFmt;
report:
        GPU_printf(format,label);
        {
            s32 x=rect->rect.x;
            s32 y=rect->rect.y;
            s32 w=rect->rect.w;
            s32 h=rect->rect.h;
            /* ASM_BARRIER: Reload all rectangle fields before fetching the second diagnostic callback. */
            __asm__("" : : "r"(x),"r"(y),"r"(w),"r"(h) : "$2");
            GPU_printf(StrRectFmt,x,y,w,h);
        }
    }
}

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

void ClearImage(int a, int b, int c, int d)
{
int v; char nv; nv = b & 0xFF; checkRECT(StrClearImage, a); v = (((d & 0xFF) << 16) | ((c & 0xFF) << 8)) | nv; (*(int (**)())(GpuFuncTable + 8))(*(int *)(GpuFuncTable + 0xC), a, 8, v);
}

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

void ClearImage2(int a, int b, int c, int d)
{
int v; char nv; int t, u, k; nv = b & 0xFF; checkRECT(ClearImage2Name, a); u = (d & 0xFF) << 16; t = (c & 0xFF) << 8; k = 0x80000000; v = (u | (t | k)) | nv; (*(int (**)())(GpuFuncTable + 8))(*(int *)(GpuFuncTable + 0xC), a, 8, v);
}

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((void *)GpuFuncTable)

void LoadImage(s32 arg0, M2C_UNK arg1) {
    checkRECT(&LoadImageName, arg0);
    M2C_FIELD(GpuFuncTable, M2C_UNK (**)(s32, s32, M2C_UNK, M2C_UNK), 8)(M2C_FIELD(GpuFuncTable, s32 *, 0x20), arg0, 8, arg1);
}

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

void StoreImage(int a, int b) {
    checkRECT(StrStoreImage, a);
    (*(int (**)())(GpuFuncTable + 8))(*(int *)(GpuFuncTable + 0x1C), a, 8, b);
}

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuMovePayload (*(B (*))&GpuMovePayload)
#define GpuFuncTable ((int *)GpuFuncTable)

int MoveImage(a0, a1, a2)
A *a0; int a1; int a2;
{
    B *p;
    int *o;
    int w;
    checkRECT(MoveImageNameStr, a0);
    if (*(short *)&a0->s4 == 0) return -1;
    if (*((short *)&a0->s4 + 1) != 0) {
    p = &GpuMovePayload;
    w = a0->w;
    o = GpuFuncTable;
    p->b = (a2 << 16) | (a1 & 0xFFFF);
    p->a = w;
    p->c = a0->s4;
    return ((int (*)())o[2])(o[6], (char *)p - 8, 0x14, 0);
    }
    return -1;
}

#undef GpuMovePayload
#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GPU_printf (*(int (*(*))())&GPU_printf)
#define GpuDebugLevel (*(unsigned char (*))&GpuDebugLevel)

int *ClearOTag(a0, a1) int *a0; int a1; {
    /* ASM_BARRIER: pins p->$16, n->$17 to match original s-reg assignment */
    register int *p asm("$16") = a0;
    int n = a1;
    int *q; int *t; int m; int (*f)();
    if (GpuDebugLevel >= 2) { f = GPU_printf; q = (int *)ClearOTagFmt; f(q, p, n); }
    n--;
    if (n != 0) {
        do {
            n--;
            q = p + 1;
            ((char *)p)[3] = 0;
            m = (int)q & 0xFFFFFF; *p = (*p & 0xFF000000) | m;
            p = q;
        } while (n != 0);
    }
    t = &OTagTerminator;
    *t = ((int)GpuOtTerminator & 0xFFFFFF) | 0x04000000;
    *p = (int)t & 0xFFFFFF;
    return p;
}

#undef GPU_printf
#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)
#define GPU_printf (*(int (*(*))())&GPU_printf)
#define GpuDebugLevel (*(unsigned char (*))&GpuDebugLevel)

unsigned *ClearOTagR(unsigned *a, int b) {
    int *q;
    if (GpuDebugLevel >= 2) GPU_printf(StrClearOTagRFmt, a, b);
    (*(int (**)())(GpuFuncTable + 0x2C))(a, b);
    q = &OTagTerminator;
    *q = ((int)GpuOtTerminator & 0xFFFFFF) | 0x04000000;
    *a = (int)q & 0xFFFFFF;
    return a;
}

#undef GpuFuncTable
#undef GPU_printf
#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)

void DrawPrim(char *a0)
{
    int t = a0[3];
    (*(int (**)())(GpuFuncTable + 0x3C))(0);
    (*(int (**)())(GpuFuncTable + 0x14))(a0 + 4, t);
}

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((RrLibGraphicsDriverPrefix *)GpuFuncTable)

void DrawOTag(s32 ordering_table) {
    /* ASM_BARRIER: Keep the third dispatch argument distinct from the final constant zero. */
    register s32 second;
    register s32 third;
    RrLibGraphicsDriverPrefix *driver;
    if (GpuDebugLevel.level >= 2) GPU_printf(DrawOTagFmt, ordering_table);
    second = ordering_table;
    driver = GpuFuncTable;
    /* ASM_BARRIER: Load the current driver before preparing the dispatch zeros. */
    __asm__("" : "+r"(driver) : : "$6");
    third = 0;
    /* ASM_BARRIER: Prevent reuse of the third argument when preparing the fourth zero. */
    __asm__("" : "+r"(third));
    driver->dispatch(driver->draw_ordering_table, second, third, 0);
}

#undef GpuFuncTable

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((int *)GpuFuncTable)
#define GPU_printf (*(int (*(*))())&GPU_printf)
#define GpuDebugLevel (*(unsigned char (*))&GpuDebugLevel)

int PutDrawEnv(int *a0) {
    unsigned char *p = &GpuDebugLevel;
    if (*p >= 2) {
        GPU_printf(PutDrawEnvFmt, a0);
    }
    SetDrawEnv2((u32 *)(a0 + 7), (RrFrameEnvironment *)a0);
    a0[7] |= 0xFFFFFF;
    ((int (*)())GpuFuncTable[2])(GpuFuncTable[6], a0 + 7, 0x40, 0);
    PsyqMemcpy(p + 0xE, a0, 0x5C);
    return (int)a0;
}

#undef GpuFuncTable
#undef GPU_printf
#undef GpuDebugLevel

/* legacy typing of shared globals in this part of the unit */
#define GpuFuncTable ((char *)GpuFuncTable)
#define GPU_printf (*(int (*(*))())&GPU_printf)
#define GpuDebugLevel (*(unsigned char (*)[])&GpuDebugLevel)

void DrawOTagEnv(int a, W3L2_P *b) {
    char *t;
    if (GpuDebugLevel[0] >= 2) GPU_printf(StrDrawOTagEnvFmt, a, b);
    t = (char *)b + 0x1C;
    SetDrawEnv2((u32 *)t, (RrFrameEnvironment *)b);
    b->w = (b->w & 0xFF000000) | (a & 0xFFFFFF);
    (*(int (**)())(GpuFuncTable + 8))(*(int *)(GpuFuncTable + 0x18), t, 0x40, 0);
    PsyqMemcpy(GpuDebugLevel + 0xE, b, 0x5C);
}

#undef GpuFuncTable
#undef GPU_printf
#undef GpuDebugLevel

int GetDrawEnv(int a) { PsyqMemcpy(a, CurrentDrawEnv, 0x5C); return a; }
