#ifndef RR_GARAGE_HISTORY_LABELS_H
#define RR_GARAGE_HISTORY_LABELS_H
#include "rr/menu_label_render.h"
#include "rr/save_progress.h"
#include "rr/menu_choice.h"
extern RrUiUvRect UiFontGlyphs[];
extern s32 ScreenFrame;
extern s8 garage_owned_cars_label_glyphs[7];
extern u16 garage_team_car_counts[4],garage_total_car_count;
extern s8 garage_car_number_label_glyphs[3],garage_maker_cars_label_glyphs[3];
extern s8 garage_logo_number_label_glyphs[3],garage_car_color_label_glyphs[4];
extern s16 garage_team_maker_car_counts[4][4],garage_logo_number,garage_clut_list_index;
s32 rsin(s32 angle);
/* Row index and nonnegative glyph keys name accessible table entries. Numeric
 * values fit two decimal digits. Services return full native arena pointers.
 * Ordering and cursor remain pointers throughout; each helper may change globals.
 * History matching captures the pulse flag before rendering the labels. */
void garage_draw_car_number_label(u32 *,void *,s32);
/* Choice byte0 remains0..3 across services; live counts fit0..80/0..320.
 * Seven label keys are nonnegative UV indices or sentinels-1/-2. */
void garage_draw_team_count_label(u32 *,void *,s32);
void garage_draw_maker_count_label(u32 *,void *,s32);
void garage_draw_logo_number_label(u32 *,void *,s32);
void garage_draw_clut_index_label(u32 *,void *,s32);
#endif
