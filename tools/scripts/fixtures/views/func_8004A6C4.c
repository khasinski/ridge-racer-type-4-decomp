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

typedef signed char s8;

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

typedef struct {
    u8 status00, type01, data02, data03, data04, data05, data06, data07, reserved08[32];
} Port;

typedef struct {
    s16 pad00[4];
    struct {
        u16 center00;
        u16 offset02;
        u16 offset04;
        u16 offset06;
        s16 deadzone08;
        s16 range0A;
        s16 pad0C;
        s16 pad0E;
    } p[2];
    struct {
        s8 deadzone00;
        s8 range01;
        s8 pad02[2];
    } w[2];
    u8 rem[12];
} Settings;

extern Settings Options;

extern Pad PlayerControllers[2];

extern Port PadPorts[2];

extern s32 D_800FFC08[2];

extern s32 D_800AD590[2];

extern s32 D_800F4CD8[2];

extern s16 SteeringRangeTable[4];

extern s16 PadRangeLimits[6];

extern s32 WheelDeadzoneTable[8];

extern s32 PadDeadzoneTable[8];

extern s32 VSyncThreshold;

extern s16 D_800F3824;

int abs(int);

void PadStepPulse(void);

void PadUpdateRumble(void);

void PadPollPort(s32);

void PadUpdate(void) {
    Port *ports[2];
    Pad *states[2];
    s32 i;
    s32 v;
    s16 val;

    __asm__ volatile("" ::: "memory"); /* ASM_BARRIER: keep prologue register saves before ports array initialization */
    ports[0] = &PadPorts[0];
    ports[1] = &PadPorts[1];
    __asm__ volatile("" ::: "memory"); /* ASM_BARRIER: schedule ports array initialization before loop preheader */
    states[0] = &PlayerControllers[0];
    states[1] = &PlayerControllers[1];

    for (i = 0; i < 2; i++) {
        if (ports[i]->type01 != states[i]->type01) {
            states[i]->changed1E = 1;
            states[i]->protocol10 = -1;
        }
        states[i]->status00 = ports[i]->status00;
        states[i]->type01 = ports[i]->type01;
        if (ports[i]->status00 != 0) {
            D_800FFC08[i] = 1;
            D_800F4CD8[i] = 0x22;
            D_800AD590[i] |= 0x10;
        } else if (D_800F4CD8 != NULL) {
            D_800F4CD8[i]--;
            if (ports[i]->type01 == 0x23) {
                u16 btn = ~((ports[i]->data02 << 8) | ports[i]->data03);
                if ((btn & 0x5000) == 0x5000 || (btn & 0xA000) == 0xA000 || (btn & 0x1C4)) {
                    D_800FFC08[i] = 2;
                    D_800F4CD8[i] = 0x22;
                    D_800AD590[i] |= 0x10;
                }
            }
        }
        D_800AD590[i] >>= 1;
        if (D_800AD590[i] != 0) {
            Port *port = ports[i];
            Pad *pad = states[i];
            port->type01 = 0;
            pad->type01 = 0;
        } else {
            D_800FFC08[i] = 0;
        }
        if (states[i]->type01 != 0 && states[i]->protocol10 < 0) {
            if (states[i]->type01 == 0x41) {
                states[i]->protocol10 = 0;
            } else if (states[i]->type01 == 0x23) {
                states[i]->protocol10 = 1;
            } else if (states[i]->type01 == 0x73) {
                states[i]->protocol10 = 0;
            } else {
                states[i]->protocol10 = -1;
            }
        }
        switch (ports[i]->type01) {
        case 0x41:
            states[i]->previous04 = states[i]->buttons02;
            states[i]->buttons02 = ~((ports[i]->data02 << 8) | ports[i]->data03);
            states[i]->pressed08 = states[i]->buttons02 & ~states[i]->previous04;
            states[i]->released06 = states[i]->previous04 & (states[i]->buttons02 ^ states[i]->previous04);
            states[i]->steering20 = (((states[i]->buttons02 >> 13) & 1) - ((states[i]->buttons02 & 0x8000) ? 1 : 0)) * SteeringRangeTable[D_800F3824] + 0x80;
            states[i]->pressure22 = (states[i]->buttons02 & 0x40) ? 0x6A : 0;
            states[i]->pressure24 = (states[i]->buttons02 & 0x80) ? 0x6A : 0;
            states[i]->pressure26 = (states[i]->buttons02 & 4) ? 0x6A : 0;
            states[i]->action28 = states[i]->pressed08 & 0x820;
            states[i]->action2C = states[i]->pressed08 & 0x40;
            states[i]->action30 = states[i]->pressed08 & 0x20;
            states[i]->action34 = states[i]->pressed08 & 0x40;
            states[i]->idleStage54 = 0;
            states[i]->idleTick58 = 0;
            break;
        case 0x23:
            states[i]->previous04 = states[i]->buttons02;
            states[i]->buttons02 = ~((ports[i]->data02 << 8) | ports[i]->data03);
            states[i]->released06 = states[i]->previous04 & (states[i]->buttons02 ^ states[i]->previous04);
            states[i]->steering20 = ports[i]->data04;
            states[i]->pressure22 = ports[i]->data05 - Options.p[i].offset02;
            states[i]->pressure24 = ports[i]->data06 - Options.p[i].offset04;
            states[i]->pressure26 = ports[i]->data07 - Options.p[i].offset06;
            if (states[i]->pressure22 < 0) {
                states[i]->pressure22 = 0;
            }
            if (states[i]->pressure24 < 0) {
                states[i]->pressure24 = 0;
            }
            if (states[i]->pressure26 < 0) {
                states[i]->pressure26 = 0;
            }
            if (states[i]->steering20 >= 0xA3) {
                states[i]->buttons02 |= 0x2000;
            }
            if (states[i]->steering20 < 0x5E) {
                states[i]->buttons02 |= 0x8000;
            }
            if (states[i]->pressure22 >= 0x36) {
                states[i]->buttons02 |= 0x40;
            }
            if (states[i]->pressure24 >= 0x36) {
                states[i]->buttons02 |= 0x80;
            }
            if (states[i]->pressure26 >= 0x36) {
                states[i]->buttons02 |= 4;
            }
            states[i]->pressed08 = states[i]->buttons02 & ~states[i]->previous04;
            states[i]->action28 = states[i]->pressed08 & 0x840;
            states[i]->action2C = states[i]->pressed08 & 0x80;
            states[i]->action30 = states[i]->pressed08 & 0x40;
            states[i]->action34 = states[i]->pressed08 & 0x80;
            states[i]->idleStage54 = 0;
            states[i]->idleTick58 = 0;
            break;
        case 0x73:
            states[i]->previous04 = states[i]->buttons02;
            states[i]->buttons02 = ~((ports[i]->data02 << 8) | ports[i]->data03);
            states[i]->pressed08 = states[i]->buttons02 & ~states[i]->previous04;
            states[i]->released06 = states[i]->previous04 & (states[i]->buttons02 ^ states[i]->previous04);
            states[i]->steering20 = ports[i]->data06;
            states[i]->pressure22 = (states[i]->buttons02 & 0x40) ? 0x6A : 0;
            states[i]->pressure24 = (states[i]->buttons02 & 0x80) ? 0x6A : 0;
            states[i]->pressure26 = (states[i]->buttons02 & 4) ? 0x6A : 0;
            states[i]->action28 = states[i]->pressed08 & 0x820;
            states[i]->action2C = states[i]->pressed08 & 0x40;
            states[i]->action30 = states[i]->pressed08 & 0x20;
            states[i]->action34 = states[i]->pressed08 & 0x40;
            states[i]->axes14 = *(u32 *)&ports[i]->data04;
            states[i]->idleStage54 = 0;
            states[i]->idleTick58 = 0;
            break;
        case 0xE3:
            states[i]->previous04 = states[i]->buttons02;
            states[i]->buttons02 = ~((ports[i]->data02 << 8) | ports[i]->data03);
            states[i]->pressed08 = states[i]->buttons02 & ~states[i]->previous04;
            states[i]->released06 = states[i]->previous04 & (states[i]->buttons02 ^ states[i]->previous04);
            states[i]->position41 = ports[i]->data04;
            states[i]->position42 = ports[i]->data05;
            states[i]->position43 = ports[i]->data06;
            states[i]->position45 = states[i]->position42;
            states[i]->previous4E = states[i]->position4C;
            states[i]->steering20 = states[i]->position4C = states[i]->position41 + ((s8)states[i]->position45 << 8);
            states[i]->pressure22 = (states[i]->buttons02 & 0x40) ? 0x6A : 0;
            states[i]->pressure24 = (states[i]->buttons02 & 0x80) ? 0x6A : 0;
            states[i]->pressure26 = (states[i]->buttons02 & 4) ? 0x6A : 0;
            states[i]->action28 = states[i]->pressed08 & 0x820;
            states[i]->action2C = states[i]->pressed08 & 0x40;
            states[i]->action30 = states[i]->pressed08 & 0x20;
            states[i]->action34 = states[i]->pressed08 & 0x40;
            states[i]->idleStage54 = 0;
            if (states[i]->previous04 == states[i]->buttons02) {
                if (VSyncThreshold == 0x80) {
                    states[i]->idleTick58 += 1;
                } else {
                    states[i]->idleTick58 += 2;
                }
                if (states[i]->idleTick58 >= 0xBB9) {
                    states[i]->idleTick58 = 0xBB8;
                }
            } else {
                states[i]->idleTick58 = 0;
            }
            if (states[i]->idleTick58 >= 0xBB8) {
                states[i]->idleStage54 = 2;
            } else if (states[i]->idleTick58 >= 0xABF) {
                states[i]->idleStage54 = 1;
            }
            break;
        case 0x12:
            states[i]->previous04 = states[i]->buttons02;
            states[i]->buttons02 = ~((ports[i]->data02 << 8) | ports[i]->data03);
            states[i]->pressed08 = states[i]->buttons02 & ~states[i]->previous04;
            states[i]->released06 = states[i]->previous04 & (states[i]->buttons02 ^ states[i]->previous04);
            states[i]->axes14 = *(u32 *)&ports[i]->data04;
            states[i]->action28 = 0;
            states[i]->action2C = 0;
            states[i]->action30 = 0;
            states[i]->action34 = 0;
            states[i]->idleStage54 = 0;
            states[i]->idleTick58 = 0;
            break;
        default:
            if (D_800FFC08[i] == 0) {
                D_800FFC08[i] = 2;
            }
            states[i]->status00 = 1;
            states[i]->buttons02 = 0;
            states[i]->pressed08 = 0;
            states[i]->repeat0A = 0;
            states[i]->steering20 = 0x80;
            states[i]->pressure22 = 0;
            states[i]->pressure24 = 0;
            states[i]->pressure26 = 0;
            states[i]->action28 = 0;
            states[i]->action2C = 0;
            states[i]->action30 = 0;
            states[i]->action34 = 0;
            states[i]->idleStage54 = 0;
            states[i]->idleTick58 = 0;
            break;
        }
        states[i]->repeat0A = states[i]->pressed08;
        if (states[i]->buttons02 == states[i]->previous04) {
            states[i]->repeatTick0C++;
            if (states[i]->repeatTick0C >= 0x22) {
                states[i]->repeat0A = states[i]->buttons02;
                states[i]->repeatTick0C = 0x1E;
            }
        } else {
            states[i]->repeatTick0C = 0;
        }
        switch (ports[i]->type01) {
        case 0x23:
            val = states[i]->steering20 - (Options.p[i].center00 + 0x80);
            if (val > 0) {
                val -= PadDeadzoneTable[Options.p[i].deadzone08];
                if (val < 0) {
                    val = 0;
                }
                if (val > SteeringRangeTable[Options.p[i].range0A]) {
                    val = SteeringRangeTable[Options.p[i].range0A];
                }
            } else {
                val += PadDeadzoneTable[Options.p[i].deadzone08];
                if (val > 0) {
                    val = 0;
                }
                if (val < -SteeringRangeTable[Options.p[i].range0A]) {
                    val = -SteeringRangeTable[Options.p[i].range0A];
                }
            }
            states[i]->steering3C = val;
            break;
        case 0xE3:
            val = states[i]->steering20;
            if (val > 0) {
                val -= WheelDeadzoneTable[Options.w[i].deadzone00];
                if (val < 0) {
                    val = 0;
                }
                if (val > PadRangeLimits[Options.w[i].range01]) {
                    val = PadRangeLimits[Options.w[i].range01];
                }
            } else {
                val += WheelDeadzoneTable[Options.w[i].deadzone00];
                if (val > 0) {
                    val = 0;
                }
                if (val < -PadRangeLimits[Options.w[i].range01]) {
                    val = -PadRangeLimits[Options.w[i].range01];
                }
            }
            states[i]->steering3C = (val << 7) / PadRangeLimits[Options.w[i].range01];
            break;
        case 0x73:
            val = states[i]->steering20 - 0x80;
            if (abs(val) < 0x18) {
                val = 0;
            }
            if (val > 0) {
                val -= 0x10;
            }
            if (val < 0) {
                val += 0x10;
            }
            states[i]->steering3C = (val * 0x7F) / 111;
            break;
        }
        if (states[i]->pressure22 >= 0x6B) {
            states[i]->pressure22 = 0x6A;
        }
        if (states[i]->pressure24 >= 0x6B) {
            states[i]->pressure24 = 0x6A;
        }
        if (states[i]->pressure26 >= 0x6B) {
            states[i]->pressure26 = 0x6A;
        }
    }
    PadStepPulse();
    PadUpdateRumble();
    for (i = 0; i < 2; i++) {
        PadPollPort(i);
    }
}
