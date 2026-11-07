#include "rr/movie_playback_frame_acquire.h"
u8 *movie_playback_next_frame(RrStrDisplayConfig *config) {
 RrGpuRectangleFields rectangle;
 u8 *data;RrLibQueueRecord32 *header;
 s32 retry=2000;RrStrPlaybackState *playback;
 while(StGetNext(&data,&header)!=0) {if(--retry==0)return 0;}
 if((u32)header->value08>=RaceContext.thresholdC8)RaceContext.stopD0=1;
 if((u32)RaceContext.counterC4>=RaceContext.budgetCC)RaceContext.stopD0=1;
 if(((RrStrPlaybackState*)RaceContext.activeB8)->width00!=header->width10 || ((RrStrPlaybackState*)RaceContext.activeB8)->height04!=header->height12) {
  ((RrStrPlaybackState*)RaceContext.activeB8)->width00=header->width10;
  ((RrStrPlaybackState*)RaceContext.activeB8)->height04=header->height12;
  playback=RaceContext.activeB8;
  rectangle.x=0;rectangle.y=0;
  rectangle.w=(s32)((u32)playback->width00*3U)/2;
  rectangle.h=512;
  ClearImage(&rectangle,0,0,0);
 }
 config->rectangles[0].y=FrameEnvs[0].environment.display.y+(s32)(256U-(u32)((RrStrPlaybackState*)RaceContext.activeB8)->height04)/2;
 config->rectangles[1].y=FrameEnvs[1].environment.display.y+(s32)(256U-(u32)((RrStrPlaybackState*)RaceContext.activeB8)->height04)/2;
 config->rectangles[1].w=(s32)((u32)((RrStrPlaybackState*)RaceContext.activeB8)->width00*3U)/2;
 config->rectangles[0].w=config->rectangles[1].w;
 config->rectangles[1].h=((RrStrPlaybackState*)RaceContext.activeB8)->height04;
 config->rectangles[0].h=config->rectangles[1].h;
 config->rectangles[2].h=((RrStrPlaybackState*)RaceContext.activeB8)->height04;
 FrameEnvs[1].environment.display.w=((RrStrPlaybackState*)RaceContext.activeB8)->width00;
 FrameEnvs[0].environment.display.w=FrameEnvs[1].environment.display.w;
 FrameEnvs[1].environment.display.h=((RrStrPlaybackState*)RaceContext.activeB8)->height04;
 FrameEnvs[0].environment.display.h=FrameEnvs[1].environment.display.h;
 return data;
}
