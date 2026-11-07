#ifndef RR_GAME_COURSE_TRKTEXT_H
#define RR_GAME_COURSE_TRKTEXT_H

#include "types.h"

int GetMappedPadButtons(int i);
int DrawFontStringCentered(int a0, int a1, unsigned char *s, int a3, int a4);
int DrawFontStringCenteredOffset(int a0, int a1, unsigned char *s, int a3, int a4);
void DrawFontStringRight(int a0, int a1, unsigned char *s, int a3, int a4);
void AddSetTexpagePrim(int a0);
void AddSlantedGradientRect(unsigned short *a0, int a1, unsigned char *a2, unsigned char *a3, int a4);

#endif /* RR_GAME_COURSE_TRKTEXT_H */
