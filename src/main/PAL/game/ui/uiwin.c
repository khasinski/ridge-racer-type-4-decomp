/* RODATA 0x800108e8 0x3c */
#include "types.h"
#include "scratchpad.h"
#include "rr/ui.h"
#include "game/ui/uitext.h"
#include "game/ui/uidraw.h"
#include "game/ui/uiwin.h"
#include "rr/bios_devices.h"
#include "rr/audio_cd_controller.h"
#include "game/gfx/drawbuf.h"

typedef struct {
    short x, y;
    unsigned short page, clut;
    unsigned char u, v, w, h;
} SpriteDesc;

typedef struct {
    signed char f00;
    signed char f01;
    signed char f02;
    unsigned char f03;
    char f04;
    unsigned char f05;
    unsigned char f06;
    unsigned char f07;
    short x;
    short y;
    short w;
    short h;
    unsigned short page;
    unsigned short clut;
    unsigned char u;
    unsigned char v;
    short pad16;
    int f18;
} MenuRow1C;

typedef struct {
    u8 b0;
    u8 b1;
    u8 b2;
    u8 b3;
    s16 s4;
    s16 s6;
    s16 s8;
    s16 sA;
    s8 c0C[8];
    u8 b14;
    u8 b15;
    u8 b16;
    u8 b17;
    s16 s18;
    s16 s1A;
    s16 s1C;
    s16 s1E;
    s16 s20;
    u8 pad22[2];
} UiElem24;

extern int AddDrawModePrim();
extern void *AddTilePrim();
extern void *AddSpriteWithDrawMode();
extern void *AddJitterSprites();
extern void *AddTileSemiTransPrim();
extern void InitRect16Row();
extern int UpdateRect16Row();
extern void LoadTimImage();
extern MenuRow1C *GlyphRows;
extern s32 rsin();

int UpdateGlyphRow(int arg0);
void InitUiRow(s32 index,s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 count,s32 g,s32 h,...);
void DrawUiRowSlotMarkers(s32 arg0, s32 arg1, s32 arg2);

