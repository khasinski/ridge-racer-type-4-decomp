/* RODATA 0x8001007c 0x3c */
#include "types.h"
#include "scratchpad.h"
#include "rr/renderer_setup.h"
#include "rr_asm.h"

/* Candidate module: CD read state machine, 80012578..80012A68.
 * Adjacent matching functions share one directory and location definition.
 * This grouping does not establish the original compiler unit boundary. */
#include "rr/cd.h"
#include "game/data/bankrec.h"
#include "game/data/cdfile1.h"
#include "game/gfx/poly1.h"
#include "psyq/libcd/dsfinish.h"
#include "game/car/carmove1.h"
#include "game/data/bankrec.h"
#include "game/data/cdfile1.h"
#include "game/gfx/model.h"
#include "game/gfx/poly1.h"
#include "game/race/objdraw.h"
#include "psyq/libgpu/primatt8.h"
#include "game/gfx/sky.h"
#include "game/data/cdfile2.h"
#include "game/data/cdfile1.h"
#include "rr/bios_devices.h"
#include "rr/relative_file.h"
#include "game/gfx/model.h"
#define B (*(int *)&LoadCursor)

struct A { char p[0x14]; unsigned f14; char q[0x20]; unsigned f38; };

struct Q { int a; int v; };

struct E {
    struct A *src; int pad0; int z8, zC, z10;
    struct Q q1[4];
    int z34, z38;
    struct Q q2[4];
    int z5c, z60, z64, z68, z6c, z70;
    int pad74[4];
    struct Q q3[8];
    int fC4;
};

typedef struct {
    s32 header;
    s32 p4, p8, pC, p10, p14;
} Resource;

extern s32 CdFileState;
extern s32 CdFileSyncCount;
extern s32 CdFileIndexCurrent;
extern s32 (*CdFileStateHandlers[])();
extern s32 CdFileSectorOffset;
extern s32 CdFileSectors;
extern s32 CdFileDestBuffer;
extern s32 CdFileBytesRead;
extern s32 CdFileSyncResult;
extern RrCdLocation CdFileLocation;
extern u8 CdFileCdParam;
extern RrCdFileEntry CdFileTable[];
extern unsigned char CarPatchPartGroup[];
extern unsigned char CarTexPatchCellTable[];
extern int LoadCursor;
extern int *CarBoundsTable[];
extern struct E CarSlotBuffers[];
extern int SceneLoadState;
extern int SceneLoadId;
extern int TrackNodeLightBits;
extern void LoadTimList();
extern int CarLoadFileIndex;
extern int LoadingCarIndex;
extern int CarLoadPatchParam1;
extern int PendingCarVariant;
extern int RequestedCarId;
extern int CarLoadPatchIndex;
void DrawSync(s32);
void LoadImage(s16 *, void *);
extern s32 CarOffsetTables[];
extern short CourseIndex;
extern short CourseLightBlendTable[];
extern char *TrackLightTable;
extern short LightMatrix[];
extern short D_800ABED6[];
extern short D_800ABEDC[];

s32 CdFileStateCheckShell(void);
s32 CdFileStateGetTN(void);
s32 CdFileStateReloadDirectory(void);
s32 CdFileStateSetMode(void);
s32 CdFileStateWaitSetMode(void);
s32 CdFileStateWaitShellClose(void);
s32 CdFileStateSetloc(void);
s32 CdFileStateWaitSetloc(void);
s32 CdFileStateStartRead(void);
s32 CdFileStateReadSync(void);
void CdFileFinish(void);
s32 CdFileStateRetry(void);
s32 CdFileReadStart(s32 a0, s32 a1);
s32 CdFileReadRange(s32 a0, s32 a1, s32 a2, s32 a3);
void RelocateOffsetTable(int *a0);
void PatchModelBankTexPages(int *a0, int a1);
void PatchOffsetTableTexPages(int *a0, int a1);
int BlitCarSlotTexturePatch(int a0, int a1, int a2, int a3, int a4);
void AllocCarSlotBuffers(struct A *a0, int a1);
int RequestCarLoad(int a0, int a1, int a2, int a3, int a4);
void SetupRenderer(s32 shift);
int SetCarTrackLighting(char *a0_in, int a1_in);

s32 CdFileStateCheckShell(void) {
    s32 var_v0;

    CdControlB(1, 0, &CdFileCdParam);
    var_v0 = CdFileCdParam & 0x10;
    if (var_v0 != 0) {
        CdFileState = 5;
        var_v0 = 0;
    }
    return var_v0;
}

s32 CdFileStateGetTN(void) {
    CdControlF(0x13, 0);
    CdFileState = 2;
    return 0;
}

s32 CdFileStateReloadDirectory(void) {
    if (CdFileSyncResult == 2) {
        LoadR4Directory();
        CdFileState = 3;
    } else {
        CdFileState = 1;
    }
    return 0;
}

s32 CdFileStateSetMode(void) {
    CdFileCdParam = 0x80;
    CdControlF(0xE, &CdFileCdParam);
    CdFileState = 4;
    CdFileSyncCount = 0;
    return 0;
}

s32 CdFileStateWaitSetMode(void) {
    if (CdFileSyncResult == 2) {
        if (++CdFileSyncCount >= 4) {
            CdFileSyncCount = 0;
            if (CdFileIndexCurrent < 0) {
                CdFileState = 0;
            } else {
                CdFileState = 6;
            }
        }
    } else {
        CdFileState = 3;
    }
    return 0;
}

