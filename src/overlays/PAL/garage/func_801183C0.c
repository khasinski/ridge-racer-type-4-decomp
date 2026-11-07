#include "rr/overlay_extra_labels.h"
#include "rr/badge_rows.h"

extern u8 garage_slot_table_column_offsets[4];
extern u8 garage_slot_table_row_offsets[4];
extern u8 garage_slot_table_header_glyphs[3];
extern s32 garage_D_801272F0[3];
extern s16 SaveSlotIndex;
extern RrUiUvRect UiFontGlyphs[];
void FormatDecimalDigits(char *output, s32 value, s32 digitsMinusOne);
void CopyStringClearTail(char *destination, char *source);
char *garage_format_time_mm_ss(s32 value);
void *AddTileSemiTransPrim(u32 *, void *, s32, s32, s32, s32, s32, s32, s32);

void *garage_draw_slot_times_table(u32 *ord, void *cur, s32 row)
{
    /* Only slot [1] is used; the array keeps the value in the frame at sp+44. */
    s32 time[2];
    s32 j;
    char text[8];
    s32 i;
    s32 width;
    char *p;
    s32 x, y;

    x = BadgeRows[row].x04;
    y = BadgeRows[row].y06;
    y += 32;
    width = BadgeRows[row].width0C;
    for (i = 0; i < 3; i++) {
        x += garage_slot_table_column_offsets[i];
        cur = AddSpritePrim(ord, cur, x, y, &UiFontGlyphs[garage_slot_table_header_glyphs[i]], 0x7FF8, 0);
    }
    for (i = 7, p = text + 7; i >= 0; i--) {
        *p-- = 0;
    }
    y -= 8;
    for (i = 0; i < 3; i++) {
        x = BadgeRows[row].x04 + 8;
        FormatDecimalDigits(text, i + 1, 0);
        text[1] = 0;
        cur = AddTextSprites(ord, cur, x, y + garage_slot_table_row_offsets[i + 1], text, 0x7FF8, 0, 0);
        time[1] = garage_D_801272F0[i];
        if (time[1] != -1) {
            x += garage_slot_table_column_offsets[1];
            cur = AddSpritePrim(ord, cur, x, y + garage_slot_table_row_offsets[i + 1], &UiFontGlyphs[42], 0x7FF8, 0);
            text[0] = i + 'A';
            text[1] = 0;
            cur = AddTextSprites(ord, cur, x + (UiFontGlyphs[42].w + 2), y + garage_slot_table_row_offsets[i + 1], text, 0x7FF8, 0, 0);
            CopyStringClearTail(text, garage_format_time_mm_ss(time[1]));
            cur = AddTextSprites(ord, cur, x + garage_slot_table_column_offsets[2], y + garage_slot_table_row_offsets[i + 1], text, 0x7FF8, 1, 0);
        } else {
            cur = AddTilePrim(ord, cur, x + garage_slot_table_column_offsets[1], y + garage_slot_table_row_offsets[i + 1] + 8, 36, 2, 255, 255, 255);
        }
        if (i == SaveSlotIndex) {
            cur = AddTileSemiTransPrim(ord, cur, BadgeRows[row].x04, y + garage_slot_table_row_offsets[i + 1] - 4, width, 24, 32, 32, 32);
        }
    }
    cur = AddDrawModePrim(ord, cur, 56);
    y += 28;
    x = BadgeRows[row].x04;
    cur = AddTilePrim(ord, cur, x, y, width, 2, 255, 255, 255);
    y -= 28;
    for (i = 0; i < 4; i++) {
        x = BadgeRows[row].x04;
        for (j = 0; j < 2; j++) {
            x += garage_slot_table_column_offsets[j + 1];
            cur = AddTilePrim(ord, cur, x, y + garage_slot_table_row_offsets[i], 2, 16, 255, 255, 255);
        }
    }
    return cur;
}
