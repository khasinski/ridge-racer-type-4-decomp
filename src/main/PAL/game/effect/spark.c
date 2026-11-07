#include "game/effect/carfx.h"
#include "game/effect/carfx.h"
#include "game/effect/spark.h"
extern short SparkFrameRanges[];

void UpdateSparkEffects() {
    int a0, a1, lo, l1, l2, a2, a3;
    lo = SparkFrameRanges[0];
    a0 = *(int *)(SparkCarState + 0x104);
    if (a0 >= lo) {
        if (a0 < SparkFrameRanges[1]) {
            func_80057138(a0 - lo, SparkFrameRanges[1] - lo - 0x20, lo);
        }
    }
    l2 = SparkFrameRanges[2];
    a2 = *(int *)(SparkCarState + 0x104);
    if (a2 >= l2) {
        if (a2 < SparkFrameRanges[3]) {
            func_800572A8(a2 - l2, SparkFrameRanges[3] - l2 - 0x20);
        }
    }
    l1 = SparkFrameRanges[4];
    a1 = *(int *)(SparkCarState + 0x104);
    if (a1 >= l1) {
        if (a1 < SparkFrameRanges[5]) {
            DrawSparkHudOverlay(0, a1 - l1, SparkFrameRanges[5] - l1 - 0x20);
        }
    }
    l1 = SparkFrameRanges[6];
    a1 = *(int *)(SparkCarState + 0x104);
    if (a1 >= l1) {
        if (a1 < SparkFrameRanges[7]) {
            DrawSparkHudOverlay(1, a1 - l1, SparkFrameRanges[7] - l1 - 0x20);
        }
    }
    l1 = SparkFrameRanges[8];
    a1 = *(int *)(SparkCarState + 0x104);
    if (a1 >= l1) {
        if (a1 < SparkFrameRanges[9]) {
            DrawSparkHudOverlay(2, a1 - l1, SparkFrameRanges[9] - l1 - 0x20);
        }
    }
}
