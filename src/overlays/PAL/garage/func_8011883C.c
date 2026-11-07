#include "rr/garage_time.h"
u8 *garage_format_time_mm_ss(s32 value){
 if(value>=0){
  s32 seconds=value/3000;
  s32 minutes=seconds/60;
  seconds%=60;
  garage_time_text[0]=minutes/10+'0';
  garage_time_text[1]=minutes%10+'0';
  garage_time_text[3]=seconds/10+'0';
  garage_time_text[4]=seconds%10+'0';
 }else{
  garage_time_text[0]='-';garage_time_text[1]='-';
  garage_time_text[3]='-';garage_time_text[4]='-';
 }
 return garage_time_text;
}
