/* GCC_VERSION: 2.7.2 */
#include "rr/channels.h"

void _padCmdParaMode(void *channel, s32 value);
void _padCmd4C(void *channel, s32 value);
void _padCmd46(void *channel, s32 value);
void _padCmd47(void *channel, s32 value);

void _padMainModeUpdate(void *raw) {
    RrChannelPatchCommand *a0=raw;
    int t = a0->phase;
    switch (t) {
    case 2:
        a0->command37 = 0x44;
        a0->data2C = &a0->first;
        a0->length36 = t;
        break;
    case 3:
        a0->command37 = 0x4D;
        a0->data2C = a0->payload;
        a0->length36 = 6;
        break;
    }
}

s32 _padMainModeComplete(void *raw) {
    RrChannelPatchCommand *arg0=raw;
    if (arg0->equal != 0) {
        if (arg0->phase == 2) {
            return 1;
        }
        arg0->phase = 0xFEU;
        goto block_5;
    }
    PadResetChannelFn(raw);
block_5:
    return 0;
}

void _padCmdParaMode(void *channel, s32 value) {
    RrChannelStatusView *state=channel;
    state->command37=0x43;
    state->data2C=&state->argument24;
    state->argument24=value;
    state->length36=1;
}

void _padCmd45(void *channel) {
    RrChannelStatusView *state=channel;
    state->command37=0x45;
    state->data2C=0;
    state->length36=0;
}

void _padCmd4C(void *channel, s32 value) {
    RrChannelStatusView *state=channel;
    state->command37=0x4C;
    state->data2C=&state->argument24;
    state->argument24=value;
    state->length36=1;
}

void _padCmd46(void *channel, s32 value) {
    RrChannelStatusView *state=channel;
    state->command37=0x46;
    state->data2C=&state->argument24;
    state->argument24=value;
    state->length36=1;
}

void _padCmd47(void *channel, s32 value) {
    RrChannelStatusView *state=channel;
    state->command37=0x47;
    state->data2C=&state->argument24;
    state->argument24=value;
    state->length36=1;
}

void _padCmd4B(void *channel) {
    RrChannelStatusView *state=channel;
    state->command37=0x4B;
    state->data2C=0;
    state->length36=0;
}
