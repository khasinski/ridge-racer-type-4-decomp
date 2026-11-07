#ifndef RR_MENU_ARENAS_H
#define RR_MENU_ARENAS_H
#include "rr/menu_choice.h"
#include "rr/menu_arena_types.h"

/* Two resident pointer cells; the second is refreshed by menu resource loading. */
extern RrMenuArenas MenuArenas;
extern u8 *CdDirectoryBuffer;
void InitMenuArenas(void);
void garage_func_80116020(void);
void garage_func_80116148(void);
RR_ASSERT_SIZE(RrMenuArenas,8);
RR_ASSERT_OFFSET(RrMenuArenas,second,4);
#endif
