#ifndef RR_GARAGE_SETUP_H
#define RR_GARAGE_SETUP_H
#include "rr/menu_setup.h"
extern s32 garage_select_arrows_unused_arg;
extern s16 garage_color_preview_state;
/* Four glyph setup calls may replace the glyph root. The rectangle helper
 * receives the then-current first glyph. The final badge root is read after
 * all layout helpers return. */
void garage_init_team_select(void);
/* base is nonnegative and base+3 fits signed32 (observed caller uses0).
 * Each published glyph bank covers rows0..3 and base..base+3. Copy phases
 * forward modulo2^32; source/destination rows may overlap. Reload the root
 * after each helper, and publish the first badge flag after the final layout. */
void garage_init_maker_select(s32 base);
#endif
