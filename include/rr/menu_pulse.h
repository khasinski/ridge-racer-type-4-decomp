#ifndef RR_MENU_PULSE_H
#define RR_MENU_PULSE_H

#include "rr/tables.h"
#include "rr/runtime_settings.h"

typedef struct {
    u8 first, second;
    s16 duration;
} RrPulseRow;

typedef struct {
    u8 pad00[4];
    s16 repeat04, once06;
    u16 repeat_tick08, once_tick0A;
    s16 repeat_step0C, once_step0E;
    u8 pad10[8];
} RrPulseState18;

RR_ASSERT_SIZE(RrPulseSetting4, 4);
RR_ASSERT_OFFSET(RrPulseSetting4, disabled02, 2);
RR_ASSERT_SIZE(RrPulseRow, 4);
RR_ASSERT_OFFSET(RrPulseRow, duration, 2);
RR_ASSERT_SIZE(RrPulseState18, 0x18);
RR_ASSERT_OFFSET(RrPulseState18, repeat04, 4);
RR_ASSERT_OFFSET(RrPulseState18, once06, 6);
RR_ASSERT_OFFSET(RrPulseState18, repeat_tick08, 8);
RR_ASSERT_OFFSET(RrPulseState18, once_tick0A, 0xA);
RR_ASSERT_OFFSET(RrPulseState18, repeat_step0C, 0xC);
RR_ASSERT_OFFSET(RrPulseState18, once_step0E, 0xE);

extern const RrPulseRow *D_8009FF80[], *D_8009FFA0[];
extern RrMenuItemState5C PlayerControllers[];
extern RrPulseState18 PadPulseState[];
void PadStepPulse(void);
s32 ovl665_func_8011629C(s32 index);

#endif
