/* RODATA 0x800100e4 0x20 */
#include "scratchpad.h"
#include "rr_asm.h"
#include "psyq/libgpu/gpudisp.h"
#include "game/data/ldscn5.h"
#include "game/gfx/sky.h"
#include "rr/audio_cd_controller.h"
#include "rr/menu_overlay_runtime.h"

#define GS ((SPD *)SCRATCHPAD)
#define W(o) (*(int *)(GS->cur + (o)))
#define LNK(n, t) \
    W(0) = *(int *)(GS->ot + (t)); \
    *(char *)(GS->cur + 3) = (n); \
    *(int *)(GS->ot + (t)) = (int)GS->cur; \
    *(char *)(GS->ot + (t) + 3) = 0

typedef struct {
    int v01;
    short v2;
    char b6;
    signed char b7;
    int c0;
    int c1;
    int c2;
} SkyTri;

typedef struct {
    int ir0;
    int otz;
    int pad08;
    char *verts;
    int pad10;
    int pad14;
    char *v0;
    char *v1;
    char *v2;
    int pad24;
    union {
        struct {
            int sxy0;
            int sxy1;
            int sxy2;
        } w;
        struct {
            short sx0, sy0;
            short sx1, sy1;
            short sx2, sy2;
        } s;
    } u;
} SkyCtx;

typedef struct {
    int *pkt;
    int ot;
} Scratch17310;

typedef struct {
    int v01;
    int v23;
    int c0;
    int c1;
    int c2;
    int c3;
    char b18;
    signed char b19;
    short pad1A;
} SkyQuad17528;

typedef struct {
    int ir0;
    int otz;
    int pad08;
    char *verts;
    int v3_0;
    int v3_1;
    char *v0;
    char *v1;
    char *v2;
    int *v3;
    union {
        struct {
            int sxy0;
            int sxy1;
            int sxy2;
            int sxy3;
        } w;
        struct {
            short sx0, sy0;
            short sx1, sy1;
            short sx2, sy2;
            short sx3, sy3;
        } s;
    } u;
} SkyCtxQuad;

typedef struct {
    int *pkt;
    int ot;
} Scratch17528;

typedef struct {
    int v01;
    int v23;
    int c0;
    int uv0;
    int uv1;
    int uv23;
    char b18;
    signed char b19;
    short pad1A;
} SkyQuad177E4;

typedef struct {
    int *pkt;
    int ot;
} Scratch177E4;

typedef struct {
    int v01;
    short v2;
    short pad06;
    int c0;
    int c1;
    int c2;
    int uv0;
    int uv1;
    union {
        int uv2;
        struct {
            char pad[3];
            signed char b1F;
        } b;
    } u1C;
} SkyTri17A90;

typedef struct {
    int *pkt;
    int ot;
} Scratch17A90;

typedef struct { char *cur; char *ot; } SPD;

extern char StrUnknownModeFmt[];
extern short *CourseSkyKeyframePtrs[];
extern short *CourseSceneryRangePtrs[];
extern char SkyLightDirMatrix[];
extern int SetGeomOffset();
extern int SetDrawArea();

SkyTri *EmitSkyDepthCuedG3(SkyTri *a0, SkyCtx *a1, int a2);
SkyQuad17528 *EmitSkyDepthCuedG4(SkyQuad17528 *a0, SkyCtxQuad *a1, int a2);
SkyQuad177E4 *SkyEmitQuadFT4(SkyQuad177E4 *a0, SkyCtxQuad *a1, int a2);
SkyTri17A90 *EmitSkyDepthCuedGT3(SkyTri17A90 *a0, SkyCtx *a1, int a2);
void DrawTrackScenery();
char *EmitSkyEnvMapGT3(char *a0, char *a1);
char *EmitSkyEnvMapGT4(char *a0, char *a1);
char *SkyEmitLitQuadG4(char *a0, char *a1);
char *EmitSkyLitGT4(char *a0, char *a1);
char *EmitSkyLitFT4(char *a0, char *a1);
void RunSkyCommandList(char *a0, int a1);
void SetSkyMatrixAndDraw(char *a0, volatile unsigned short *a1);
void RenderSkyToRect(int a0, int a1, short *a2, int a3);

SkyTri *EmitSkyDepthCuedG3(SkyTri *a0, SkyCtx *a1, int a2) {
    int v0 = a0->v01;
    int a3 = a0->v2;
    char *verts = a1->verts;
    int v1 = v0 >> 16;
    int *pkt;
    int *g;

    v0 &= 0xFFFF;
    a1->v0 = verts + (v0 << 3);
    a1->v1 = verts + (v1 << 3);
    a1->v2 = verts + (a3 << 3);
    pkt = ((Scratch17310 *)SCRATCHPAD)->pkt;
    do {
        /* ASM_BARRIER: load V0..V2 from a1->v0..v2 into GTE $0..$5 */
        __asm__ volatile(
            "lw $13, 0x18(%0)\n\t"
            "lw $14, 0x1C(%0)\n\t"
            "lw $15, 0x20(%0)\n\t"
            "lwc2 $0, 0($13)\n\t"
            "lwc2 $1, 4($13)\n\t"
            "lwc2 $2, 0($14)\n\t"
            "lwc2 $3, 4($14)\n\t"
            "lwc2 $4, 0($15)\n\t"
            "lwc2 $5, 4($15)"
            :
            : "r"(a1)
            : "$13", "$14", "$15"
        );
        v0 = a0[1].v01;
        /* ASM_BARRIER: GTE RTPT after 1-cycle load delay */
        __asm__ volatile("nop\n\t.word 0x4A280030");
        a3 = a0[1].v2;
        v1 = v0 >> 16;
        v0 &= 0xFFFF;
        a1->v0 = verts + (v0 << 3);
        a1->v1 = verts + (v1 << 3);
        a1->v2 = verts + (a3 << 3);
        RR_GTE_OP(0x4B58002D);
        RR_GTE14_SWC2_OTZ(&a1->otz);
        if (a1->otz > 0) {
            RR_GTE_SWC2_SXYZ(&a1->u.w.sxy0);
            v0 = a1->u.w.sxy0;
            v1 = a1->u.w.sxy1;
            a3 = a1->u.w.sxy2;
            pkt[2] = v0;
            pkt[4] = v1;
            pkt[6] = a3;
            v0 = a1->u.s.sx0;
            v1 = a1->u.s.sx1;
            a3 = a1->u.s.sx2;
            if ((v0 >= 0 || v1 >= 0 || a3 >= 0) &&
                (v0 < 0x140 || v1 < 0x140 || a3 < 0x140)) {
                v0 = a1->u.s.sy0;
                v1 = a1->u.s.sy1;
                a3 = a1->u.s.sy2;
                if ((v0 >= 0 || v1 >= 0 || a3 >= 0) &&
                    (v0 < 0x100 || v1 < 0x100 || a3 < 0x100)) {
                    v0 = a0->c0;
                    v1 = a0->c1;
                    a3 = a0->c2;
                    /* ASM_BARRIER: load IR0, RGB0..RGB2, run DPCT, store RGB0..RGB2 */
                    __asm__ volatile(
                        "lwc2 $8, 0(%0)\n\t"
                        "mtc2 %1, $6\n\t"
                        "mtc2 %1, $20\n\t"
                        "mtc2 %2, $21\n\t"
                        "mtc2 %3, $22\n\t"
                        "nop\n\t"
                        "nop\n\t"
                        ".word 0x4AF8002A\n\t"
                        "swc2 $20, 4(%4)\n\t"
                        "swc2 $21, 12(%4)\n\t"
                        "swc2 $22, 20(%4)"
                        :
                        : "r"(a1), "r"(v0), "r"(v1), "r"(a3), "r"(pkt)
                        : "memory"
                    );
                    g = (int *)SCRATCHPAD;
                    pkt[0] = *(int *)((a0->b7 << 2) + g[1] + 0xAF8);
                    ((char *)pkt)[3] = 6;
                    *(int *)((a0->b7 << 2) + g[1] + 0xAF8) = (int)pkt;
                    pkt += 7;
                    *(char *)((a0->b7 << 2) + g[1] + 0xAFB) = 0;
                }
            }
        }
        a0++;
        a2--;
    } while (a2 > 0);
    *(int **)SCRATCHPAD = pkt;
    return a0;
}

