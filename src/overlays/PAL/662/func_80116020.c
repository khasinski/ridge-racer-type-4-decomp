#include "rr/overlay662_setup.h"

void ovl662_func_80116020(void)
{
    s32 i;
    ModeFrame=0;
    ScreenFrame=0;
    ovl662_D_801272D4=0;
    ovl662_D_801272C0=0;
    SelectedTeamIndex=2;
    SelectionRows.bytes[0]=2;
    CourseListIndex=0;
    for(i=0;i<4;i++)InitGlyphRow(i,-2,0,1);
    InitRect16Row(GlyphRows->glyph07,GlyphRows->x08,GlyphRows->y0A,GlyphRows->w0C,GlyphRows->h0E,0,0);
    InitUiRow(0,12,72,240,32,0,0,4,2,SelectedTeamIndex);
    InitBadgeRow(0,36,336,248,34,0,0x301);
    BadgeRows->half02=1;
}
