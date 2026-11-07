/* MASPSX_FLAGS: --assembler-profile=gnu */
/* GCC_VERSION: 2.7.2 */
/* CC1_FLAGS: -fsigned-char -mhard-float */
#include "rr/library.h"
#include "psyq/libcd/dsread.h"
#include "psyq/libcd/dsfile.h"
#include "psyq/libcd/cdfile.h"
#include "psyq/libcd/dsfile.h"
#include "rr/audio_cd_controller.h"

typedef struct {
    unsigned char pos[4];
    unsigned int size;
    char name[16];
} CdlFILE;

typedef struct {
    int x;
} __attribute__((packed)) U4;

struct DirPath {
    int a;
    int b;
    int c;
    char name[32];
};

extern int CD_dirNopen;
extern int CD_nopen;
extern int CD_debug;
extern CdlFILE DsDirFileCache[];
extern const char CdPathLevelErrorFmt[];
extern const char StrCdDirNotFoundFmt[];
extern const char CdSearchFileDiscErrorMsg[];
extern const char StrCdSearchFileFmt[];
extern const char StrFoundFmt[];
extern const char FileNotFoundFmt[];
extern int PsyqStrncmp();
extern struct DirPath CD_pathtbl[];
extern const char CdNewMediaReadErrorStr[];
extern const char StrCd001[];
extern const char CdNewMediaFormatErrorMsg[];
extern const char StrCdNewmediaReadError[];
extern const char StrCdNewmediaSearchingDir[];
extern const char CdFileInfoFmt[];
extern const char StrCdNewMediaDirCountFmt[];
extern void PsyqMemcpy(char *, void *, int);
extern char CdPathTblName[];
extern s32 PsyqStrcmp(const char *, const char *);

CdlFILE *CdSearchFile(CdlFILE *fp, char *name);
int _cmp(int a, int b);
int DS_newmedia(void);
s32 DS_searchdir(s32 key, const char *name);

CdlFILE *CdSearchFile(CdlFILE *fp, char *name) {
    char buf[16];
    char *p;
    char *d;
    int i;
    int dir;
    /* ASM_BARRIER: use hard reg $0 so jump2 cross-jumping does not merge this return with .L8008E130 */
    register CdlFILE *zero asm("$0");

    if (CD_dirNopen != CD_nopen) {
        if (DS_newmedia() == 0)
            return 0;
        CD_dirNopen = CD_nopen;
    }
    if (*name != '\\') {
        return zero;
    err:
        buf[0] = 0;
    } else {
        buf[0] = 0;
        dir = 1;
        p = name;
        i = 0;
        do {
            for (d = buf; *p != '\\'; d++) {
                if (*p == 0)
                    goto done;
                *d = *p++;
            }
            if (*p == 0)
                goto done;
            p++;
            *d = 0;
            if ((dir = DS_searchdir(dir, buf)) == -1)
                goto err;
            i++;
        } while (i < 8);
    }
done:
    if (i >= 8) {
        if (CD_debug > 0)
            DebugPrintStub(CdPathLevelErrorFmt, name, i);
        return 0;
    }
    if (buf[0] == 0) {
        if (CD_debug > 0)
            DebugPrintStub(StrCdDirNotFoundFmt, name);
        return 0;
    }
    *d = 0;
    if (DS_cachefile(dir) == 0) {
        if (CD_debug > 0)
            DebugPrintStub(CdSearchFileDiscErrorMsg);
        return 0;
    }
    if (CD_debug >= 2)
        DebugPrintStub(StrCdSearchFileFmt, buf);
    for (i = 0; i < 64; i++) {
        if (DsDirFileCache[i].name[0] == 0)
            break;
        if (_cmp(DsDirFileCache[i].name, buf)) {
            if (CD_debug >= 2)
                DebugPrintStub(StrFoundFmt, buf);
            *fp = DsDirFileCache[i];
            return &DsDirFileCache[i];
        }
    }
    if (CD_debug > 0)
        DebugPrintStub(FileNotFoundFmt, buf);
    return 0;
}

