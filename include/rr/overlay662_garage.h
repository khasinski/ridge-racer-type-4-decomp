#ifndef RR_OVERLAY662_GARAGE_H
#define RR_OVERLAY662_GARAGE_H
#include "rr/layout.h"
typedef struct RrGarageOptionRecord RrGarageOptionRecord;
/* One pointer owner has flat initialization and3/5 navigation group views. */
typedef struct {RrGarageOptionRecord *first[3],*second[5];} RrGarageOptionPointerGroups;
typedef union {RrGarageOptionRecord *flat[8];RrGarageOptionPointerGroups groups;} RrGarageOptionPointers;
typedef struct {s32 first[3],second[5];} RrGarageOptionIndexGroups;
typedef union {s32 flat[8];RrGarageOptionIndexGroups groups;} RrGarageOptionIndices;
/* Complete TIM byte image; byte alignment retains the original unaligned copy path. */
typedef struct {u8 data[0x840];} RrGarageTimBytes;
typedef struct {
    s32 fade00,word04;
    s16 reset08[10];
    u8 config1C,config1D,config1E,config1F,config20,active21,config22,layer23;
    u32 *buffers24[2];
    RrGarageTimBytes *tim2C[2];
    u32 *scratch34; /* Complete512-word preview image. */
    u32 *image38[2],*image40[2],*palette48[2];
    u32 *paletteBackup50[2]; /* Other palette of each undo pair; eight words. */
    u32 *paletteScratch58[2];
    s32 current60,saved64[5];
    RrGarageOptionPointers options78;
    RrGarageOptionIndices optionIndices98;
    s32 selectorsB8[4];
    s16 halfC8,halfCA;
    u32 phaseCC;
    u8 shadeD0,flagsD1,stateD2,dirtyD3;
} RrGarageSelectionPrefix;
extern RrGarageSelectionPrefix garage_logo_state;
RR_ASSERT_SIZE(RrGarageSelectionPrefix,0xd4);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,phaseCC,0xcc);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,shadeD0,0xd0);
void garage_update_selection_pulse(void);
void garage_reset_editor_state(s32 mode);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,reset08,0x08);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,config1C,0x1c);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,flagsD1,0xd1);
/* Negative modes initialize option pointers and scalar selections. */
void garage_init_option_tables(s32 mode);
RR_ASSERT_SIZE(RrGarageOptionPointers,0x20);
RR_ASSERT_SIZE(RrGarageOptionIndices,0x20);
RR_ASSERT_OFFSET(RrGarageOptionPointerGroups,second,0x0C);
RR_ASSERT_OFFSET(RrGarageOptionIndexGroups,second,0x0C);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,options78,0x78);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,optionIndices98,0x98);
/* active21 is a two-buffer selector. Copy forward, then retain the selection. */
void garage_save_undo_copy(void);
/* Return1 when all 512 words of the selected layer and side are zero. */
s32 garage_is_layer_empty(void);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,layer23,0x23);
void garage_alt_undo_toggle(void);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,buffers24,0x24);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,tim2C,0x2C);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,scratch34,0x34);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,image38,0x38);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,image40,0x40);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,palette48,0x48);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,dirtyD3,0xd3);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,paletteBackup50,0x50);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,paletteScratch58,0x58);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,saved64,0x64);
RR_ASSERT_OFFSET(RrGarageSelectionPrefix,selectorsB8,0xb8);
/* Shared 64-byte working owner: scalar animation counters and halfword clips. */
typedef struct {s16 x,y;} RrGaragePoint;
RR_ASSERT_SIZE(RrGaragePoint,4);
typedef struct {
    u32 prefix00[5];
    RrGaragePoint cursor14;
    RrGaragePoint cursor18;
    u32 word1C;
    RrGaragePoint first20,second24;
    u32 word28;
    s32 distance2C;
    u32 tail30[4];
} RrGaragePointWorkingView;
typedef union {
    s16 halves[32];u32 words[16];
    RrGaragePointWorkingView points;
} RrGarageWorkingStorage;
RR_ASSERT_SIZE(RrGarageWorkingStorage,64);
RR_ASSERT_SIZE(RrGaragePointWorkingView,64);
RR_ASSERT_OFFSET(RrGaragePointWorkingView,cursor14,0x14);
RR_ASSERT_OFFSET(RrGaragePointWorkingView,cursor18,0x18);
RR_ASSERT_OFFSET(RrGaragePointWorkingView,first20,0x20);
RR_ASSERT_OFFSET(RrGaragePointWorkingView,second24,0x24);
RR_ASSERT_OFFSET(RrGaragePointWorkingView,distance2C,0x2C);
extern RrGarageWorkingStorage garage_logo_work;
s32 garage_step_button_repeat(s32 mode);
typedef u16 RrGarageRectangle[4];
/* Nonzero controller movement replaces both coordinates; neutral preserves output. */
void garage_read_mouse_motion(RrGaragePoint *output);
/* Signed rectangle center; halves of negative odd extents round downward. */
RrGaragePoint garage_get_button_center(s32 index);
extern RrGarageRectangle *garage_logo_buttons[];

/* Overlay-local selection state; known scalar fields are menu indices.
 * Clip working storage uses signed halfwords; rectangle payloads preserve
 * raw halfword bits and are interpreted as signed coordinates by hit testing. */
void garage_save_cursor_selection(void);
/* Returns0 only for a point in [x,x+w) by [y,y+h); arguments use signed low16. */
s32 garage_cursor_outside_rect(s32 x,s32 y,s32 width,s32 height);
s32 garage_cursor_outside_button(s32 index);
/* Capture the selected pointer once, then copy the four halfwords in order.
 * Payload storage may overlap the clip destination. */
void garage_set_clip_from_button(s32 index);
#endif
