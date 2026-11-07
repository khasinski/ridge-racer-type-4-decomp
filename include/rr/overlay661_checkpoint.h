#ifndef RR_OVERLAY661_CHECKPOINT_H
#define RR_OVERLAY661_CHECKPOINT_H
#include "rr/race_checkpoint_types.h"
#include "rr/sequence_queue.h"
/* The notification latch tracks valid sequence-table IDs. When pending is0,
 * the wrapped ID9-(target-counter), after mapping9to15, must be below25 if
 * it is at least6. Services preserve this notification-state contract. This
 * keeps the mask shift within0..18, as required by the existing25-row table.
 * The latch is reloaded after a sequence callback before its bit is set. */
void versus_select_func_80114924(RrRaceCheckpointVehicle *vehicle);
#endif