s32 CdFileStateWaitShellClose(void) {
    CdControlB(1, 0, &CdFileCdParam);
    if (!(CdFileCdParam & 0x10)) {
        CdFileState = 1;
    }
    return 0;
}

s32 CdFileStateSetloc(void) {
    CdControlF(2, &CdFileLocation);
    CdFileState = 7;
    return 0;
}

s32 CdFileStateWaitSetloc(void) {
    if (CdFileSyncResult == 2) {
        CdFileState = 8;
    } else {
        CdFileState = 0xB;
    }
    return 0;
}

s32 CdFileStateStartRead(void) {
    if (CdRead(CdFileSectors, CdFileDestBuffer, 0x80)) {
        CdFileState = 9;
    } else {
        CdFileState = 0xB;
    }
    return 0;
}

s32 CdFileStateReadSync(void) {
    s32 r = CdReadSync(1, 0);
    if (r == 0) {
        CdFileState = 0xA;
    } else if (r == -1) {
        CdFileState = 0xB;
    }
    return 0;
}

void CdFileFinish(void) {
    CdFileIndexCurrent = -1;
    CdFileState = 0;
    CdFileBytesRead = CdFileSectors << 0xB;
}

s32 CdFileStateRetry(void)
{
    CdControlB(1, 0, &CdFileCdParam);
    if (CdFileCdParam & 0x10) {
        CdFileState = 5;
        return 0;
    }
    CdFileState = 6;
    return 0;
}

s32 CdFileStep(void) {
    if (CdFileState > 0) {
        CdFileSyncResult = CdSync(1, 0);
        if (CdFileSyncResult != 0) {
            return CdFileStateHandlers[CdFileState]();
        }
    }
    return 0;
}

s32 CdFileReadStart(s32 a0, s32 a1) {
    if (CdFileState != 0) return 0;
    CdFileDestBuffer = a1;
    CdFileIndexCurrent = a0;
    CdFileState = 6;
    CdFileSectorOffset = 0;
    CdFileLocation = CdFileTable[a0].location;
    CdFileBytesRead = 0;
    CdFileSectors = CdFileTable[CdFileIndexCurrent].sectors;
    return 1;
}

s32 CdFileReadRange(s32 a0, s32 a1, s32 a2, s32 a3) {
    if (CdFileState != 0) return 0;
    CdFileDestBuffer = a3;
    CdFileIndexCurrent = a0;
    CdFileState = 6;
    CdFileSectorOffset = a1;
    CdIntToPos(CdPosToInt(&CdFileTable[a0]) + a1, CdFileLocation.c);
    CdFileSectors = a2;
    CdFileBytesRead = 0;
    return 1;
}

s32 CdFileBytesLoaded(void) {
    return CdFileBytesRead;
}

void RelocateOffsetTable(int *a0)
{
    int n = *a0;
    int *p = a0 + 1;
    int t;
    if (n-- > 0) {
        int *q = a0 + 3;
        do {
            t = n--;
            *p += (int)a0;
            p += 3;
            *q += (int)a0;
            q += 3;
        } while (t > 0);
    }
}

void PatchModelBankTexPages(int *a0, int a1) {
    int n = a0[0];
    char **list = (char **)(a0 + 3);
    int x = (a1 << 6) + 0x180;
    int tp_x = (x & 0x3FF) >> 6;
    int clut_x = (x >> 4) & 0x3F;
    int u_add = a1 << 6;
    int u_mask = ~0xC0;
    int clut2 = a1 + 0x1028;
    int t, z;

    while (z = 0, ((t = n--) + z + z + z + z + z + z + z + z + z + z + z + z + z + z + z + z + z + z + z + z) != 0) {
        char *p = *list++;
        unsigned int cmd;
        while (cmd = *(unsigned int *)p, p += 4, cmd != 0) {
            int count = cmd >> 16;
            switch (cmd & 0xFFFF) {
            case 0:
                p += count * 16;
                break;
            case 2:
                p += count * 24;
                break;
            case 1:
                while (count-- > 0) {
                    *(short *)(p + 0xE) = (*(short *)(p + 0xE) & ~0x1F) | tp_x;
                    *(short *)(p + 0xA) = (*(short *)(p + 0xA) & ~0x403C) | clut_x;
                    p += 0x18;
                }
                break;
            case 3:
                while (count-- > 0) {
                    if (*(short *)(p + 0x16) < 0) {
                        *(short *)(p + 0x16) = 0xA;
                        *(short *)(p + 0x10) = (*(short *)(p + 0x10) & u_mask) | u_add;
                        *(short *)(p + 0x14) = (*(short *)(p + 0x14) & u_mask) | u_add;
                        *(short *)(p + 0x18) = (*(short *)(p + 0x18) & u_mask) | u_add;
                        *(short *)(p + 0x1A) = (*(short *)(p + 0x1A) & u_mask) | u_add;
                        *(short *)(p + 0x12) = (*(short *)(p + 0x12) & ~0x403C) | clut2;
                        *(short *)(p + 0x1E) |= 0x78;
                    } else {
                        *(short *)(p + 0x16) = (*(short *)(p + 0x16) & ~0x1F) | tp_x;
                        *(short *)(p + 0x12) = (*(short *)(p + 0x12) & ~0x403C) | clut_x;
                        *(short *)(p + 0x1E) |= 0x78;
                    }
                    p += 0x20;
                }
                break;
            }
        }
    }
}

