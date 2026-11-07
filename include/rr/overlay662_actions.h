#ifndef RR_OVERLAY662_ACTIONS_H
#define RR_OVERLAY662_ACTIONS_H
#include "rr/overlay662_garage.h"
#include "rr/menu_choice.h"

typedef void (*RrGarageStep)(s32 code);
typedef struct {RrGarageStep callback;} RrGarageActionPrefix;
extern RrGarageActionPrefix *garage_logo_actions[];
/* Each used index selects a valid action object with a nonnull callback.
 * Within each pair1/2 and4/8, the lower bit has priority. */
void garage_dispatch_direction_input(u32 flags);
RR_ASSERT_SIZE(RrGarageActionPrefix,4);
RR_ASSERT_OFFSET(RrGarageActionPrefix,callback,0);
#endif
