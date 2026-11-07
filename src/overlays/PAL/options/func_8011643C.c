#include "rr/overlay665_render.h"
#ifndef RR_PREVIEW_RENDER_SCRATCH
#define RR_PREVIEW_RENDER_SCRATCH ((RrPreviewRenderScratch *)0x1F800000)
#endif

void options_func_8011643C(void) {
    RrPreviewRenderScratch *scratch = RR_PREVIEW_RENDER_SCRATCH;
    RrPreviewMatrixWorkspace sp10;
    RrPreviewMatrixWorkspace sp30;
    s32 scale[3];
    s32 temp_s6;
    s32 var_a1;
    s32 var_a1_2;
    s32 var_a1_3;
    s32 var_a1_4;
    s32 var_a1_5;
    s32 var_a1_6;
    RrPreviewLighting40 *temp_s0;
    RrPreviewLighting40 *temp_s0_2;
    RrPreviewLighting40 *temp_s0_3;
    RrPreviewLighting40 *temp_s0_4;
    RrPreviewView44 *temp_s1;
    RrPreviewView44 *temp_s1_2;
    RrPreviewView44 *temp_s4;
    RrPreviewView44 *temp_s5;
    RrPreviewView44 *temp_s5_2;

    temp_s6 = scratch->depth60;
    scratch->depth60 = 3;
    scratch->projection.word10 = 0;
    scratch->projection.word0C = 0;
    scratch->projection.word08 = 0;
    scratch->projection.word20 = 0;
    scratch->projection.word1C = 0;
    scratch->projection.word18 = 0;
    BuildCameraMatrices();
    SetupViewport(6);
    SetGeomOffset((s16)options_D_80119768.centerX38, (s16)options_D_80119768.centerY3A);
    ScaleMatrixRow1(2, scratch->matrix28.halves);
    switch (options_D_80119768.kind18) {
    case 0:
            temp_s1 = &options_D_80119768.view74;
            temp_s0 = &options_D_80119768.lighting4C;
            scale[2] = options_D_80119768.scale3C;
            scale[1] = options_D_80119768.scale3C;
            scale[0] = options_D_80119768.scale3C;
            options_D_80119768.view74.rotation10 = (u16) options_D_80119768.rotation44;
            options_D_80119768.view74.position00.words[2] = (s32) options_D_80119768.position48;
            SetupObjectLightMatrices((char *)temp_s1, (char *)temp_s0, &sp10.matrix, &sp30.matrix);
            ScaleMatrix(sp10.words, scale);
            SetGeomScreen(options_D_80119768.depth40);
            options_func_80116358(&sp10, &temp_s0->matrix);
            SelectModelBank(8);
            SetGteObjectTransform(scratch->tail7C.projection.halves, temp_s1->position00.halves, (s32 *)sp10.words);
            var_a1 = 1;
            scratch->counter78 = 0;
            SubmitModelTwoTri(scratch, var_a1, 0, 0);
            goto block_33;
    case 1:
    temp_s1_2 = &options_D_80119768.view74;
    temp_s0_4 = &options_D_80119768.lighting4C;
    scale[2] = options_D_80119768.scale3C;
    scale[1] = options_D_80119768.scale3C;
    scale[0] = options_D_80119768.scale3C;
    options_D_80119768.view74.rotation10 = (u16) options_D_80119768.rotation44;
    options_D_80119768.view74.position00.words[2] = (s32) options_D_80119768.position48;
    SetupObjectLightMatrices((char *)temp_s1_2, (char *)temp_s0_4, &sp10.matrix, &sp30.matrix);
    ScaleMatrix(sp10.words, scale);
    SetGeomScreen(options_D_80119768.depth40);
    options_func_80116358(&sp10, &temp_s0_4->matrix);
    SelectModelBank(8);
    SetGteObjectTransform(scratch->tail7C.projection.halves, temp_s1_2->position00.halves, (s32 *)sp10.words);
    scratch->counter78 = 0;
    var_a1 = ModelBankFormat < 1;
SubmitModelTwoTri(scratch, var_a1, 0, 0);
 goto block_33;

    case 2:
temp_s4 = &options_D_80119768.view74;
        temp_s0_3 = &options_D_80119768.lighting4C;
        scale[2] = options_D_80119768.scale3C;
        scale[1] = options_D_80119768.scale3C;
        scale[0] = options_D_80119768.scale3C;
        options_D_80119768.view74.rotation10 = (u16) options_D_80119768.rotation44;
        options_D_80119768.view74.position00.words[2] = (s32) options_D_80119768.position48;
        SetupObjectLightMatrices((char *)temp_s4, (char *)temp_s0_3, &sp10.matrix, &sp30.matrix);
        ScaleMatrix(sp10.words, scale);
        SetGeomScreen(options_D_80119768.depth40);
        SelectModelBank(8);
        BuildRotMatrixX(sp30.halves, options_D_80119768.view74.rotation18);
        MulMatrix2((s32 *)sp10.words, sp30.halves);
        options_func_80116358(&sp30, &temp_s0_3->matrix);
        SetGteObjectTransform(RR_PREVIEW_RENDER_SCRATCH->tail7C.projection.halves, temp_s4->position00.halves, (s32 *)sp30.words);
        var_a1_4 = 1;
        scratch->counter78 = 0;
        if (ModelBankFormat >= 3) {
            var_a1_4 = 2;
        }
        SubmitModelTwoTri(RR_PREVIEW_RENDER_SCRATCH, var_a1_4, 0, 0);
        if (options_D_80119768.layout0C >= 2) {
            if (options_D_80119768.detail10 >= 2) {
                SetGteObjectTransform(RR_PREVIEW_RENDER_SCRATCH->tail7C.projection.halves, temp_s4->position00.halves, (s32 *)sp30.words);
                var_a1_5 = 1;
                scratch->counter78 = 0;
                if (ModelBankFormat >= 7) {
                    var_a1_5 = 6;
                }
                SubmitModelTwoTri(RR_PREVIEW_RENDER_SCRATCH, var_a1_5, 0, 0);
            }
        }
        BuildRotMatrixX(sp30.halves, options_D_80119768.view74.rotation20);
        MulMatrix2((s32 *)sp10.words, sp30.halves);
        options_func_80116358(&sp30, &options_D_80119768.lighting4C.matrix);
        temp_s5_2 = &options_D_80119768.view74;
        SetGteObjectTransform(RR_PREVIEW_RENDER_SCRATCH->tail7C.projection.halves, temp_s5_2->position00.halves, (s32 *)sp30.words);
        var_a1_6 = 1;
        RR_PREVIEW_RENDER_SCRATCH->counter78 = 0;
        if (ModelBankFormat >= 4) {
            var_a1_6 = 3;
        }
        SubmitModelTwoTri(RR_PREVIEW_RENDER_SCRATCH, var_a1_6, 0, 0);
        if (options_D_80119768.layout0C >= 2) {
            if (options_D_80119768.detail10 >= 2) {
                SetGteObjectTransform(RR_PREVIEW_RENDER_SCRATCH->tail7C.projection.halves, temp_s5_2->position00.halves, (s32 *)sp30.words);
                var_a1 = 1;
                RR_PREVIEW_RENDER_SCRATCH->counter78 = 0;
                if (ModelBankFormat >= 8) {
                    var_a1 = 7;
                }
                SubmitModelTwoTri(RR_PREVIEW_RENDER_SCRATCH, var_a1, 0, 0);
            goto block_33;
            }
        }
        goto block_33;
    case 3:
temp_s5 = &options_D_80119768.view74;
            temp_s0_2 = &options_D_80119768.lighting4C;
            scale[2] = options_D_80119768.scale3C;
            scale[1] = options_D_80119768.scale3C;
            scale[0] = options_D_80119768.scale3C;
            options_D_80119768.view74.rotation10 = (u16) options_D_80119768.rotation44;
            options_D_80119768.view74.position00.words[2] = (s32) options_D_80119768.position48;
            SetupObjectLightMatrices((char *)temp_s5, (char *)temp_s0_2, &sp10.matrix, &sp30.matrix);
            ScaleMatrix(sp10.words, scale);
            SetGeomScreen(options_D_80119768.depth40);
            options_func_80116358(&sp10, &temp_s0_2->matrix);
            SelectModelBank(8);
            SetGteObjectTransform(scratch->tail7C.projection.halves, temp_s5->position00.halves, (s32 *)sp10.words);
            var_a1_2 = 1;
            scratch->counter78 = 0;
            if (ModelBankFormat >= 5) {
                var_a1_2 = 4;
            }
            SubmitModelTwoTri(scratch, var_a1_2, 0, 0);
            BuildRotMatrixZ((s16 *)sp30.halves, options_D_80119768.view74.rotation1C + 0x400);
            MulMatrix2((s32 *)sp10.words, sp30.halves);
            options_func_80116358(&sp30, &temp_s0_2->matrix);
            SetGteObjectTransform(scratch->tail7C.projection.halves, temp_s5->position00.halves, (s32 *)sp30.words);
            var_a1_3 = 1;
            scratch->counter78 = 0;
            if (ModelBankFormat >= 6) {
                var_a1_3 = 5;
            }
            SubmitModelTwoTri(scratch, var_a1_3, 0, 0);
            if (options_D_80119768.layout0C >= 2) {
                if (options_D_80119768.detail10 >= 2) {
                    if (options_D_80119768.detail10 < 4) {
                        SetGteObjectTransform(scratch->tail7C.projection.halves, temp_s5->position00.halves, (s32 *)sp30.words);
                        var_a1 = 1;
                        scratch->counter78 = 0;
                        if (ModelBankFormat >= 9) {
                            var_a1 = 8;
                        }
                        SubmitModelTwoTri(scratch, var_a1, 0, 0);
                        goto block_33;
                    }
                }
            }
            goto block_33;
    default:scratch->depth60=temp_s6;return;
    }
block_33:
    scratch->depth60 = temp_s6;
}
