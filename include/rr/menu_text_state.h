#ifndef RR_MENU_TEXT_STATE_H
#define RR_MENU_TEXT_STATE_H
#include "rr/layout.h"
/* Same anonymous record and member types as resident80032A14's S.
 * Keeping the member names also preserves cross-TU C type compatibility. */
typedef struct {
    u16 *p;
    s8 b4,b5,st,b7;
    u8 b8;
    s8 b9;
    u8 bA,bB;
    s16 sC,sE,s10,s12,s14,s16;
    s32 i18;
    u8 b1C,b1D,b1E,b1F;
    s32 arr[0xC8];
} RrMenuTextState;
extern RrMenuTextState *DialogueState;
void SkipDialoguePage(void *state);
void UpdateDialogue(s32 x,s32 y,RrMenuTextState *state);
RR_ASSERT_SIZE(RrMenuTextState,0x340);
RR_ASSERT_OFFSET(RrMenuTextState,i18,0x18);
RR_ASSERT_OFFSET(RrMenuTextState,arr,0x20);
#endif
