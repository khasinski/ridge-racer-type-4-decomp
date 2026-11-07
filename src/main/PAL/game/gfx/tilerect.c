/* RODATA 0x8001011c 0x20 */
#include "scratchpad.h"
#include "rr/tiled_rectangle.h"
#include "rr_asm.h"
#include "game/data/cdfile2.h"
#include "game/effect/spfade.h"
#include "game/gfx/sky.h"
#include "game/data/ldscn5.h"
#include "game/gfx/gtemodel.h"
#include "game/gfx/tilerect.h"
#include "game/data/cdfile1.h"
#include "game/data/cdfile2.h"
#include "psyq/libgpu/gpuenv.h"
#include "rr/menu_background.h"
#include "rr/sprite_preset.h"
#include "rr/memory.h"
#include "rr/controller_hud.h"

#define GS ((SPD *)SCRATCHPAD)
#define W(o) (*(int *)(GS->cur + (o)))
#define LNK(n, t) \
    W(0) = *(int *)(GS->ot + (t)); \
    *(char *)(GS->cur + 3) = (n); \
    *(int *)(GS->ot + (t)) = (int)GS->cur; \
    *(char *)(GS->ot + (t) + 3) = 0

struct B8 { unsigned char c[8]; };

typedef struct {
    char pad0[0x20];
    struct B8 unk20;
    char unk28[0x08];
    int unk30;
    int unk34;
    int w38;
    short s3C;
    short s3E;
    short s40;
    short s42;
    short s44;
    short s46;
    short s48;
    short s4A;
} S_8010BF70;

typedef struct {
    char *cur;
    char *ot;
} SPD;

typedef struct {
    int v01;
    int v23;
    int pad08;
    int uv0;
    int uv1;
    int uv23;
    signed char b18;
    signed char b19;
    unsigned short vz;
    int vxy;
} Poly1A52C;

typedef struct {
    int rot[5];
    int pad14[5];
    int ir3;
    int vxy;
    short vz;
    short pad32;
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
} Ctx1A52C;

typedef struct {
    unsigned short v0;
    unsigned short v1;
    unsigned short v2;
    unsigned short v3;
    int pad08;
    int uv0;
    int uv1;
    int uv23;
    signed char b18;
    signed char b19;
    short pad1A;
} Poly1A8DC;

typedef struct {
    int pad00[7];
    int otz;
    int mac0;
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
} Ctx1A8DC;

typedef struct {
    int v01;
    unsigned short v2;
    unsigned short pad;
    int c0;
    int c1;
    int c2;
} Tri20;

typedef struct {
    char pad[0x2C];
    char *p2C;
    Tri20 *p30;
} S_8001AC24;

extern struct B8 TileRectClearRect;
extern int SetMenuFrameOptions();
extern int InitMenuArenas();
extern short MenuFadeTimer;
extern int VSyncThreshold;
extern short ModeId;
extern short ModeGroup;
extern short ModeSubState;
extern int TileRectState;
extern struct B8 TileRectSceneInitRot;
extern struct B8 TileRectSkyRect;
extern int TileRectSkyShade;
extern char *LoadCursor;
extern unsigned char SelectionTableIndex;
extern int MenuConfirmPressed;
extern S_8010BF70 *TileRectScene;
extern int SetupRenderer();
extern int SetSineLightColors();
extern int ResetScreenCounters();
extern int LoadTimImage();
extern int LoadTimList();
extern int BuildCameraMatrices();
extern int RotMatrix();
extern int TileFadeColorTable[];
extern int TrackObjectFlashColors[];

void EmitTiledRect(RrGpuRectangleFields *rectangle, s32 intensity, u32 *head);
void TileRectSceneModeEnter(void);
void TileRectSceneModeUpdate(void);
void DrawTrackObjectBillboards(int a0);
int *EmitModelTrisG3(S_8001AC24 *a0, int *a1, int *a2);

