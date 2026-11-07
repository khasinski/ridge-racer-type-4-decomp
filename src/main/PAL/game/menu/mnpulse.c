#include "rr/menu_pulse.h"

void PadStepPulse(void) {
    RrMenuItemState5C *next_output;
    RrPulseState18 *next_state;
    /* ASM_BARRIER: reuse V1 for selector, step and duration in disjoint lifetimes. */
    register s32 word asm("$3");
    s32 port;
    u16 repeat_tick;
    u16 once_tick;
    const RrPulseRow *repeat_row;
    const RrPulseRow *once_row;

    RrPulseSetting4 *settings;
    const RrPulseRow **repeat, **once;
    s32 small;
    port = 0;
    settings = Options.controls.pulse;
    repeat = PadPulseRepeatRows;
    small = 64;
    once = D_8009FF80;
    next_output = PlayerControllers;
    next_state = PadPulseState;
    do {
        /* ASM_BARRIER: keep the per-port state cursor in A2 inside the loop. */
        register RrPulseState18 *state asm("$6") = next_state;
        /* ASM_BARRIER: keep the output cursor in A1 after address initialization. */
        register RrMenuItemState5C *output asm("$5") = next_output;
        if (settings[port].disabled02 != 0) {
            /* ASM_BARRIER: retain the disabled-port path before the shared clear. */
            __asm__ volatile("" ::: "memory");
            output->output19 = 0U;
            goto clear_second;
        }
        word = state->repeat04;
        if (word != 0) {
            s32 index;
            if (output->kind1C != 0) {
                index = word - 1;
                word = state->repeat_step0C;
                repeat_row = repeat[index] + word;
                output->output19 = (u8) repeat_row->first;
                output->output1A = (u8) repeat_row->second;
            } else {
                index = word + 2;
                word = state->repeat_step0C;
                repeat_row = repeat[index] + word;
                output->output19 = small;
                output->output1A = (u8) repeat_row->first;
            }
            repeat_tick = state->repeat_tick08 + 1;
            state->repeat_tick08 = repeat_tick;
            if ((s16) repeat_tick >= (word = repeat_row->duration)) {
                state->repeat_step0C = (s16) ((u16) state->repeat_step0C + 1);
                if (repeat_row[1].duration < 0) {
                    state->repeat_tick08 = 0U;
                    state->repeat_step0C = 0;
                }
            }
        } else {
            output->output19 = 0U;
            output->output1A = 0U;
        }
        word = state->once06;
        if (word != 0) {
            s32 index;
            if (output->kind1C != 0) {
                index = word - 1;
                word = state->once_step0E;
                once_row = once[index] + word;
                output->output19 = (u8) once_row->first;
                output->output1A = (u8) once_row->second;
            } else {
                index = word + 2;
                word = state->once_step0E;
                once_row = once[index] + word;
                output->output19 = small;
                output->output1A = (u8) once_row->first;
            }
            once_tick = state->once_tick0A + 1;
            state->once_tick0A = once_tick;
            if ((s16) once_tick >= (word = once_row->duration)) {
                state->once_step0E = (s16) ((u16) state->once_step0E + 1);
                if (once_row[1].duration < 0) {
                    state->once06 = 0;
                    if (output->kind1C != 0) {
                        output->output19 = 0U;
                    } else {
                        output->output19 = small;
                    }
clear_second:
                    output->output1A = 0U;
                }
            }
        }
        next_output = output + 1;
        port += 1;
        next_state = state + 1;
    } while (port < 2);
}
