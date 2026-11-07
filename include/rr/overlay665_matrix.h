#ifndef RR_OVERLAY665_MATRIX_H
#define RR_OVERLAY665_MATRIX_H
#include "rr/matrix_workspace.h"

typedef struct {u8 red,green,blue,pad03;} RrPreviewColor4;
typedef struct {s16 x,y,z,pad06;} RrPreviewVector8;
typedef struct {
    RrPreviewColor4 colors[3];
    RrPreviewVector8 vectors[3];
} RrPreviewLightInputFields36;
typedef union {
    RrPreviewLightInputFields36 fields;
    u16 halves[18];
    s16 signedHalves[18];
    u8 bytes[36];
} RrPreviewLightInput36;
typedef struct {
    RrPreviewLightInput36 matrix;
    u8 ambient[4];
} RrPreviewLighting40;

/* Input is halfword-aligned; matrix source is word-aligned. Capture all32
 * source bytes before the ordered basis/color writes, including aliasing. */
void options_set_preview_light_matrix(const RrPreviewMatrixWorkspace *source,
    const RrPreviewLightInput36 *input);
RR_ASSERT_SIZE(RrPreviewColor4,4);
RR_ASSERT_SIZE(RrPreviewVector8,8);
RR_ASSERT_SIZE(RrPreviewLightInputFields36,36);
RR_ASSERT_OFFSET(RrPreviewLightInputFields36,vectors,12);
RR_ASSERT_SIZE(RrPreviewLightInput36,36);
RR_ASSERT_SIZE(RrPreviewLighting40,40);
RR_ASSERT_OFFSET(RrPreviewLighting40,ambient,36);
#endif
