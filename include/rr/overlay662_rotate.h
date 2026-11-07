#ifndef RR_OVERLAY662_ROTATE_H
#define RR_OVERLAY662_ROTATE_H
#include "rr/overlay662_buttons.h"
#include "rr/overlay662_options.h"
#include "rr/menu_choice.h"
/* Negative commands reset the selection;1/2/4/8 navigate;other positive
 * commands leave it unchanged. Command0 checks hit slot1 and the current
 * working flags, then rotates the selected64x64 four-bit image clockwise.
 * After services,active21 is0/1 and both selected image pointers address
 * disjoint complete512-word buffers. Rectangle table slot1 addresses the
 * first member of a complete RrGarageButton. Services may change all roots. */
void garage_rotate_tool_action(s32 command);
typedef char RrGarageRotateAddressWidth[sizeof(unsigned long)==sizeof(void*)?1:-1];
#endif
