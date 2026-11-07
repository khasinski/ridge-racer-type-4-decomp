#ifndef RR_UI_PANEL_H
#define RR_UI_PANEL_H

#include "rr/badge_rows.h"
#include "rr/sprite_emit.h"

/* The panel uses the second ordering head after the shared context prefix. */
typedef struct {RrIconSpriteContext prefix;u32 head74;} RrPanelContext;
typedef void *(*RrPanelDraw)(u32 *,void *,s32);
extern RrPanelDraw PanelDrawCallbacks[];

/* The selected row and signed high-byte callback index must name accessible
 * entries; index*row size must fit signed32. FFFF status emits nothing.
 * Captures layout and the ordering head before helpers, then reloads the row
 * table for its lower edge and final viewport. All helper returns are cursors. */
void DrawBadgeRow(s32 index);
RR_ASSERT_SIZE(RrPanelContext, 0x78);
RR_ASSERT_OFFSET(RrPanelContext, head74, 0x74);

#endif
