#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define ViewSegmentIndices (*(unsigned char (*)[])&ViewSegmentIndices)

#undef ViewSegmentIndices

extern short CourseIndex;

extern int ConfigureTrackObjectParticles();

extern int ViewUsesCarTrackNode;

extern int DefaultSegmentOctantValue;

extern char ViewSegmentIndices[];

void InitViewState(void) {
    int i;
    char *p;
    char *g = (char *)0x1F800000;
    InitViewLighting();
    ViewUsesCarTrackNode = 1;
    DefaultSegmentOctantValue = 0x1400;
    *(int *)(g + 0x78) = 0;
    *(short *)(g + 0x5E) = 0;
    if (CourseIndex == 7) {
        for (i = 0; i < 5; i++) {
            ConfigureTrackObjectParticles(i, 6, 0, 0);
        }
    }
    i = 0x1F;
    p = ViewSegmentIndices + i;
    do {
        *p-- = 0;
        i--;
    } while (i >= 0);
}
