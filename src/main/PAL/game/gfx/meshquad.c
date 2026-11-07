#include "rr_asm.h"
#include "rr/gpu.h"

/* Candidate module: mesh quad packet builders (GT4, G4, F4), 80057A44..80058038. */
#include "types.h"
#include "scratchpad.h"
#include "game/camera/camview.h"
#include "game/effect/spark.h"
#include "game/gfx/meshquad.h"
#include "rr/overlay665_render.h"
#include "game/gfx/gfxotag.h"
#include "game/effect/carfx.h"
#define RTPT() RR_GTE_RTPT_MEM()
#define RTPS() RR_GTE_RTPS_MEM()
#define NCLIP() RR_GTE_NCLIP_MEM()
#define AVSZ4() RR_GTE_AVSZ4_MEM()

struct Q
{
  char pad0[0xA0];
  int fA0;
  char pad1[0x100 - 0xA4];
  int f100;
  int f104;
  int f108;
};

extern struct 
{
  char pad[0x24C];
  struct Q *p;
} RaceContext;
extern struct 
{
  char pad[0x10E];
  unsigned short f10E;
} PlayerCars;
extern struct 
{
  char pad[8];
  short f8;
} *CarBoundsTable;

RrMeshQuadGT4 *DrawMeshQuadGT4(RrMeshQuadGT4 *p, s32 *s);
RrMeshQuadG4 *DrawMeshQuadG4(RrMeshQuadG4 *p, s32 *s);
RrMeshQuadF4 *DrawMeshQuadF4(RrMeshQuadF4 *p, s32 *s);
void DrawMeshPrimList(int arg0, int arg1, int arg2);

int StepCarPreviewDrive(int a0)
{
  int sp10[2];
  int s1;
  s1 = 4;
  RaceContext.p->f100 = (RaceContext.p->fA0 - s1) - (CarBoundsTable->f8 + ((short) (((short) PlayerCars.f10E) / 3)));
  s1 = UpdateCarPreviewCameraPath(&PlayerCars, RaceContext.p->f104, a0, sp10);
  UpdateSparkEffects();
  ScaleMatrixRow1(2, SCRATCH_ROT_MATRIX);
  DrawCarPreviewScene();
  RaceContext.p->f104 = RaceContext.p->f104 + 1;
  RaceContext.p->f108 = RaceContext.p->f108 + 1;
  if (RaceContext.p->f104 >= 0x322)
  {
    RaceContext.p->f104 = 0x321;
  }
  if (RaceContext.p->f108 >= 0x516)
  {
    RaceContext.p->f108 = 0x515;
  }
  if (Pad0Buttons & 0x800)
  {
    if (RaceContext.p->f104 >= 0x33)
    {
      s1 = 1;
    }
  }
  return s1;
}