void EmitTiledRect(RrGpuRectangleFields *rectangle, s32 intensity, u32 *head) {
    register s32 value;
    register u32 command;
    register RrGpuRectangleFields *rect = rectangle;
    register u32 *ordering = head;
    register s32 height, y, width, x;
    register s32 tile_h, tile_w;
    /* ASM_BARRIER: keep the common tile limit in S2 across both loops. */
    register s32 limit asm("$18");
    /* ASM_BARRIER: retain the Y texture remainder in V1 until row packing. */
    register s32 v asm("$3");
    /* ASM_BARRIER: retain the X texture remainder in T3 for UV packing. */
    register s32 u asm("$11");
    register s32 shade = intensity;
    /* ASM_BARRIER: materialize the per-packet length in V0 after the tag store. */
    register s32 length asm("$2");
    register u32 packed;
    /* ASM_BARRIER: retain packed row fields in S0/A2/T9/T8 across the X loop. */
    register u32 top asm("$16"), bits_h asm("$6"), vpart asm("$25"), ypage asm("$24");
    /* ASM_BARRIER: stage E1000200 in A0 within each tile. */
    register u32 drawmode asm("$4");
    u32 high, middle;
    /* ASM_BARRIER: retain the signed page-field shift in V1. */
    register s32 xpage asm("$3");
    /* ASM_BARRIER: retain the output/biased-field cursors in T4/T0. */
    register u8 *packet asm("$12"), *fields asm("$8");
    RrGpuScratchPointers *scratch;
    if (shade >= 0) {
        value = shade < 256;

        if (value) goto clamp_low;
        value = 255;
    } else {
clamp_low:
        value = shade;
        if (shade < 0) value = 0;
    }
    shade = value;
    command = 0xE6000000U;
    /* ASM_BARRIER: stage E6000000 before forming the high color byte. */
    __asm__("" : : "r"(command));
    high = (u32)shade<<16;
    packet = *(u8 **)SCRATCHPAD;
    *(u32 *)(packet+4) = command;
    /* ASM_BARRIER: publish the first mask command before completing color packing. */
    __asm__("" : "+r"(shade) : : "memory");
    middle = (u32)shade<<8;
    middle |= 0x66000000U;
    high |= middle;
    shade = (s32)((u32)shade|high);
    *(u32 *)packet = *ordering;
    packet[3] = 1;
    *ordering = (u32)(unsigned long)packet;
    ((u8 *)ordering)[3] = 0;
    height = rect->h;
    y = rect->y;
    packet+=sizeof(RrGpuSingleCommandPacket);
    if (height > 0) {
        limit = 256;
        do {
            v = y&255;
            value = limit-v;
            /* ASM_BARRIER: retain the unbounded row extent before the minimum. */
            __asm__("" : : "r"(value));
            tile_h = value;
            if (tile_h >= height) tile_h = height;
            width = rect->w;
            x = rect->x;
            value = y-1;
            if (width > 0) {

                top = (u32)value<<16;
                bits_h = (u32)tile_h<<16;
                /* ASM_BARRIER: keep the texture remainder separate from packed V. */
                __asm__("" : : "r"(v));
                vpart = (u32)v<<8;
                value = y&256;
                /* ASM_BARRIER: retain the signed Y-page shift after masking. */
                __asm__("" : "+r"(value));
                ypage = (u32)(value>>4);
                fields = packet+3;
                do {
                    u = x&63;
                    value = limit-u;
                    /* ASM_BARRIER: retain the unbounded column extent before the minimum. */
                    __asm__("" : : "r"(value));
                    tile_w = value;
                    if (tile_w >= width) tile_w = width;
                    drawmode = 0xE1000200U;

                    value = x&65535;
                    xpage = x&1023;
                    x+=tile_w;
                    packed = top|(u32)value;
                    *(u32 *)(fields+9) = packed;
                    packed = bits_h|(u32)tile_w;
                    *(u32 *)(fields+17) = packed;
                    packed = vpart|(u32)u;
                    /* ASM_BARRIER: retain an arithmetic X-page shift. */
                    __asm__("" : "+r"(xpage));
                    xpage >>= 6;
                    xpage |= 256;
                    xpage = (s32)ypage|xpage;
                    *(u32 *)(fields+5) = (u32)shade;
                    *(u32 *)(fields+13) = packed;
                    *(u32 *)(fields+1) = (u32)xpage|drawmode;
                    packed = *ordering;
                    width-=tile_w;
                    *(u32 *)packet = packed;
                    length = 5;

                    fields[0] = length;
                    fields+=sizeof(RrTiledRectanglePacket);
                    *ordering = (u32)(unsigned long)packet;
                    packet+=sizeof(RrTiledRectanglePacket);
                    ((u8 *)ordering)[3] = 0;
                } while (width > 0);
            }
            height-=tile_h;
            y+=tile_h;
        } while (height > 0);
    }
    scratch = (RrGpuScratchPointers *)SCRATCHPAD;
    *(u32 *)(packet+4) = 0xE6000001U;
    *(u32 *)packet = scratch->order[703];
    packet[3] = 1;
    scratch->order[703] = (u32)(unsigned long)packet;
    ((u8 *)&scratch->order[703])[3] = 0;
    scratch->cursor = packet+sizeof(RrGpuSingleCommandPacket);
}

