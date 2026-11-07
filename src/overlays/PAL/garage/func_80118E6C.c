#include "rr/overlay662_car_choice.h"

void garage_func_80118E6C(void) {
    s16 v0 = CourseChoiceTable[1][garage_D_801272E4].first;
    s16 v1;
    garage_D_801272BC = v0;
    v1 = CourseChoiceTable[1][garage_D_801272E4].second;
    PlayerCourseChoices[0].first = v0;
    garage_D_801272E2 = v1;
    PlayerCourseChoices[0].second = v1;
    garage_func_80118ED8();
}
