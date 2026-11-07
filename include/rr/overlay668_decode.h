#ifndef RR_OVERLAY668_DECODE_H
#define RR_OVERLAY668_DECODE_H
#include "rr/library.h"

/* Stream state prefix: two display buffers followed by the queue header.
 * Pointer fields retain their native width outside the PAL build. */
typedef struct {
    u8 reserved00[0x10];
    void *commands10[2];
    void *buffers18[2];
    RrLibQueueRecord32 *queue20;
} RrStrStreamPrefix;
extern RrStrStreamPrefix *StrStreamState;
void ovl668_func_80114EE8(RrLibInterruptCallback callback);
RR_ASSERT_OFFSET(RrStrStreamPrefix, commands10, 0x10);
RR_ASSERT_OFFSET(RrStrStreamPrefix, buffers18, 0x18);
RR_ASSERT_OFFSET(RrStrStreamPrefix, queue20, 0x20);
RR_ASSERT_SIZE(RrStrStreamPrefix, 0x24);

typedef union {u32 word;u16 halves[2];} RrStrCommandHeader;
typedef struct {
    RrStrCommandHeader header;
    u32 luma[16],chroma[16];
} RrStrQuantizationCommand;
typedef struct {RrStrCommandHeader header;u32 matrix[32];} RrStrMatrixCommand;
extern RrStrQuantizationCommand ovl668_D_80116D10;
extern RrStrMatrixCommand ovl668_D_80116D94;

/* Reset the resident callback driver for mode0, then dispatch the device mode. */
void ovl668_func_80116298(s32 mode);
void ovl668_func_80116558(s32 mode);

/* Forward copies of16,16,32 words. Output has room for64 words; when it
 * overlaps table storage, later reads observe earlier writes. Returns output. */
u32 *ovl668_func_801162CC(u32 *output);
/* Publish the first32 input words to quantization storage, then submit both
 * stored commands. Matrix storage stays live through the first submission.
 * Input may overlap the tables; returns the original full input pointer. */
u32 *ovl668_func_80116358(u32 *input);
void ovl668_func_80116648(const void *command,u32 word_count);
/* The first halfword of the little-endian command header. */
u32 ovl668_func_801163F0(const RrStrCommandHeader *header);
/* Exchange interrupt callback channel1 and return its previous full pointer. */
RrLibInterruptCallback ovl668_func_80116534(RrLibInterruptCallback callback);

RR_ASSERT_SIZE(RrStrCommandHeader,4);
RR_ASSERT_SIZE(RrStrQuantizationCommand,132);
RR_ASSERT_OFFSET(RrStrQuantizationCommand,luma,4);
RR_ASSERT_OFFSET(RrStrQuantizationCommand,chroma,68);
RR_ASSERT_SIZE(RrStrMatrixCommand,132);
RR_ASSERT_OFFSET(RrStrMatrixCommand,matrix,4);
void ovl668_func_80114DF4(void);
#endif
