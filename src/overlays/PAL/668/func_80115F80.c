#include "rr/overlay668_replay_controller.h"
void ovl668_func_80115F80(void) {
    s32 temp_a1;
    s32 temp_s1;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v1;
    s32 var_a0;
    s32 var_s0;
RrReplayCameraContextPrefix *ctx;

    switch (RaceContext.counter444) {                          
    case 0:
        ovl668_func_80115DFC();
        goto increment;
    case 4:
        SetDispMask(1);
    case 1: case 2: case 3:
 increment:
        RaceContext.counter444=(u32)RaceContext.counter444+1;
        break;
    case 5:                                        
        var_a0 = -1;
        for(var_s0=0;ovl668_D_80116CA8[var_s0].key>=0;var_s0++){
          if(ovl668_D_80116CA8[var_s0].key==RaceContext.state44C){var_a0=ovl668_D_80116CA8[var_s0].action;break;}
        }
        switch (var_a0) {                          
        case 0:                                    
            RaceContext.phase4AC = 1;
            RaceContext.counter4B4 = 0;
            CdAudioBeginPlay(0);
            break;
        case 1:                                    
            RaceContext.mode4A8 = 2;
            break;
        }
        ovl668_func_80115554();
        ctx=&RaceContext;
        ctx->position450[0] = (s32) ((u32)ctx->position450[0] + (ctx->delta478[0] / 6));
        temp_a1 = ctx->delta478[2] / 6;
        ctx->position450[2] = (s32) ((u32)ctx->position450[2] + temp_a1);
        ovl668_func_80115460(ctx->position450);
        ovl668_func_801156FC();
        ovl668_func_80115850();
        ovl668_func_80115BD0(2);
        temp_s1 = ctx->mode4A8;
        switch (temp_s1) {                         
        case 1:                                    
            temp_v0 = (u32)ctx->counter4A4 + 4;
            ctx->counter4A4 = temp_v0;
            if (temp_v0 >= 0xFF) {
                ctx->counter4A4 = 0xFF;
                ctx->mode4A8 = 0;
}
            AddScreenFadeTiles(&CurrentDrawBuf->head70, (s32)(255U - (u32)ctx->counter4A4), 0x45);
            break;
        case 2:                                    
            temp_v0_2 = (u32)ctx->counter4A4 - 4;
            ctx->counter4A4 = temp_v0_2;
            if (temp_v0_2 <= 0) {
                ctx->counter4A4 = 0;
                ctx->mode4A8 = 0;
                CdAudioFade(0, 0);
                ModeGroup = (s16) temp_s1;
                ModeId = 3;
                ctx->flagB0 = 1;
                D_800F4D8A = 0;
            }
            AddScreenFadeTiles(&CurrentDrawBuf->head70, (s32)(255U - (u32)ctx->counter4A4), 0x45);
            break;
        }
        temp_v1 = RaceContext.phase4AC;
        if (temp_v1 != 0) {
            if (temp_v1 == 1) {
                var_s0 = 0;
                D_80127E90 = (u32)D_80127E90 - (u32)D_80127E94;
                for(;var_s0<62;var_s0++)ovl668_func_801159C0(var_s0,160);
            }
        }
        if ((PlayerControllers[0].buttons08 & 0x800) && (RaceContext.mode4A8 == 0)) {
            RaceContext.mode4A8 = 2;
            CdAudioFade(0, 0x15);
        }
        SetupPlayerView(0);
        DrawTrackSceneryForPlayer(0);
        RaceContext.state44C = (s32) ((u32)RaceContext.state44C + 1);
        ovl668_func_80115BD0(1);
        ovl668_func_80115BD0(0);
        /* fallthrough */
    default:                                       
        return;
    }
}
