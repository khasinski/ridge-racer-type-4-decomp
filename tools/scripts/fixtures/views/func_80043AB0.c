#include "rr/track_interpolation.h"

/* legacy typing of shared globals in this part of the unit */
#define PsyqStrcpy ((void (*)())PsyqStrcpy)

#define PasswordBytes (*(unsigned char (*)[])&PasswordBytes)

#undef PsyqStrcpy

#undef PasswordBytes

/* legacy typing of shared globals in this part of the unit */
#define BuildRotMatrixZ ((void (*)())BuildRotMatrixZ)

#define BuildRotMatrixY ((void (*)())BuildRotMatrixY)

#define BuildRotMatrixX ((void (*)())BuildRotMatrixX)

#define MulMatrix ((void (*)())MulMatrix)

#define MulMatrix2 ((void (*)())MulMatrix2)

#define ApplyMatrix ((void (*)())ApplyMatrix)

#undef BuildRotMatrixZ

#undef BuildRotMatrixY

#undef BuildRotMatrixX

#undef MulMatrix

#undef MulMatrix2

#undef ApplyMatrix

void GetTrackPointAhead(RrRaceVehicle *vehicle, s32 *output, s32 distance) {
    s16 current_y;
    s32 fraction;
    s32 index, length;
    /* ASM_BARRIER: use v1 for the PAL count page before loading vehicle fields. */
    register u8 *page __asm__("$3");
    s32 neighbour;
    RrTrackSample3C *next;
    RrTrackSample3C *current;

    if (ReverseCourse == 0) {
        index = vehicle->trackNodeF0 - ((s32) (distance - vehicle->trackFraction170) >> 0xA);
        neighbour = index - 1;
        if (index < 0) {
            index += TrackNodeCount;
            neighbour = index - 1;
        }
        if (neighbour < 0) {
            neighbour += TrackNodeCount;
        }
    } else {
        page=(u8*)0x800B0000U;
        /* ASM_BARRIER: preserve the page base for the signed count displacement. */
        __asm__("" : "+r"(page));
        length=*(s32*)(page-0x4134);
        index = vehicle->trackNodeF0 + ((s32) (distance - vehicle->trackFraction170) >> 0xA);
        neighbour = index + 1;
        if (index >= length) {
            index -= length;
            neighbour = index + 1;
        }
        /* This signed test is present in the original, including negative indices. */
        if (neighbour >= 0) {
            neighbour -= length;
        }
    }
    current = TrackNodes + index;
    next = TrackNodes + neighbour;
    fraction = (distance - vehicle->trackFraction170) & 0x3FF;
    output[0] = (s32) ((((s32) ((next->x08 - current->x08) * fraction) / 1024) + current->x08) * 4);
    current_y = current->y10;
    output[1] = (s32) (((s32) ((next->y10 - current_y) * fraction) / 1024) + current_y);
    output[2] = (s32) ((((s32) ((next->z0C - current->z0C) * fraction) / 1024) + current->z0C) * 4);
}
