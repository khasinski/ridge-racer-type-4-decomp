#ifndef RR_OVERLAY665_CALIBRATION_H
#define RR_OVERLAY665_CALIBRATION_H
#include "rr/overlay665_panels.h"
#include "rr/frame_storage_types.h"
#include "rr/diagnostics.h"
extern RrFrameStorage FrameEnvs[2];
extern RrUiUvRect ovl665_D_80119018[2],ovl665_D_80119020[2];
extern const char ovl665_D_801148B4[];
void ovl665_func_80115798(void);
/* Two complete frame-storage owners and both two-entry glyph tables exist.
 * The selected frame has a full ordering context. Each threaded emitter cursor
 * and the final8-byte command need aligned writable packet backing; callbacks
 * may change roots,settings,scratch cursor and returned cursor independently.
 * Software pointers retain native width; only DMA links serialize low24 bits.
 * Display adjustments wrap32 before clamping,and countdown wraps signed16. */
void ovl665_func_80115820(void);
#endif
