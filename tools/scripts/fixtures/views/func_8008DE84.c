/* MASPSX_FLAGS: --assembler-profile=gnu */

/* GCC_VERSION: 2.7.2 */

/* CC1_FLAGS: -fsigned-char -mhard-float */

/* legacy typing of shared globals in this part of the unit */
#define PsyqStrncmp ((int (*)(void *, const char *, int))PsyqStrncmp)

#undef PsyqStrncmp

typedef struct {
    unsigned char pos[4];
    unsigned int size;
    char name[16];
} CdlFILE;

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

extern int DS_cachefile(int);

extern int DebugPrintStub();

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