void TileRectSceneModeEnter(void) {
    struct B8 sp10;
    VSyncThreshold = 0x80;
    InitMenuArenas();
    SetMenuFrameOptions(0, 1, 0);
    UpdateScreenFade(0x1000C);
    sp10 = TileRectClearRect;
    ClearImage2(&sp10, 0, 0, 0);
    MenuFadeTimer = 0;
    TileRectState = 0;
    ModeGroup = 0;
    ModeId = 4;
    ModeSubState = 0;
}

void TileRectSceneModeUpdate(void) {
    struct B8 sp10;
    int s1;

    UpdateScreenFade(0x1000C);
    if (MenuFadeTimer != 0) {
        MenuFadeTimer += 4;
        if (MenuFadeTimer >= 0x100) {
            ResetScreenCounters();
            ModeGroup = 1;
            ModeId = 0xB;
            ModeSubState = 0;
        }
    }

    switch (TileRectState) {
    case 0:
        if (CdFileReadStart(GetR4FileIndex(0, SelectionTableIndex + 1), LoadCursor) == 0) {
            return;
        }
        TileRectState = 1;
    case 1:
        if (CdFileBytesLoaded() == 0) {
            return;
        }
        LoadTimList(LoadCursor);
        TileRectState = 2;
    case 2:
        if (CdFileReadStart(GetR4FileIndex(0, 0x14), LoadCursor) == 0) {
            return;
        }
        TileRectState = 3;
    case 3:
        if (CdFileBytesLoaded() == 0) {
            return;
        }
        LoadTimImage(LoadCursor);
        TileRectState = 4;
    case 4:
        if (CdFileReadStart(GetR4FileIndex(0, SelectionTableIndex + 0xA), LoadCursor) == 0) {
            return;
        }
        TileRectState = 5;
    case 5:
        s1 = CdFileBytesLoaded();
        if (s1 == 0) {
            return;
        }
        RelocateOffsetTable(SkyCommandList = LoadCursor);
        TileRectScene = (S_8010BF70 *)(LoadCursor += s1);
        LoadCursor = (char *)TileRectScene + 0x4C;
        SetupRenderer(3);
        TileRectScene->unk20 = TileRectSceneInitRot;
        PsyqMemset(TileRectScene->unk28, 0, 0x10);
        {
            S_8010BF70 *p = TileRectScene;
            TileRectState = 6;
            p->s48 = 0x40;
            /* ASM_BARRIER: keep p in $v0 across TileRectState = 6 and field stores */
            __asm__("" : "+r"(p));
            p->w38 = 0;
            p->s3C = 0;
            p->s3E = 0;
            p->s40 = 0;
            p->s42 = 0;
            p->s44 = 0;
            p->s46 = 0;
            p->s4A = 0x1000;
            p->s3C = 0;
        }
    case 6:
        if (TileRectScene->s3C < 0x40) {
            if (TileRectScene->s3C == 8) {
                TileRectScene->s42 = 4;
            } else if (TileRectScene->s3C == 0x20) {
                TileRectScene->s44 = 4;
            }
            break;
        }
        SetMenuFrameOptions(1, 1, 0);
        TileRectState = 7;
    case 7:
        if (MenuFadeTimer == 0 && MenuConfirmPressed != 0) {
            MenuFadeTimer = 4;
            TileRectScene->s44 = -4;
            TileRectScene->s42 = -4;
        }
        break;
    }

    TileRectScene->s3C++;
    *(short *)((char *)TileRectScene + 0x22) -= 4;
    RampValueToLimit(&TileRectScene->s3E, &TileRectScene->s42, 0x80);
    RampValueToLimit(&TileRectScene->s40, &TileRectScene->s44, 0x80);

    if (TileRectScene->s4A != 0) {
        S_8010BF70 *p;
        /* ASM_BARRIER: keep interpolated IR2 result in $s2 */
        register int ir2 __asm__("$18");

        if (TileRectScene->s4A / 32 != 0) {
            TileRectScene->s4A -= TileRectScene->s4A / 32;
        } else if (TileRectScene->s4A >= 3) {
            TileRectScene->s4A -= 2;
        } else {
            TileRectScene->s4A = 0;
        }

        p = TileRectScene;
        /* ASM_BARRIER: GTE INTPL setup and execution on p->s4A */
        __asm__ volatile(
            "lhu $8, 0x4A(%1)\n\t"
            "nop\n\t"
            "mtc2 $8, $8\n\t"
            "addiu $9, $0, 0x720\n\t"
            "mtc2 $9, $9\n\t"
            "addu $10, $0, $0\n\t"
            "ctc2 $10, $21\n\t"
            "addiu $11, $0, 0x90\n\t"
            "mtc2 $11, $10\n\t"
            "addiu $8, $0, 0xFF\n\t"
            "ctc2 $8, $22\n\t"
            "nop\n\t"
            "nop\n\t"
            ".word 0x4A980011\n\t"
            "addiu %1, %1, 0x30\n\t"
            "swc2 $9, 0(%1)\n\t"
            "mfc2 %0, $10"
            : "=r"(ir2), "+r"(p)
            :
            : "memory"
        );
        TileRectSkyShade = ir2;
    }

    EmitPresetSprite(TileRectScene->s3E, 0x25);
    EmitPresetSprite(TileRectScene->s40, 0x26);

    W(4) = 0xE1000229;
    LNK(1, 4);
    GS->cur += 8;
    RampValueTo4096(&TileRectScene->s46, &TileRectScene->s48);

    if (TileRectScene->s46 != 0) {
        /* ASM_BARRIER: Keep &sp10 in $a2 before unaligned struct copy */
        register struct B8 *a2 asm("$6");
        SetSineLightColors(TileRectScene->s46);
        BuildCameraMatrices();
        RotMatrix(&TileRectScene->unk20, TileRectScene);
        /* ASM_BARRIER: Emit 8-byte unaligned struct copy using $8, $9, $10 without spilling $8 at earlier block move */
        __asm__(
            "addiu $6, $29, 0x10\n\t"
            "lui $2, %%hi(TileRectSkyRect)\n\t"
            "addiu $8, $2, %%lo(TileRectSkyRect)\n\t"
            "lwl $9, 3($8)\n\t"
            "lwr $9, 0($8)\n\t"
            "lwl $10, 7($8)\n\t"
            "lwr $10, 4($8)\n\t"
            "swl $9, 0x13($29)\n\t"
            "swr $9, 0x10($29)\n\t"
            "swl $10, 0x17($29)\n\t"
            "swr $10, 0x14($29)"
            : "=r"(a2), "=m"(sp10)
        );
        RenderSkyToRect(TileRectScene->unk28, TileRectScene, a2, TileRectSkyShade);
    }
    W(4) = 0xE6000001;
    LNK(1, 0xAFC);
    GS->cur += 8;
}

