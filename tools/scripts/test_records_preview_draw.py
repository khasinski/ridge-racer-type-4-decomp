"""Preview draw wrapper: defined angle wrap, matrix bounds and fresh context."""
from overlay_units import read_c_source
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class Overlay663PreviewDrawTest(unittest.TestCase):
    def test_angles_matrix_and_mutating_abi(self):
        root=Path(__file__).resolve().parents[2]
        body=read_c_source(root/'src/overlays/PAL/records/func_801158B4.c')
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#include "rr/frame_storage_types.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/overlay663_preview_draw.h"
RrRaceVehicle PlayerCars[2];RrFrameStorage *CurrentDrawBuf;
static RrFrameOrderingPrefix frames[3],frames_before[3];
static RrRaceVehicle untouched,final_car;
static u16 *matrix,expected_matrix[9];
static unsigned scenario,variant,calls;
static s32 input_y;static u32 rng=0x158B4;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s32 word(u32 bits){s32 value;memcpy(&value,&bits,4);return value;}
static s16 half(u32 bits){u16 h=(u16)bits;s16 value;memcpy(&value,&h,2);return value;}
static void mutate(unsigned n){unsigned j;
 for(j=0;j<sizeof(PlayerCars[0]);j++)((u8*)PlayerCars)[j]=(u8)(scenario+j+n*31);
 PlayerCars[0].pos20[0]=word(scenario*0x9E3779B9U+n);
 PlayerCars[0].pos20[1]=word(scenario*0x1234567U+n);
 PlayerCars[0].pos20[2]=word(scenario*0x87654321U+n);
 CurrentDrawBuf=(RrFrameStorage*)&frames[n%3];
}
void BuildCameraMatrices(void){assert(calls++==0);mutate(0);}
void SetupViewport(s32 mode){assert(calls++==1&&mode==6);mutate(1);
 PlayerCars[0].rot50=word(0x12340000U|scenario);
 PlayerCars[0].rot54=input_y;
 PlayerCars[0].rot58=word(0xFEDC0000U|(scenario^0x9E37U));
}
u16 *RotMatrix(s16 *rotation,u16 *output){unsigned j;int64_t wide=(int64_t)input_y+8;
 u32 wrapped=(u32)((uint64_t)wide&4095U);
 assert(calls++==2);assert(((uintptr_t)output&3)==0);
 assert(rotation[0]==half(scenario)&&rotation[1]==(s16)wrapped&&rotation[2]==half(scenario^0x9E37U));
 assert(PlayerCars[0].rot54==(s32)wrapped);
 matrix=output;for(j=0;j<9;j++)output[j]=expected_matrix[j]=(u16)(rotation[j%3]+j*17);
 mutate(2);return output;
}
void EmitTiledRect(RrGpuRectangleFields *rectangle,s32 intensity,u32 *head){unsigned j;
 assert(calls++==3);assert(rectangle->x==100&&rectangle->y==80&&rectangle->w==120&&rectangle->h==280);
 assert(intensity==128&&head==frames[2].ordering70);assert((uintptr_t)head>UINT32_MAX);
 for(j=0;j<9;j++)matrix[j]=expected_matrix[j]^=(u16)(scenario+j);
 mutate(3);
}
void SetSineLightColors(s32 scale){assert(calls++==4&&scale==4096);mutate(4);}
void SetSkyMatrixAndDraw(char *position,volatile u16 *rotation){unsigned j;
 assert(calls++==5);assert(position==(char*)PlayerCars[0].pos20);assert(rotation==matrix);
 for(j=0;j<9;j++)assert(rotation[j]==expected_matrix[j]);
 assert(PlayerCars[0].pos20[0]==word(scenario*0x9E3779B9U+4));
 assert(PlayerCars[0].pos20[1]==word(scenario*0x1234567U+4));
 assert(PlayerCars[0].pos20[2]==word(scenario*0x87654321U+4));
 mutate(5);memcpy(&final_car,&PlayerCars[0],sizeof(final_car));
}
'''+body+r'''
int main(void){unsigned j;
 for(scenario=0;scenario<65536;scenario++)for(variant=0;variant<2;variant++){
  for(j=0;j<sizeof(PlayerCars);j++)((u8*)PlayerCars)[j]=(u8)next();
  for(j=0;j<sizeof(frames);j++)((u8*)frames)[j]=(u8)next();
  memcpy(&untouched,&PlayerCars[1],sizeof(untouched));memcpy(frames_before,frames,sizeof(frames));
  input_y=word((variant?0x80000000U:0x7FFF0000U)|scenario);
  CurrentDrawBuf=(RrFrameStorage*)&frames[0];calls=0;matrix=0;
  records_draw_preview();assert(calls==6);
  assert(memcmp(&untouched,&PlayerCars[1],sizeof(untouched))==0);
  assert(memcmp(&final_car,&PlayerCars[0],sizeof(final_car))==0);
  assert(memcmp(frames_before,frames,sizeof(frames))==0);
  assert(CurrentDrawBuf==(RrFrameStorage*)&frames[2]);
 }
 return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
