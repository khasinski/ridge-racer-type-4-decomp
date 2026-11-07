#include "rr/overlay_menu.h"
s32 ovl662_func_8011662C(u32 unused,s32 mode){
 s32 selected;
 if(mode)selected=CourseListSavedIndex;else selected=SelectionRows.bytes[0];
 DrawMenuStepArrows(160,204,48,0,selected,0,4);
 return UpdateGlyphRow(selected);
}
