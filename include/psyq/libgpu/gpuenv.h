#ifndef RR_PSYQ_LIBGPU_GPUENV_H
#define RR_PSYQ_LIBGPU_GPUENV_H

#include "types.h"

extern char StrStoreImage[];
extern char MoveImageNameStr[];
extern const char DrawOTagFmt[];
char *SetDefDrawEnv(char *p, int a, int b, int c, int d);
s32 SetGraphDebug(s32 enabled);
void ClearImage2(int a, int b, int c, int d);
unsigned *ClearOTagR(unsigned *a, int b);
int PutDrawEnv(int *a0);

#endif /* RR_PSYQ_LIBGPU_GPUENV_H */
