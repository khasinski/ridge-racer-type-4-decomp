"""Frame dispatcher: defined PAL counter wrap and fresh full-width published roots."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class Overlay662FrameStepTest(unittest.TestCase):
    def test_counter_edges_order_and_fresh_service_state(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/garage/func_8011AAF4.c').read_text()
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/overlay662_frame_step.h"
static struct World{u32 head[4];RrGarageSelectionPrefix garage;u32 buffers[4][8];RrGarageTimBytes tim[4];u32 clock;s32 pulse;u32*image;RrGarageTimBytes*file;u32 tail[4];}world,expected,checkpoints[8];
static unsigned scenario,calls;static u32 rng=0x1AAF4;
#define garage_logo_state world.garage
#define PlayTimeFrames world.clock
#define ScreenFrame world.pulse
#define GarageActiveBuffer world.image
#define GarageLogoTimPtr world.file
'''+body+r'''
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s32 word(u32 x){s32 v;memcpy(&v,&x,4);return v;}
static void effect(struct World*w,unsigned n){unsigned j;
 w->clock=scenario*0x9E3779B9U+n;w->pulse=word(scenario*0x1234567U+n);
 w->garage.active21=(u8)((scenario+n)&1);w->garage.flagsD1=(u8)(scenario+n);w->garage.saved64[0]=word(scenario^0x80000000U^n);
 for(j=0;j<2;j++){w->garage.buffers24[j]=world.buffers[(scenario+n+j)%4];w->garage.tim2C[j]=world.tim+(scenario+n+j+1)%4;}
 w->image=world.buffers[(scenario+n+2)%4];w->file=world.tim+(scenario+n+3)%4;
}
static void boundary(unsigned n){assert(calls++==n);assert(memcmp(&world,&checkpoints[n],sizeof(world))==0);effect(&world,n);}
void garage_update_selection_pulse(void){boundary(0);}
void garage_update_cursor(void){boundary(1);}
void garage_update_actions(s32 mode){assert(mode==0);boundary(2);}
void garage_draw_cursor_sprite(void){boundary(3);}
void garage_draw_animated_panels(void){boundary(4);}
void garage_add_editor_draw_env(void){boundary(5);}
void garage_update_editor_overlays(void){boundary(6);}
void garage_upload_editor_vram(void){boundary(7);}
static void reference(void){unsigned n;expected.pulse=word((u32)expected.pulse+1U);expected.clock++;if(expected.clock>0x014996FFU)expected.clock=0x014996FFU;
 for(n=0;n<8;n++){checkpoints[n]=expected;effect(&expected,n);}
 expected.image=expected.garage.buffers24[expected.garage.active21];expected.file=expected.garage.tim2C[expected.garage.active21];
}
int main(void){unsigned j;static const u32 clocks[]={0,1,0x014996FEU,0x014996FFU,0x01499700U,0xFFFFFFFFU,0xFFFFFFFEU,0x80000000U};
 for(scenario=0;scenario<65536;scenario++){
 memset(&world,0,sizeof(world));for(j=0;j<sizeof(world);j++)((u8*)&world)[j]=(u8)next();effect(&world,9);
 world.clock=(scenario&16)?next():clocks[scenario&7];switch(scenario&3){case 0:world.pulse=INT32_MAX;break;case 1:world.pulse=INT32_MIN;break;case 2:world.pulse=-1;break;default:world.pulse=word(next());break;}
 expected=world;calls=0;reference();garage_update_editor_frame();assert(calls==8);assert(memcmp(&world,&expected,sizeof(world))==0);assert((uintptr_t)world.image>UINT32_MAX&&(uintptr_t)world.file>UINT32_MAX);
 }return 0;}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
