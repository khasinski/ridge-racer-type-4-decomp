#ifndef RR_OVERLAY662_PALETTE_EDIT_H
#define RR_OVERLAY662_PALETTE_EDIT_H
#include "rr/overlay662_image_drag.h"
#include "rr/palette16.h"
/* Palette pointers address words[0] of a complete32-byte RrPalette16 owner.
 * active21 is0/1 and config22 is0..15. Active horizontal clips give x149..212
 * (component0..31). Backup copies both512-word images and8 palette words. */
extern s32 garage_D_80127780,garage_D_80127788,garage_D_80127790;
extern u16 garage_D_80127784,garage_D_8012778C,garage_D_80127794;
void garage_func_80124258(void);
void garage_func_801243DC(void);
void garage_func_80124560(void);
/* Step buttons capture the selected palette entry before input handling.
 * Timer is published before sound. After sound, update the captured entry
 * using its fresh contents and the component captured before backup/sound. */
void garage_func_8012349C(void);
void garage_func_801235F4(void);
void garage_func_80123750(void);
void garage_func_801238AC(void);
void garage_func_80123A0C(void);
void garage_func_80123B68(void);
#endif
