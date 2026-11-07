#include "rr/overlay668_display.h"
void movie_playback_func_8011535C(RrStrDisplayConfig *config,s32 mode)
{
    u32 ready=config->ready0C;
    volatile u32 counter=0x800000;
    if(ready==0) do {
        if (--counter==0) {
            config->ready0C=1;
            config->alternate04=!config->alternate04;
            config->rectangles[2].x=config->rectangles[config->alternate04].x;
            config->rectangles[2].y=config->rectangles[config->alternate04].y;
        }
    } while(config->ready0C==0);
    config->ready0C=0;
}
