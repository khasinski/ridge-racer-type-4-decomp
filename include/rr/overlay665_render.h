#ifndef RR_OVERLAY665_RENDER_H
#define RR_OVERLAY665_RENDER_H
#include "rr/overlay665_visual.h"
#include "rr/renderer_setup.h"

#include "rr/overlay665_render_types.h"

extern s32 ModelBankFormat;
void BuildCameraMatrices(void);
void SetupViewport(s32);
void SetGeomOffset(s32 x,s32 y);
void ScaleMatrixRow1(s32 scale,void *matrix);
void SetupObjectLightMatrices(char *view,char *lighting,
    RrMatrixWords32 *first,RrMatrixWords32 *second);
u32 *ScaleMatrix(u32 *matrix,s32 scale[3]);
void SetGeomScreen(s32);
void SelectModelBank();
void SetGteObjectTransform(s16 *output,s16 *view,s32 *matrix);
void BuildRotMatrixZ(s16 *matrix,s32 angle);
void BuildRotMatrixX(void *matrix,s32 angle);
void SubmitModelTwoTri(RrPreviewRenderScratch *scratch,s32 object,s32 blend,s32 depth);
void options_func_8011643C(void);
#endif
