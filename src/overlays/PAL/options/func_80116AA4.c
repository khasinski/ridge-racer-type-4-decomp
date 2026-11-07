#include "rr/options_strokes.h"

void options_draw_button_guides(const RrStrokeStep4 *const *paths,s32 factor) {
    u16 xy[4];
    RrStrokeColor4 color=options_button_guide_line_color;
    const RrStrokeStep4 *step;
    s32 sum,limit,n,i;

    if (factor) {
        for (i=0;i<5;i++) {
            xy[0]=options_button_guide_origins[i][0];
            xy[1]=options_button_guide_origins[i][1];
            step=paths[i];
            sum=0;
            while (step->direction!=-1) {
                sum=(s32)((u32)sum+(u32)(s32)step->length);
                step++;
            }
            limit=(s32)((u32)sum*(u32)factor)/16;
            step=paths[i];
            sum=0;
            while (step->direction!=-1) {
                if (limit>=(s32)((u32)sum+(u32)(s32)step->length)) {
                    n=step->length;
                    sum=(s32)((u32)sum+(u32)(s32)step->length);
                } else {
                    n=(s32)((u32)limit-(u32)sum);
                    sum=(s32)((u32)sum+(u32)n);
                }
                switch (step->direction) {
                case 0:
                    xy[2]=xy[0];
                    xy[3]=(u32)xy[1]-(u32)n;
                    AddLineF2Prim((const s16 *)xy,color.bytes,0,0);
                    break;
                case 1:
                    xy[2]=xy[0];
                    xy[3]=(u32)xy[1]+(u32)n;
                    AddLineF2Prim((const s16 *)xy,color.bytes,0,0);
                    break;
                case 2:
                    xy[2]=(u32)xy[0]-(u32)n;
                    xy[3]=xy[1];
                    AddLineF2Prim((const s16 *)xy,color.bytes,0,0);
                    xy[1]++;
                    xy[3]++;
                    AddLineF2Prim((const s16 *)xy,color.bytes,0,0);
                    break;
                case 3:
                    xy[2]=(u32)xy[0]+(u32)n;
                    xy[3]=xy[1];
                    AddLineF2Prim((const s16 *)xy,color.bytes,0,0);
                    xy[1]++;
                    xy[3]++;
                    AddLineF2Prim((const s16 *)xy,color.bytes,0,0);
                    break;
                }
                xy[0]=xy[2];
                xy[1]=xy[3];
                step++;
            }
        }
    }
}
