#include "rr/overlay662_transition.h"

void garage_func_80115474(void)
{
    ResetScreenCounters();
    LoadCursor=MenuArenas.first;
    UnpackChoiceGlyph((s8 *)&SelectionRows,D_800ABE2C);
    ImageRows[1]=6;
}
