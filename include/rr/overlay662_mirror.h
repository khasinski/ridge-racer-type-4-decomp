#ifndef RR_OVERLAY662_MIRROR_H
#define RR_OVERLAY662_MIRROR_H
#include "rr/overlay662_buttons.h"
#include "rr/overlay662_options.h"
#include "rr/menu_choice.h"
/* Negative commands reset this tool;1/2/4/8 navigate;other positive commands
 * preserve state. Command0 tests hit slot2,observes fresh working flags,
 * optionally restores options,backs up,sets button2 timer7 and plays sound1.
 * It then captures fresh layer/active roots and mirrors a64x64 four-bit image.
 * Active21 is0/1 after services;the selected pair owns disjoint512-word
 * buffers. Rectangle table slot2 points at a complete RrGarageButton's first
 * member. Services may replace roots and mutate state. */
void garage_func_8011EFFC(s32 command);
#endif
