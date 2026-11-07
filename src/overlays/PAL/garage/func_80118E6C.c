#include "rr/overlay662_car_choice.h"

void garage_enter_car_select(void) {
    s16 v0 = CourseChoiceTable[1][garage_car_index].first;
    s16 v1;
    garage_D_801272BC = v0;
    v1 = CourseChoiceTable[1][garage_car_index].second;
    PlayerCourseChoices[0].first = v0;
    garage_clut_choice = v1;
    PlayerCourseChoices[0].second = v1;
    garage_init_car_select();
}
