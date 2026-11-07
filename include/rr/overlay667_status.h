#ifndef RR_OVERLAY667_STATUS_H
#define RR_OVERLAY667_STATUS_H
#include "rr/menu_transition.h"
extern s32 serial_link_D_80118EE4;
/* Only kind2 rejects elapsed values25000..UINT32_MAX. */
s32 serial_link_sio_timeout_check(s32 kind,u32 elapsed);
s32 serial_link_func_80115058(void);
/* Publishes scene marker315 through the canonical unsigned16 cell. */
void serial_link_set_scene_id(void);
#endif
