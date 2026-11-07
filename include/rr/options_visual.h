#ifndef RR_OPTIONS_VISUAL_H
#define RR_OPTIONS_VISUAL_H
#include "rr/layout.h"
#include "rr/trig.h"
#include "rr/line_emit.h"
#include "rr/projection_state.h"
#include "rr/menu_transition.h"
#include "rr/menu_setup.h"
#include "rr/frame_environment.h"
#include "rr/bios_devices.h"
#include "rr/options_matrix.h"
#include "rr/options_visual_types.h"
extern RrOptionsVisualState options_menu_state;
extern u8 options_marker_line_color[4];
extern s32 DialogueContext;
void options_init_menu_state(void);
/* Signed32 angles are normalized by the actual trig helpers. Each result lies
 * in[-4096,4096], so multiplication by40 and negation are signed32-safe.
 * Output coordinates retain their low16 bits, including wrapped centers. */
void options_draw_angle_marker(s32 angle);
RR_ASSERT_SIZE(RrOptionsVisualState,0xA0);
RR_ASSERT_OFFSET(RrOptionsVisualState,centerX38,0x38);
RR_ASSERT_OFFSET(RrOptionsVisualState,centerY3A,0x3A);
RR_ASSERT_OFFSET(RrOptionsVisualState,lighting4C,0x4C);
RR_ASSERT_OFFSET(RrOptionsVisualState,view74,0x74);
RR_ASSERT_SIZE(RrPreviewView44,0x2C);
RR_ASSERT_OFFSET(RrPreviewView44,rotation18,0x18);
#endif