void DrawTrackObjectBillboards(int a0) {
    Ctx1A52C *a3 = (Ctx1A52C *)(SCRATCHPAD + 0x7C);
    /* ASM_BARRIER: pin scratchpad base pointer to $a1 ($5) to match original pre-loop CSE */
    register int *g asm("$5");
    int *v1;
    int *pkt;
    Poly1A52C *t3;
    char *verts;
    /* ASM_BARRIER: pin count to $t8 ($24) to match original caller-saved loop allocation */
    register int count asm("$24");
    int s2;
    int s1;
    int s0;
    int v0, v1_tmp, a1, a2, t2;

    a3->rot[4] = 0x4000;
    a3->rot[2] = 0x4000;
    a3->rot[0] = 0x4000;
    a3->rot[3] = 0;
    a3->rot[1] = 0;
    RR_GTE_SETROT(a3);
    RR_GTE_SETLLM((int *)SCRATCH_ROT_MATRIX);
    RR_GTE_SETBK_P(CameraViewMatrixTrans);

    g = (int *)SCRATCHPAD;
    /* ASM_BARRIER: keep SCRATCHPAD base pointer in $a1 and mark t2 global across basic blocks */
    __asm__("" : "+r"(g), "=r"(t2));
    v1 = (int *)((char *)g[0x48 / 4] - 4);
    pkt = (int *)g[0];
    v1 = (int *)((char *)v1 + a0 * 12);
    t3 = (Poly1A52C *)v1[3];
    verts = (char *)v1[1];
    count = ((unsigned short *)t3)[1];
    t3 = (Poly1A52C *)((char *)t3 + 4);
    v0 = t3->vxy;
    a2 = t3->vz;
    /* ASM_BARRIER: schedule count post-decrement and order global-alloc priorities t3 ($t3) > pkt ($t7) > verts ($t9) */
    __asm__("" : "+r"(count) : "r"(v0), "r"(a2), "r"(t3), "r"(t3), "r"(t3), "r"(t3), "r"(pkt), "r"(pkt));
    a3->vxy = v0;
    a3->vz = a2;
    s2 = g[1] + 0x200;
    s1 = g[0x60 / 4] + 2;
    while (count-- > 0) {
        RR_GTE_MVMVA_LLM_BK_LDV0(&a3->vxy);
        v0 = t3[1].vxy;
        a2 = t3[1].vz;
        a3->vxy = v0;
        a3->vz = a2;
        RR_GTE_SWC2_IR3(&a3->ir3);
        v0 = a3->ir3;
        if (v0 >= 0) {
            s0 = v0 >> s1;
            if (s0 < 0x1C0) {
                RR_GTE_TR_FROM_IR();
                v0 = t3->v01;
                a1 = t3->v23;
                a2 = v0 >> 16;
                t2 = a1 >> 16;
                v0 &= 0xFFFF;
                a1 &= 0xFFFF;
                RR_GTE_RTPT_LDV(
                    verts + (v0 << 3),
                    verts + (a2 << 3),
                    verts + (a1 << 3)
                );
                v0 = t3->uv0;
                a2 = t3->uv1;
                pkt[3] = v0;
                pkt[5] = a2;
                v0 = t3->uv23;
                a2 = v0 >> 16;
                pkt[7] = v0;
                pkt[9] = a2;
                RR_GTE_SWC2_SXYZ(&a3->u.w.sxy0);
                RR_GTE_RTPS_LDV(verts + (t2 << 3));
                a2 = a3->u.w.sxy0;
                a1 = a3->u.w.sxy1;
                t2 = a3->u.w.sxy2;
                v0 = TileFadeColorTable[(ScreenFrame - (v1_tmp = t3->b18 - 0x40)) & 0x3F];
                pkt[2] = a2;
                pkt[4] = a1;
                pkt[6] = t2;
                pkt[1] = v0;
                RR_GTE_SWC2_SXY2(&a3->u.w.sxy3);
                pkt[8] = a3->u.w.sxy3;
                {
                    int x0 = a3->u.s.sx0;
                    int x1 = a3->u.s.sx1;
                    int x2 = a3->u.s.sx2;
                    int x3 = a3->u.s.sx3;
                    if ((x0 >= 0 || x1 >= 0 || x2 >= 0 || x3 >= 0) &&
                        (x0 < 0x140 || x1 < 0x140 || x2 < 0x140 || x3 < 0x140)) {
                        int y0 = a3->u.s.sy0;
                        int y1 = a3->u.s.sy1;
                        int y2 = a3->u.s.sy2;
                        int y3 = a3->u.s.sy3;
                        if ((y0 >= 0 || y1 >= 0 || y2 >= 0 || y3 >= 0) &&
                            (y0 < 0x100 || y1 < 0x100 || y2 < 0x100 || y3 < 0x100)) {
                            pkt[0] = *(int *)((t3->b19 << 2) + ((s0 << 2) + s2));
                            ((char *)pkt)[3] = 9;
                            *(int *)((t3->b19 << 2) + ((s0 << 2) + s2)) = (int)pkt;
                            pkt += 10;
                            *(char *)((t3->b19 << 2) + ((s0 << 2) + s2) + 3) = 0;
                        }
                    }
                }
            }
        }
        t3++;
    }
    *(int **)SCRATCHPAD = pkt;
}

