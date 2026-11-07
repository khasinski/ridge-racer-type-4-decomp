#ifndef RR_OVERLAY662_HISTORY_LABELS_H
#define RR_OVERLAY662_HISTORY_LABELS_H
#include "rr/menu_label_render.h"
#include "rr/save_progress.h"
#include "rr/menu_choice.h"
extern RrUiUvRect UiFontGlyphs[];
extern s32 ScreenFrame;
extern s8 garage_D_8012621C[7];
extern u16 garage_D_801272D8[4],garage_D_801272FC;
extern s8 garage_D_80126218[3],garage_D_80126224[3];
extern s8 garage_D_80126228[3],garage_D_8012622C[4];
extern s16 garage_D_80127328[4][4],garage_D_801272BC,garage_D_801272BE;
s32 rsin(s32 angle);
/* Row index and nonnegative glyph keys name accessible table entries. Numeric
 * values fit two decimal digits. Services return full native arena pointers.
 * Ordering and cursor remain pointers throughout; each helper may change globals.
 * History matching captures the pulse flag before rendering the labels. */
void garage_func_80117654(u32 *,void *,s32);
/* Choice byte0 remains0..3 across services; live counts fit0..80/0..320.
 * Seven label keys are nonnegative UV indices or sentinels-1/-2. */
void garage_func_80117854(u32 *,void *,s32);
void garage_func_80117AA4(u32 *,void *,s32);
void garage_func_80117C98(u32 *,void *,s32);
void garage_func_80117DFC(u32 *,void *,s32);
#endif
