#ifndef RR_OVERLAY662_TOOL_ACTION_H
#define RR_OVERLAY662_TOOL_ACTION_H
#include "rr/overlay662_distance.h"
#include "rr/menu_choice.h"
#include "rr/overlay662_pixels.h"
#include "rr/overlay662_buttons.h"
void garage_func_8011E5C0(s32 mode);
/* Mode0 hit-tests table slot4 before checking input. Negative modes reset
 * indices;1/2/4/8 configure navigation and call the mapper (its table contract
 * applies). Other positive words are idle. Sound may replace current state. */
void garage_func_8011F4C0(s32 mode);
/* Mode0 uses button owners in slots7/8. Timer value7 is published before sound;
 * after sound, reload the current byte selector/zoom and apply PAL byte wrap. */
void garage_func_8011FB2C(s32 mode);
void garage_func_8011FCE0(s32 mode);
/* Slot9 selects the base tool; post-sound state is preserved. */
void garage_func_8011FE84(s32 mode);
/* Slot5 clears the freshly selected512-word layer after backup/sound.
 * An already empty layer rejects without backup or clearing. */
void garage_func_8011F678(s32 mode);
/* unsigned long is the complete address word on both PAL and native LP64. */
typedef unsigned long RrGarageImageAddress;
typedef char RrGarageImageAddressWidth[(sizeof(RrGarageImageAddress)==sizeof(void*))?1:-1];
/* Slot3 backs up images/palette, then copies rows63..0 from the inverse
 * buffer into the freshly selected layer. Buffers cover512 words; ordered
 * writes observe overlaps rather than staging a complete input copy. */
void garage_func_8011F290(s32 mode);
/* Slots10..14 update buffer/tool/brush state before sound. */
void garage_func_80120004(s32 mode);
void garage_func_8012019C(s32 mode);
void garage_func_8012031C(s32 mode);
void garage_func_8012049C(s32 mode);
void garage_func_80120648(s32 mode);
/* Slot15 captures undo state, backs up, publishes timer7 and plays sound.
 * Then swap the selected buffer/TIM, image and palette pairs. Sound may change
 * active21 (must remain0..1) and every pointer; use the fresh selection.
 * Both sides of the buffer/TIM pair retain word-aligned, complete TIM-sized backing. */
void garage_func_801207C8(s32 mode);
/* Slot16 composites fresh selected image38 into image40 after backup/sound. */
void garage_func_801209C4(s32 mode);
/* Slots19/20 select layer0/1 and publish their button timer before sound. */
void garage_func_80121564(s32 mode);
void garage_func_801216D8(s32 mode);
/* Slots24..26 publish palette colors4..6 before sound. */
void garage_func_80121CDC(s32 mode);
void garage_func_80121E6C(s32 mode);
void garage_func_80121FF4(s32 mode);
/* Slot21 publishes palette color1 before sound. */
void garage_func_8012184C(s32 mode);
/* Slots22/23 publish palette colors2/3 before sound. */
void garage_func_801219CC(s32 mode);
void garage_func_80121B54(s32 mode);
/* Slots27..35 publish palette colors7..15 before sound. */
void garage_func_80122188(s32 mode);
void garage_func_80122310(s32 mode);
void garage_func_801224AC(s32 mode);
void garage_func_80122638(s32 mode);
void garage_func_801227C4(s32 mode);
void garage_func_80122954(s32 mode);
void garage_func_80122AEC(s32 mode);
void garage_func_80122C74(s32 mode);
void garage_func_80122E14(s32 mode);
#endif
