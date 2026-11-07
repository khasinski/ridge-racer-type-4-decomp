#ifndef RR_VERSUS_SELECT_INIT_H
#define RR_VERSUS_SELECT_INIT_H
#include "rr/menu_arenas.h"
#include "rr/menu_setup.h"
#include "rr/overlay_menu.h"
#include "rr/menu_selection.h"
#include "rr/projection_state.h"
/* BIOS random service; this initializer consumes only its low two bits. */
s32 PsyqRand(void);
void versus_select_init_course_select(void);
#endif