void PatchOffsetTableTexPages(int *a0, int a1) {
    int n = a0[0];
    char *list = (char *)(a0 + 1);
    int x = (a1 << 6) + 0x180;
    int tp_x = (x & 0x3FF) >> 6;
    int clut_x = (x >> 4) & 0x3F;
    int m1 = ~0x1F;
    int m2 = ~0x403C;

    while (n-- > 0) {
        char *p = *(char **)(list + 8);
        unsigned int cmd;
        list += 0xC;
        while (cmd = *(unsigned int *)p, p += 4, cmd != 0) {
            int count = cmd >> 16;
            switch (cmd & 0xFFFF) {
            case 7:
                p += count * 20;
                break;
            case 8:
                p += count * 28;
                break;
            case 1:
                while (count-- > 0) {
                    *(short *)(p + 0x12) = (*(short *)(p + 0x12) & m1) | tp_x;
                    *(short *)(p + 0xE) = (*(short *)(p + 0xE) & m2) | clut_x;
                    p += 0x1C;
                }
                break;
            case 9:
                p += count * 20;
                break;
            case 5:
            case 10:
                p += count * 24;
                break;
            case 3:
                p += count * 20;
                break;
            case 4:
                p += count * 12;
                break;
            case 6:
                p += count * 8;
                break;
            case 11:
                while (count-- > 0) {
                    *(short *)(p + 0x1A) = (*(short *)(p + 0x1A) & m1) | tp_x;
                    *(short *)(p + 0x16) = (*(short *)(p + 0x16) & m2) | clut_x;
                    p += 0x20;
                }
                break;
            case 14:
                while (count-- > 0) {
                    *(short *)(p + 0x16) = (*(short *)(p + 0x16) & m1) | tp_x;
                    *(short *)(p + 0x12) = (*(short *)(p + 0x12) & m2) | clut_x;
                    p += 0x1C;
                }
                break;
            case 15:
                while (count-- > 0) {
                    *(short *)(p + 0x1A) = (*(short *)(p + 0x1A) & m1) | tp_x;
                    *(short *)(p + 0x16) = (*(short *)(p + 0x16) & m2) | clut_x;
                    p += 0x24;
                }
                break;
            }
        }
    }
}

int BlitCarSlotTexturePatch(int a0, int a1, int a2, int a3, int a4)
{
    int b1 = a1;
    int xx;
    /* ASM_BARRIER: pins a<<4 temp to a3 ($7) to match reg choice */
    register int b16 asm("$7");
    int t;
    int c;
    int a, b, x;
    char *s0, *p0, *p1;
    char * volatile *g;
    /* ASM_BARRIER: keeps (SCRATCHPAD + 0x7C) pointer in a1 ($5) */
    register short *sp asm("$5");

    sp = (short *)(SCRATCHPAD + 0x7C);
    if (a0 == 2 && b1 == 0 && a2 == 0x13)
        return 0x13;
    t = CarTexPatchCellTable[b1 * 16 + a3];
    /* ASM_BARRIER: forces copy t->a (original addu a0,v1,zero) */
    __asm__("" : "=r"(a) : "0"(t));
    xx = (t & 7) * 0x10 + 0x340; x = xx;
    if (t < 0) a = t + 7;
    a >>= 3;
    c = CarPatchPartGroup[a2];
    b16 = a << 4; b = b16 + (a << 2);
    if (c >= 4) {
        if (c >= 7)
            b += 0x78;
        else
            b += 0x3C;
    }
    g = (char * *)SCRATCHPAD;
    s0 = g[0];
    sp[0] = xx;
    sp[1] = b;
    *(int *)(sp + 2) = 0x140010;
    SetDrawMove(s0, sp, (a4 << 6) + 0x1B0, 0xD8);
    *(int *)s0 = *(int *)g[1];
    s0[3] = 5;
    *(int *)g[1] = (int)s0;
    g[1][3] = 0;
    s0 += 0x18;
    g[0] = s0;
    *(int *)(s0 + 4) = 0xE6000000;
    p1 = g[1]; p0 = g[0]; *(int *)p0 = *(int *)p1;
    g[0][3] = 1;
    *(int *)g[1] = (int)g[0];
    g[1][3] = 0;
    g[0] += 8;
    /* ASM_BARRIER: keeps li v0,0 after final sw (stops sched hoisting it) */
    { int r; __asm__ volatile("" : "=r"(r) : "0"(0)); return r; }
}

void AllocCarSlotBuffers(struct A *a0, int a1)
{
    int s1;
    int a3, v1, t, u;
    struct E *s0;
    struct Q *p;

    s0 = (struct E *)((char *)CarSlotBuffers + a1 * 0x320);
    a3 = LoadCursor;
    u = ((a0->f14 + 2) / 3) * 0x18;
    t = ((a0->f38 + 2) / 3) * 0x18;
    s1 = 4;
    do {
        p = s0->q1;
        v1 = 3;
        s0->src = a0;
        s0->z8 = 0;
        s0->zC = 0;
        s0->z10 = 0;
        s0->z34 = 0;
        s0->z38 = 0;
        s0->z5c = 0;
        s0->z60 = 0;
        s0->z64 = 0;
        s0->z68 = 0;
        s0->z6c = 0;
        s0->z70 = 0;
        do {
            p->v = a3;
            p++;
            v1 -= 1;
            a3 += u;
        } while (v1 >= 0);
        p = s0->q2;
        v1 = 3;
        do {
            p->v = a3;
            p++;
            v1 -= 1;
            a3 += t;
        } while (v1 >= 0);
        p = s0->q3;
        v1 = 7;
        do {
            p->v = a3;
            p++;
            v1 -= 1;
            a3 += t;
        } while (v1 >= 0);
        s0++;
        s1 -= 1;
    } while (s1 > 0);
    LoadCursor = a3;
    if (*(unsigned char *)((char *)CarBoundsTable[a1] + 0xA) == 2) {
        s0 = (struct E *)((char *)CarSlotBuffers + a1 * 0x320);
        s1 = 4;
        do {
            s1 -= 1;
            s0->fC4 = LoadCursor;
            PsyqBzero(LoadCursor, 0x100);
            s0++;
            LoadCursor += 0x100;
        } while (s1 > 0);
    }
}

