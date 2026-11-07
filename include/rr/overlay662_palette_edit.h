#ifndef RR_OVERLAY662_PALETTE_EDIT_H
#define RR_OVERLAY662_PALETTE_EDIT_H
#include "rr/overlay662_image_drag.h"
#include "rr/palette16.h"
/* Palette pointers address words[0] of a complete32-byte RrPalette16 owner.
 * active21 is0/1 and config22 is0..15. Active horizontal clips give x149..212
 * (component0..31). Backup copies both512-word images and8 palette words. */
extern s32 garage_red_slider_backed_up,garage_green_slider_backed_up,garage_blue_slider_backed_up;
extern u16 garage_red_slider_start,garage_green_slider_start,garage_blue_slider_start;
void garage_palette_red_slider(void);
void garage_palette_green_slider(void);
void garage_palette_blue_slider(void);
/* Step buttons capture the selected palette entry before input handling.
 * Timer is published before sound. After sound, update the captured entry
 * using its fresh contents and the component captured before backup/sound. */
void garage_palette_red_down(void);
void garage_palette_red_up(void);
void garage_palette_green_down(void);
void garage_palette_green_up(void);
void garage_palette_blue_down(void);
void garage_palette_blue_up(void);
#endif
