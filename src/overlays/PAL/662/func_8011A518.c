#include "rr/overlay662_arena_init.h"

void ovl662_func_8011A518(void)
{
    s32 stage,i;
    register u8 *base __asm__("$3");
    u32 *clear;
    register u8 *second __asm__("$6");
    RrGarageSelectionPrefix *owner;
    register u8 *allocated __asm__("$2");
    stage=(s32)((u32)ovl662_D_8012625C+1U);
    ovl662_D_8012625C=stage;
    switch (stage) {
    case 1:
        ovl662_func_8011B2D4(-1,0,0,0);
        ovl662_func_8011E36C(-1);
        ovl662_func_8011D278(-1);
        ovl662_func_8011A414();
        base=LoadCursorBase+4;
        owner=&ovl662_D_801276A8;
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
        D_800F3918=base;
        owner->buffers24[0]=(u32*)base;
        owner->tim2C[0]=(RrGarageTimBytes*)second;
        D_800F3910=second;
        break;
    case 2:
        StoreImage(&ovl662_D_8012624C,ovl662_D_801276A8.image38[0]);
        StoreImage(&ovl662_D_80126254,ovl662_D_801276A8.palette48[0]);
        clear=ovl662_D_801276A8.image40[0];
        for(i=511;i>=0;i--)*clear++=0;
        break;
    case 3:
        ovl662_D_8012625C=0;
        ovl662_func_8011D120();
        ovl662_D_801276A8.dirtyD3=0;
        ClearScreenOverlayState();
        InitBadgeRow(0,12,304,296,88,0,0);
        InitUiRow(0,140,136,28,32,0,0,1,0,104);
        InitUiRow(1,140,176,28,32,0,0,1,0,105);
        ModeGroup=stage;
        ModeId=4;
        D_800F4D8A=0;
        break;
    }
}
