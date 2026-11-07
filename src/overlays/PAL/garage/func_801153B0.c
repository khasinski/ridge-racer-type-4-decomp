#include "rr/overlay662_car_choice.h"

void garage_enter_car_info(void) {
    SetDispMask(0);
    garage_car_index = 0;
    CopyStringClearTail(garage_player_name_copy, SaveProgress.prefix.progress.prefix00.fields.name);
    garage_init_car_info();
    UnpackChoiceGlyph((s8*)SelectionRows.bytes, CourseChoiceTable[1][0].glyph);
    PlayerCourseChoices[0].first = CourseChoiceTable[1][garage_car_index].first;
    PlayerCourseChoices[0].second = CourseChoiceTable[1][garage_car_index].second;
    ModeGroup = 1;
    ModeId = 0x40;
    ModeSubState = 0;
    DebugPrintStub(garage_car_info_scene_name);
}
