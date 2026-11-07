#ifndef RR_OVERLAY665_STROKES_H
#define RR_OVERLAY665_STROKES_H
#include "rr/layout.h"
#include "rr/line_emit.h"

typedef struct {s16 direction,length;} RrStrokeStep4;
typedef struct {u8 bytes[4];} RrStrokeColor4;
extern RrStrokeColor4 options_button_guide_line_color;
extern u16 options_button_guide_origins[5][2];

/* Five readable path slots; each selected path terminates at direction -1.
 * Other directions are 0..3. Paths remain live during rendering. Arithmetic
 * wraps as MIPS words; coordinate storage wraps to sixteen bits. */
void options_draw_button_guides(const RrStrokeStep4 *const *paths,s32 factor);
RR_ASSERT_SIZE(RrStrokeStep4,4);
RR_ASSERT_SIZE(RrStrokeColor4,4);
RR_ASSERT_OFFSET(RrStrokeStep4,length,2);
#endif
