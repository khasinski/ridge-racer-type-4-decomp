#ifndef RR_GAME_CAMERA_CAMPATH_H
#define RR_GAME_CAMERA_CAMPATH_H

#include "rr/presentation_types.h"

extern MotionKey32 MotionTrackTable[];
extern CameraScriptEntry8 HudCircularGaugeVerts[];
extern CameraScriptEntry8 *MotionSequenceTable[];
void DrawPresentationTrackModel(void);
void InitMotionTrack(int a0);
/* Disabled/unknown states do not read the clock or key. Starting requires an
 * active clock and a nonzero wrapped unsigned duration. Interpolation excludes
 * INT_MIN/-1; positions and elapsed wrap16, word calculations wrap32.
 * Clock, readonly key and mutable controller are disjoint. The first vector
 * copy includes its reserved half; completion writes only x/y/z. */
void UpdateMotionTrack(MotionClock20 *clock);
void InitMotionController(void);
void RestartMotionSequence(void);
void SetPresentationDrawEnabled(s32 enabled);
void SetPresentationFullViewport(s32 fullViewport);
void SetPresentationStencilEnabled(s32 stencilEnabled);

#endif /* RR_GAME_CAMERA_CAMPATH_H */
