#include "rr/overlay662_palette_initializer.h"
void garage_init_color_select(void) {
 s32 i;s16 val;RrBadgeRow1C *badge;
 ModeFrame=0;ScreenFrame=0;garage_color_preview_state=0;garage_select_arrows_unused_arg=0;MenuCursor=0;
 LoadCursor=MenuArenas.second;
 if(!garage_returned_from_logo_editor)MenuItemCount=2;else MenuItemCount=1;
 InitRect16Row(0,128,128,64,128,0,0);
 InitUiRow(0,12,64,68,32,0,0,1,0,83);
 InitUiRow(1,12,104,68,32,0,0,1,0,85);
 InitBadgeRow(0,108,320,112,34,0,0x601);
 badge=BadgeRows;garage_clut_list_count=25;badge->half02=1;
 for(i=0;i<25;i++) {
  if(i>=21&&SaveProgress.prefix.reserved50.bytes[i-13]==0)garage_car_color_map[i]=25;
  else garage_car_color_map[i]=i;
 }
 garage_image_fade_phase=0;
 if(garage_returned_from_logo_editor==1){garage_clut_choice=0;garage_clut_list_index=0;}
 else{val=CourseChoiceTable[1][garage_car_index].second;garage_clut_choice=val;
 if((s16)val>0)garage_clut_list_index=val-1;else garage_clut_list_index=0;}
 garage_clut_choice=(s8)garage_car_color_map[garage_clut_list_index];
 val=garage_clut_choice;
 if(!val)UploadCarClut(DefaultCarClut,0,0);else UploadCarClut(CarClutData,val-1,0);
 garage_color_preview_state=0;
}
