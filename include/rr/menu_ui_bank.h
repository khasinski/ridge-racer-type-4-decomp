#ifndef RR_MENU_UI_BANK_H
#define RR_MENU_UI_BANK_H
#include "rr/menu_setup.h"
/* Resident3A1B4 selects40 rows;3E8D4 allocates count*36 bytes.
 * Byte-indexed cell operations may cross row boundaries within this owner. */
typedef union {
    RrUiRow24 rows[40];
    u8 bytes[0x5A0];
    struct {u8 prefix00[12];u8 cells0C[0x594];} cell_view;
} RrMenuUiBank;
RR_ASSERT_SIZE(RrMenuUiBank,0x5A0);
/* D_F4D80 must address row0 of a complete menu bank, with four glyph rows.
 * The choice byte may span0..255; sound may replace both roots and any
 * signed16 selection before the moved choice is clamped to0..3. */
void garage_func_80116690(void);
/* Select the second choice byte; retains glyph flags and publishes selection+4. */
void garage_func_80116808(void);
#endif