int _cmp(int a, int b) { return PsyqStrncmp(a, b, 12) == 0; }

/* legacy typing of shared globals in this part of the unit */
#define PsyqStrncmp ((int (*)(void *, const char *, int))PsyqStrncmp)

int DS_newmedia(void) {
    int sp18;
    unsigned char *p;
    int i;
    unsigned char *p8c;
    int pad;
    int dummy;

    if (cd_read(1, 0x10, CdIsoSectorBuf) != 1) {
        if (CD_debug > 0) {
            DebugPrintStub(CdNewMediaReadErrorStr);
        }
        return 0;
    }
    if (PsyqStrncmp(CdIsoSectorBuf + 1, StrCd001, 5) != 0) {
        if (CD_debug > 0) {
            DebugPrintStub(CdNewMediaFormatErrorMsg);
        }
        return 0;
    }
    p8c = CdIsoSectorBuf;
    /* ASM_BARRIER: prevent cse2 from folding s0+0x8C back to symbol+0x8C */
    __asm__ volatile("" : "=r"(p8c) : "0"(p8c));
    *(U4 *)&sp18 = *(U4 *)(p8c + 0x8C);
    if (cd_read(1, sp18, p8c) != 1) {
        if (CD_debug > 0) {
            DebugPrintStub(StrCdNewmediaReadError, sp18);
        }
        return 0;
    }
    p = p8c;
    if (CD_debug >= 2) {
        DebugPrintStub(StrCdNewmediaSearchingDir);
    }
    i = 0;
    while (p < p8c + 0x800) {
        if (p[0] == 0) {
            break;
        }
        *(U4 *)&CD_pathtbl[i].c = *(U4 *)(p + 2);
        CD_pathtbl[i].b = p[6];
        CD_pathtbl[i].a = i + 1;
        PsyqMemcpy(CD_pathtbl[i].name, p + 8, p[0]);
        dummy = (p < p8c + 0x800);
        CD_pathtbl[i].name[p[0]] = 0;
        pad = (p[0] & 1) + 8;
        p += p[0] + pad;
        if (CD_debug >= 2) {
            DebugPrintStub(
                CdFileInfoFmt,
                CD_pathtbl[i].c,
                CD_pathtbl[i].a,
                CD_pathtbl[i].b,
                CD_pathtbl[i].name
            );
        }
        i++;
        if (i >= 0x80) {
            break;
        }
    }
    if (i < 0x80) {
        CD_pathtbl[i].b = 0;
    }
    DS_cachedDir = 0;
    if (CD_debug >= 2) {
        DebugPrintStub(StrCdNewMediaDirCountFmt, i);
    }
    return 1;
}

#undef PsyqStrncmp

s32 DS_searchdir(s32 key, const char *name) {
    s32 i = 0;
    register char *entry_name;
    /* ASM_BARRIER: Retain the original table-offset register across comparisons. */
    register s32 offset __asm__("$17");
    /* ASM_BARRIER: Keep independently initialized scan count and byte offset without folding their zeros. */
    __asm__("" : "=r"(i) : "0"(i) : "$18");
    entry_name = CdPathTblName;
    offset = 0;
    do {
        register u8 *page;
        s32 entry_key;
        /* PAL mapping: page + offset + 0x954 addresses CdPathTblParent[i]. */
        page = (u8 *)0x80110000U;
        /* ASM_BARRIER: Expose the PAL page without folding its table offset into a macro load. */
        __asm__("" : "=r"(page) : "0"(page));
        page += offset;
        entry_key = ((RrLibNamedEntry *)(page + 0x954))->key;
        if (entry_key == 0) break;
        if (entry_key == key) {
            if (PsyqStrcmp(name, entry_name) == 0) return i + 1;
        }
        entry_name += 0x2C;
        offset += 0x2C;
        i++;
    } while (i < 128);
    return -1;
}
