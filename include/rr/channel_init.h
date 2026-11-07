#ifndef RR_CHANNEL_INIT_H
#define RR_CHANNEL_INIT_H
#include "rr/channels.h"
#include "rr/channel_advance.h"
#include "rr/bios_devices.h"
#include "psyq/libpad/padchk.h"
extern u8 PadPacketBuffers[],PadTxBuffers[];
extern s32 PadMultitapEnabled;
extern u32 (*PadTxByteCallback)(const RrChannelTransferView *);
extern RrChannelUpdateCallback PadAllocGatesFn;
extern s32 (*PadChannelToPortFn)(void *);
extern void (*PadSaveCommandCallback)(RrChannelTransferView *);
void _padResetChannel(void *);
s32 _padChannelToPort(void *);
void *_padPortToChannel(s32);
void _padInitDirSeq(void),_padSetVsyncParam(void);
/* Two input buffers must each permit two byte stores. They may overlap each
 * other or byte fields in the record arena; pointer slots must remain valid.
 * Clears both canonical records, installs callbacks, and publishes ready last. */
void PadInitDirect(u8 *first,u8 *second);
#endif
