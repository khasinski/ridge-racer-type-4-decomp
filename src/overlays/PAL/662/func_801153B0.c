#include "rr/overlay662_car_choice.h"

void ovl662_func_801153B0(void) {
    SetDispMask(0);
    ovl662_D_801272E4 = 0;
    CopyStringClearTail(ovl662_D_801272C8, SaveProgress.prefix.progress.prefix00.fields.name);
    ovl662_func_8011512C();
    UnpackChoiceGlyph((s8*)SelectionRows.bytes, CourseChoiceTable[1][0].glyph);
    PlayerCourseChoices[0].first = CourseChoiceTable[1][ovl662_D_801272E4].first;
    PlayerCourseChoices[0].second = CourseChoiceTable[1][ovl662_D_801272E4].second;
    ModeGroup = 1;
    ModeId = 0x40;
    D_800F4D8A = 0;
    DebugPrintStub(ovl662_D_80114888);
}
