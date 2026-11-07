/* MASPSX_FLAGS: --assembler-profile=gnu */
/* GCC_VERSION: 2.7.2 */
/* CC1_FLAGS: -fsigned-char */
#include "psyq/libcd/dsread.h"
#include "psyq/libcd/cdfile.h"
#include "psyq/libcd/dsfile.h"

typedef struct {
    int x;
} __attribute__((packed)) U4;

/* MASPSX_FLAGS: --assembler-profile=gnu */
/* GCC_VERSION: 2.7.2 */
/* CC1_FLAGS: -fsigned-char */

typedef struct {
    unsigned char pos[4];
    int size;
    char name[16];
} CdlFILE;

struct DirPath {
    int a;
    int b;
    int c;
    char name[32];
};

extern int CD_debug;
extern struct DirPath DS_pathTable[];
extern CdlFILE DsDirFileCache[];
extern const char CdCacheFileDirNotFoundMsg[];
extern const char CdCacheFileSearchMsg[];
extern unsigned short DsDirNameDot;
extern short D_80011CE4;
extern char D_80011CE6;
extern const char CdFileEntryFmt[];
extern const char StrCdCacheFileCountFmt[];

extern int DebugPrintStub();
extern void CdIntToPos(int, void *);
extern void PsyqMemcpy(char *, void *, int);

int DS_cachefile(int dir) {
    int sp18;
    unsigned char *p;
    int i;

    if (dir == DS_cachedDir) {
        return 1;
    }
    if (cd_read(1, DS_pathTable[dir].a, CdIsoSectorBuf) != 1) {
        if (CD_debug > 0) {
            DebugPrintStub(CdCacheFileDirNotFoundMsg);
            return -1;
        }
        return -1;
    }
    p = CdIsoSectorBuf;
    if (CD_debug >= 2) {
        DebugPrintStub(CdCacheFileSearchMsg);
    }
    i = 0;
    while (p < CdIsoSectorBuf + 0x800) {
        if (p[0] == 0) {
            break;
        }
        *(U4 *)&sp18 = *(U4 *)(p + 2);
        CdIntToPos(sp18, DsDirFileCache[i].pos);
        *(U4 *)&DsDirFileCache[i].size = *(U4 *)(p + 10);
        switch (i) {
        case 0:
            __builtin_memcpy(DsDirFileCache[0].name, &DsDirNameDot, 2);
            break;
        case 1:
            __builtin_memcpy(DsDirFileCache[1].name, &D_80011CE4, 3);
            break;
        default:
            PsyqMemcpy(DsDirFileCache[i].name, p + 0x21, p[0x20]);
            DsDirFileCache[i].name[p[0x20]] = 0;
            break;
        }
        if (CD_debug >= 2) {
            DebugPrintStub(
                CdFileEntryFmt,
                DsDirFileCache[i].pos[0],
                DsDirFileCache[i].pos[1],
                DsDirFileCache[i].pos[2],
                DsDirFileCache[i].size,
                DsDirFileCache[i].name
            );
        }
        p += p[0];
        i++;
        if (i >= 0x40) {
            break;
        }
    }
    DS_cachedDir = dir;
    if (i < 0x40) {
        DsDirFileCache[i].name[0] = 0;
    }
    if (CD_debug >= 2) {
        DebugPrintStub(StrCdCacheFileCountFmt, i);
    }
    return 1;
}
