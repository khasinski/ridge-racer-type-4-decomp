#ifndef RR_GARAGE_RENDER_STEP_H
#define RR_GARAGE_RENDER_STEP_H
/* Entry-point API only: callers do not need the renderer context data views. */
void garage_draw_animated_panels(void);
void garage_add_editor_draw_env(void);
void garage_update_editor_overlays(void);
#endif