int UpdateGlyphRow(int arg0) {
    SpriteDesc sp30;
    SpriteDesc sp40;
    MenuRow1C *base;
    MenuRow1C *p;
    int s7;
    unsigned int s5;
    int v0;
    void *a1;

    base = GlyphRows;
    p = (MenuRow1C *)((arg0 * 0x1C) + (int)base);
    sp40.page = sp30.page = p->page;
    sp40.clut = sp30.clut = p->clut;
    sp40.x = sp30.x = p->x;
    sp40.y = sp30.y = p->y;
    sp30.y += p->h / 2 - 10;
    sp40.u = sp30.u = p->u;
    sp40.w = sp30.w = p->w;
    sp30.v = p->v + p->h;
    sp30.h = 0x10;
    sp40.v = p->v;
    sp40.h = p->h;

    s7 = p->f02;
    s5 = p->f03;

    switch (s7) {
    case -2:
        GlyphRows[arg0].f03 = 0;
        GlyphRows[arg0].f02 = -4;
        GlyphRows[arg0].f00 = 0;
        break;
    case -3:
        break;
    case -4:
        InitRect16Row(GlyphRows[arg0].f07, GlyphRows[arg0].x, GlyphRows[arg0].y, GlyphRows[arg0].w, GlyphRows[arg0].h, 0, 0);
        GlyphRows[arg0].f02 = -5;
        break;
    case -5:
        if (UpdateRect16Row(GlyphRows[arg0].f07) != 0) {
            GlyphRows[arg0].f02 = -1;
        }
        break;
    case -6:
        GlyphRows[arg0].f03 = 0;
        GlyphRows[arg0].f02 = -7;
    case -7:
        UpdateRect16Row(GlyphRows[arg0].f07);
        if (GlyphRows[arg0].f03 >= 0x14) {
            GlyphRows[arg0].f02 = -1;
            GlyphRows[arg0].f03 = 0;
        } else {
            GlyphRows[arg0].f03++;
        }
        break;
    case -1:
        a1 = *(void **)SCRATCHPAD;
        v0 = ((unsigned short)s5 * 0x7C) / 10;
        *(void **)SCRATCHPAD = (void *)AddDrawModePrim(
            CurrentDrawBuf + 0x74,
            AddTileSemiTransPrim(CurrentDrawBuf + 0x74, a1, GlyphRows[arg0].x, GlyphRows[arg0].y, GlyphRows[arg0].w, GlyphRows[arg0].h, v0, v0, v0),
            0x45
        );
        if ((unsigned short)s5 >= 10) {
            GlyphRows[arg0].f02 = 0;
            GlyphRows[arg0].f03 = 0;
        } else {
            GlyphRows[arg0].f03++;
        }
        UpdateRect16Row(GlyphRows[arg0].f07);
        break;
    case 0:
        a1 = *(void **)SCRATCHPAD;
        if ((unsigned short)s5 >= 3) {
            a1 = AddJitterSprites(CurrentDrawBuf + 0x70, a1, &sp30, 1, 0xFF, 3);
        }
        *(void **)SCRATCHPAD = AddTilePrim(
            CurrentDrawBuf + 0x74,
            a1,
            GlyphRows[arg0].x,
            GlyphRows[arg0].y,
            GlyphRows[arg0].w,
            GlyphRows[arg0].h,
            8, 8, 8
        );
        if ((unsigned short)s5 >= 0x15) {
            GlyphRows[arg0].f02 = 1;
            GlyphRows[arg0].f03 = 0;
        } else {
            GlyphRows[arg0].f03++;
        }
        if (s5 == 1) {
            LoadTimImage(GlyphRows[arg0].f18 + ((GlyphRows[arg0].f06 * GlyphRows[arg0].f00) << 11));
        }
        break;
    case 1:
        a1 = AddJitterSprites(CurrentDrawBuf + 0x70, *(void **)SCRATCHPAD, &sp30, 1, (0x80 - s5) * 2, 2);
        a1 = AddJitterSprites(CurrentDrawBuf + 0x70, a1, &sp40, 1, s5, 1);
        *(void **)SCRATCHPAD = AddTilePrim(
            CurrentDrawBuf + 0x7C,
            a1,
            GlyphRows[arg0].x,
            GlyphRows[arg0].y,
            GlyphRows[arg0].w,
            GlyphRows[arg0].h,
            8, 8, 8
        );
        if (s5 >= 0x80) {
            GlyphRows[arg0].f03 = 0;
            GlyphRows[arg0].f02 = 2;
        } else {
            GlyphRows[arg0].f03 += 8;
        }
        break;
    case 2:
    default:
        a1 = *(void **)SCRATCHPAD;
        if (GlyphRows[arg0].f00 >= 0) {
            a1 = AddSpriteWithDrawMode(
                CurrentDrawBuf + 0x70,
                a1,
                GlyphRows[arg0].x,
                GlyphRows[arg0].y,
                GlyphRows[arg0].w,
                GlyphRows[arg0].h,
                GlyphRows[arg0].u,
                GlyphRows[arg0].v,
                GlyphRows[arg0].page,
                GlyphRows[arg0].clut,
                0,
                0x80
            );
        }
        *(void **)SCRATCHPAD = AddTilePrim(
            CurrentDrawBuf + 0x74,
            a1,
            GlyphRows[arg0].x,
            GlyphRows[arg0].y,
            GlyphRows[arg0].w,
            GlyphRows[arg0].h,
            8, 8, 8
        );
        if (s5 >= GlyphRows[arg0].f05 && GlyphRows[arg0].f01 >= 2) {
            GlyphRows[arg0].f03 = 0;
            GlyphRows[arg0].f02 = 0;
            GlyphRows[arg0].f00 = ++GlyphRows[arg0].f00 % GlyphRows[arg0].f01;
        } else {
            GlyphRows[arg0].f03++;
        }
        break;
    }

    return s7 >= 0;
}

/* legacy typing of shared globals in this part of the unit */
#define UiRows (*(int (*))&UiRows)

int AllocUiRows(int a) {
    int sz = UiRowCount * 36;
    UiRows = a;
    PsyqBzero(a, sz);
    DebugPrintStub(SizeLdFmt, sz);
    return sz;
}

#undef UiRows