RrMeshQuadGT4 *DrawMeshQuadGT4(RrMeshQuadGT4 *p, s32 *s) {
    s32 *g = (s32 *)SCRATCHPAD;
    RrPolyGT4WordPacket *out = (RrPolyGT4WordPacket *)g[0];
    s32 sh = g[0x18];
    u8 *verts = (u8 *)s[0];
    s32 n = s[1];
    s32 nv;
    register s32 x __asm__("$3");
    register s32 y __asm__("$4");
    RrPolyGT4WordPacket *t1 = out;
    if (n-- > 0) {
        s32 c3c = 0x3C;
        do {
            u8 *v0 = verts + (p->v0 << 3);
            u8 *v1 = verts + (p->v1 << 3);
            u8 *v2 = verts + (p->v2 << 3);
            __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\tlwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(v0), "r"(v1), "r"(v2));
            t1->len = 0xC;
            RTPT();
            ((u8 *)t1)[7] = c3c;
            {
                x = p->color08; y = p->color0C;
                t1->color04 = x;
                t1->color10 = y;
            }
            NCLIP();
            __asm__ volatile ("swc2 $24, 0(%0)" : : "r"(s + 6) : "memory");
            __asm__ volatile ("swc2 $12, 0(%0)\n\tswc2 $13, 4(%0)\n\tswc2 $14, 8(%0)" : : "r"(s + 2) : "memory");
            if (s[6] > 0) {
                u8 *v3 = verts + (p->v3 << 3);
                s32 x2, y2;
                __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(v3));
                x = s[2]; y = s[3];
                RTPS();
                t1->xy0 = x;
                t1->xy1 = y;
                __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(s + 5));
                x2 = s[4]; y2 = s[5];
                AVSZ4();
                __asm__ volatile ("swc2 $7, 0(%0)" : : "r"(s + 8) : "memory");
                s[8] = s[8] >> sh;
                if (s[8] < 0x1C1) {
                    t1->xy2 = x2;
                    t1->xy3 = y2;
                    {
                        x = p->uvClut18; y = p->uvTpage1C;
                        t1->uvClut0C = x;
                        t1->uvTpage18 = y;
                    }
                    {
                        x = p->uv2; y = p->uv3;
                        t1->uv2 = x;
                        t1->uv3 = y;
                    }
                    {
                        x = p->color10; y = p->color14;
                        t1->color1C = x;
                        t1->color28 = y;
                    }
                    {
                        s32 *ot = (s32 *)((char *)(s[0x15] * 4 + ((s8)p->otBias25 * 4 + (s[8] * 4 + s[0x16]))) + 0x200);
                        nv = 0xFFFFFF;
                        *(s32 *)t1 = (*(s32 *)t1 & 0xFF000000) | (*ot & nv);
                    }
                    {
                        s32 *ot = (s32 *)((char *)(s[0x15] * 4 + ((s8)p->otBias25 * 4 + (s[8] * 4 + s[0x16]))) + 0x200);
                        *ot = (*ot & 0xFF000000) | ((s32)t1 & 0xFFFFFF);
                    }
                    t1++;
                }
            }
            p++;
        } while (n-- > 0);
    }
    *(s32 *)SCRATCHPAD = (s32)t1;
    return p;
}

RrMeshQuadG4 *DrawMeshQuadG4(RrMeshQuadG4 *p, s32 *s) {
    s32 *g = (s32 *)SCRATCHPAD;
    RrPolyG4WordPacket *out = (RrPolyG4WordPacket *)g[0];
    s32 sh = g[0x18];
    u8 *verts = (u8 *)s[0];
    s32 n = s[1];
    s32 nv;
    register s32 x __asm__("$3");
    register s32 y __asm__("$4");
    RrPolyG4WordPacket *t1 = out;
    if (n-- > 0) {
        s32 c3c = 0x38;
        do {
            u8 *v0 = verts + (p->v0 << 3);
            u8 *v1 = verts + (p->v1 << 3);
            u8 *v2 = verts + (p->v2 << 3);
            __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\tlwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(v0), "r"(v1), "r"(v2));
            t1->len = 8;
            RTPT();
            ((u8 *)t1)[7] = c3c;
            {
                x = p->color08; y = p->color0C;
                t1->color04 = x;
                t1->color0C = y;
            }
            NCLIP();
            __asm__ volatile ("swc2 $24, 0(%0)" : : "r"(s + 6) : "memory");
            __asm__ volatile ("swc2 $12, 0(%0)\n\tswc2 $13, 4(%0)\n\tswc2 $14, 8(%0)" : : "r"(s + 2) : "memory");
            if (s[6] > 0) {
                u8 *v3 = verts + (p->v3 << 3);
                s32 x2, y2;
                __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(v3));
                x = s[2]; y = s[3];
                RTPS();
                t1->xy0 = x;
                t1->xy1 = y;
                __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(s + 5));
                x2 = s[4]; y2 = s[5];
                AVSZ4();
                __asm__ volatile ("swc2 $7, 0(%0)" : : "r"(s + 8) : "memory");
                s[8] = s[8] >> sh;
                if (s[8] < 0x1C1) {
                    t1->xy2 = x2;
                    t1->xy3 = y2;
                    {
                        x = p->color10; y = p->color14;
                        t1->color14 = x;
                        t1->color1C = y;
                    }
                    {
                        s32 *ot = (s32 *)((char *)(s[0x15] * 4 + ((s8)p->otBias19 * 4 + (s[8] * 4 + s[0x16]))) + 0x200);
                        nv = 0xFFFFFF;
                        *(s32 *)t1 = (*(s32 *)t1 & 0xFF000000) | (*ot & nv);
                    }
                    {
                        s32 *ot = (s32 *)((char *)(s[0x15] * 4 + ((s8)p->otBias19 * 4 + (s[8] * 4 + s[0x16]))) + 0x200);
                        *ot = (*ot & 0xFF000000) | ((s32)t1 & 0xFFFFFF);
                    }
                    t1++;
                }
            }
            p++;
        } while (n-- > 0);
    }
    *(s32 *)SCRATCHPAD = (s32)t1;
    return p;
}

