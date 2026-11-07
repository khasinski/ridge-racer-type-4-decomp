#include "types.h"
#include "m2c_macros.h"
#include "psyq/libpad/padctl.h"
#include "game/car/carstat.h"
#include "rr/bios_devices.h"
#include "rr/channel_init.h"
#include "rr/runtime_settings.h"
#include "rr/race_setup.h"

extern int PlayerControllers[];
extern char PadPulseState[];
extern char PadPorts[];
extern unsigned short PadButtonLayoutPresetsA[][8];
extern unsigned short PadButtonLayoutPresetsB[][8];
extern unsigned short PadButtonLayoutPresetsC[][8];
extern unsigned short PadButtonMap[][3][8];
extern unsigned short PadButtonMapAux[][2][8];

void SetPadButtonMapLayout(int a, int b, int c, int d);

void InitGamePads(void) {
    PsyqBzero(PlayerControllers, 0xB8);
    PsyqBzero(PadPulseState, 0x30);
    PlayerControllers[4] = -1;
    PlayerControllers[27] = -1;
    PadInitDirect(PadPorts, PadPorts + 0x28);
    PadStartCom();
}

void SetPadButtonMapLayout(int a, int b, int c, int d) {
    int i;
    for (i = 0; i < 8; i++) {
        PadButtonMap[a][0][i] = PadButtonLayoutPresetsA[b][i];
        PadButtonMap[a][1][i] = PadButtonLayoutPresetsB[c][i];
        PadButtonMap[a][2][i] = PadButtonLayoutPresetsC[d][i];
        PadButtonMapAux[0][0][i] = PadButtonLayoutPresetsC[b][i];
        PadButtonMapAux[0][1][i] = PadButtonLayoutPresetsB[c][i];
    }
}

void ApplyOptionsPadButtonMaps(void) {
    SetPadButtonMapLayout(0, M2C_FIELD(&Options, s16 *, 0), M2C_FIELD(&Options, s16 *, 0x14), M2C_FIELD(&Options, s8 *, 0x2B));
    SetPadButtonMapLayout(1, M2C_FIELD(&Options, s16 *, 4), M2C_FIELD(&Options, s16 *, 0x24), M2C_FIELD(&Options, s8 *, 0x2F));
}