void DrawTrackObjectFlashQuads(int a0) {
    Ctx1A8DC *a3 = (Ctx1A8DC *)(SCRATCHPAD + 0x7C);
    /* ASM_BARRIER: pin scratchpad base pointer to $a1 ($5) to match original pre-loop CSE */
    register int *g asm("$5");
    int *v1;
    int *pkt;
    Poly1A8DC *t1;
    char *verts;
    /* ASM_BARRIER: pin count to $t3 ($11) to match original caller-saved loop allocation */
    register int count asm("$11");
    int t5;
    int t6;
    int t7;
    int v0, v1_tmp, a1, a2, off, g64;

    RR_GTE_SETROT((int *)SCRATCH_ROT_MATRIX);
    RR_GTE_SETTR((int *)(SCRATCHPAD + 0x3C));

    off = a0 * 12;
    g = (int *)SCRATCHPAD;
    /* ASM_BARRIER: keep SCRATCHPAD base pointer in $a1 and finish a0*12 before v1 load so v1 gets $a0 */
    __asm__("" : "+r"(g) : "r"(off));
    v1 = (int *)((char *)g[0x48 / 4] - 4);
    pkt = (int *)g[0];
    t7 = g[0x60 / 4] + 2;
    v1 = (int *)((char *)v1 + off);
    t1 = (Poly1A8DC *)v1[3];
    t5 = g[1] + 0x200;
    verts = (char *)v1[1];
    count = ((unsigned short *)t1)[1];
    t1 = (Poly1A8DC *)((char *)t1 + 4);
    g64 = g[0x64 / 4];
    /* ASM_BARRIER: materialize count in $t3 after g[0x64/4] load and order global-alloc priorities t1 ($t1) > pkt ($t2) > verts ($t4) > t5 ($t5) */
    __asm__("" : "+r"(count) : "r"(t1), "r"(t1), "r"(t1), "r"(t5), "r"(g64));
    t6 = (g64 != 0) << 31;
    while (count-- > 0) {
        RR_GTE_RTPT_LDV(
            verts + (t1->v0 << 3),
            verts + (t1->v1 << 3),
            verts + (t1->v2 << 3)
        );
        a2 = t1->uv0;
        a1 = t1->uv1;
        pkt[3] = a2;
        pkt[5] = a1;
        a2 = t1->uv23;
        a1 = a2 >> 16;
        pkt[7] = a2;
        pkt[9] = a1;
        RR_GTE14_NCLIP();
        RR_GTE_SWC2_SXYZ(&a3->u.w.sxy0);
        RR_GTE14_SWC2_MAC0(&a3->mac0);
        if ((a3->mac0 ^ t6) >= 0) {
            RR_GTE_RTPS_LDV(verts + (t1->v3 << 3));
            a2 = TrackObjectFlashColors[(ScreenFrame + t1->b18) & 0x3F];
            a1 = a3->u.w.sxy0;
            v0 = a3->u.w.sxy1;
            v1_tmp = a3->u.w.sxy2;
            pkt[2] = a1;
            pkt[4] = v0;
            pkt[6] = v1_tmp;
            pkt[1] = a2;
            RR_GTE14_NOP2();
            RR_GTE14_AVSZ4();
            RR_GTE_SWC2_SXY2(&a3->u.w.sxy3);
            RR_GTE14_SWC2_OTZ(&a3->otz);
            pkt[8] = a3->u.w.sxy3;
            {
                /* ASM_BARRIER: keep shifted otz in $v0 ($2) so sw $v0, 0x1C($a3) stays before slti $v0, $v0, 0x1C0 */
                register int otz asm("$2") = a3->otz;
                if (otz > 0) {
                    otz >>= t7;
                    a3->otz = otz;
                    if (otz < 0x1C0) {
                        int x0 = a3->u.s.sx0;
                        int x1 = a3->u.s.sx1;
                        int x2 = a3->u.s.sx2;
                        int x3 = a3->u.s.sx3;
                        if ((x0 >= 0 || x1 >= 0 || x2 >= 0 || x3 >= 0) &&
                            (x0 < 0x140 || x1 < 0x140 || x2 < 0x140 || x3 < 0x140)) {
                            int y0 = a3->u.s.sy0;
                            int y1 = a3->u.s.sy1;
                            int y2 = a3->u.s.sy2;
                            int y3 = a3->u.s.sy3;
                            if ((y0 >= 0 || y1 >= 0 || y2 >= 0 || y3 >= 0) &&
                                (y0 < 0x100 || y1 < 0x100 || y2 < 0x100 || y3 < 0x100)) {
                                /* ASM_BARRIER: give pkt+3 GIV higher priority than t1+0x19 GIV ($a2 vs $t0) and t6 higher priority than t7 ($t6 vs $t7) */
                                __asm__("" : : "m"(((char *)pkt)[1]), "m"(((char *)pkt)[2]), "r"(t1), "r"(pkt), "r"(pkt), "r"(t6));
                                pkt[0] = *(int *)((t1->b19 << 2) + ((a3->otz << 2) + t5));
                                ((char *)pkt)[3] = 9;
                                *(int *)((t1->b19 << 2) + ((a3->otz << 2) + t5)) = (int)pkt;
                                pkt += 10;
                                *(char *)((t1->b19 << 2) + ((a3->otz << 2) + t5) + 3) = 0;
                            }
                        }
                    }
                }
            }
        }
        t1++;
    }
    *(int **)SCRATCHPAD = pkt;
}

