#include "rr/overlay662_setup.h"

void garage_init_maker_select(s32 arg0) {
    s32 i;
    garage_D_801272C0 = 0;
    CourseListIndex = 0;
    CourseListSavedIndex = 0;
    for (i = 0; i < 4; i++) {
        /* Unsigned addition wraps; the PAL signed-word conversion keeps its bits. */
        GlyphRows[arg0 + i].phase18 = (s32)((u32)GlyphRows[i].phase18 + 0x30000U);
        InitGlyphRow(arg0 + i, -2, 0, 1);
        InitRect16Row(
            GlyphRows[arg0 + i].glyph07,
            GlyphRows[arg0 + i].x08,
            GlyphRows[arg0 + i].y0A,
            GlyphRows[arg0 + i].w0C,
            GlyphRows[arg0 + i].h0E,
            0, 0
        );
    }
    InitBadgeRow(0, 108, 336, 112, 34, 0, 0x401);
    InitUiRow(0, 40, 72, 192, 32, 0, 0, 4, 3, CourseListIndex + 4);
    BadgeRows[0].half02 = 1;
}
