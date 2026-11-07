#define NULL 0

/* legacy typing of shared globals in this part of the unit */
#define PlayerControllers (*(T (*)[])&PlayerControllers)

#undef PlayerControllers

/* legacy typing of shared globals in this part of the unit */
#define PadPulseState (*(T_2 (*)[])&PadPulseState)

#undef PadPulseState

/* legacy typing of shared globals in this part of the unit */
#define PadPulseState (*(T_3 (*)[])&PadPulseState)

#undef PadPulseState

typedef unsigned char u8;

typedef signed short s16;

typedef unsigned short u16;

typedef signed int s32;

typedef unsigned int u32;

typedef struct {
    u8 status00, type01;
    u16 buttons02, previous04, released06, pressed08, repeat0A, repeatTick0C, reserved0E;
    s32 protocol10;
    u32 axes14;
    u8 reserved18, output19, output1A, flag1B;
    s16 kind1C, changed1E;
    s16 steering20, pressure22, pressure24, pressure26;
    s32 action28, action2C, action30, action34;
    u32 reserved38;
    s16 steering3C, reserved3E;
    u8 flag40, position41, position42, position43, reserved44, position45, reserved46[6];
    u16 position4C, previous4E;
    u32 reserved50;
    s32 idleStage54, idleTick58;
} Pad;

extern Pad PlayerControllers[2];

int GetControllerWarning(int i) {
    return PlayerControllers[i].f54;
}
