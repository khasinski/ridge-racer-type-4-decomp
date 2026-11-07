#include "rr/options_panels.h"
void options_update_brightness_test(void) {
    s32 i,x;
    const RrUiUvRect *glyph;
    void *cursor;
    RrGpuSingleCommandPacket *packet;
    switch(options_menu_state.layout0C) {
    case 0:
        options_init_brightness_test();
        SetMotionControllerVisible(0);
        options_menu_state.layout0C=(s32)((u32)options_menu_state.layout0C+1U);
        break;
    case 1:
        if(BadgeRows[0].state10==3)options_menu_state.layout0C=2;
        break;
    case 2:
        if(MenuConfirmPressed) {
            PlaySfx(1);
            BadgeRows[0].state10=4;
            MenuFadeTimer=64;
            options_menu_state.layout0C=(s32)((u32)options_menu_state.layout0C+1U);
        }
        break;
    case 3:
        MenuFadeTimer--;
        AddScreenFadeTiles(((RrFrameOrderingPrefix*)CurrentDrawBuf)->ordering70,(64-MenuFadeTimer)*4,69);
        if(MenuFadeTimer<=0 && BadgeRows[0].state10==11) {
            options_init_main_menu();
            options_menu_state.layout0C=0;
            options_menu_state.scene00=(s32)((u32)options_menu_state.scene00-1U);
            return;
        }
        break;
    }
    cursor=RR_PROJECTION_SCRATCH->pointers.cursor;
    i=0;
    glyph=&options_brightness_swatch_uv;
    x=24;
    for(;i<8;x+=36,i++) {
        cursor=AddSpritePrim(((RrFrameOrderingPrefix*)CurrentDrawBuf)->ordering70,cursor,x,144,glyph,0x7F38,((7-i)*223)/7+32);
    }
    packet=(RrGpuSingleCommandPacket*)cursor;
    RR_PROJECTION_SCRATCH->pointers.cursor=(u8*)packet;
    ((u8*)&packet->tag)[3]=1;
    {
        RrFrameOrderingPrefix *context=(RrFrameOrderingPrefix*)CurrentDrawBuf;
        packet->command=0xE1000239U;
        packet->tag=(packet->tag&0xFF000000U)|(context->ordering70[0]&0xFFFFFFU);
        context->ordering70[0]=(context->ordering70[0]&0xFF000000U)|((u32)(unsigned long)packet&0xFFFFFFU);
        packet++;
        RR_PROJECTION_SCRATCH->pointers.cursor=(u8*)packet;
    }
    for(i=0;i<8;i++) {
        DrawHudDigit(0,28+i*36,192,128);
        DrawHudDigit(i+1,34+i*36,192,128);
    }
    packet=(RrGpuSingleCommandPacket*)RR_PROJECTION_SCRATCH->pointers.cursor;
    ((u8*)&packet->tag)[3]=1;
    {
        RrFrameOrderingPrefix *context=(RrFrameOrderingPrefix*)CurrentDrawBuf;
        packet->command=0xE1000238U;
        packet->tag=(packet->tag&0xFF000000U)|(context->ordering70[0]&0xFFFFFFU);
        context->ordering70[0]=(context->ordering70[0]&0xFF000000U)|((u32)(unsigned long)packet&0xFFFFFFU);
        packet++;
        RR_PROJECTION_SCRATCH->pointers.cursor=(u8*)packet;
    }
    DialogueState->b4=27;
    UpdateBadgeRowAnim(0);
    if(BadgeRows[0].state10==3) {
        UpdateDialogue(BadgeRows[0].x04+10,BadgeRows[0].y06+40,DialogueState);
    }
    UpdateScreenFade(0x1007C);
}