int RequestSceneLoad(int a0) {
    if (SceneLoadState != 0) {
        return 0;
    }
    SceneLoadId = a0;
    return SceneLoadState = 1;
}

int SceneLoadStep(void)
{
    int *a1;
    /* ASM_BARRIER: pin saved regs to original assignment */
    register int s1 __asm__("$17");
    register int s4 __asm__("$20");
    int s2,s3,s5,s6,s7,fp;
    int sp10, sp14, sp18;
    /* ASM_BARRIER: pin temps to the original v0/v1/a0 choices */
    register int a0 __asm__("$4");
    register int u __asm__("$2");
    register int t __asm__("$3");
    register int *pw __asm__("$2"); int s0;

    switch (SceneLoadState) {
    case 1:
        if (CdFileReadStart(GetSceneFileIndex(SceneLoadId), LoadCursor) == 0)
            return 0;
        SceneLoadState = 2;
    case 2:
        if (CdFileBytesLoaded() == 0)
            return 0;
        a0 = *(int *)(LoadCursor + 4) + LoadCursor;
        s0 = *(int *)(LoadCursor + 8) + LoadCursor;
        *(int *)(LoadCursor + 4) = a0;
        *(int *)(LoadCursor + 8) = s0;
        LoadTimList(a0);
        LoadTimList(s0);
        SceneLoadState = 3;
    case 3:
        if (CdFileReadStart(GetSceneFileIndex(SceneLoadId) + 1, LoadCursor) == 0)
            return 0;
        SceneLoadState = 4;
    case 4:
        s1 = CdFileBytesLoaded();
        if (s1 == 0)
            return 0;
        a1 = (int *)B;
        a1++; t = *a1; sp10 = t + B; *a1 = sp10;
        a1++; t = *a1; s2 = t + B; *a1 = s2;
        a1++; t = *a1; sp14 = t + B; *a1 = sp14;
        a1++; t = *a1; sp18 = t + B; *a1 = sp18;
        a1++; t = *a1; fp = t + B; *a1 = fp;
        a1++; t = *a1; s6 = t + B; *a1 = s6;
        a1++; t = *a1; s7 = t + B; *a1 = s7;
        a1++; t = *a1; s3 = t + B; *a1 = s3;
        a1++; t = *a1; s5 = t + B; *a1 = s5;
        /* ASM_BARRIER: hoist lui of SceneLoadId above the store (hi part only) */
        a1++; t = *a1; u = B; s4 = t + u; __asm__("" : "=r"(pw) : "0"(0x80110000)); *a1 = s4;
        a1++;
        if (*(int *)((char *)pw - 0x6BC4) == 7) {
            a0 = *a1; a0 += B;
            *a1 = a0;
            InitLineG2TrailPool(a0);
        }
        TrackSceneryList = s2;
        LoadCursor = LoadCursor + s1;
        RelocateRelativeFile(sp10);
        RelocateOffsetTable(s2);
        RegisterModelBank(sp14, 4);
        RelocateModelPointers(sp18);
        SetTrackNodeData(fp);
        SetLightTable(s6);
        TrackNodeLightBits = s7;
        LoadCursor = LoadCursor + LoadTrackObjects(s3, LoadCursor);
        SetSegmentOctantTable(s5);
        SetOctantLookupTable(s4);
        SceneLoadState = 0;
        return 1;
    }
}

int RequestCarLoad(int a0, int a1, int a2, int a3, int a4)
{
    int r;
    register int p0 asm("$17") = a0;
    register int p1 asm("$16") = a1;
    register int p2 asm("$18") = a2;
    register int p3 asm("$19") = a3;
    if (SceneLoadState != 0) {
        return 0;
    }
    CarLoadFileIndex = GetCarFileIndex();
    r = 1;
    LoadingCarIndex = a4;
    CarLoadPatchParam1 = p1;
    PendingCarVariant = p0;
    RequestedCarId = p2;
    CarLoadPatchIndex = p3;
    SceneLoadState = r;
    return r;
}

/* legacy typing of shared globals in this part of the unit */
#define RegisterModelBank ((void (*)(void *, s32))RegisterModelBank)
#define LoadCursor ((Resource *)LoadCursor)
#define CarBoundsTable (*(void *(*)[])&CarBoundsTable)

