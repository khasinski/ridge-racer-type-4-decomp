#include "rr/overlay662_palette_initializer.h"
void ovl662_func_80119B44(void) {
 s32 i;s16 val;RrBadgeRow1C *badge;
 ModeFrame=0;ScreenFrame=0;ovl662_D_801272D4=0;ovl662_D_801272C0=0;MenuCursor=0;
 LoadCursor=MenuArenas.second;
 if(!ovl662_D_801272E8)MenuItemCount=2;else MenuItemCount=1;
 InitRect16Row(0,128,128,64,128,0,0);
 InitUiRow(0,12,64,68,32,0,0,1,0,83);
 InitUiRow(1,12,104,68,32,0,0,1,0,85);
 InitBadgeRow(0,108,320,112,34,0,0x601);
 badge=BadgeRows;ovl662_D_8012739A=25;badge->half02=1;
 for(i=0;i<25;i++) {
  if(i>=21&&SaveProgress.prefix.reserved50.bytes[i-13]==0)ovl662_D_801272A0[i]=25;
  else ovl662_D_801272A0[i]=i;
 }
 ovl662_D_80127398=0;
 if(ovl662_D_801272E8==1){ovl662_D_801272E2=0;ovl662_D_801272BE=0;}
 else{val=CourseChoiceTable[1][ovl662_D_801272E4].second;ovl662_D_801272E2=val;
 if((s16)val>0)ovl662_D_801272BE=val-1;else ovl662_D_801272BE=0;}
 ovl662_D_801272E2=(s8)ovl662_D_801272A0[ovl662_D_801272BE];
 val=ovl662_D_801272E2;
 if(!val)UploadCarClut(D_800F3B20,0,0);else UploadCarClut(CarClutData,val-1,0);
 ovl662_D_801272D4=0;
}
