#ifndef RR_MOVIE_PLAYBACK_STREAM_H
#define RR_MOVIE_PLAYBACK_STREAM_H
#include "rr/movie_playback_display.h"
#include "rr/movie_playback_control.h"
#include "psyq/libcd/dsstrm32.h"
/* The shared live allocation, including the display configuration at0x24. */
typedef struct {
    s32 width00,height04;
    u8 reserved08[8];
    RrStrCommandHeader *commands10[2];
    void *buffers18[2];
    u32 commandIndex20;
    RrStrDisplayConfig display24;
} RrStrPlaybackState;
extern s32 StCdIntrFlag,VSyncThreshold,ModeFrame;
extern u8 OptionsMusicVolume;
void SetDispMask(s32);
void SsSetSerialAttr(s8,s8,s8);
void SsSetSerialVol(s8,s32,s32);
s32 VSync(s32);
s32 movie_playback_next_vlc(RrStrDisplayConfig *);
void movie_playback_kick_cd(u8 *location);
void movie_playback_start(u8 *resource);
void LoadImage(const RrGpuRectangleFields *rectangle,const void *pixels);
void movie_playback_dct_out_callback(void);
/* Both selectors are0..1. The full playback state remains live across callbacks.
 * Coordinates narrow to signed16, DMA output covers width*height/2 words.
 * Captured rectangle is passed by address; its pixel buffer uses the old index. */
RR_ASSERT_SIZE(RrStrPlaybackState,0x4C);
RR_ASSERT_OFFSET(RrStrPlaybackState,display24,0x24);
RR_ASSERT_OFFSET(RrStrPlaybackState,buffers18,0x18);
#endif