s32 CarLoadStep(void) {
    s16 rect[4];
    void *s0;
    void *s1;
    void *s2;
    void *s3;

    switch (SceneLoadState) {
    case 1:
        if (CdFileReadStart(CarLoadFileIndex, LoadCursor) == 0) {
            return 0;
        }
        SceneLoadState = 2;
    case 2:
        if (CdFileBytesLoaded() == 0) {
            return 0;
        }
        {
            s32 x;
            LoadCursor->p4 += (s32)LoadCursor;
            LoadCursor->p8 += (s32)LoadCursor;
            LoadCursor->pC += (s32)LoadCursor;
            LoadCursor->p10 += (s32)LoadCursor;
            LoadCursor->p14 += (s32)LoadCursor;
            s0 = (void *)LoadCursor->pC;
            s1 = (void *)LoadCursor->p10;
            s2 = (void *)LoadCursor->p14;
            s3 = (void *)LoadCursor->p8;
            SetCarRecordPtr((void *)LoadCursor->p4, LoadingCarIndex);
            CarBoundsTable[LoadingCarIndex] = s3;
            RegisterModelBank(s0, LoadingCarIndex);
            /* ASM_BARRIER: preserve local-alloc priority of s0 ($s0) over s3 ($s3) */
            __asm__("" : : "r"(s0));
            PatchModelBankTexPages(s0, LoadingCarIndex);
            CarOffsetTables[LoadingCarIndex] = (s32)s1;
            RelocateOffsetTable(s1);
            PatchOffsetTableTexPages(s1, LoadingCarIndex);
            rect[2] = 0x40;
            rect[1] = 0;
            rect[3] = 0x100;
            x = (LoadingCarIndex << 6) + 0x180;
            rect[0] = x;
        }
        LoadImage(rect, s2);
        DrawSync(0);
        LoadCursor = s2;
        AllocCarSlotBuffers(CarOffsetTables[LoadingCarIndex], LoadingCarIndex);
        if (CarLoadPatchIndex != 0) {
            BlitCarSlotTexturePatch(CarLoadPatchParam1, PendingCarVariant, RequestedCarId, CarLoadPatchIndex - 1, LoadingCarIndex);
        }
        SceneLoadState = 0;
        return 1;
    }
}

#undef RegisterModelBank
#undef LoadCursor
#undef CarBoundsTable

int RequestVramImageLoad(void) {
    if (SceneLoadState != 0) return 0;
    return SceneLoadState = 1;
}

/* legacy typing of shared globals in this part of the unit */
#define DrawSync ((int (*)())DrawSync)
#define LoadImage ((int (*)())LoadImage)

int VramImageLoadStep(void) {
    short sp[4];
    switch (SceneLoadState) {
    case 1:
        if (CdFileReadStart(GetR4FileIndex(0, 0), LoadCursor) == 0) return 0;
        SceneLoadState = 2;
    case 2:
        if (CdFileBytesLoaded() == 0) return 0;
        sp[0] = 0x340;
        sp[2] = 0x80;
        sp[1] = 0;
        sp[3] = 0xB4;
        LoadImage(sp, LoadCursor);
        DrawSync(0);
        SceneLoadState = 0;
        return 1;
    default:
        return 2;
    }
}

#undef DrawSync
#undef LoadImage

void SetupRenderer(s32 shift) {
    /* ASM_BARRIER: reuse the incoming argument register for the scratch base. */
    register RrRendererScratch *scratch __asm__("$4");
    *(s32*)(SCRATCHPAD + 0x60)=shift;
    scratch=(RrRendererScratch*)SCRATCHPAD;
    scratch->renderFlags64=MirrorMode;
    DefaultFT4Color.b=128;
    DefaultFT4Color.g=128;
    DefaultFT4Color.r=128;
    DefaultFT4Color.code=0x2C;
    PolyGT4WhiteColor.b=255;
    PolyGT4WhiteColor.g=255;
    PolyGT4WhiteColor.r=255;
    PolyGT4WhiteColor.code=0x3C;
    scratch->viewport6C.rect.w=320;
    scratch->viewport6C.rect.x=0;
    scratch->viewport6C.rect.y=0;
    scratch->viewport6C.rect.h=256;
}

/* legacy typing of shared globals in this part of the unit */
#define TrackNodeLightBits ((short *)TrackNodeLightBits)

