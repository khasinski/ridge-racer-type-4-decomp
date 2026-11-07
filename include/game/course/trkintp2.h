#ifndef RR_GAME_COURSE_TRKINTP2_H
#define RR_GAME_COURSE_TRKINTP2_H

#include "types.h"

extern unsigned char PasswordCipherKey[];
int FindNearestTrackEntry();
void SetCarAttachedCamera();
void CarLocalToWorld(char *a, int b, int *c);

#endif /* RR_GAME_COURSE_TRKINTP2_H */
