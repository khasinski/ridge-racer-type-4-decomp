#include "rr/overlay662_progress_init.h"

void garage_init_main_menu(void)
{
    s32 i,j,k;
    ModeFrame=0; ScreenFrame=0; MenuCursor=0; MenuItemCount=3;
    ResetScreenCounters();
    MenuArenas.second=CdDirectoryBuffer+0x2D000;
    InitMenuArenas();
    LoadCursor=MenuArenas.second;
    InitUiRow(0,118,176,68,32,0,0,1,0,54);
    InitUiRow(1,118,216,68,32,0,0,1,1,15);
    InitUiRow(2,118,256,68,32,0,0,1,0,85);
    SetupRenderer(5);
    garage_total_car_count=0;
    LoadCursor=MenuArenaBase;
    MenuArenas.first=MenuArenaBase;
    for(i=0;i<4;i++) {
        garage_team_car_counts[i]=0;
        for(j=0;j<4;j++) {
            garage_team_maker_car_counts[i][j]=0;
            for(k=0;k<20;k++)
                garage_team_maker_car_counts[i][j]+=(SaveProgress.prefix.progress.progress10[i][j]>>k)&1;
            garage_team_car_counts[i]+=garage_team_maker_car_counts[i][j];
        }
        garage_total_car_count+=garage_team_car_counts[i];
    }
}
