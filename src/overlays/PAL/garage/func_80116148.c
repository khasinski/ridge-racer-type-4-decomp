#include "rr/menu_arenas.h"
void garage_init_collection(void){
 ResetScreenCounters();
 MenuArenas.second=CdDirectoryBuffer+0x2D000;
 InitMenuArenas();
 LoadCursor=MenuArenas.second;
 SetMenuFrameOptions(0,1,1);
 garage_init_team_select();
}
