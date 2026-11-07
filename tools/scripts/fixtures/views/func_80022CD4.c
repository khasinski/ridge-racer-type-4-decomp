#include "m2c_macros.h"

#define H(p, o) (*(unsigned short *)((char *)(p) + (o)))

#define W(b, o) (*(int *)((char *)(b) + (o)))

#define H(b, o) (*(short *)((char *)(b) + (o)))

#define UH(b, o) (*(unsigned short *)((char *)(b) + (o)))

void ApplyRaceStartLaunch(RrRaceVehicle *arg0) {
    s32 temp_a2;
    void *temp_a1;

    temp_a1 = (u8 *)arg0 + 0x1F4;
    M2C_FIELD(temp_a1, s32 *, 0x74) = 0;
    temp_a2 = (s32)((u32)M2C_FIELD(temp_a1, s32 *, 0x88) - 0x1964U);
    if (M2C_FIELD(temp_a1, s16 *, 0x86) >= 2) {
        M2C_FIELD(temp_a1, s32 *, 0xA0) = 0;
    }
    if (temp_a2 < 0) {
        M2C_FIELD(temp_a1, s32 *, 0x74) = (s32) (temp_a2 >> 2);
        M2C_FIELD(temp_a1, s32 *, 0xA0) = (s32) ((s32)((u32)M2C_FIELD(temp_a1, s32 *, 0xA0) * 7U) / 10);
        return;
    }
    M2C_FIELD(temp_a1, s32 *, 0x74) = temp_a2;
    M2C_FIELD(temp_a1, s32 *, 0x88) = (s32) ((u32)M2C_FIELD(temp_a1, s32 *, 0x88) - (u32)temp_a2);
    M2C_FIELD(temp_a1, s32 *, 0xA0) = (s32) ((u32)M2C_FIELD(temp_a1, s32 *, 0xA0) - (u32)temp_a2);
}