int *EmitModelTrisG3(S_8001AC24 *a0, int *a1, int *a2) {
    Tri20 *p = a0->p30;
    char *verts = a0->p2C;
    /* ASM_BARRIER: allocate count to $t5 ($13) so verts gets $t6 ($14) */
    register int count __asm__("$13") = ((unsigned short *)p)[1];
    int x0, x1, x2, y0, y1, y2;
    int *ret;

    p = (Tri20 *)((char *)p + 4);
    do {
        x0 = p->v01;
        x2 = p->v2;
        x1 = x0 >> 16;
        x0 &= 0xFFFF;
        RR_GTE_RTPT_LDV(
            verts + (x0 << 3),
            verts + (x1 << 3),
            verts + (x2 << 3)
        );
        x0 = p->c0;
        x1 = p->c1;
        x2 = p->c2;
        a2[1] = x0;
        a2[3] = x1;
        a2[5] = x2;
        /* ASM_BARRIER: GTE SXY0/SXY1/SXY2 stores and reads */
        __asm__ volatile(
            "swc2 $12, 8(%3)\n\t"
            "swc2 $13, 16(%3)\n\t"
            "swc2 $14, 24(%3)\n\t"
            "mfc2 %0, $12\n\t"
            "mfc2 %1, $13\n\t"
            "mfc2 %2, $14"
            : "=r"(x0), "=r"(x1), "=r"(x2)
            : "r"(a2)
            : "memory"
        );
        y0 = x0 >> 16;
        y1 = x1 >> 16;
        y2 = x2 >> 16;
        if ((y0 >= 0 || y1 >= 0 || y2 >= 0) &&
            (y0 < 0x100 || y1 < 0x100 || y2 < 0x100)) {
            x0 <<= 16;
            x1 <<= 16;
            x2 <<= 16;
            x0 >>= 16;
            x1 >>= 16;
            x2 >>= 16;
            if ((x0 >= 0 || x1 >= 0 || x2 >= 0) &&
                (x0 < 0x140 || x1 < 0x140 || x2 < 0x140)) {
                a2[0] = *a1;
                ((char *)a2)[3] = 6;
                *a1 = (int)a2;
                a2 += 7;
                ((char *)a1)[3] = 0;
            }
        }
        p++;
        count--;
    } while (count > 0);
    a2[1] = 0xE1000220;
    a2[0] = *a1;
    ((char *)a2)[3] = 1;
    ret = a2 + 2;
    *a1 = (int)a2;
    ((char *)a1)[3] = 0;
    return ret;
}
