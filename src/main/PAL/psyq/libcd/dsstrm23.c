/* CC1_FLAGS: -fcall-used-$1 */
/* $at as the %hi() base: with $1 allocatable cc1 2.8.1 emits lui $1,%hi / sw %lo($1) and its reorg fills the jr/jal delay slot with the sw (see DECOMP_GUIDE "delay-slot store"). */
#include "psyq/libcd/dsstrm32.h"
#include "game/data/globals.h"
extern int StEndFrame;
void StSetMask(int a, int b, int c) { StMaskEnabled = a; StStartFrame = b; StEndFrame = c; }
