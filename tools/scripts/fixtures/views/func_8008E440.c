/* MASPSX_FLAGS: --assembler-profile=gnu */

/* GCC_VERSION: 2.7.2 */

/* CC1_FLAGS: -fsigned-char -mhard-float */

#include "rr/library.h"

/* legacy typing of shared globals in this part of the unit */
#define PsyqStrncmp ((int (*)(void *, const char *, int))PsyqStrncmp)

#undef PsyqStrncmp

extern char CdPathTblName[];

extern s32 PsyqStrcmp(const char *, const char *);

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
