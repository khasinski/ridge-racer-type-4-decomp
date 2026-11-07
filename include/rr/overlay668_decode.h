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
void movie_playback_stream_init(RrLibInterruptCallback callback);
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
extern RrStrQuantizationCommand movie_playback_dct_quant_command;
extern RrStrMatrixCommand movie_playback_dct_matrix_command;

/* Reset the resident callback driver for mode0, then dispatch the device mode. */
void movie_playback_dec_dct_reset(s32 mode);
void movie_playback_mdec_reset(s32 mode);

/* Forward copies of16,16,32 words. Output has room for64 words; when it
 * overlaps table storage, later reads observe earlier writes. Returns output. */
u32 *movie_playback_dec_dct_get_env(u32 *output);
/* Publish the first32 input words to quantization storage, then submit both
 * stored commands. Matrix storage stays live through the first submission.
 * Input may overlap the tables; returns the original full input pointer. */
u32 *movie_playback_dec_dct_put_env(u32 *input);
void movie_playback_mdec_in(const void *command,u32 word_count);
/* The first halfword of the little-endian command header. */
u32 movie_playback_dec_dct_buf_size(const RrStrCommandHeader *header);
/* Exchange interrupt callback channel1 and return its previous full pointer. */
RrLibInterruptCallback movie_playback_dec_dct_out_callback(RrLibInterruptCallback callback);

RR_ASSERT_SIZE(RrStrCommandHeader,4);
RR_ASSERT_SIZE(RrStrQuantizationCommand,132);
RR_ASSERT_OFFSET(RrStrQuantizationCommand,luma,4);
RR_ASSERT_OFFSET(RrStrQuantizationCommand,chroma,68);
RR_ASSERT_SIZE(RrStrMatrixCommand,132);
RR_ASSERT_OFFSET(RrStrMatrixCommand,matrix,4);
void movie_playback_shutdown(void);
#endif
