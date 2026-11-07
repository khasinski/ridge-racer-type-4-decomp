#include "types.h"
#include "game/sound/cdmain.h"
#include "psyq/libcd/s_1.h"
#include "game/sound/cdpos.h"
#include "rr/cd.h"
#include "rr/overlay668_stream.h"
#include "rr/menu_overlay_runtime.h"

extern int CdSearchFile();
extern int CdPosToInt();
extern char R4StrPath[];
extern char CdAudioTrackTable[];
extern struct { char p0[0x2634]; s32 x2634; s32 x2638; char p1[0x4]; s32 x2640; } SoundState;
extern int CdAudioSetVolume();

void CdAudioFindStreamFile(void);

void CdAudioFindStreamFile(void) {
    struct { u8 pad[8]; u8 b[8]; u8 c[24]; } L;
    SoundState.x2638 = -1;
    SoundState.x2640 = -1;
    if (CdControlB(1, 0, L.b) != 0) {
        u32 v = L.b[0];
        if ((v & 0x10) == 0) {
            if ((v & 1) == 0) {
                CdSearchFile(L.c, R4StrPath);
                SoundState.x2634 = CdPosToInt(L.c) & 0xFFFFFF;
                SoundState.x2638 = (int)CdAudioTrackTable;
            }
        }
    }
}

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(struct { char p0[0x2658]; u8 x2658; char p1[0xB]; s32 x2664; char p2[0x8]; s32 x2670; s32 x2674; s32 x2678; char p3[0x20]; s32 x269C; char p4[0x8]; s32 x26A8; char p5[0xC]; s32 x26B8; s32 x26BC; char p6[0x4]; s16 x26C4; s16 x26C6; s16 x26C8; s16 x26CA; char p7[0x19C]; s32 x2868; } (*))&SoundState)

void CdAudioInit(void) {
    SoundState.x2658 = 0xC8;
    CdControlB(0xE, &SoundState.x2658, 0);
    SsSetSerialVol(0, 0x7F, 0x7F);
    CdReadyCallback((int)CdAudioReadyCallback);
    SoundState.x2664 = -1;
    SoundState.x2670 = -1;
    SoundState.x2674 = 0;
    SoundState.x2678 = 0;
    SoundState.x269C = -1;
    SoundState.x26A8 = 0;
    SoundState.x26B8 = 0;
    SoundState.x26BC = 0;
    SoundState.x26C4 = 0;
    SoundState.x26C6 = 0;
    SoundState.x26C8 = 0;
    SoundState.x26CA = 0;
    CdAudioSetVolume(0x7F);
    SoundState.x2868 = -1;
    RaceBgmCdTrack = 0;
}

#undef SoundState
