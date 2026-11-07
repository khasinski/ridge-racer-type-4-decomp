/* CC1_FLAGS: -fno-strength-reduce */
#include "rr/overlay665_matrix.h"

void options_set_preview_light_matrix(const RrPreviewMatrixWorkspace *arg0, const RrPreviewLightInput36 *input) {
    RrPreviewMatrixWorkspace mat[2];
    s32 i;
    u16 *dst1,*dst2;
    const u16 *src2;
    const u8 *arg1=input->bytes;
    i=0;
    dst2=mat[1].halves;
    mat[0]=*arg0;
    dst1=LightMatrix.halves;
    src2=input->halves;
    do {
        dst1[0]=src2[6];
        dst1[1]=src2[7];
        dst1[2]=src2[8];
        i++;
        src2+=4;
        dst2[0]=arg1[0]<<4;
        dst1+=3;
        dst2[3]=arg1[1]<<4;
        dst2[6]=arg1[2]<<4;
        arg1+=4;
        dst2++;
    } while (i<3);
    MulMatrix2((s32 *)LightMatrix.words,mat[0].halves);
    SetLightMatrix((s32 *)mat[0].words);
}
