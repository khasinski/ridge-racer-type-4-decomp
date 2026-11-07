#ifndef RR_OVERLAY662_EXIT_DIALOG_H
#define RR_OVERLAY662_EXIT_DIALOG_H
#include "rr/overlay662_modal.h"
/* Modes1/2/4/8 configure the shared navigation mapper. Other nonzero modes
 * preserve state. Mode0 handles exit confirmation through two live UI rows
 * and complete badge/text owners. Flags0 wait for fade0 before leaving;
 * entry reads dirtyD3 after sound to choose immediate fade or confirmation.
 * Hit/reset/sound services may replace roots and update later state. */
void garage_func_80121074(s32 mode);
#endif
