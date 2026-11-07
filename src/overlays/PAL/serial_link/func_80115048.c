#include "types.h"
extern s32 serial_link_error_count;
s32 serial_link_get_error_count(void) { return serial_link_error_count; }
