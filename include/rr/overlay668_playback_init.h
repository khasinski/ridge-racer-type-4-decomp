#ifndef RR_OVERLAY668_PLAYBACK_INIT_H
#define RR_OVERLAY668_PLAYBACK_INIT_H
#include "rr/overlay668_stream.h"
#include "rr/frame_environment.h"
#include "rr/driver_control.h"
#include "rr/menu_text_rows.h"
/* CD file descriptor: position, byte count and16-byte filename storage. */
typedef struct {RrCdLocation location;u32 byteSize;char name[16];} RrStrCdFileInfo;
extern void *MenuArenaBase,*LoadCursor;
extern s16 ModeGroup,ModeId,ModeSubState;
void ovl668_func_80114910(s32 resource,RrStrCdFileInfo *file,u32 *sectors);
/* After the display setup service, MenuArenaBase covers an aligned complete
 * playback owner. Publish its base and end, then clear the captured owner.
 * Services may replace roots and scalar state; the resource id is read after
 * both CD calls and must satisfy the resource service's directory contract.
 * The local output descriptors are not read by this initializer. */
void ovl668_func_801149E8(void);
RR_ASSERT_SIZE(RrStrCdFileInfo,24);
RR_ASSERT_OFFSET(RrStrCdFileInfo,byteSize,4);
RR_ASSERT_OFFSET(RrStrCdFileInfo,name,8);
#endif
