#include "rr/movie_playback_resource.h"
void movie_playback_get_movie_file_info(s32 arg0, RrStrCdFileInfo *arg1, u32 *arg2) {
    /* Existing PAL compiler frame reserves this area; it carries no data. */
    u8 reserved_stack10[8];
    /* Emit 0x0FFFFFFF mask in $3 before &SoundState and offset math */
    register u32 mask __asm__("$3") = 0x0FFFFFFF;
    /* Hold &SoundState in $2 across offset calculation */
    register RrStrArchiveAudioState *base __asm__("$2");
    /* Keep offset addition in $4 before loading archive2638 */
    register s32 offset __asm__("$4");
    RrStrArchiveEntry *entry;
    u32 v0;
    (void)reserved_stack10;
    __asm__("" : "=r"(mask) : "0"(mask));
    base = &SoundState;
    __asm__("" : "=r"(base) : "0"(base));
    offset = (s32)((u32)arg0 * 8U + 0x130U);
    __asm__("" : "=r"(offset) : "0"(offset));
    entry = (RrStrArchiveEntry *)(base->archive2638 + offset);
    CdIntToPos((s32)((entry->position & mask) + base->sector2634), arg1->location.c);
    arg1->byteSize = entry->sectors << 11;
    *arg2 = entry->sectors;
    RaceContext.locationBC = arg1->location;
    RaceContext.currentC0 = RaceContext.locationBC;
    v0 = (*arg2 / 6) - 20;
    RaceContext.thresholdC8 = v0;
    RaceContext.budgetCC = v0 * 2;
}