int SetCarTrackLighting(char *a0_in, int a1_in) {
    /* ASM_BARRIER: keep temp in $v0 */
    register int v0 __asm__("$2");
    /* ASM_BARRIER: keep temp in $v1 */
    register int v1 __asm__("$3");
    /* ASM_BARRIER: keep a0_in / temp in $a0 */
    register int a0 __asm__("$4") = (int)a0_in;
    /* ASM_BARRIER: keep a1_in / temp in $a1 */
    register int a1 __asm__("$5") = a1_in;
    /* ASM_BARRIER: keep temp in $a2 */
    register int a2 __asm__("$6");
    /* ASM_BARRIER: keep temp in $a3 */
    register int a3 __asm__("$7");
    /* ASM_BARRIER: keep temp in $t0 */
    register int t0 __asm__("$8");
    /* ASM_BARRIER: keep temp in $t1 */
    register int t1 __asm__("$9");
    /* ASM_BARRIER: keep temp in $t2 */
    register int t2 __asm__("$10");
    /* ASM_BARRIER: keep a1_in copy in $t3 */
    register int t3 __asm__("$11");
    /* ASM_BARRIER: keep scratchpad (SCRATCHPAD + 0x7C) base in $t7 */
    register int *t7 __asm__("$15");
    /* ASM_BARRIER: keep destination pointer in $t8 */
    register short *t8 __asm__("$24");

    t3 = a1;
    t7 = (int *)(SCRATCHPAD + 0x7C);
    /* ASM_BARRIER: order t3 and t7 setup */
    __asm__ volatile("" : "+r"(t3), "+r"(t7));
    t7[0] = t3;
    a3 = *(short *)(a0 + 0x172);
    t2 = *(short *)(a0 + 0x170);
    v0 = *(int *)(a0 + 0xF0);
    v1 = (int)TrackNodeLightBits;
    /* ASM_BARRIER: order initial loads from a0 and TrackNodeLightBits */
    __asm__ volatile("" : "+r"(a3), "+r"(t2), "+r"(v0), "+r"(v1));
    t1 = a3 << 3;
    a3 = t1 + 0x2000;
    t2 <<= 2;
    v0 = (v0 << 1) + v1;
    /* ASM_BARRIER: order shifts and pointer addition before table loads */
    __asm__ volatile("" : "+r"(t1), "+r"(a3), "+r"(t2), "+r"(v0));
    t0 = *(short *)(v0 + 0);
    a2 = *(short *)(v0 + 2);
    /* ASM_BARRIER: load t0 and a2 before division-by-4096 branch */
    __asm__ volatile("" : "+r"(t0), "+r"(a2), "+r"(a3), "+r"(t1));
    a1 = a3;
    if (a3 < 0) {
        a1 = t1 + 0x2FFF;
    }
    v1 = a1 >> 12;
    v0 = (v1 << 1) + v1;
    a0 = v0 + 3;
    /* ASM_BARRIER: compute bit shift amounts v0 and a0 */
    __asm__ volatile("" : "+r"(v0), "+r"(a0), "+r"(a2), "+r"(t0));
    a1 = a2 >> a0;
    v1 = a2 >> v0;
    a2 = v1 & 7;
    a0 = t0 >> a0;
    v0 = t0 >> v0;
    t0 = v0 & 7;
    a3 &= 0xFFF;
    a1 &= 7;
    /* ASM_BARRIER: order bit extractions before CourseIndex load */
    __asm__ volatile("" : "+r"(a2), "+r"(t0), "+r"(a3), "+r"(a1));
    /* ASM_BARRIER: load %hi(CourseIndex) before masking a0 */
    __asm__ volatile("lui %0, %%hi(CourseIndex)" : "=r"(v0) : "r"(a0));
    a0 &= 7;
    /* ASM_BARRIER: load %lo(CourseIndex) after masking a0 */
    __asm__ volatile("lh %0, %%lo(CourseIndex)(%1)" : "=r"(v1) : "r"(v0), "r"(a0));
    v0 = (int)CourseLightBlendTable;
    /* ASM_BARRIER: load CourseLightBlendTable address before index shifts */
    __asm__ volatile("" : "+r"(v0));
    a2 <<= 1;
    a1 <<= 1;
    a0 <<= 1;
    v1 = (v1 << 4) + v0;
    /* ASM_BARRIER: order index shifts and row base calculation */
    __asm__ volatile("" : "+r"(a2), "+r"(a1), "+r"(a0), "+r"(v1));
    a2 += v1;
    a1 += v1;
    v0 = (t0 << 1) + v1;
    a0 += v1;
    /* ASM_BARRIER: order table element addresses before loads */
    __asm__ volatile("" : "+r"(a2), "+r"(a1), "+r"(v0), "+r"(a0));
    v1 = *(short *)a2;
    a1 = *(short *)a1;
    a2 = *(short *)v0;
    v0 = *(short *)a0;
    /* ASM_BARRIER: interpolate table values via GTE INTPL */
    __asm__ volatile(
        "mtc2 %4, $8\n\t"
        "ctc2 %0, $21\n\t"
        "ctc2 %2, $22\n\t"
        "mtc2 %3, $9\n\t"
        "mtc2 %1, $10\n\t"
        "nop\n\t"
        "nop\n\t"
        ".word 0x4A980011\n\t"
        "mtc2 %5, $8\n\t"
        "mfc2 %1, $10\n\t"
        "nop\n\t"
        "ctc2 %1, $21\n\t"
        "nop\n\t"
        "nop\n\t"
        ".word 0x4A980011\n\t"
        "mfc2 %1, $9\n\t"
        "nop\n\t"
        "mtc2 %1, $8"
        : "+r"(v0), "+r"(v1), "+r"(a1), "+r"(a2)
        : "r"(a3), "r"(t2)
    );

    a0 = t3 & 0xF;
    v0 = (t3 >> 4) & 0xF;
    /* ASM_BARRIER: extract low and high nibbles of t3 and load %hi(TrackLightTable) */
    __asm__ volatile("lui %0, %%hi(TrackLightTable)" : "=r"(a1) : "r"(a0), "r"(v0));
    v1 = (((v0 << 1) + v0) << 2) - v0;
    v1 <<= 2;
    t7[2] = v0;
    v0 = (((a0 << 1) + a0) << 2) - a0;
    /* ASM_BARRIER: load %lo(TrackLightTable) after subu v0, v0, a0 */
    __asm__ volatile("lw %0, %%lo(TrackLightTable)(%0)" : "+r"(a1) : "r"(v1), "r"(v0));
    v0 <<= 2;
    t7[1] = a0;
    /* ASM_BARRIER: store t7[1] before pointer additions */
    __asm__ volatile("" : "+r"(v0));
    v1 = a1 + v1;
    a1 = a1 + v0;
    /* ASM_BARRIER: keep struct pointers in v1 and a1 */
    __asm__ volatile("" : "+r"(v1), "+r"(a1));

    a2 = *(int *)(v1 + 0x10);
    a3 = *(short *)(v1 + 0x14);
    a0 = *(int *)(a1 + 0x10);
    t2 = *(short *)(a1 + 0x14);
    /* ASM_BARRIER: load first vector pair at +0x10 */
    __asm__ volatile("" : "+r"(a2), "+r"(a3), "+r"(a0), "+r"(t2));
    t0 = a2 >> 16;
    v0 = a2 << 16;
    /* ASM_BARRIER: unpack low half of a2 via v0 */
    __asm__ volatile("" : "+r"(t0), "+r"(v0));
    a2 = v0 >> 16;
    t1 = a0 >> 16;
    v0 = a0 << 16;
    /* ASM_BARRIER: unpack low half of a0 via v0 */
    __asm__ volatile("" : "+r"(a2), "+r"(t1), "+r"(v0));
    a0 = v0 >> 16;
    /* ASM_BARRIER: finish unpacking first vector pair */
    __asm__ volatile("" : "+r"(a0));
    RR_GTE_SETFC(a2, t0, a3);
    /* ASM_BARRIER: load IR1..IR3 and run INTPL */
    __asm__ volatile(
        "mtc2 %0, $9\n\t"
        "mtc2 %1, $10\n\t"
        "mtc2 %2, $11\n\t"
        "nop\n\t"
        "nop\n\t"
        ".word 0x4A980011"
        :
        : "r"(a0), "r"(t1), "r"(t2)
    );

    t8 = LightMatrix;
    a2 = *(int *)(v1 + 0x18);
    a3 = *(short *)(v1 + 0x1C);
    a0 = *(int *)(a1 + 0x18);
    t2 = *(short *)(a1 + 0x1C);
    /* ASM_BARRIER: load second vector pair at +0x18 */
    __asm__ volatile("" : "+r"(t8), "+r"(a2), "+r"(a3), "+r"(a0), "+r"(t2));
    t0 = a2 >> 16;
    v0 = a2 << 16;
    /* ASM_BARRIER: unpack low half of a2 via v0 */
    __asm__ volatile("" : "+r"(t0), "+r"(v0));
    a2 = v0 >> 16;
    t1 = a0 >> 16;
    v0 = a0 << 16;
    /* ASM_BARRIER: unpack low half of a0 via v0 */
    __asm__ volatile("" : "+r"(a2), "+r"(t1), "+r"(v0));
    a0 = v0 >> 16;
    /* ASM_BARRIER: finish unpacking second vector pair */
    __asm__ volatile("" : "+r"(a0));
    RR_GTE_STORE_IR_S16(t8);
    RR_GTE_SETFC(a2, t0, a3);
    /* ASM_BARRIER: load IR1..IR3 and run INTPL */
    __asm__ volatile(
        "mtc2 %0, $9\n\t"
        "mtc2 %1, $10\n\t"
        "mtc2 %2, $11\n\t"
        "nop\n\t"
        "nop\n\t"
        ".word 0x4A980011"
        :
        : "r"(a0), "r"(t1), "r"(t2)
    );

    t8 = D_800ABED6;
    a2 = *(int *)(v1 + 0x20);
    a3 = *(short *)(v1 + 0x24);
    a0 = *(int *)(a1 + 0x20);
    t2 = *(short *)(a1 + 0x24);
    /* ASM_BARRIER: load third vector pair at +0x20 */
    __asm__ volatile("" : "+r"(t8), "+r"(a2), "+r"(a3), "+r"(a0), "+r"(t2));
    t0 = a2 >> 16;
    v0 = a2 << 16;
    /* ASM_BARRIER: unpack low half of a2 via v0 */
    __asm__ volatile("" : "+r"(t0), "+r"(v0));
    a2 = v0 >> 16;
    t1 = a0 >> 16;
    v0 = a0 << 16;
    /* ASM_BARRIER: unpack low half of a0 via v0 */
    __asm__ volatile("" : "+r"(a2), "+r"(t1), "+r"(v0));
    a0 = v0 >> 16;
    /* ASM_BARRIER: finish unpacking third vector pair */
    __asm__ volatile("" : "+r"(a0));
    RR_GTE_STORE_IR_S16(t8);
    RR_GTE_SETFC(a2, t0, a3);
    /* ASM_BARRIER: load IR1..IR3 and run INTPL */
    __asm__ volatile(
        "mtc2 %0, $9\n\t"
        "mtc2 %1, $10\n\t"
        "mtc2 %2, $11\n\t"
        "nop\n\t"
        "nop\n\t"
        ".word 0x4A980011"
        :
        : "r"(a0), "r"(t1), "r"(t2)
    );

    t8 = D_800ABEDC;
    a3 = *(int *)(v1 + 0x28);
    a0 = *(int *)(a1 + 0x28);
    /* ASM_BARRIER: load color pair at +0x28 */
    __asm__ volatile("" : "+r"(t8), "+r"(a3), "+r"(a0));
    v0 = a3 << 4;
    /* ASM_BARRIER: shift R via v0 */
    __asm__ volatile("" : "+r"(v0));
    a2 = v0 & 0xFF0;
    v0 = a3 >> 4;
    /* ASM_BARRIER: shift G via v0 */
    __asm__ volatile("" : "+r"(a2), "+r"(v0));
    t0 = v0 & 0xFF0;
    v0 = a3 >> 12;
    /* ASM_BARRIER: shift B via v0 */
    __asm__ volatile("" : "+r"(t0), "+r"(v0));
    a3 = v0 & 0xFF0;
    /* ASM_BARRIER: finish unpacking RGB at +0x28 */
    __asm__ volatile("" : "+r"(a3));
    RR_GTE_STORE_IR_S16(t8);
    RR_GTE_SETFC(a2, t0, a3);
    /* ASM_BARRIER: load RGB into $6 and run DPCS */
    __asm__ volatile(
        "mtc2 %0, $6\n\t"
        "nop\n\t"
        "nop\n\t"
        ".word 0x4A780010"
        :
        : "r"(a0)
    );

    a3 = *(int *)(v1 + 0x4);
    a0 = *(int *)(a1 + 0x4);
    /* ASM_BARRIER: load color pair at +0x4 */
    __asm__ volatile("" : "+r"(a3), "+r"(a0));
    v0 = a3 << 4;
    /* ASM_BARRIER: shift R via v0 */
    __asm__ volatile("" : "+r"(v0));
    a2 = v0 & 0xFF0;
    v0 = a3 >> 4;
    /* ASM_BARRIER: shift G via v0 */
    __asm__ volatile("" : "+r"(a2), "+r"(v0));
    t0 = v0 & 0xFF0;
    v0 = a3 >> 12;
    /* ASM_BARRIER: shift B via v0 */
    __asm__ volatile("" : "+r"(t0), "+r"(v0));
    a3 = v0 & 0xFF0;
    /* ASM_BARRIER: finish unpacking RGB at +0x4 */
    __asm__ volatile("" : "+r"(a3));
    /* ASM_BARRIER: transfer interpolated IR1..IR3 to BK ($13..$15) */
    __asm__ volatile(
        "mfc2 $12, $9\n\t"
        "mfc2 $13, $10\n\t"
        "mfc2 $14, $11\n\t"
        "ctc2 $12, $13\n\t"
        "ctc2 $13, $14\n\t"
        "ctc2 $14, $15"
        :
        :
        : "$12", "$13", "$14"
    );
    RR_GTE_SETFC(a2, t0, a3);
    /* ASM_BARRIER: load RGB into $6 and run DPCS */
    __asm__ volatile(
        "mtc2 %0, $6\n\t"
        "nop\n\t"
        "nop\n\t"
        ".word 0x4A780010"
        :
        : "r"(a0)
    );

    t8 = (short *)(SCRATCHPAD + 0xB4);
    a3 = *(int *)(v1 + 0x8);
    a0 = *(int *)(a1 + 0x8);
    /* ASM_BARRIER: load color pair at +0x8 */
    __asm__ volatile("" : "+r"(t8), "+r"(a3), "+r"(a0));
    v0 = a3 << 4;
    /* ASM_BARRIER: shift R via v0 */
    __asm__ volatile("" : "+r"(v0));
    a2 = v0 & 0xFF0;
    v0 = a3 >> 4;
    /* ASM_BARRIER: shift G via v0 */
    __asm__ volatile("" : "+r"(a2), "+r"(v0));
    t0 = v0 & 0xFF0;
    v0 = a3 >> 12;
    /* ASM_BARRIER: shift B via v0 */
    __asm__ volatile("" : "+r"(t0), "+r"(v0));
    a3 = v0 & 0xFF0;
    /* ASM_BARRIER: finish unpacking RGB at +0x8 */
    __asm__ volatile("" : "+r"(a3));
    RR_GTE_ST3_COL(t8);
    RR_GTE_SETFC(a2, t0, a3);
    /* ASM_BARRIER: load RGB into $6 and run DPCS */
    __asm__ volatile(
        "mtc2 %0, $6\n\t"
        "nop\n\t"
        "nop\n\t"
        ".word 0x4A780010"
        :
        : "r"(a0)
    );

    t8 = (short *)(SCRATCHPAD + 0xB6);
    a3 = *(int *)(v1 + 0xC);
    a0 = *(int *)(a1 + 0xC);
    /* ASM_BARRIER: load color pair at +0xC */
    __asm__ volatile("" : "+r"(t8), "+r"(a3), "+r"(a0));
    v0 = a3 << 4;
    /* ASM_BARRIER: shift R via v0 */
    __asm__ volatile("" : "+r"(v0));
    a2 = v0 & 0xFF0;
    v0 = a3 >> 4;
    /* ASM_BARRIER: shift G via v0 */
    __asm__ volatile("" : "+r"(a2), "+r"(v0));
    t0 = v0 & 0xFF0;
    v0 = a3 >> 12;
    /* ASM_BARRIER: shift B via v0 */
    __asm__ volatile("" : "+r"(t0), "+r"(v0));
    a3 = v0 & 0xFF0;
    /* ASM_BARRIER: finish unpacking RGB at +0xC */
    __asm__ volatile("" : "+r"(a3));
    RR_GTE_ST3_COL(t8);
    RR_GTE_SETFC(a2, t0, a3);
    /* ASM_BARRIER: load RGB into $6 and run DPCS */
    __asm__ volatile(
        "mtc2 %0, $6\n\t"
        "nop\n\t"
        "nop\n\t"
        ".word 0x4A780010"
        :
        : "r"(a0)
    );

    t8 = (short *)(SCRATCHPAD + 0xB8);
    /* ASM_BARRIER: stage (SCRATCHPAD + 0xB8) in $t8 */
    __asm__ volatile("" : "+r"(t8));
    RR_GTE_ST3_COL(t8);
    t8 = (short *)(SCRATCHPAD + 0xB4);
    /* ASM_BARRIER: stage (SCRATCHPAD + 0xB4) in $t8 */
    __asm__ volatile("" : "+r"(t8));
    RR_GTE_SETLCM(t8);
    return t7[1];
}

#undef TrackNodeLightBits
