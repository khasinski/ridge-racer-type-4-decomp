#include "rr/overlay659.h"

u32 race_results_module_id __attribute__((section(".race_results.header"))) = 10;

u8 race_results_finish_rank_limits[4]
    __attribute__((section(".race_results.finish_rank_limits"))) = {2, 1, 0, 0};

s32 race_results_result_labels[34] __attribute__((section(".race_results.result_labels"))) = {
    7701, 0,   4378, 3,    4,    -1, 269,  525, 781,  20, 1038, 7701, 0,  4378, 18, -1,   269,
    525,  781, 20,   1038, 7701, 5,  7701, 17,  7701, -1, 6,    24,   20, 4107, -1, 7701, -1};

s32 race_results_preview_timer __attribute__((section(".race_results.preview_timer"))) = 0;
