#ifndef RR_GARAGE_NAME_INIT_H
#define RR_GARAGE_NAME_INIT_H
#include "rr/garage_progress_init.h"
#include "rr/menu_transition.h"
#include "rr/string_tail_copy.h"
#include "rr/garage_name_state.h"
/* Name source and destination obey string-tail-copy bounds. Requires a fresh
 * published bank with two badge rows after each badge-layout call. */
void garage_init_name_entry(void);
#endif
