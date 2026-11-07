from overlay_units import read_c_source
"""Scene exits reload their frame after real reset/transition helpers."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

class MenuScreenExitTest(unittest.TestCase):
    def test_helpers_frames_and_halfword_cells(self):
        root=Path(__file__).resolve().parents[2]
        bodies="\n".join(read_c_source(root/p) for p in [
            'tools/scripts/fixtures/views/func_80036068.c','tools/scripts/fixtures/views/func_8003CAB4.c',
            'src/overlays/PAL/time_attack_course_select/func_8011712C.c','src/overlays/PAL/versus_select/func_80114AA0.c'])
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(t)==n,"size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(offsetof(t,f)==n,"offset")
#include "rr/menu_screen_exit.h"
_Static_assert(__builtin_types_compatible_p(__typeof__(ModeGroup),u16),"mode width");
u16 ModeGroup,SceneId,GameTypeSceneIds[256];
s16 ModeId,ModeSubState,RaceType;
s32 MaskBitSetting;
RrGpuRectangle *CurrentDrawBuf;
static RrFrameOrderingPrefix frames[4],saved[4];
static unsigned phase,case_id,which;
static u32 rng=0x1712C;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static void mutate(unsigned n){
 ModeGroup=(u16)(case_id+n);ModeId=(s16)(case_id-n);
 ModeSubState=(s16)(case_id+n*3);MaskBitSetting=(s32)(case_id+n);
 SceneId=(u16)(case_id^n);RaceType=(s16)((case_id+n)&255);
 CurrentDrawBuf=&frames[n].rectangle;
}
int SoundSetMasterVolume(s32 a,s32 b){assert(phase++==0);assert(a==0&&b==0);mutate(0);return -1;}
int StopRaceRumble(void){assert(phase++==1);mutate(1);return 7;}
void SetupRace(void){assert(phase++==2);assert(MaskBitSetting==0);mutate(2);}
void AddScreenFadeTiles(u32 *ordering,s32 intensity,u32 mode){
 assert(phase++==3);assert(ordering==frames[2].ordering70);
 assert((uintptr_t)ordering>UINT32_MAX);
 assert(intensity==255&&mode==69);
 assert(ModeGroup==1&&ModeId==(which?38:53)&&ModeSubState==0);
 assert(SceneId==GameTypeSceneIds[(case_id+2)&255]);
 assert(MaskBitSetting==(s32)(case_id+2));
 assert(memcmp(frames,saved,sizeof(frames))==0);
 ordering[(case_id>>8)%704]^=0x76543210U;
 mutate(3);
}
'''+bodies+r'''
int main(void){unsigned i,j;
 for(i=0;i<256;i++)GameTypeSceneIds[i]=(u16)(i*257U);
 for(which=0;which<2;which++)for(case_id=0;case_id<65536;case_id++){
  for(j=0;j<sizeof(frames);j++)((u8*)frames)[j]=(u8)next();
  memcpy(saved,frames,sizeof(frames));phase=0;
  ModeGroup=(u16)case_id;ModeId=(s16)case_id;ModeSubState=(s16)~case_id;
  CurrentDrawBuf=&frames[3].rectangle;
  if(which)versus_select_func_80114AA0();else time_attack_course_select_func_8011712C();
  assert(phase==4);assert(CurrentDrawBuf==&frames[3].rectangle);
  assert(ModeGroup==(u16)(case_id+3));assert(ModeId==(s16)(case_id-3));
  assert(ModeSubState==(s16)(case_id+9));assert(MaskBitSetting==(s32)(case_id+3));
  assert(SceneId==(u16)(case_id^3));
  saved[2].ordering70[(case_id>>8)%704]^=0x76543210U;
  assert(memcmp(frames,saved,sizeof(frames))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
