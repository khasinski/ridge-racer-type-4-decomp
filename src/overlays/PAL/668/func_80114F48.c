#include "rr/overlay668_stream.h"
#define STR_STATE ((RrStrPlaybackState *)RaceContext.activeB8)

void ovl668_func_80114F48(void) {
    RrGpuRectangleFields sp10;
    u32 prev2C;

    if (StCdIntrFlag != 0) {
        StCdInterrupt();
        StCdIntrFlag = 0;
    }
    prev2C = STR_STATE->display24.word08;
    sp10 = STR_STATE->display24.rectangles[2];
    STR_STATE->display24.word08 = (STR_STATE->display24.word08 == 0);
    STR_STATE->display24.rectangles[2].x += STR_STATE->display24.rectangles[2].w;
    if (STR_STATE->display24.rectangles[2].x < STR_STATE->display24.rectangles[STR_STATE->display24.alternate04].x + STR_STATE->display24.rectangles[STR_STATE->display24.alternate04].w) {
        ovl668_func_80116478(
            STR_STATE->buffers18[STR_STATE->display24.word08],
            (STR_STATE->display24.rectangles[2].w * STR_STATE->display24.rectangles[2].h) / 2
        );
    } else {
        STR_STATE->display24.ready0C = 1;
        STR_STATE->display24.alternate04 = (STR_STATE->display24.alternate04 == 0);
        STR_STATE->display24.rectangles[2].x = STR_STATE->display24.rectangles[STR_STATE->display24.alternate04].x;
        STR_STATE->display24.rectangles[2].y = STR_STATE->display24.rectangles[STR_STATE->display24.alternate04].y;
    }
    LoadImage(&sp10, StrStreamState->buffers18[prev2C]);
}
