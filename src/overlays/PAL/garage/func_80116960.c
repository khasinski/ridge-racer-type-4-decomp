#include "rr/overlay662_available_cars.h"

void garage_init_car_list(void){
 u8 parts[3];
 s32 decoded;
 s32 arenaOffset;
 s32 i,count;s32 j;
 s16 *status;RrGarageAvailableCar *entry;
 u8 *choice;
 RrGarageAvailableCar *captured;
 ModeFrame=0;ScreenFrame=0;garage_D_801272D4=0;garage_car_index=0;MenuCursor=0;MenuItemCount=2;
 ResetScreenCounters();
 arenaOffset=0x2D000;i=0;count=i;
 MenuArenas.second=CdDirectoryBuffer+arenaOffset;
 InitMenuArenas();LoadCursor=MenuArenas.second;
 InitUiRow(0,24,88,68,32,0,0,1,0,83);
 InitUiRow(1,24,128,68,32,0,0,1,0,85);
 InitPreviewImageRows();InitBadgeRow(1,24,192,84,34,0,0x201);SetupRenderer(5);
 status=garage_car_status;entry=garage_collected_cars;
 do{
  SelectionRows.bytes[2]=i;
  *status=(SaveProgress.prefix.progress.progress10[SelectionRows.bytes[0]][SelectionRows.bytes[1]]>>i)&1;
  if(*status!=0){
   entry->value=PackChoiceGlyph(&SelectionRows);j=0;captured=entry;choice=(u8*)&CourseChoiceTable;
   do{
    UnpackChoiceGlyph((s8*)parts,((RrCourseChoice4*)(choice+36))->glyph);
    if(parts[2]<39)parts[2]%=20;
    decoded=PackChoiceGlyph(parts);
    if(captured->value==(s16)decoded)*status=2;
    j++;choice+=4;
   }while(j<8);
   entry++;count++;
  }
  i++;status++;
 }while(i<20);
 garage_car_list_count=count;
 UnpackChoiceGlyph((s8*)&SelectionRows,garage_collected_cars[0].value);
 PlayerCourseChoices[0].first=0;PlayerCourseChoices[0].second=0;
 ModeGroup=1;ModeId=67;ModeSubState=0;
 LoadCursor=MenuArenaBase;MenuArenas.first=MenuArenaBase;
}
