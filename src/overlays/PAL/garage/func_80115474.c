#include "rr/overlay662_transition.h"

void garage_reload_car_preview(void)
{
    ResetScreenCounters();
    LoadCursor=MenuArenas.first;
    UnpackChoiceGlyph((s8 *)&SelectionRows,CourseChoiceGroup1Glyph);
    ImageRows[1]=6;
}
