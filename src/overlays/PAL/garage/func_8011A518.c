#include "rr/overlay662_arena_init.h"

void garage_init_editor_buffers(void)
{
    s32 stage,i;
    register u8 *base __asm__("$3");
    u32 *clear;
    register u8 *second __asm__("$6");
    RrGarageSelectionPrefix *owner;
    register u8 *allocated __asm__("$2");
    stage=(s32)((u32)garage_arena_init_stage+1U);
    garage_arena_init_stage=stage;
    switch (stage) {
    case 1:
        garage_add_ui_sprite(-1,0,0,0);
        garage_update_actions(-1);
        garage_init_option_tables(-1);
        garage_init_editor_state();
        base=LoadCursorBase+4;
        owner=&garage_logo_state;
        base-=((RrGarageArenaAddress)LoadCursorBase&3U);
        second=base+0x840;
        /* Retain each publication of the shared arena cursor. */
        *(void *volatile*)&LoadCursor=base;
        *(void *volatile*)&LoadCursor=second;
        allocated=base+0x1080;
        *(void *volatile*)&LoadCursor=allocated;
        owner->buffers24[1]=(u32*)allocated;
        allocated=base+0x18C0;
        *(void *volatile*)&LoadCursor=allocated;
        owner->tim2C[1]=(RrGarageTimBytes*)allocated;
        allocated=base+0x2100;
        *(void *volatile*)&LoadCursor=allocated;
        owner->scratch34=(u32*)allocated;
        allocated=base+0x2900;
        *(void *volatile*)&LoadCursor=allocated;
        owner->paletteScratch58[0]=(u32*)allocated;
        allocated=base+0x2920;
        *(void *volatile*)&LoadCursor=allocated;
        owner->paletteScratch58[1]=(u32*)allocated;
        allocated=base+0x2940;
        *(void *volatile*)&LoadCursor=allocated;
        allocated=base+0x40;
        owner->image38[0]=(u32*)allocated;
        allocated=base+0x880;
        owner->image40[0]=(u32*)allocated;
        allocated=base+0x14;
        owner->palette48[0]=(u32*)allocated;
        allocated=base+0x854;
        owner->paletteBackup50[0]=(u32*)allocated;
        allocated=base+0x10C0;
        owner->image38[1]=(u32*)allocated;
        allocated=base+0x1900;
        owner->image40[1]=(u32*)allocated;
        allocated=base+0x1094;
        owner->palette48[1]=(u32*)allocated;
        allocated=base+0x18D4;
        owner->paletteBackup50[1]=(u32*)allocated;
        GarageImageBlock0=base;
        owner->buffers24[0]=(u32*)base;
        owner->tim2C[0]=(RrGarageTimBytes*)second;
        GarageTimBuffer1=second;
        break;
    case 2:
        StoreImage(&garage_logo_vram_rect,garage_logo_state.image38[0]);
        StoreImage(&garage_logo_clut_vram_rect,garage_logo_state.palette48[0]);
        clear=garage_logo_state.image40[0];
        for(i=511;i>=0;i--)*clear++=0;
        break;
    case 3:
        garage_arena_init_stage=0;
        garage_save_undo_copy();
        garage_logo_state.dirtyD3=0;
        ClearScreenOverlayState();
        InitBadgeRow(0,12,304,296,88,0,0);
        InitUiRow(0,140,136,28,32,0,0,1,0,104);
        InitUiRow(1,140,176,28,32,0,0,1,0,105);
        ModeGroup=stage;
        ModeId=4;
        ModeSubState=0;
        break;
    }
}
