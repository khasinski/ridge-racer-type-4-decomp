#include "types.h"

#define ADDPRIM(ot, p) \
    (p)->addr = ((SPRT *)(ot))->addr; \
    ((SPRT *)(ot))->addr = (u32)(p)

#define SCRATCH ((Scratch *)0x1F800000)

/* The two original entries differ only in the sprite's vertical position. */
#define RR_ICON_SPRITE_BODY(Y) \
    RrSpritePacket *p; \
    RrGpuScratchPointers *scratch; \
    RrIconSpriteContext *context; \
    u32 low=0xFFFFFFU; \
    /* ASM_BARRIER: reuse the consumed color argument for the tag length mask. */ \
    register u32 high __asm__("$5"); \
    u32 u; \
    u16 clut; \
    u32 offset; \
    /* ASM_BARRIER: retain the sprite length until its first byte store. */ \
    register s32 length __asm__("$2"); \
    u32 command; \
    /* ASM_BARRIER: retain the context-slot page across both packet insertions. */ \
    register u8 *context_page __asm__("$8"); \
    /* ASM_BARRIER: finish the low-address mask before forming packet arguments. */ \
    __asm__("" : "+r"(low), "+r"(index)); \
    command=0xE1000000U; \
    /* ASM_BARRIER: stage the command page before the scratch base and length. */ \
    __asm__("" : "+r"(command) : : "$9", "$2"); \
    scratch=(RrGpuScratchPointers*)0x1F800000U; \
    length=4; \
    /* ASM_BARRIER: prepare the scratch capture and length before indexing. */ \
    __asm__("" : "+r"(scratch), "+r"(length)); \
    offset=(u32)index*2U; \
    u=(offset+(u32)index)*8U+112U; \
    p=(RrSpritePacket*)scratch->cursor; \
    /* ASM_BARRIER: finish UV/cursor capture before the context-slot page. */ \
    __asm__("" : "+r"(u), "+r"(p)); \
    context_page=(u8*)0x800B0000U; \
    /* ASM_BARRIER: materialize the context-slot page before packet fields. */ \
    __asm__("" : "+r"(context_page)); \
    ((u8*)p)[3]=length; \
    p->code=0x66; \
    p->x=148; \
    p->y=Y; \
    p->w=24; \
    p->h=16; \
    clut=*(u16*)((u8*)D_8001121C+offset); \
    p->r=color; \
    p->g=color; \
    p->b=color; \
    p->u=u; \
    p->v=172; \
    context=*(RrIconSpriteContext**)(context_page-0x33EC); \
    high=0xFF000000U; \
    p->clut=clut; \
    *(u32*)p=(*(u32*)p&high)|(context->head70&low); \
    context->head70=(context->head70&high)|((u32)(unsigned long)p&low); \
    p++; \
    scratch->cursor=(u8*)p; \
    ((u8*)p)[3]=1; \
    context=*(RrIconSpriteContext**)(context_page-0x33EC); \
    command|=0x23EU; \
    *((u32*)p+1)=command; \
    *(u32*)p=(*(u32*)p&high)|(context->head70&low); \
    context->head70=(context->head70&high)|((u32)(unsigned long)p&low); \
    scratch->cursor=(u8*)p+sizeof(RrGpuSingleCommandPacket);

/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf ((u8 *)CurrentDrawBuf)

#undef CurrentDrawBuf

/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf ((u8 *)CurrentDrawBuf)

#undef CurrentDrawBuf

/* legacy typing of shared globals in this part of the unit */
#define CdAudioSelectTrack ((int (*)())CdAudioSelectTrack)

#undef CdAudioSelectTrack

/* legacy typing of shared globals in this part of the unit */
#define HudGlyphUvRects (*(char (*)[])&HudGlyphUvRects)

#undef HudGlyphUvRects

/* legacy typing of shared globals in this part of the unit */
#define HudGlyphUvRectsBank0 (*(u8 (*)[])&HudGlyphUvRectsBank0)

#define HudGlyphUvRects (*(u8 (*)[])&HudGlyphUvRects)

#undef HudGlyphUvRectsBank0

#undef HudGlyphUvRects

/* legacy typing of shared globals in this part of the unit */
#define HudGlyphUvRectsBank0 (*(char (*)[])&HudGlyphUvRectsBank0)

#define HudGlyphUvRects (*(char (*)[])&HudGlyphUvRects)

#define CurrentDrawBuf ((char *)CurrentDrawBuf)

#undef HudGlyphUvRectsBank0

#undef HudGlyphUvRects

#undef CurrentDrawBuf

/* legacy typing of shared globals in this part of the unit */
#define CurrentDrawBuf ((char *)CurrentDrawBuf)

#undef CurrentDrawBuf

extern s16 CourseIndex;

extern void CdAudioSelectTrack();

extern int PsyqRand();

extern short RaceType;

extern int CdAudioInit();

extern signed char SelectionState[];

extern int BgmNumberToCdTrack[];

extern int AllProgressComplete;

extern int RaceBgmListIndex;

extern int D_800F7660;

void SelectCourseBgmTrack() {
    int a0, a1, v;
    CdAudioInit();
    if (RaceType == 1) {
        a0 = SelectionState[SelectionState[0x17] + 8] < 3;
    } else {
        a0 = 1;
    }
    if (AllProgressComplete != 0 && RaceType != 1) {
        D_800F7660 = 0x10;
    } else {
        D_800F7660 = 0xF;
    }
    if (a0 == 0) {
        a1 = CourseIndex + 3;
        CdAudioSelectTrack(0, a1);
    } else if (RaceBgmListIndex == 0) {
        a1 = PsyqRand() % 14 + 0xB;
        CdAudioSelectTrack(0, a1);
    } else {
        a1 = BgmNumberToCdTrack[RaceBgmListIndex - 1];
        CdAudioSelectTrack(0, a1);
    }
}