SkyQuad17528 *EmitSkyDepthCuedG4(SkyQuad17528 *a0, SkyCtxQuad *a1, int a2) {
    int v1 = a0->v01;
    int a3 = a0->v23;
    char *verts = a1->verts;
    int t0 = v1 >> 16;
    int t1 = a3 >> 16;
    int *pkt;
    int *g;
    int *p;

    v1 &= 0xFFFF;
    a3 &= 0xFFFF;
    a1->v0 = verts + (v1 << 3);
    a1->v1 = verts + (t0 << 3);
    a1->v2 = verts + (a3 << 3);
    a1->v3 = (int *)(verts + (t1 << 3));
    pkt = ((Scratch17528 *)SCRATCHPAD)->pkt;
    do {
        /* ASM_BARRIER: load V0..V2 from a1->v0..v2 into GTE $0..$5 and run RTPT */
        __asm__ volatile(
            "lw $15, 0x18(%0)\n\t"
            "lw $24, 0x1C(%0)\n\t"
            "lw $25, 0x20(%0)\n\t"
            "lwc2 $0, 0($15)\n\t"
            "lwc2 $1, 4($15)\n\t"
            "lwc2 $2, 0($24)\n\t"
            "lwc2 $3, 4($24)\n\t"
            "lwc2 $4, 0($25)\n\t"
            "lwc2 $5, 4($25)\n\t"
            "nop\n\t"
            "nop\n\t"
            ".word 0x4A280030"
            :
            : "r"(a1)
            : "$15", "$24", "$25"
        );
        p = a1->v3;
        v1 = p[0];
        t0 = p[1];
        a1->v3_0 = v1;
        a1->v3_1 = t0;
        v1 = a0[1].v01;
        a3 = a0[1].v23;
        t0 = v1 >> 16;
        t1 = a3 >> 16;
        v1 &= 0xFFFF;
        a3 &= 0xFFFF;
        a1->v0 = verts + (v1 << 3);
        a1->v1 = verts + (t0 << 3);
        a1->v2 = verts + (a3 << 3);
        a1->v3 = (int *)(verts + (t1 << 3));
        RR_GTE_OP(0x4B58002D);
        RR_GTE14_SWC2_OTZ(&a1->otz);
        if (a1->otz > 0) {
            RR_GTE_SWC2_SXYZ(&a1->u.w.sxy0);
            /* ASM_BARRIER: load V0 from a1->v3_0..v3_1 */
            __asm__ volatile("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(&a1->v3_0));
            v1 = a1->u.w.sxy0;
            t0 = a1->u.w.sxy1;
            /* ASM_BARRIER: GTE RTPS */
            __asm__ volatile(".word 0x4A180001");
            a3 = a1->u.w.sxy2;
            pkt[2] = v1;
            pkt[4] = t0;
            pkt[6] = a3;
            RR_GTE_SWC2_SXY2(&a1->u.w.sxy3);
            RR_GTE_SWC2_SXY2(pkt + 8);
            v1 = a1->u.s.sx0;
            t0 = a1->u.s.sx1;
            a3 = a1->u.s.sx2;
            t1 = a1->u.s.sx3;
            if ((v1 >= 0 || t0 >= 0 || a3 >= 0 || t1 >= 0) &&
                (v1 < 0x140 || t0 < 0x140 || a3 < 0x140 || t1 < 0x140)) {
                v1 = a1->u.s.sy0;
                t0 = a1->u.s.sy1;
                a3 = a1->u.s.sy2;
                t1 = a1->u.s.sy3;
                if ((v1 >= 0 || t0 >= 0 || a3 >= 0 || t1 >= 0) &&
                    (v1 < 0x100 || t0 < 0x100 || a3 < 0x100 || t1 < 0x100)) {
                    v1 = a0->c0;
                    t0 = a0->c1;
                    a3 = a0->c2;
                    t1 = a0->c3;
                    /* ASM_BARRIER: load IR0, RGB0..RGB2, run DPCT, store RGB0..RGB2, load RGB3, run DPCS */
                    __asm__ volatile(
                        "lwc2 $8, 0(%0)\n\t"
                        "mtc2 %1, $6\n\t"
                        "mtc2 %1, $20\n\t"
                        "mtc2 %2, $21\n\t"
                        "mtc2 %3, $22\n\t"
                        "nop\n\t"
                        "nop\n\t"
                        ".word 0x4AF8002A\n\t"
                        "swc2 $20, 4(%5)\n\t"
                        "swc2 $21, 12(%5)\n\t"
                        "swc2 $22, 20(%5)\n\t"
                        "mtc2 %4, $6\n\t"
                        "nop\n\t"
                        "nop\n\t"
                        ".word 0x4A780010"
                        :
                        : "r"(a1), "r"(v1), "r"(t0), "r"(a3), "r"(t1), "r"(pkt)
                        : "memory"
                    );
                    RR_GTE14_SWC2_RGB2(pkt + 7);
                    g = (int *)SCRATCHPAD;
                    pkt[0] = *(int *)((a0->b19 << 2) + g[1] + 0xAF8);
                    ((char *)pkt)[3] = 8;
                    *(int *)((a0->b19 << 2) + g[1] + 0xAF8) = (int)pkt;
                    pkt += 9;
                    *(char *)((a0->b19 << 2) + g[1] + 0xAFB) = 0;
                }
            }
        }
        a0++;
        a2--;
    } while (a2 > 0);
    *(int **)SCRATCHPAD = pkt;
    return a0;
}

SkyQuad177E4 *SkyEmitQuadFT4(SkyQuad177E4 *a0, SkyCtxQuad *a1, int a2) {
    int v1 = a0->v01;
    int a3 = a0->v23;
    char *verts = a1->verts;
    int t0 = v1 >> 16;
    int t1 = a3 >> 16;
    int v0;
    int *pkt;
    int *g;
    int *p;

    v1 &= 0xFFFF;
    a3 &= 0xFFFF;
    a1->v0 = verts + (v1 << 3);
    a1->v1 = verts + (t0 << 3);
    a1->v2 = verts + (a3 << 3);
    a1->v3 = (int *)(verts + (t1 << 3));
    pkt = ((Scratch177E4 *)SCRATCHPAD)->pkt;
    do {
        /* ASM_BARRIER: load V0..V2 from a1->v0..v2 into GTE $0..$5 and run RTPT */
        __asm__ volatile(
            "lw $15, 0x18(%0)\n\t"
            "lw $24, 0x1C(%0)\n\t"
            "lw $25, 0x20(%0)\n\t"
            "lwc2 $0, 0($15)\n\t"
            "lwc2 $1, 4($15)\n\t"
            "lwc2 $2, 0($24)\n\t"
            "lwc2 $3, 4($24)\n\t"
            "lwc2 $4, 0($25)\n\t"
            "lwc2 $5, 4($25)\n\t"
            "nop\n\t"
            "nop\n\t"
            ".word 0x4A280030"
            :
            : "r"(a1)
            : "$15", "$24", "$25"
        );
        p = a1->v3;
        v1 = p[0];
        t0 = p[1];
        a1->v3_0 = v1;
        a1->v3_1 = t0;
        v1 = a0[1].v01;
        a3 = a0[1].v23;
        t0 = v1 >> 16;
        t1 = a3 >> 16;
        v1 &= 0xFFFF;
        a3 &= 0xFFFF;
        a1->v0 = verts + (v1 << 3);
        a1->v1 = verts + (t0 << 3);
        a1->v2 = verts + (a3 << 3);
        a1->v3 = (int *)(verts + (t1 << 3));
        RR_GTE_OP(0x4B58002D);
        v1 = a0->uv0;
        t0 = a0->uv1;
        RR_GTE14_SWC2_OTZ(&a1->otz);
        if (a1->otz > 0) {
            RR_GTE_SWC2_SXYZ(&a1->u.w.sxy0);
            /* ASM_BARRIER: load V0 from a1->v3_0..v3_1 */
            __asm__ volatile("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(&a1->v3_0));
            a3 = a0->uv23;
            /* ASM_BARRIER: GTE RTPS after 1-cycle load delay */
            __asm__ volatile("nop\n\t.word 0x4A180001");
            v0 = a3 >> 16;
            /* ASM_BARRIER: keep sra $v0, $a3, 16 right after rtps and before pkt stores */
            __asm__ volatile("" : "+r"(v0));
            pkt[3] = v1;
            pkt[5] = t0;
            pkt[9] = v0;
            pkt[7] = a3;
            v1 = *(int *)((char *)a1 + 0x28);
            t0 = *(int *)((char *)a1 + 0x2C);
            a3 = *(int *)((char *)a1 + 0x30);
            pkt[2] = v1;
            pkt[4] = t0;
            pkt[6] = a3;
            RR_GTE_SWC2_SXY2(&a1->u.w.sxy3);
            RR_GTE_SWC2_SXY2(pkt + 8);
            v1 = a1->u.s.sx0;
            t0 = a1->u.s.sx1;
            a3 = a1->u.s.sx2;
            t1 = a1->u.s.sx3;
            if ((v1 >= 0 || t0 >= 0 || a3 >= 0 || t1 >= 0) &&
                (v1 < 0x140 || t0 < 0x140 || a3 < 0x140 || t1 < 0x140)) {
                v1 = a1->u.s.sy0;
                t0 = a1->u.s.sy1;
                a3 = a1->u.s.sy2;
                t1 = a1->u.s.sy3;
                if ((v1 >= 0 || t0 >= 0 || a3 >= 0 || t1 >= 0) &&
                    (v1 < 0x100 || t0 < 0x100 || a3 < 0x100 || t1 < 0x100)) {
                    t1 = a0->c0;
                    /* ASM_BARRIER: load IR0, RGB0, run DPCS */
                    __asm__ volatile(
                        "lwc2 $8, 0(%0)\n\t"
                        "mtc2 %1, $6\n\t"
                        "nop\n\t"
                        "nop\n\t"
                        ".word 0x4A780010"
                        :
                        : "r"(a1), "r"(t1)
                        : "memory"
                    );
                    RR_GTE14_SWC2_RGB2(pkt + 1);
                    g = (int *)SCRATCHPAD;
                    pkt[0] = *(int *)((a0->b19 << 2) + g[1] + 0xAF8);
                    ((char *)pkt)[3] = 9;
                    *(int *)((a0->b19 << 2) + g[1] + 0xAF8) = (int)pkt;
                    pkt += 10;
                    *(char *)((a0->b19 << 2) + g[1] + 0xAFB) = 0;
                }
            }
        }
        a0++;
        a2--;
    } while (a2 > 0);
    *(int **)SCRATCHPAD = pkt;
    return a0;
}

SkyTri17A90 *EmitSkyDepthCuedGT3(SkyTri17A90 *a0, SkyCtx *a1, int a2) {
    int v0 = a0->v01;
    int a3 = a0->v2;
    char *verts = a1->verts;
    int v1 = v0 >> 16;
    int *pkt;
    int *g;

    v0 &= 0xFFFF;
    a1->v0 = verts + (v0 << 3);
    a1->v1 = verts + (v1 << 3);
    a1->v2 = verts + (a3 << 3);
    pkt = ((Scratch17A90 *)SCRATCHPAD)->pkt;
    do {
        /* ASM_BARRIER: load V0..V2 from a1->v0..v2 into GTE $0..$5 */
        __asm__ volatile(
            "lw $13, 0x18(%0)\n\t"
            "lw $14, 0x1C(%0)\n\t"
            "lw $15, 0x20(%0)\n\t"
            "lwc2 $0, 0($13)\n\t"
            "lwc2 $1, 4($13)\n\t"
            "lwc2 $2, 0($14)\n\t"
            "lwc2 $3, 4($14)\n\t"
            "lwc2 $4, 0($15)\n\t"
            "lwc2 $5, 4($15)"
            :
            : "r"(a1)
            : "$13", "$14", "$15"
        );
        v0 = a0[1].v01;
        /* ASM_BARRIER: GTE RTPT after 1-cycle load delay */
        __asm__ volatile("nop\n\t.word 0x4A280030");
        a3 = a0[1].v2;
        v1 = v0 >> 16;
        v0 &= 0xFFFF;
        a1->v0 = verts + (v0 << 3);
        a1->v1 = verts + (v1 << 3);
        a1->v2 = verts + (a3 << 3);
        v0 = a0->uv0;
        v1 = a0->uv1;
        a3 = a0->u1C.uv2;
        pkt[3] = v0;
        pkt[6] = v1;
        pkt[9] = a3;
        RR_GTE_OP(0x4B58002D);
        RR_GTE14_SWC2_OTZ(&a1->otz);
        if (a1->otz > 0) {
            RR_GTE_SWC2_SXYZ(&a1->u.w.sxy0);
            v0 = a1->u.w.sxy0;
            v1 = a1->u.w.sxy1;
            a3 = a1->u.w.sxy2;
            pkt[2] = v0;
            pkt[5] = v1;
            pkt[8] = a3;
            v0 = a1->u.s.sx0;
            v1 = a1->u.s.sx1;
            a3 = a1->u.s.sx2;
            if ((v0 >= 0 || v1 >= 0 || a3 >= 0) &&
                (v0 < 0x140 || v1 < 0x140 || a3 < 0x140)) {
                v0 = a1->u.s.sy0;
                v1 = a1->u.s.sy1;
                a3 = a1->u.s.sy2;
                if ((v0 >= 0 || v1 >= 0 || a3 >= 0) &&
                    (v0 < 0x100 || v1 < 0x100 || a3 < 0x100)) {
                    v0 = a0->c0;
                    v1 = a0->c1;
                    a3 = a0->c2;
                    /* ASM_BARRIER: load IR0, RGB0..RGB2, run DPCT, store RGB0..RGB2 */
                    __asm__ volatile(
                        "lwc2 $8, 0(%0)\n\t"
                        "mtc2 %1, $6\n\t"
                        "mtc2 %1, $20\n\t"
                        "mtc2 %2, $21\n\t"
                        "mtc2 %3, $22\n\t"
                        "nop\n\t"
                        "nop\n\t"
                        ".word 0x4AF8002A\n\t"
                        "swc2 $20, 4(%4)\n\t"
                        "swc2 $21, 16(%4)\n\t"
                        "swc2 $22, 28(%4)"
                        :
                        : "r"(a1), "r"(v0), "r"(v1), "r"(a3), "r"(pkt)
                        : "memory"
                    );
                    g = (int *)SCRATCHPAD;
                    pkt[0] = *(int *)((a0->u1C.b.b1F << 2) + g[1] + 0xAF8);
                    ((char *)pkt)[3] = 9;
                    *(int *)((a0->u1C.b.b1F << 2) + g[1] + 0xAF8) = (int)pkt;
                    pkt += 10;
                    *(char *)((a0->u1C.b.b1F << 2) + g[1] + 0xAFB) = 0;
                }
            }
        }
        a0++;
        a2--;
    } while (a2 > 0);
    *(int **)SCRATCHPAD = pkt;
    return a0;
}

void DrawTrackScenery(a0) int a0; {
    int *s4;
    short *a1;
    int s1;
    int s2;
    int s3;
    int *p;
    int w;
    int r;
    int k;
    int *s0;
    s0 = (int *)(SCRATCHPAD + 0x7C);
    s4 = TrackSceneryList;
    RR_GTE_SETROT((int *)SCRATCH_ROT_MATRIX);
    RR_GTE_CLRTR();
    a1 = CourseSkyKeyframePtrs[CourseIndex];
    if (a1 != 0) {
        while (a1[0] < a0) a1 += 2;
        {
            int pv = a1[-1];
            int a3 = a1[1];
            int sum = pv + a3;
            if (sum == 0) {
                *s0 = 0;
            } else if (sum == 0x2000) {
                *s0 = 0x1000;
            } else {
                int a2 = a1[0];
                if (a0 == a2) {
                    *s0 = a3;
                } else {
                    int d = ((a2 - a0) << 12) / (a2 - a1[-2]);
                    RR_GTE_MTC2_IR0(d);
                    RR_GTE_MTC2_IR1((unsigned short)a1[1]);
                    RR_GTE_CTC2_FCR((unsigned short)a1[-1]);
                    RR_GTE_INTPL();
                    RR_GTE_SWC2_IR1(s0);
                }
            }
        }
    } else {
        *s0 = 0;
    }
    k = 0;
    RR_GTE_SETFC(k, k, k);
    a1 = CourseSceneryRangePtrs[CourseIndex];
    if (a1 != 0) {
        while (a1[0] < a0) a1 += 2;
        s3 = a1[1];
        s1 = 0;
    } else {
        s1 = 0;
        s3 = *s4;
    }
    s2 = 4;
    if (s3 > 0) {
        do {
            p = (int *)((char *)s4 + s2);
            s0[3] = p[0];
            r = p[2];
            while (w = *(int *)r, r += 4, w != 0) {
                k = w & 0xFFFF;
                w = w >> 16;
                switch (k) {
                case 7: r = (int)EmitSkyDepthCuedG3(r, s0, w); break;
                case 8: r = (int)EmitSkyDepthCuedG4(r, s0, w); break;
                case 1: r = (int)SkyEmitQuadFT4(r, s0, w); break;
                case 11: r = (int)EmitSkyDepthCuedGT3(r, s0, w); break;
                default: DebugPrintStub(StrUnknownModeFmt, k, w); return;
                }
            }
            s1++;
            s2 += 12;
        } while (s1 < s3);
    }
}

char *EmitSkyEnvMapGT3(char *a0, char *a1) {
    volatile int *hw;
    int n;
    int *pkt;
    int shift;
    char *verts;

    *(int *)(a1 + 0xF4) = 0x34808080;
    RR_GTE14_LWC2_RGB(a1 + 0xF4);

    hw = (volatile int *)SCRATCHPAD;
    n = *(int *)(a1 + 0xE4);
    pkt = (int *)hw[0];
    shift = hw[0x60 / 4];
    verts = *(char **)(a1 + 0x70);

    while (n-- > 0) {
        RR_GTE_RTPT_LDV(
            verts + (*(unsigned short *)(a0 + 0) << 3),
            verts + (*(unsigned short *)(a0 + 2) << 3),
            verts + (*(unsigned short *)(a0 + 4) << 3)
        );
        RR_GTE14_NOP2();
        RR_GTE14_NCLIP();
        RR_GTE14_SWC2_MAC0(a1 + 0xEC);

        if (*(int *)(a1 + 0xEC) > 0) {
            RR_GTE14_SWC2_SXYZ(a1 + 0x140);
            pkt[2] = *(int *)(a1 + 0x140);
            pkt[5] = *(int *)(a1 + 0x144);
            pkt[8] = *(int *)(a1 + 0x148);
            RR_GTE14_NOP2();
            RR_GTE_OP(0x4B58002D);

            if (*(short *)(a1 + 0x140) >= 0 || *(short *)(a1 + 0x144) >= 0 || *(short *)(a1 + 0x148) >= 0) {
                if (*(short *)(a1 + 0x140) < 0x140 || *(short *)(a1 + 0x144) < 0x140 || *(short *)(a1 + 0x148) < 0x140) {
                    if (*(short *)(a1 + 0x142) >= 0 || *(short *)(a1 + 0x146) >= 0 || *(short *)(a1 + 0x14A) >= 0) {
                        if (*(short *)(a1 + 0x142) < 0x200 || *(short *)(a1 + 0x146) < 0x200 || *(short *)(a1 + 0x14A) < 0x200) {
                            int u, v, z;

                            RR_GTE14_SWC2_OTZ(a1 + 0xE8);
                            RR_GTE14_LWC2_V0_V1_V2(
                                *(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 6) << 3),
                                *(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 8) << 3),
                                *(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 10) << 3)
                            );
                            RR_GTE14_NOP2();
                            RR_GTE14_LIGHT0();
                            RR_GTE14_LIGHT1();
                            RR_GTE14_LIGHT2();
                            /* ASM_BARRIER: store GTE RGB0/RGB1/RGB2 into packet offsets 4, 16, 28 */
                            __asm__ volatile("swc2 $20, 4(%0); swc2 $21, 16(%0); swc2 $22, 28(%0)" : : "r"(pkt) : "memory");
                            RR_GTE14_NOP2();
                            RR_GTE_OP(0x4A486012);
                            /* ASM_BARRIER: store GTE IR1/IR2/IR3 to context at +0x100 with memory clobber */
                            __asm__ volatile("swc2 $9, 0(%0); swc2 $10, 4(%0); swc2 $11, 8(%0)" : : "r"(a1 + 0x100) : "memory");
                            RR_GTE14_NOP2();
                            RR_GTE_OP(0x4A48E012);

                            u = (*(int *)(a1 + 0x100) >> 6) + 0x40;
                            v = (*(int *)(a1 + 0x104) >> 7) + 0x40;
                            if (u < 0) {
                                u = 0;
                            } else if (u >= 0x80) {
                                u = 0x7F;
                            }
                            if (v < 0) {
                                v = 0;
                            } else if (v >= 0x80) {
                                v = 0x7F;
                            }
                            pkt[3] = (v << 8) | u | *(int *)(a1 + 0xFC);

                            /* ASM_BARRIER: store GTE IR1/IR2/IR3 to context at +0x10C with memory clobber */
                            __asm__ volatile("swc2 $9, 0(%0); swc2 $10, 4(%0); swc2 $11, 8(%0)" : : "r"(a1 + 0x10C) : "memory");
                            RR_GTE14_NOP2();
                            RR_GTE_OP(0x4A496012);

                            u = (*(int *)(a1 + 0x10C) >> 6) + 0x40;
                            v = (*(int *)(a1 + 0x110) >> 7) + 0x40;
                            if (u < 0) {
                                u = 0;
                            } else if (u >= 0x80) {
                                u = 0x7F;
                            }
                            if (v < 0) {
                                v = 0;
                            } else if (v >= 0x80) {
                                v = 0x7F;
                            }
                            pkt[6] = (v << 8) | u | *(int *)(a1 + 0xF8);

                            /* ASM_BARRIER: store GTE IR1/IR2/IR3 to context at +0x118 with memory clobber */
                            __asm__ volatile("swc2 $9, 0(%0); swc2 $10, 4(%0); swc2 $11, 8(%0)" : : "r"(a1 + 0x118) : "memory");
                            u = (*(int *)(a1 + 0x118) >> 6) + 0x40;
                            v = (*(int *)(a1 + 0x11C) >> 7) + 0x40;
                            if (u < 0) {
                                u = 0;
                            } else if (u >= 0x80) {
                                u = 0x7F;
                            }
                            if (v < 0) {
                                v = 0;
                            } else if (v >= 0x80) {
                                v = 0x7F;
                            }
                            pkt[9] = (v << 8) | u;

                            z = *(int *)(a1 + 0xE8) >> shift;
                            if (z > 0) {
                                if (z < 0x1C0) {
                                    volatile int *g = (volatile int *)SCRATCHPAD;
                                    pkt[0] = *(int *)((*(signed char *)(a0 + 0x11) << 2) + ((z << 2) + g[1]) + 0x200);
                                    ((char *)pkt)[3] = 9;
                                    *(int *)((*(signed char *)(a0 + 0x11) << 2) + ((z << 2) + g[1]) + 0x200) = (int)pkt;
                                    pkt += 10;
                                    *(char *)((*(signed char *)(a0 + 0x11) << 2) + ((z << 2) + g[1]) + 0x203) = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
        a0 += 0x14;
    }

    /* ASM_BARRIER: extend live ranges of n and a0 past loop for register allocation priority */
    __asm__("" : : "r"(n), "r"(n), "r"(n), "r"(a0), "r"(a0), "r"(a0));
    *(int **)SCRATCHPAD = pkt;
    return a0;
}

char *EmitSkyEnvMapGT4(char *a0, char *a1) {
    volatile int *hw;
    int n;
    int *pkt;
    int shift;
    char *verts;

    *(int *)(a1 + 0xF4) = 0x3C808080;
    RR_GTE14_LWC2_RGB(a1 + 0xF4);

    hw = (volatile int *)SCRATCHPAD;
    n = *(int *)(a1 + 0xE4);
    pkt = (int *)hw[0];
    shift = hw[0x60 / 4];
    verts = *(char **)(a1 + 0x70);

    while (n-- > 0) {
        RR_GTE_RTPT_LDV(
            verts + (*(unsigned short *)(a0 + 0) << 3),
            verts + (*(unsigned short *)(a0 + 2) << 3),
            verts + (*(unsigned short *)(a0 + 4) << 3)
        );
        RR_GTE14_NOP2();
        RR_GTE14_NCLIP();
        RR_GTE14_SWC2_MAC0(a1 + 0xEC);
        RR_GTE14_SWC2_SXYZ(a1 + 0x140);
        RR_GTE_RTPS_LDV(verts + (*(unsigned short *)(a0 + 6) << 3));
        pkt[2] = *(int *)(a1 + 0x140);
        pkt[5] = *(int *)(a1 + 0x144);
        pkt[8] = *(int *)(a1 + 0x148);
        RR_GTE14_NOP2();
        RR_GTE14_NCLIP();
        RR_GTE14_SWC2_SXY2(a1 + 0x14C);
        RR_GTE14_SWC2_MAC0(a1 + 0xF0);

        if (*(int *)(a1 + 0xEC) > 0 || *(int *)(a1 + 0xF0) < 0) {
            RR_GTE14_NOP2();
            RR_GTE14_AVSZ4();
            pkt[11] = *(int *)(a1 + 0x14C);

            if (*(short *)(a1 + 0x140) >= 0 || *(short *)(a1 + 0x144) >= 0 || *(short *)(a1 + 0x148) >= 0 || *(short *)(a1 + 0x14C) >= 0) {
                if (*(short *)(a1 + 0x140) < 0x140 || *(short *)(a1 + 0x144) < 0x140 || *(short *)(a1 + 0x148) < 0x140 || *(short *)(a1 + 0x14C) < 0x140) {
                    if (*(short *)(a1 + 0x142) >= 0 || *(short *)(a1 + 0x146) >= 0 || *(short *)(a1 + 0x14A) >= 0 || *(short *)(a1 + 0x14E) >= 0) {
                        if (*(short *)(a1 + 0x142) < 0x200 || *(short *)(a1 + 0x146) < 0x200 || *(short *)(a1 + 0x14A) < 0x200 || *(short *)(a1 + 0x14E) < 0x200) {
                            int u, v, z;

                            RR_GTE14_SWC2_OTZ(a1 + 0xE8);
                            RR_GTE14_LWC2_V0_V1_V2(
                                *(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 8) << 3),
                                *(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 10) << 3),
                                *(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 12) << 3)
                            );
                            RR_GTE14_NOP2();
                            RR_GTE14_LIGHT0();
                            RR_GTE14_LIGHT1();
                            RR_GTE14_LIGHT2();
                            /* ASM_BARRIER: store GTE RGB0/RGB1/RGB2 into packet offsets 4, 16, 28 */
                            __asm__ volatile("swc2 $20, 4(%0); swc2 $21, 16(%0); swc2 $22, 28(%0)" : : "r"(pkt) : "memory");
                            RR_GTE14_NOP2();
                            RR_GTE_OP(0x4A486012);
                            /* ASM_BARRIER: store GTE IR1/IR2/IR3 to context at +0x100 with memory clobber */
                            __asm__ volatile("swc2 $9, 0(%0); swc2 $10, 4(%0); swc2 $11, 8(%0)" : : "r"(a1 + 0x100) : "memory");
                            RR_GTE14_NOP2();
                            RR_GTE_OP(0x4A48E012);

                            u = (*(int *)(a1 + 0x100) >> 6) + 0x40;
                            v = (*(int *)(a1 + 0x104) >> 7) + 0x40;
                            if (u < 0) {
                                u = 0;
                            } else if (u >= 0x80) {
                                u = 0x7F;
                            }
                            if (v < 0) {
                                v = 0;
                            } else if (v >= 0x80) {
                                v = 0x7F;
                            }
                            pkt[3] = (v << 8) | u | *(int *)(a1 + 0xFC);

                            /* ASM_BARRIER: store GTE IR1/IR2/IR3 to context at +0x10C with memory clobber */
                            __asm__ volatile("swc2 $9, 0(%0); swc2 $10, 4(%0); swc2 $11, 8(%0)" : : "r"(a1 + 0x10C) : "memory");
                            RR_GTE14_NOP2();
                            RR_GTE_OP(0x4A496012);

                            u = (*(int *)(a1 + 0x10C) >> 6) + 0x40;
                            v = (*(int *)(a1 + 0x110) >> 7) + 0x40;
                            if (u < 0) {
                                u = 0;
                            } else if (u >= 0x80) {
                                u = 0x7F;
                            }
                            if (v < 0) {
                                v = 0;
                            } else if (v >= 0x80) {
                                v = 0x7F;
                            }
                            pkt[6] = (v << 8) | u | *(int *)(a1 + 0xF8);

                            /* ASM_BARRIER: store GTE IR1/IR2/IR3 to context at +0x118 with memory clobber */
                            __asm__ volatile("swc2 $9, 0(%0); swc2 $10, 4(%0); swc2 $11, 8(%0)" : : "r"(a1 + 0x118) : "memory");
                            RR_GTE14_LWC2_V0(*(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 14) << 3));
                            RR_GTE14_NOP2();
                            RR_GTE14_LIGHT0();
                            RR_GTE14_SWC2_RGB2(pkt + 10);
                            RR_GTE14_NOP2();
                            RR_GTE_OP(0x4A486012);

                            u = (*(int *)(a1 + 0x118) >> 6) + 0x40;
                            v = (*(int *)(a1 + 0x11C) >> 7) + 0x40;
                            if (u < 0) {
                                u = 0;
                            } else if (u >= 0x80) {
                                u = 0x7F;
                            }
                            if (v < 0) {
                                v = 0;
                            } else if (v >= 0x80) {
                                v = 0x7F;
                            }
                            pkt[9] = (v << 8) | u;

                            /* ASM_BARRIER: store GTE IR1/IR2/IR3 to context at +0x124 with memory clobber */
                            __asm__ volatile("swc2 $9, 0(%0); swc2 $10, 4(%0); swc2 $11, 8(%0)" : : "r"(a1 + 0x124) : "memory");
                            u = (*(int *)(a1 + 0x124) >> 6) + 0x40;
                            v = (*(int *)(a1 + 0x128) >> 7) + 0x40;
                            if (u < 0) {
                                u = 0;
                            } else if (u >= 0x80) {
                                u = 0x7F;
                            }
                            if (v < 0) {
                                v = 0;
                            } else if (v >= 0x80) {
                                v = 0x7F;
                            }
                            pkt[12] = (v << 8) | u;

                            z = *(int *)(a1 + 0xE8) >> shift;
                            if (z > 0) {
                                if (z < 0x1C0) {
                                    volatile int *g = (volatile int *)SCRATCHPAD;
                                    pkt[0] = *(int *)((*(signed char *)(a0 + 0x15) << 2) + ((z << 2) + g[1]) + 0x200);
                                    ((char *)pkt)[3] = 0xC;
                                    *(int *)((*(signed char *)(a0 + 0x15) << 2) + ((z << 2) + g[1]) + 0x200) = (int)pkt;
                                    pkt += 13;
                                    *(char *)((*(signed char *)(a0 + 0x15) << 2) + ((z << 2) + g[1]) + 0x203) = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
        a0 += 0x18;
    }

    /* ASM_BARRIER: extend live range of n past loop for register allocation priority */
    __asm__("" : : "r"(n), "r"(n), "r"(n));
    *(int **)SCRATCHPAD = pkt;
    return a0;
}

char *SkyEmitLitQuadG4(char *a0, char *a1) {
    volatile int *hw = (volatile int *)SCRATCHPAD;
    int n = *(int *)(a1 + 0xE4);
    int *pkt = (int *)hw[0];
    int shift = hw[0x60 / 4];
    char *verts = *(char **)(a1 + 0x70);

    while (n-- > 0) {
        RR_GTE_RTPT_LDV(
            verts + (*(unsigned short *)(a0 + 0) << 3),
            verts + (*(unsigned short *)(a0 + 2) << 3),
            verts + (*(unsigned short *)(a0 + 4) << 3)
        );
        RR_GTE14_NOP2();
        RR_GTE14_NCLIP();
        RR_GTE14_SWC2_MAC0(a1 + 0xEC);
        RR_GTE14_NOP2();
        RR_GTE_OP(0x4B58002D);
        RR_GTE14_SWC2_OTZ(a1 + 0xE8);
        if (*(int *)(a1 + 0xE8) > 0) {
            RR_GTE14_SWC2_SXYZ(a1 + 0x140);
            RR_GTE_RTPS_LDV(verts + (*(unsigned short *)(a0 + 6) << 3));
            RR_GTE14_NOP2();
            RR_GTE14_NCLIP();
            RR_GTE14_SWC2_MAC0(a1 + 0xF0);
            if (*(int *)(a1 + 0xEC) > 0 || *(int *)(a1 + 0xF0) < 0) {
                RR_GTE14_SWC2_SXY2(a1 + 0x14C);
                pkt[2] = *(int *)(a1 + 0x140);
                pkt[4] = *(int *)(a1 + 0x144);
                pkt[6] = *(int *)(a1 + 0x148);
                pkt[8] = *(int *)(a1 + 0x14C);
                if ((*(short *)(a1 + 0x140) >= 0 || *(short *)(a1 + 0x144) >= 0 || *(short *)(a1 + 0x148) >= 0 || *(short *)(a1 + 0x14C) >= 0) &&
                    (*(short *)(a1 + 0x140) < 0x140 || *(short *)(a1 + 0x144) < 0x140 || *(short *)(a1 + 0x148) < 0x140 || *(short *)(a1 + 0x14C) < 0x140)) {
                    if ((*(short *)(a1 + 0x142) >= 0 || *(short *)(a1 + 0x146) >= 0 || *(short *)(a1 + 0x14A) >= 0 || *(short *)(a1 + 0x14E) >= 0) &&
                        (*(short *)(a1 + 0x142) < 0x200 || *(short *)(a1 + 0x146) < 0x200 || *(short *)(a1 + 0x14A) < 0x200 || *(short *)(a1 + 0x14E) < 0x200)) {
                        int z = *(int *)(a1 + 0xE8) >> shift;
                        if (z > 0) {
                            if (z < 0x1C0) {
                                volatile int *g = (volatile int *)SCRATCHPAD;
                                RR_GTE14_LWC2_RGB(a0 + 0x10);
                                RR_GTE14_LWC2_V0_V1_V2(
                                    *(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 8) << 3),
                                    *(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 10) << 3),
                                    *(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 12) << 3)
                                );
                                RR_GTE14_NOP2();
                                RR_GTE_OP(0x4B18043F);
                                RR_GTE14_SWC2_RGB012(pkt);
                                RR_GTE14_LWC2_V0(*(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 14) << 3));
                                RR_GTE14_NOP2();
                                RR_GTE_OP(0x4B08041B);
                                RR_GTE14_SWC2_RGB2(pkt + 7);
                                pkt[0] = *(int *)((*(signed char *)(a0 + 0x15) << 2) + ((z << 2) + g[1]) + 0x200);
                                ((char *)pkt)[3] = 8;
                                *(int *)((*(signed char *)(a0 + 0x15) << 2) + ((z << 2) + g[1]) + 0x200) = (int)pkt;
                                pkt += 9;
                                *(char *)((*(signed char *)(a0 + 0x15) << 2) + ((z << 2) + g[1]) + 0x203) = 0;
                            }
                        }
                    }
                }
            }
        }
        a0 += 0x18;
        /* ASM_BARRIER: boost n ref count in loop so n gets $t3 */
        __asm__("" : "+r"(n));
    }
    /* ASM_BARRIER: boost a0 ref count and extend verts live range past loop so a0 gets $t4 and verts gets $t5 */
    __asm__("" : : "r"(a0), "r"(a0), "r"(a0), "r"(verts));
    *(int **)SCRATCHPAD = pkt;
    return a0;
}

char *EmitSkyLitGT4(char *a0, char *a1) {
    volatile int *hw = (volatile int *)SCRATCHPAD;
    int n = *(int *)(a1 + 0xE4);
    int *pkt = (int *)hw[0];
    int shift = hw[0x60 / 4];
    char *verts = *(char **)(a1 + 0x70);
    int z;

    while (n-- > 0) {
        RR_GTE_RTPT_LDV(
            verts + (*(unsigned short *)(a0 + 0) << 3),
            verts + (*(unsigned short *)(a0 + 2) << 3),
            verts + (*(unsigned short *)(a0 + 4) << 3)
        );
        RR_GTE14_NOP2();
        RR_GTE14_NCLIP();
        RR_GTE14_SWC2_MAC0(a1 + 0xEC);
        RR_GTE14_NOP2();
        RR_GTE_OP(0x4B58002D);
        RR_GTE14_SWC2_OTZ(a1 + 0xE8);
        if (*(int *)(a1 + 0xE8) > 0) {
            RR_GTE14_SWC2_SXYZ(a1 + 0x140);
            RR_GTE_RTPS_LDV(verts + (*(unsigned short *)(a0 + 6) << 3));
            RR_GTE14_NOP2();
            RR_GTE14_NCLIP();
            RR_GTE14_SWC2_MAC0(a1 + 0xF0);
            if (*(int *)(a1 + 0xEC) > 0 || *(int *)(a1 + 0xF0) < 0) {
                RR_GTE14_SWC2_SXY2(a1 + 0x14C);
                pkt[2] = *(int *)(a1 + 0x140);
                pkt[5] = *(int *)(a1 + 0x144);
                pkt[8] = *(int *)(a1 + 0x148);
                pkt[11] = *(int *)(a1 + 0x14C);
                if ((*(short *)(a1 + 0x140) >= 0 || *(short *)(a1 + 0x144) >= 0 || *(short *)(a1 + 0x148) >= 0 || *(short *)(a1 + 0x14C) >= 0) &&
                    (*(short *)(a1 + 0x140) < 0x140 || *(short *)(a1 + 0x144) < 0x140 || *(short *)(a1 + 0x148) < 0x140 || *(short *)(a1 + 0x14C) < 0x140)) {
                    if ((*(short *)(a1 + 0x142) >= 0 || *(short *)(a1 + 0x146) >= 0 || *(short *)(a1 + 0x14A) >= 0 || *(short *)(a1 + 0x14E) >= 0) &&
                        (*(short *)(a1 + 0x142) < 0x200 || *(short *)(a1 + 0x146) < 0x200 || *(short *)(a1 + 0x14A) < 0x200 || *(short *)(a1 + 0x14E) < 0x200)) {
                        pkt[3] = *(int *)(a0 + 0x14);
                        pkt[6] = *(int *)(a0 + 0x18);
                        pkt[9] = *(int *)(a0 + 0x1C);
                        pkt[12] = *(short *)(a0 + 0x1E);
                        z = *(int *)(a1 + 0xE8) >> shift;
                        if (z > 0) {
                            if (z < 0x1C0) {
                                volatile int *g = (volatile int *)SCRATCHPAD;
                                RR_GTE14_LWC2_RGB(a0 + 0x10);
                                RR_GTE14_LWC2_V0_V1_V2(
                                    *(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 8) << 3),
                                    *(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 10) << 3),
                                    *(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 12) << 3)
                                );
                                RR_GTE14_NOP2();
                                RR_GTE14_LIGHT0();
                                RR_GTE14_LIGHT1();
                                RR_GTE14_LIGHT2();
                                /* ASM_BARRIER: store GTE RGB0..RGB2 into POLY_GT4 packet offsets 4, 16, 28 */
                                __asm__ volatile("swc2 $20, 4(%0); swc2 $21, 16(%0); swc2 $22, 28(%0)" : : "r"(pkt) : "memory");
                                RR_GTE14_LWC2_V0(*(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 14) << 3));
                                RR_GTE14_NOP2();
                                RR_GTE14_LIGHT0();
                                RR_GTE14_SWC2_RGB2(pkt + 10);
                                pkt[0] = *(int *)((*(signed char *)(a0 + 0x21) << 2) + ((z << 2) + g[1]) + 0x200);
                                ((char *)pkt)[3] = 0xC;
                                *(int *)((*(signed char *)(a0 + 0x21) << 2) + ((z << 2) + g[1]) + 0x200) = (int)pkt;
                                pkt += 13;
                                *(char *)((*(signed char *)(a0 + 0x21) << 2) + ((z << 2) + g[1]) + 0x203) = 0;
                                /* ASM_BARRIER: extend z live range to end of block so pkt+3 gets $t0 and z gets $t1 */
                                __asm__("" : : "r"(z));
                            }
                        }
                    }
                }
            }
        }
        a0 += 0x24;
        /* ASM_BARRIER: boost n ref count in loop so n gets $t3 */
        __asm__("" : "+r"(n));
    }
    /* ASM_BARRIER: boost a0 ref count and extend verts live range past loop */
    __asm__("" : : "r"(a0), "r"(a0), "r"(a0), "r"(verts));
    *(int **)SCRATCHPAD = pkt;
    return a0;
}

char *EmitSkyLitFT4(char *a0, char *a1) {
    volatile int *hw = (volatile int *)SCRATCHPAD;
    int n = *(int *)(a1 + 0xE4);
    int *pkt = (int *)hw[0];
    int shift = hw[0x60 / 4];
    char *verts = *(char **)(a1 + 0x70);

    while (n-- > 0) {
        RR_GTE_RTPT_LDV(
            verts + (*(unsigned short *)(a0 + 0) << 3),
            verts + (*(unsigned short *)(a0 + 2) << 3),
            verts + (*(unsigned short *)(a0 + 4) << 3)
        );
        RR_GTE14_NOP2();
        RR_GTE14_NCLIP();
        RR_GTE14_SWC2_MAC0(a1 + 0xEC);
        RR_GTE14_NOP2();
        RR_GTE_OP(0x4B58002D);
        RR_GTE14_SWC2_OTZ(a1 + 0xE8);
        if (*(int *)(a1 + 0xE8) > 0) {
            RR_GTE14_SWC2_SXYZ(a1 + 0x140);
            RR_GTE_RTPS_LDV(verts + (*(unsigned short *)(a0 + 6) << 3));
            RR_GTE14_NOP2();
            RR_GTE14_NCLIP();
            RR_GTE14_SWC2_MAC0(a1 + 0xF0);
            if (*(int *)(a1 + 0xEC) > 0 || *(int *)(a1 + 0xF0) < 0) {
                RR_GTE14_SWC2_SXY2(a1 + 0x14C);
                pkt[2] = *(int *)(a1 + 0x140);
                pkt[4] = *(int *)(a1 + 0x144);
                pkt[6] = *(int *)(a1 + 0x148);
                pkt[8] = *(int *)(a1 + 0x14C);
                if ((*(short *)(a1 + 0x140) >= 0 || *(short *)(a1 + 0x144) >= 0 || *(short *)(a1 + 0x148) >= 0 || *(short *)(a1 + 0x14C) >= 0) &&
                    (*(short *)(a1 + 0x140) < 0x140 || *(short *)(a1 + 0x144) < 0x140 || *(short *)(a1 + 0x148) < 0x140 || *(short *)(a1 + 0x14C) < 0x140)) {
                    if ((*(short *)(a1 + 0x142) >= 0 || *(short *)(a1 + 0x146) >= 0 || *(short *)(a1 + 0x14A) >= 0 || *(short *)(a1 + 0x14E) >= 0) &&
                        (*(short *)(a1 + 0x142) < 0x200 || *(short *)(a1 + 0x146) < 0x200 || *(short *)(a1 + 0x14A) < 0x200 || *(short *)(a1 + 0x14E) < 0x200)) {
                        int z;
                        pkt[3] = *(int *)(a0 + 0x14);
                        pkt[5] = *(int *)(a0 + 0x18);
                        pkt[7] = *(int *)(a0 + 0x1C);
                        pkt[9] = *(short *)(a0 + 0x1E);
                        z = *(int *)(a1 + 0xE8) >> shift;
                        if (z > 0) {
                            if (z < 0x1C0) {
                                volatile int *g = (volatile int *)SCRATCHPAD;
                                RR_GTE14_LWC2_RGB(a0 + 0x10);
                                RR_GTE14_LWC2_V0(*(char **)(a1 + 0x70) + (*(unsigned short *)(a0 + 8) << 3));
                                RR_GTE14_NOP2();
                                RR_GTE14_LIGHT0();
                                RR_GTE14_SWC2_RGB2(pkt + 1);
                                pkt[0] = *(int *)((*(signed char *)(a0 + 0x21) << 2) + ((z << 2) + g[1]) + 0x200);
                                ((char *)pkt)[3] = 9;
                                *(int *)((*(signed char *)(a0 + 0x21) << 2) + ((z << 2) + g[1]) + 0x200) = (int)pkt;
                                pkt += 10;
                                *(char *)((*(signed char *)(a0 + 0x21) << 2) + ((z << 2) + g[1]) + 0x203) = 0;
                            }
                        }
                    }
                }
            }
        }
        a0 += 0x24;
        /* ASM_BARRIER: weight pkt, n, a0 loop references for register allocation order */
        __asm__("" : "+r"(n) : "r"(pkt), "r"(a0));
    }
    /* ASM_BARRIER: extend verts live range after loop so pkt=$t0, n=$t1, a0=$t2, verts=$t3 */
    __asm__("" : : "r"(pkt), "r"(n), "r"(a0), "r"(verts));
    *(int **)SCRATCHPAD = pkt;
    return a0;
}

void RunSkyCommandList(char *a0, int a1)
{
    int *s = (int *)(SCRATCHPAD + 0x7C);
    int w, v;
    int *p;
    a0 += a1 * 12;
    v = *(int *)(a0 + 4);
    s[0x3F] = 0x20180000;
    s[0x3E] = 0x60000;
    s[0x1C] = v;
    p = *(int **)(a0 + 0xC);
    w = *p++;
    while (w) {
        s[0x39] = w >> 16;
        switch (w & 0xFFFF) {
        case 9:
            p = EmitSkyEnvMapGT3(p, s);
            break;
        case 10:
            p = EmitSkyEnvMapGT4(p, s);
            break;
        case 13:
            p = SkyEmitLitQuadG4(p, s);
            break;
        case 15:
            p = EmitSkyLitGT4(p, s);
            break;
        case 16:
            p = EmitSkyLitFT4(p, s);
            break;
        }
        w = *p++;
    }
}

void SetSkyMatrixAndDraw(char *a0, volatile unsigned short *a1)
{
    volatile unsigned short *p = (unsigned short *)(SCRATCHPAD + 0x7C);
    RR_GTE_SETROT(SkyLightDirMatrix);
    RR_GTE_MV3_COL(a1);
    RR_GTE_ST3_COL(p);
    RR_GTE_MV3_COL(a1 + 1);
    RR_GTE_ST3_COL((unsigned short *)(SCRATCHPAD + 0x7E));
    RR_GTE_MV3_COL(a1 + 2);
    RR_GTE_ST3_COL((unsigned short *)SCRATCH_CAR_WORK);
    RR_GTE_SETLLM(p);
    p[0] = a1[0];
    p[1] = a1[1];
    p[2] = a1[2];
    p[3] = a1[3] << 1;
    p[4] = a1[4] << 1;
    p[5] = a1[5] << 1;
    p[6] = a1[6];
    p[7] = a1[7];
    p[8] = a1[8];
    RR_GTE_SETROT(p);
    RR_GTE_SETTR(a0);
    RunSkyCommandList(SkyCommandList, 0);
}

void RenderSkyToRect(int a0, int a1, short *a2, int a3) {
    short sp10[4];
    int c; 
    register int *p __asm__("$4"); /* ASM_BARRIER: pin temp reg to match original allocation */
    int k; 
    int m; 
    int *q, *p1;
    int n;

    W(8) = 0x65000000;
    W(0xC) = (a2[1] << 16) | a2[0];
    W(0x14) = a2[2] | 0x01000000;
    W(0x10) = 0;
    W(4) = 0xE1000307;
    LNK(5, 0x10);
    p1 = (int *)(GS->cur + 0x18);
    GS->cur = (char *)p1;
    if (a2[3] >= 0x101) {
        p1[2] = 0x65000000;
        W(0xC) = ((a2[1] + 0x100) << 16) | a2[0];
        W(0x14) = ((a2[3] - 0x100) << 16) | a2[2];
        W(0x10) = 0;
        W(4) = 0xE1000317;
        LNK(5, 0x10);
        GS->cur = GS->cur + 0x18;
    }
    sp10[2] = 0x140;
    sp10[0] = 0;
    sp10[1] = 0;
    sp10[3] = 0x1E0;
    SetDrawArea(GS->cur, sp10);
    LNK(2, 0x10);
    GS->cur = GS->cur + 0xC;
    SetDrawOffset(GS->cur, sp10);
    LNK(2, 0x10);
    GS->cur = GS->cur + 0xC;
    SetGeomOffset((short)(unsigned short)a2[2] / 2, (short)(unsigned short)a2[3] / 2);
    SetSkyMatrixAndDraw(a0, a1);
    c = a3 << 16;
    n = a3 << 8;
    m = 0x66000000;
    n = n | m;
    c = c | n;
    c = c | a3;
    p = (int *)GS->cur;
    p[2] = c;
    p[3] = 0xFFFF0000;
    p[5] = a2[2] | 0x01000000;
    p[4] = 0;
    p[1] = 0xE1000307;
    p[0] = *(int *)(GS->ot + 0x10);
    *(char *)((int)p + 3) = 5;
    *(int *)(GS->ot + 0x10) = (int)p;
    *(char *)(GS->ot + 0x13) = 0;
    p = p + 6;
    q = p;
    q[2] = c;
    q[3] = 0xFF0000;
    q[5] = ((a2[3] - 0x100) << 16) | a2[2];
    q[4] = 0;
    k = 0xE1000317;
    q[1] = k;
    q[0] = *(int *)(GS->ot + 0x10);
    *(char *)((int)q + 3) = 5;
    *(int *)(GS->ot + 0x10) = (int)q;
    *(char *)(GS->ot + 0x13) = 0;
    p = p + 6;
    GS->cur = (char *)p;
    p[1] = 0x60000000;
    W(8) = 0;
    W(0xC) = (a2[3] << 16) | a2[2];
    W(0) = *(int *)(GS->ot + 0xAF0);
    *(char *)(GS->cur + 3) = 3;
    *(int *)(GS->ot + 0xAF0) = (int)GS->cur;
    *(char *)(GS->ot + 0xAF3) = 0;
    sp10[0] = 0x1C0;
    sp10[1] = 0;
    p = (int *)(GS->cur + 0x10);
    GS->cur = (char *)p;
    sp10[2] = a2[2];
    sp10[3] = a2[3];
    SetDrawArea(p, sp10);
    LNK(2, 0xAF0);
    GS->cur = GS->cur + 0xC;
    SetDrawOffset(GS->cur, sp10);
    LNK(2, 0xAF0);
    GS->cur = GS->cur + 0xC;
}
