#include "types.h"

/* legacy typing of shared globals in this part of the unit */
#define D_800F4C38 (*(unsigned char (*)[])&D_800F4C38)

#undef D_800F4C38

extern short CourseIndex;

extern int func_80070934();

extern int ViewUsesCarTrackNode;

extern int D_800AD5B8;

extern char D_800F4C38[];

void func_8006E794(void) {
    int i;
    char *p;
    char *g = (char *)0x1F800000;
    InitViewLighting();
    ViewUsesCarTrackNode = 1;
    D_800AD5B8 = 0x1400;
    *(int *)(g + 0x78) = 0;
    *(short *)(g + 0x5E) = 0;
    if (CourseIndex == 7) {
        for (i = 0; i < 5; i++) {
            func_80070934(i, 6, 0, 0);
        }
    }
    i = 0x1F;
    p = D_800F4C38 + i;
    do {
        *p-- = 0;
        i--;
    } while (i >= 0);
}