RrMeshQuadF4 *DrawMeshQuadF4(RrMeshQuadF4 *p, s32 *s) {
    s32 *g = (s32 *)SCRATCHPAD;
    RrPolyF4Packet *out = (RrPolyF4Packet *)g[0];
    s32 sh = g[0x18];
    u8 *verts = (u8 *)s[0];
    s32 n = s[1];
    s32 nv;
    s32 x;
    s32 y;
    RrPolyF4Packet *t1 = out;
    if (n-- > 0) {
        s32 c3c = 0x28;
        do {
            u8 *v0 = verts + (p->v0 << 3);
            u8 *v1 = verts + (p->v1 << 3);
            u8 *v2 = verts + (p->v2 << 3);
            __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)\n\tlwc2 $2, 0(%1)\n\tlwc2 $3, 4(%1)\n\tlwc2 $4, 0(%2)\n\tlwc2 $5, 4(%2)" : : "r"(v0), "r"(v1), "r"(v2));
            t1->len = 5;
            ((u8 *)t1)[7] = c3c;
            RTPT();
            {
                s32 w = p->color08;
                t1->colorCode04 = w;
            }
            NCLIP();
            __asm__ volatile ("swc2 $24, 0(%0)" : : "r"(s + 6) : "memory");
            __asm__ volatile ("swc2 $12, 0(%0)\n\tswc2 $13, 4(%0)\n\tswc2 $14, 8(%0)" : : "r"(s + 2) : "memory");
            {
                u8 *v3 = verts + (p->v3 << 3);
                __asm__ volatile ("lwc2 $0, 0(%0)\n\tlwc2 $1, 4(%0)" : : "r"(v3));
                x = s[2]; y = s[3];
                RTPS();
                t1->xy0 = x;
                t1->xy1 = y;
                __asm__ volatile ("nop\n\tnop\n\t.word 0x4B400006");
                __asm__ volatile ("swc2 $14, 0(%0)" : : "r"(s + 5));
                __asm__ volatile ("swc2 $24, 0(%0)" : : "r"(s + 7));
                if (s[6] > 0 || s[7] < 0) {
                    x = s[4]; y = s[5];
                    AVSZ4();
                    __asm__ volatile ("swc2 $7, 0(%0)" : : "r"(s + 8) : "memory");
                    s[8] = s[8] >> sh;
                    if (s[8] < 0x1C1) {
                        t1->xy2 = x;
                        t1->xy3 = y;
                        {
                            s32 *ot = (s32 *)((char *)(s[0x15] * 4 + ((s8)p->otBias0D * 4 + (s[8] * 4 + s[0x16]))) + 0x200);
                            nv = 0xFFFFFF;
                            *(s32 *)t1 = (*(s32 *)t1 & 0xFF000000) | (*ot & nv);
                        }
                        {
                            s32 *ot = (s32 *)((char *)(s[0x15] * 4 + ((s8)p->otBias0D * 4 + (s[8] * 4 + s[0x16]))) + 0x200);
                            *ot = (*ot & 0xFF000000) | ((s32)t1 & 0xFFFFFF);
                        }
                        t1++;
                    }
                }
            }
            p++;
        } while (n-- > 0);
    }
    *(s32 *)SCRATCHPAD = (s32)t1;
    return p;
}

void DrawMeshPrimList(int arg0, int arg1, int arg2) {
    int *g = (int *)(SCRATCHPAD + 0x7C);
    int *a;
    int v;
    int t;
    g[0x54 / 4] = arg2;
    g[0x58 / 4] = *(int *)(SCRATCHPAD + 0x4);
    t = arg0 + arg1 * 0xC;
    g[0] = *(int *)(t + 4);
    a = *(int **)(t + 0xC);
    v = *a;
    a++;
    if (v != 0) {
        do {
            g[1] = v >> 16;
            switch (v & 0xFFFF) {
            case 0:
                a = DrawMeshQuadF4(a, g);
                break;
            case 12:
                a = DrawMeshQuadGT4(a, g);
                break;
            case 8:
                a = DrawMeshQuadG4(a, g);
                break;
            }
            v = *a;
            a++;
        } while (v != 0);
    }
}
