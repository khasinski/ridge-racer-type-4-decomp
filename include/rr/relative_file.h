#ifndef RR_RELATIVE_FILE_H
#define RR_RELATIVE_FILE_H
#include "rr/layout.h"
typedef union {u32 relative;u8 *absolute;} RrRelativeSlot;
typedef struct {u32 count;RrRelativeSlot first[3];u32 offsets[16];} RrRelativeHeader;
RR_ASSERT_SIZE(RrRelativeSlot,4);
RR_ASSERT_SIZE(RrRelativeHeader,0x50);
RR_ASSERT_OFFSET(RrRelativeHeader,first,4);
RR_ASSERT_OFFSET(RrRelativeHeader,offsets,0x10);
extern RrRelativeHeader *RelativeFileBase;
extern s32 RelativeFileCount;
extern RrRelativeSlot *SegmentViewTables;
extern u8 *D_800F391C,*RelFileViewRecords;
extern u8 *RelFileSection00;
extern u8 *RelFileSection01;
extern u8 *RelFileSection02;
extern u8 *RelFileOffsetPtr3;
extern u8 *RelFileOffsetPtr4;
extern u8 *RelFileSection05;
extern u8 *RelFileOffsetPtr6;
extern u8 *RelFileOffsetPtr7;
extern u8 *D_800FFC40;
extern u8 *RelFileOffsetPtr9;
extern u8 *RelFileOffsetPtr10;
extern u8 *RelFileSection11;
extern u8 *RelFileOffsetPtr12;
extern u8 *RelFileOffsetPtr13;
extern u8 *RelFileOffsetPtr14;
extern u8 *RelFileSection15;

/* PAL adds modulo32; signed offsets retain negative displacements on LP64. */
static __inline__ u8 *rr_relative_file_address(RrRelativeHeader *base,u32 offset){return (u8*)((long)(s32)offset+(unsigned long)base);}

/* Eight relocation slots per record; count multiplication wraps modulo32.
 * The writable image contains the header followed immediately by those slots.
 * Offset words add modulo32 on PAL; native views retain full pointer widths. */
void RelocateRelativeFile(RrRelativeHeader *image);
#endif
