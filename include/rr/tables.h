#ifndef RR_TABLES_H
#define RR_TABLES_H

#include "rr/layout.h"

/* Partial entry layout for MenuStateTable (8-byte stride). */

typedef struct {
    char p[4];
    u8 i;
    char q[3];
} RrEntry800A4258;

/* Four-byte rows selected by RrEntry800A4258.i. The two tables use
 * different subsets of the row, so retain neutral field names. */
typedef struct {
    u8 a, b, c, d;
} RrMenuChoice;

/* Partial PlayerControllers view used by menu transitions (0x5C stride). */
typedef struct {
    u8 pad00,type01;
    u16 buttons02;
    u8 pad04[4];u16 buttons08,buttons0A;u8 pad0C[4];s32 status10;
    u8 axis14;
    s8 axis15;
    u8 axis16, axis17, pad18;
    u8 output19, output1A;
    u8 pad1B;
    s16 kind1C;
    u8 pad1E[0x28 - 0x1E];
    s32 a, b;
    s32 accept30;u8 pad34[8];s16 steering3C;u8 pad3E[2],flag40,pad41[0x1B];
} RrMenuItemState5C;

RR_ASSERT_SIZE(RrEntry800A4258, 0x8);
RR_ASSERT_OFFSET(RrEntry800A4258, i, 0x4);
RR_ASSERT_SIZE(RrMenuChoice, 4);
RR_ASSERT_SIZE(RrMenuItemState5C, 0x5C);
RR_ASSERT_OFFSET(RrMenuItemState5C, type01, 1);
RR_ASSERT_OFFSET(RrMenuItemState5C, buttons02, 2);
RR_ASSERT_OFFSET(RrMenuItemState5C, axis14, 0x14);
RR_ASSERT_OFFSET(RrMenuItemState5C, axis15, 0x15);
RR_ASSERT_OFFSET(RrMenuItemState5C, axis16, 0x16);
RR_ASSERT_OFFSET(RrMenuItemState5C, axis17, 0x17);
RR_ASSERT_OFFSET(RrMenuItemState5C, flag40, 0x40);
RR_ASSERT_OFFSET(RrMenuItemState5C, output19, 0x19);
RR_ASSERT_OFFSET(RrMenuItemState5C, output1A, 0x1A);
RR_ASSERT_OFFSET(RrMenuItemState5C, kind1C, 0x1C);
RR_ASSERT_OFFSET(RrMenuItemState5C,status10,0x10);
RR_ASSERT_OFFSET(RrMenuItemState5C,steering3C,0x3C);
RR_ASSERT_OFFSET(RrMenuItemState5C,buttons08,8);
RR_ASSERT_OFFSET(RrMenuItemState5C,buttons0A,10);
RR_ASSERT_OFFSET(RrMenuItemState5C,accept30,0x30);
RR_ASSERT_OFFSET(RrMenuItemState5C, a, 0x28);
RR_ASSERT_OFFSET(RrMenuItemState5C, b, 0x2C);

#endif
