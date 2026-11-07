#ifndef RR_OVERLAY662_ARENA_INIT_H
#define RR_OVERLAY662_ARENA_INIT_H
#include "rr/overlay662_init.h"
#include "rr/overlay662_options.h"
#include "rr/menu_transition.h"
#include "rr/menu_setup.h"
#include "rr/overlay662_sprites.h"
extern u8 *LoadCursorBase;
extern void *GarageImageBlock0,*GarageTimBuffer1;
extern s32 ovl662_D_8012625C;
extern RrGpuRectangle ovl662_D_8012624C,ovl662_D_80126254;
void StoreImage(RrGpuRectangle *,const u32 *);
void ovl662_func_8011E36C(s32);
typedef unsigned long RrGarageArenaAddress;
typedef char RrGarageArenaAddressWidth[sizeof(RrGarageArenaAddress)==sizeof(void*)?1:-1];
/* At stage1 the published root has0x2944 writable bytes with enough alignment
 * for the next four-byte boundary. The two complete TIM spans are0x840 bytes,
 * scratch is512 words, and each of the final palettes has8 words.
 * At stage2 both upload sources remain valid through the call; the fresh
 * image40[0] after the second upload has512 writable words. Services may change
 * all globals. The captured incremented stage supplies the final scene value. */
void ovl662_func_8011A518(void);
#endif