void InitUiRow(s32 index,s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 count,s32 g,s32 h,...) {
 register s32 i;
 u32 offset;
 register RrUiRow24 *tail;
 /* ASM_BARRIER: Keep the real PAL pointer page in its shared address register. */
 register u8 *page __asm__("$9");
 if(index<UiRowCount) {
  /* Pointer storage is re-read after byte/halfword stores that can alias it. */
  page=(u8*)0x800F0000U;
  /* Preserve PAL offset wrapping while widening the signed offset for host pointers. */
  offset=(u32)index*sizeof(RrUiRow24);
  ((RrUiRow24 *)((unsigned long)(long)(s32)offset+(unsigned long)*(RrUiRow24 *volatile *)(page+0x4D80)))->byte15=g;
  ((RrUiRow24 *)((unsigned long)(long)(s32)offset+(unsigned long)*(RrUiRow24 *volatile *)(page+0x4D80)))->byte16=h;
  ((RrUiRow24 *)((unsigned long)(long)(s32)offset+(unsigned long)*(RrUiRow24 *volatile *)(page+0x4D80)))->byte01=f;
  ((RrUiRow24 *)((unsigned long)(long)(s32)offset+(unsigned long)*(RrUiRow24 *volatile *)(page+0x4D80)))->byte14=e;
  {
   RrUiRow24 *row=(RrUiRow24 *)((unsigned long)(long)(s32)offset+(unsigned long)*(RrUiRow24 *volatile *)(page+0x4D80));
   row->byte02=count;
   /* Capture this fresh base before the halfword stores, as in the original. */
   tail=*(RrUiRow24 *volatile *)(page+0x4D80);

   row->half04=a;row->half06=b;row->half08=c;row->half0A=d;
  }
  ((RrUiRow24 *)((unsigned long)(long)(s32)offset+(unsigned long)tail))->byte17=0;
  i=0;
  ((RrUiRow24 *)((unsigned long)(long)(s32)offset+(unsigned long)*(RrUiRow24 *volatile *)(page+0x4D80)))->half20=0;
  if(count>0) {
   register u8 *loop_page=page;
   register u32 clear_offset;
   clear_offset=offset;
   do {
    *(u8 *)((unsigned long)(long)(s32)clear_offset+(unsigned long)*(RrUiRow24 *volatile *)(loop_page+0x4D80)+i+RR_FIELD_OFFSET(RrUiRow24,cells))=0;
    i++;
   }while(i<count);
  }
 }
}

/* legacy typing of shared globals in this part of the unit */
#define UiRows ((UiElem24 *)UiRows)
#define AddTilePrim ((s32 (*)())AddTilePrim)
#define AddSpriteWithDrawMode ((s32 (*)())AddSpriteWithDrawMode)

void DrawUiRowSlotMarkers(s32 arg0, s32 arg1, s32 arg2) {
    s32 pad[8];
    s32 s0;
    s32 s1;
    s32 s3;
    s32 s5;
    /* ASM_BARRIER: Allocate s6 to $s6 so s5 gets $s5 in global-alloc */
    register s32 s6 asm("s6");
    s32 a0;
    s32 v;
    s32 t;
    s32 b;
    s32 d;
    s32 c;
    UiElem24 *p;

    if (UiRows[arg0].b2 == 0) {
        *(s32 *)SCRATCHPAD = AddTilePrim(CurrentDrawBuf + 0x70, *(s32 *)SCRATCHPAD, arg1, arg2, 1, UiRows[arg0].sA, 0xFF, 0xFF, 0xFF);
    } else {
        s1 = 0;
        s3 = *(s32 *)SCRATCHPAD;
        s5 = ((UiRows[arg0].b17 - 0x80) << 4) & 0xFFF;
        if ((b = UiRows[arg0].b2) != 0) {
            do {
                switch (UiRows[arg0].c0C[s1]) {
                default:
                case 0:
                    a0 = 0xC00;
                    s0 = 0;
                    break;
                case 2:
                    a0 = s5;
                    s0 = 0;
                    break;
                case 1:
                    a0 = 0xC00;
                    s0 = 3;
                    break;
                case 3:
                case 4:
                    a0 = s5;
                    s0 = 3;
                    break;
                }
                s6 = arg1 + s1 * 10 + 4;
                c = 0x1E;
                v = rsin(a0);
                t = s0 + 1;
                s0 = ((t + v / 0x801) << 3) + 0xD0;
                s3 = AddSpriteWithDrawMode(CurrentDrawBuf + 0x70, s3, s6, arg2 + 8, 8, 0x10, s0, 0x10, c, 0x7F3A, 0, 0x80);
                s1++;
            } while (s1 < UiRows[arg0].b2);
        }
        UiRows[arg0].b17 += 8;
        p = UiRows;
        /* ASM_BARRIER: Prevent sched1/sched2 from hoisting reloads above b17 += 8 store */
        __asm__ volatile("");
        d = arg1 - 12;
        b = p[arg0].b17;
        /* ASM_BARRIER: Keep unused b17 load live and reload UiRows without a branch basic-block boundary */
        __asm__("" : "=m"(UiRows) : "r"(b), "r"(d));
        *(s32 *)SCRATCHPAD = s3;
        DrawUiBox(arg1, arg2, s6 - d, UiRows[arg0].sA, 0xCE, 0xCE, 0xCE);
    }
}

#undef UiRows
#undef AddTilePrim
#undef AddSpriteWithDrawMode
