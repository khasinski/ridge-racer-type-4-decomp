#ifndef RR_OPTIONS_CALIBRATION_H
#define RR_OPTIONS_CALIBRATION_H
#include "rr/options_panels.h"
#include "rr/frame_storage_types.h"
#include "rr/diagnostics.h"
extern RrFrameStorage FrameEnvs[2];
extern RrUiUvRect options_vertical_arrow_uvs[2],options_left_arrow_uv[2];
extern const char options_screen_offset_fmt[];
void options_init_screen_adjust(void);
/* Two complete frame-storage owners and both two-entry glyph tables exist.
 * The selected frame has a full ordering context. Each threaded emitter cursor
 * and the final8-byte command need aligned writable packet backing; callbacks
 * may change roots,settings,scratch cursor and returned cursor independently.
 * Software pointers retain native width; only DMA links serialize low24 bits.
 * Display adjustments wrap32 before clamping,and countdown wraps signed16. */
void options_update_screen_adjust(void);
#endif
