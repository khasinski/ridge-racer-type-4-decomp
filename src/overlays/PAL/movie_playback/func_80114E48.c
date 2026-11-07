#include "rr/overlay668_display.h"
void movie_playback_func_80114E48(u8 *base){
 ((RrStrBufferSet*)RaceContext.activeB8)->buffers[0]=base;
 base+=0xa000;
 ((RrStrBufferSet*)RaceContext.activeB8)->buffers[1]=base;
 base+=0xa000;
 ((RrStrBufferSet*)RaceContext.activeB8)->buffers[2]=base;
 base+=0x2d00;
 ((RrStrBufferSet*)RaceContext.activeB8)->buffers[3]=base;
 base+=0x2d00;
 ((RrStrBufferSet*)RaceContext.activeB8)->buffers[4]=base;
}
