#ifndef RR_OVERLAY662_TIM_H
#define RR_OVERLAY662_TIM_H
#include "rr/overlay662_garage.h"
#include "rr/overlay662_pixels.h"
#include "rr/palette16.h"
typedef struct {u32 header[5];RrPalette16 palette;u32 imageHeader[3];u32 pixels[512];} RrGarageTimLayout;
typedef union {RrGarageTimBytes file;u32 words[528];RrGarageTimLayout layout;} RrGarageTimImage;
extern RrGarageTimImage SaveDataBackup;
/* Select side0/1. Source TIM and destination are complete2112-byte owners;
 * struct copy requires disjoint owners or the exact same owner. Palette is a
 * genuine32-byte RrPalette16 owner. Image input covers512 words and may alias
 * the destination; compositor reads observe all preceding TIM/header writes. */
void ovl662_func_80126044(void);
RR_ASSERT_SIZE(RrGarageTimBytes,0x840);
RR_ASSERT_SIZE(RrGarageTimImage,0x840);
RR_ASSERT_SIZE(RrGarageTimLayout,0x840);
RR_ASSERT_OFFSET(RrGarageTimLayout,palette,0x14);
RR_ASSERT_OFFSET(RrGarageTimLayout,pixels,0x40);
#endif
