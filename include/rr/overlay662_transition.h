#ifndef RR_OVERLAY662_TRANSITION_H
#define RR_OVERLAY662_TRANSITION_H
#include "rr/overlay662_garage.h"
#include "rr/overlay662_render_step.h"
#include "rr/menu_transition.h"
#include "rr/menu_background.h"
#include "rr/menu_arenas.h"
#include "rr/menu_glyph.h"
extern s16 CourseChoiceGroup1Glyph;
extern u8 *ImageRows;
void garage_func_80119B44(void);
/* Render first, then increment the fresh word04 modulo2^32. A signed result
 * of28 or more publishes transition3/5/0. */
void garage_func_8011AA84(void);
/* Initialize the view, publish transition1/70/0, then run the final helper. */
void garage_func_80119DB4(void);
/* Requires a two-byte writable D_F2AB0 backing. Uses the first full arena
 * pointer, encodes three glyph bytes, then sets byte1 of the mode backing. */
void garage_func_80115474(void);
#endif
