/* RODATA 0x80010014 0x14 */
#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"

/* Read the two-sector R4 directory, then convert relative boundaries to entries.
 * The disc supplies count + 1 valid offsets; sector arithmetic must fit s32. */
#include "rr/cd.h"
#include "psyq/libcd/dsfinish.h"
#include "psyq/libcd/s_1.h"
#include "game/data/cdfile1.h"
#include "rr/course_choices.h"

typedef struct {RrCdLocation location; s32 size; u8 name[16];} SearchResult;

extern s32 CdSearchFile(SearchResult *, const char *);
extern s32 CdRead(s32, void *, s32);
extern s32 CdPosToInt(const RrCdLocation *);
extern void func_800963C4(s32);
extern void DebugPrintStub(const char *, ...);
extern const char NowLoadingFmt[];
extern const char R4BinPath[];
extern const char StrFileNotFoundFmt[];
extern const char StrReadSectorFmt[];
extern u32 *CdDirectoryBuffer;
extern RrCdFileEntry CdFileTable[];
extern int rsin();
extern int rcos();
extern int LoadingTriangleAngle;
extern int LoadingTriangleFlip;
extern struct P { int w0, w4; short x0, y0; int c1; short x1, y1; int c2; short x2, y2; } D_80109400[];
extern struct P *LoadingTrianglePacket;

int GetCarFileIndex(int a0, int a1, int a2, int a3);
int GetR4FileIndex(int a0, int a1);
void DrawLoadingTriangle(void);

s32 func_80012208(s32 arg0) {
    return arg0 * 2;
}

int GetCarFileIndex(int a0, int a1, int a2, int a3) {
    int v = ((a1 * 4 + a0) * 0x14) + a2;
    if (a3 == 0) return v + 0x12;
    return v + 0x152;
}

int GetR4FileIndex(int a0, int a1) {
    switch (a0) {
    case 1: return a1 + 0x292;
    case 2: return a1 + 0x2D9;
    case 3: return a1 + 0x384;
    case 4: return a1 + 0x397;
    case 0: return a1 + 0x3A0;
    }
}

void LoadR4Directory(void)
{
    SearchResult file;
    s32 status, found;
    /* ASM_BARRIER: retain the two-sector transfer count across CD calls. */
    register s32 sectors __asm__("$16");
    /* ASM_BARRIER: preserve the PAL diagnostic page across retries. */
    register u8 *message_page __asm__("$18");
    u32 count;
    /* ASM_BARRIER: retain the directory index independently of its cursor. */
    register u32 i __asm__("$17");
    s32 base, sector, next;
    u32 *directory;
    RrCdFileEntry *entry;
    DebugPrintStub(NowLoadingFmt,R4BinPath,CdDirectoryBuffer);
    found=CdSearchFile(&file,R4BinPath);
    message_page=(u8 *)0x80010000U;
    if (!found) {
        DebugPrintStub(StrFileNotFoundFmt,R4BinPath);
        func_800963C4(1);
    }
    sectors=2;
    /* ASM_BARRIER: keep the saved count and address opaque to constant folding. */
    __asm__("" : "+r"(sectors), "+r"(message_page));
    do {
        CdControl(2,(const u8 *)&file.location,0);
        CdRead(sectors,CdDirectoryBuffer,0x80);
        do { status=CdReadSync(1,0); } while(status>0);
    } while(status);
    DebugPrintStub((const char *)(message_page+0x68),sectors);
    base=CdPosToInt(&file.location);
    directory=CdDirectoryBuffer;
    i=0;
    count=*directory++;
    sector=base+*directory++;
    entry=CdFileTable;
    while(i<count) {
        CdIntToPos(sector,entry->location.c);
        next=base+*directory++;
        entry->sectors=next-sector;
        sector=next;
        i++;
        entry++;
    }
}

/* legacy typing of shared globals in this part of the unit */
#define CdDirectoryBuffer (*(s32 (*))&CdDirectoryBuffer)

void InitCdFileDirectory(void) {
    CdSetDebug(1);
    LoadR4Directory();
    LoadCursor = CdDirectoryBuffer;
}

#undef CdDirectoryBuffer

void DrawLoadingTriangle(void)
{
    int a, v, w;
    unsigned *t;
    unsigned **g;
    register unsigned m asm("$5");
    unsigned h;
    a = LoadingTriangleAngle + 0x10;
    v = LoadingTriangleFlip ^ 1;
    LoadingTriangleFlip = v;
    LoadingTriangleAngle = a;
    LoadingTrianglePacket = &D_80109400[v];
    LoadingTrianglePacket->x0 = (rsin(a) >> 7) + 0x50;
    LoadingTrianglePacket->y0 = (rcos(LoadingTriangleAngle) >> 7) + 0x80;
    LoadingTrianglePacket->x1 = (rsin(LoadingTriangleAngle + 0x555) >> 7) + 0x50;
    LoadingTrianglePacket->y1 = (rcos(LoadingTriangleAngle + 0x555) >> 7) + 0x80;
    LoadingTrianglePacket->x2 = (rsin(LoadingTriangleAngle - 0x555) >> 7) + 0x50;
    LoadingTrianglePacket->y2 = (rcos(LoadingTriangleAngle - 0x555) >> 7) + 0x80;
    w = 0x300000FF;
    m = 0xFFFFFF;
    g = (unsigned **)SCRATCHPAD;
    h = 0xFF000000;
    LoadingTrianglePacket->c1 = 0xFF00;
    LoadingTrianglePacket->c2 = 0xFF0000;
    LoadingTrianglePacket->w4 = w;
    *((unsigned char *)LoadingTrianglePacket + 3) = 6;
    LoadingTrianglePacket->w0 = (LoadingTrianglePacket->w0 & h) | (**(g + 1) & m);
    t = *(g + 1);
    *t = (*t & h) | ((unsigned)LoadingTrianglePacket & m);
}
