#ifndef RR_OVERLAY662_NAME_STATE_H
#define RR_OVERLAY662_NAME_STATE_H
#include "rr/layout.h"
/* Complete name storage occupies273A8..273B4, before the selected-key word.
 * Eight positions are displayed and edited; the remaining bytes retain the
 * string terminator/tail used by the resident string-copy helper. */
extern s8 garage_name_cursor,garage_name_key_column,garage_name_key_row;
extern u8 garage_name_buffer[12];
extern s32 garage_name_key_index;
#endif
