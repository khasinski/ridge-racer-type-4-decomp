#ifndef RR_OVERLAY662_IMAGE_FADE_H
#define RR_OVERLAY662_IMAGE_FADE_H
#include "rr/badge_rows.h"
#include "rr/trig.h"
#include "rr/textured_triangle.h"
/* Four fullword coordinate/UV arrays;two three-vertex windows share this
 * complete owner. GPU material uses the low16/8 bits of each entry. */
typedef struct {s32 x[4],y[4],u[4],v[4];} RrGarageFadeGeometry;
extern s32 ScreenFrame;
extern s16 garage_image_fade_phase;
/* Mode1 moves top/bottom vertices by sine(clampedPhase*8)/64 and emits only
 * the black backdrop. Other modes emit two textured triangles while captured
 * phase>0,then a pulsating backdrop. Initial phase clamps only above128.
 * Each helper observes a fresh context head70. The first sine supplies fresh
 * clock state to the second;the final phase is reloaded after all services.
 * The returned cursor is published once after drawing. Phase advances modulo16
 * by8 in mode1,otherwise4,or saturates128 if the reloaded signed phase>=128.
 * Scratch/context/packet alignment and capacity follow the shared emitters. */
void garage_draw_color_preview_image(s32 mode);
RR_ASSERT_SIZE(RrGarageFadeGeometry,64);
RR_ASSERT_OFFSET(RrGarageFadeGeometry,y,16);
RR_ASSERT_OFFSET(RrGarageFadeGeometry,u,32);
RR_ASSERT_OFFSET(RrGarageFadeGeometry,v,48);
#endif
