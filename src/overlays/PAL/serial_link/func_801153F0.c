#include "rr/overlay667_reset.h"
void serial_link_func_801153F0(void){
 if(serial_link_D_80118F04>=7 || serial_link_D_80118EFC>=7 || serial_link_D_80118EEF==1){
  serial_link_D_80118F04=0;serial_link_D_80118EFC=0;serial_link_D_80118EF8=0;
  serial_link_D_80118F00=0;serial_link_D_80118EE8=0;ModeFrame=0;
  serial_link_func_80114A40();ModeGroup=1;ModeId=44;ModeSubState=0;
 }
}
