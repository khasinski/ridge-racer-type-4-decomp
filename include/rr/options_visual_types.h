#ifndef RR_OPTIONS_VISUAL_TYPES_H
#define RR_OPTIONS_VISUAL_TYPES_H
#include "rr/options_matrix.h"
typedef union {s32 words[3];s16 halves[6];} RrPreviewPosition12;
#include "rr/menu_selection_word.h"
typedef struct {
    RrPreviewPosition12 position00;
    s32 reserved0C;
    s16 rotation10,rotation12,rotation14;
    u8 pad16[2];
    s16 rotation18;u8 pad1A[2];
    s16 rotation1C;u8 pad1E[2];
    s16 rotation20;u8 tail22[10];
} RrPreviewView44;
/* Exact160-byte owner cleared by14918. Only observed scalar fields are named;
 * each RGB row and signed direction row retains its fourth padding component. */
typedef struct {
    s32 scene00,reserved04,selection08;
    s32 layout0C,detail10;
    u8 pad14[4];s32 kind18;u8 pad1C[16];s32 reset2C;u8 pad30[4];RrMenuSelectionWord32 exit34;
    u16 centerX38,centerY3A;
    s32 scale3C,depth40;
    u16 rotation44;u8 pad46[2];s32 position48;
    RrPreviewLighting40 lighting4C;
    RrPreviewView44 view74;
} RrOptionsVisualState;
#endif
