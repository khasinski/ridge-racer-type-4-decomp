"""Playback dispatcher uses the full signed phase and fresh resource pointer."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class Overlay668DispatchTest(unittest.TestCase):
    def test_phase_and_mutating_boundaries(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/movie_playback/func_80114AA8.c').read_text()
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
#include "rr/overlay668_dispatch.h"
s32 D_800AC704;
void *LoadCursor;
static u8 resources[2][259],before[2][259];
static unsigned calls,selected,offset,case_id;
static s32 initial;
static void finish(unsigned chosen){
 assert(calls++==0);selected=chosen;
 assert(D_800AC704==initial);
 D_800AC704=(s32)((chosen+1)%3);
 LoadCursor=resources[(case_id+1)&1]+(258-offset);
}
void movie_playback_func_80114B2C(u8 *resource){
 assert(resource==resources[case_id&1]+offset);
 assert((uintptr_t)resource>UINT32_MAX);
 *resource^=0xA5;finish(0);
}
void movie_playback_func_80114C3C(void){finish(1);}
void movie_playback_func_80114DF4(void){finish(2);}
'''+body+r'''
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
int main(void){unsigned mode,j;u32 bits;
 for(case_id=0;case_id<65536;case_id++)for(mode=0;mode<4;mode++){
  for(j=0;j<sizeof(resources);j++)((u8*)resources)[j]=(u8)(j+case_id);
  memcpy(before,resources,sizeof(resources));
  offset=case_id%259;calls=0;selected=99;
  bits=mode==3 ? ((case_id<<16)|((case_id^0xB935U)&65535)) : mode;
  initial=word(bits);D_800AC704=initial;
  LoadCursor=mode==0 ? resources[case_id&1]+offset : 0;
  movie_playback_func_80114AA8();
  if(mode<3){
   assert(calls==1&&selected==mode);
   assert(D_800AC704==(s32)((mode+1)%3));
   assert(LoadCursor==resources[(case_id+1)&1]+(258-offset));
   if(mode==0)before[case_id&1][offset]^=0xA5;
  }else{assert(calls==0&&selected==99);assert(D_800AC704==initial);assert(LoadCursor==0);}
  assert(memcmp(before,resources,sizeof(resources))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
