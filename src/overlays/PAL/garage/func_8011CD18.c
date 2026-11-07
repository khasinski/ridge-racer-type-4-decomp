#include "rr/overlay662_palette_refresh.h"

void garage_func_8011CD18(void)
{
    RrPalette16 *palette, *backup, *scratch;
    s32 i;
    s32 value;
    register s32 pulse __asm__("$2");
    s32 adjusted0, adjusted1, adjusted2;
    s32 phase0, phase1, phase2;
    s32 angle1, angle2;
    u16 *source, *target;
    u16 color;

    palette = (RrPalette16 *)garage_D_801276A8.palette48[garage_D_801276A8.active21];
    backup = (RrPalette16 *)garage_D_801276A8.paletteBackup50[garage_D_801276A8.active21];
    scratch = (RrPalette16 *)garage_D_801276A8.paletteScratch58[garage_D_801276A8.active21];
    /* Address/lifetime metadata only; no instruction is emitted. */
    __asm__("" : : "m"(garage_D_801268C8));
    palette->entries[0] = 0;
    scratch->entries[0] = 0x8000;
    __asm__("" : : "m"(scratch->entries[0]));
    phase0 = garage_D_801268C8;
    adjusted0 = phase0;
    if (phase0 < 0) adjusted0 = (s32)((u32)phase0 + 4095U);
    pulse = rsin(phase0 - ((adjusted0 >> 12) * 4096));
    scratch->entries[0] |= (pulse / 128 + 32) >> 3;
    __asm__("" : : "m"(scratch->entries[0]));
    phase1 = garage_D_801268C8;
    angle1 = (s32)((u32)phase1 + 85U);
    adjusted1 = angle1;
    if (angle1 < 0) adjusted1 = (s32)((u32)angle1 + 4095U);
    pulse = rsin(angle1 - ((adjusted1 >> 12) * 4096));
    scratch->entries[0] |= ((pulse / 128 + 32) >> 3) << 5;
    __asm__("" : : "m"(scratch->entries[0]));
    phase2 = garage_D_801268C8;
    angle2 = (s32)((u32)phase2 + 170U);
    adjusted2 = angle2;
    if (angle2 < 0) adjusted2 = (s32)((u32)angle2 + 4095U);
    value = rsin(angle2 - ((adjusted2 >> 12) * 4096));
    i = 1;
    if (value < 0) value += 127;
    target = &scratch->entries[1];
    scratch->entries[0] |= (((value >> 7) + 32) >> 3) << 10;
    __asm__("" : : "m"(scratch->entries[0]));
    source = &palette->entries[1];
    garage_D_801268C8 = (s32)((u32)garage_D_801268C8 + 32U);
    do {
        i++;
        color = *source | 0x8000;
        *source = color;
        *target = color;
        target++;
        source++;
    } while (i < 16);
    backup->raw = palette->raw;
    LoadImage(&garage_D_801268A0, garage_D_801276A8.scratch34);
    LoadImage(&garage_D_801268A8, garage_D_801276A8.image38[garage_D_801276A8.active21]);
    LoadImage(&garage_D_801268B0, garage_D_801276A8.image40[garage_D_801276A8.active21]);
    LoadImage(&garage_D_801268B8, palette);
    LoadImage(&garage_D_801268C0, scratch);
}
