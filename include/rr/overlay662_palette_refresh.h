#ifndef RR_OVERLAY662_PALETTE_REFRESH_H
#define RR_OVERLAY662_PALETTE_REFRESH_H
#include "rr/overlay662_garage.h"
#include "rr/palette16.h"
#include "rr/gpu.h"
#include "rr/trig.h"
extern s32 garage_clut_pulse_phase;
extern RrGpuRectangleFields garage_preview_image_vram_rect,garage_layer0_image_vram_rect,garage_layer1_image_vram_rect,garage_logo_clut_upload_rect,garage_pulse_clut_upload_rect;
void LoadImage(const RrGpuRectangleFields *rectangle,const void *pixels);
/* active21 stays0..1. Each palette pointer addresses words[0] of a complete
 * RrPalette16 owner; owners may coincide. Captured owners stay valid through
 * all callbacks. Sine returns -4096..4096. Callbacks may change the counter,
 * palette contents and garage roots; each following phase/root is read fresh.
 * The counter and phase offsets wrap as 32-bit words. GPU upload is a service. */
void garage_upload_editor_vram(void);
#endif
