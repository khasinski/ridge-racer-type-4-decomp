#ifndef RR_GAME_RACE_RACEGRID_H
#define RR_GAME_RACE_RACEGRID_H

#include "types.h"

void EnableLinkSioTx(void);
int IsLinkPeerPresent(void);
void OpenLinkSerial(void);
void CloseLinkSerial(void);
void BeginLinkSync(void);

#endif /* RR_GAME_RACE_RACEGRID_H */
