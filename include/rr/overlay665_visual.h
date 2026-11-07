#ifndef RR_OVERLAY665_VISUAL_H
#define RR_OVERLAY665_VISUAL_H
#include "rr/layout.h"
#include "rr/trig.h"
#include "rr/line_emit.h"
#include "rr/projection_state.h"
#include "rr/menu_transition.h"
#include "rr/menu_setup.h"
#include "rr/frame_environment.h"
#include "rr/bios_devices.h"
#include "rr/overlay665_matrix.h"
#include "rr/overlay665_visual_types.h"
extern RrOverlay665VisualState ovl665_D_80119768;
extern u8 ovl665_D_8011973C[4];
extern s32 DialogueContext;
void ovl665_func_80114918(void);
/* Signed32 angles are normalized by the actual trig helpers. Each result lies
 * in[-4096,4096], so multiplication by40 and negation are signed32-safe.
 * Output coordinates retain their low16 bits, including wrapped centers. */
void ovl665_func_801169BC(s32 angle);
RR_ASSERT_SIZE(RrOverlay665VisualState,0xA0);
RR_ASSERT_OFFSET(RrOverlay665VisualState,centerX38,0x38);
RR_ASSERT_OFFSET(RrOverlay665VisualState,centerY3A,0x3A);
RR_ASSERT_OFFSET(RrOverlay665VisualState,lighting4C,0x4C);
RR_ASSERT_OFFSET(RrOverlay665VisualState,view74,0x74);
RR_ASSERT_SIZE(RrPreviewView44,0x2C);
RR_ASSERT_OFFSET(RrPreviewView44,rotation18,0x18);
#endif
