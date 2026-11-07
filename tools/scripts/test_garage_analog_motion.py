"""Analog garage motion with independent signed-wrap arithmetic and real owners."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

class GarageAnalogMotionTest(unittest.TestCase):
    def test_fractional_accumulators_all_signed_halves(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/garage/func_8011D338.c').read_text()
        source=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/overlay662_input.h"
typedef union {s16 halves[2];RrGaragePoint point;} Accumulator;
typedef union {RrMenuItemState5C rows[3];RrGaragePoint points[69];} Controller;
static struct World {u32 guard[4];Controller controller;Accumulator accumulator;RrGaragePoint out;u32 tail[4];} w,e;
#define PlayerControllers w.controller.rows
#define garage_analog_cursor_remainder w.accumulator.halves
static unsigned calls;
void *PsyqMemset(void *p,s32 value,u32 count){assert(value==0 && count==4);calls++;return memset(p,value,count);}
''' + body + r'''
static u32 seed=0x1d338;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 value){u16 raw=value;s16 result;memcpy(&result,&raw,2);return result;}
static RrGaragePoint *output(struct World *v,unsigned index){
 if(index==0)return &v->out;
 if(index==1)return &v->accumulator.point;
 if(index==2)return &v->controller.points[5];
 return &v->controller.points[0];
}
static void reference(unsigned index){
 int64_t movement[2]={0,0};unsigned i;const RrMenuItemState5C *row=&e.controller.rows[0];
 if(row->type01==0x73 && row->kind1C!=0 && row->flag40==0){
  for(i=0;i<2;i++){
   int64_t axis=i?row->axis17:row->axis16,old=e.accumulator.halves[i];
   if(axis>=112 && axis<=144){e.accumulator.halves[i]=0;continue;}
   int64_t delta=(axis<112?axis-112:axis-143)*((row->buttons02&8)?4:1);
   if((delta<0 && old<0) || (delta>0 && old>0))delta+=old;
   int64_t accumulated=half((u32)delta);
   movement[i]=(accumulated/112)*(i?2:1);
   e.accumulator.halves[i]=half((u32)(accumulated-movement[i]*112));
  }
  if(movement[0] || movement[1]){output(&e,index)->x=movement[0];output(&e,index)->y=movement[1];}
 }
}
static void run(unsigned index){
 memcpy(&e,&w,sizeof(w));reference(index);calls=0;
 assert((uintptr_t)output(&w,index)>UINT32_MAX);
 garage_read_analog_motion(output(&w,index));
 assert(calls==1 && memcmp(&w,&e,sizeof(w))==0);
}
static void setup(unsigned x,unsigned y,unsigned speed){
 memset(&w,0xA5,sizeof(w));w.controller.rows[0].type01=0x73;w.controller.rows[0].kind1C=-1;w.controller.rows[0].flag40=0;
 w.controller.rows[0].buttons02=(u16)((x*257U+y)&~8U)|(speed?8:0);
 w.controller.rows[0].axis16=x;w.controller.rows[0].axis17=y;
}
int main(void){unsigned mode,x,y,k,i;const u16 edges[]={0,1,111,112,113,32766,32767,32768,32769,65423,65424,65425,65534,65535,7,65529};
 /* All65536 possible accumulator values crossed with every axis byte and speed.
  * The permutations cover every vertical accumulator/axis pair as well. */
 for(mode=0;mode<2;mode++)for(k=0;k<65536;k++)for(x=0;x<256;x++){
  setup(x,(x*73U+17U)&255,mode);
  w.accumulator.halves[0]=half(k);w.accumulator.halves[1]=half(k*40503U+19U);
  run((k+x+mode)&3);
 }
 /* Full axis pairs around signed overflow and quantization boundaries. */
 for(mode=0;mode<2;mode++)for(k=0;k<16;k++)for(x=0;x<256;x++)for(y=0;y<256;y++){
  setup(x,y,mode);w.accumulator.halves[0]=half(edges[k]);w.accumulator.halves[1]=half(edges[(k+7)&15]);run((x+y+k)&3);
 }
 /* Every type and flag byte, including disabled and noncanonical kind values. */
 for(k=0;k<131072;k++){
  for(i=0;i<sizeof(w);i++)((u8*)&w)[i]=(u8)next();
  w.controller.rows[0].type01=(k&1)?0x73:(u8)(k>>1);
  w.controller.rows[0].flag40=(u8)(k>>9);
  if((k&7)==0)w.controller.rows[0].kind1C=0;
  run(k&3);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
