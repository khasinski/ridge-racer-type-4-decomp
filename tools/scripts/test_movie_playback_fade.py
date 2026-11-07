"""Exhaust the signed16 fade envelope against an independent bounded formula."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

class MoviePlaybackFadeTest(unittest.TestCase):
    def test_complete_signed16_domain(self):
        root=Path(__file__).resolve().parents[2]
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "rr/movie_playback_fade.h"
''' + (root/'src/overlays/PAL/movie_playback/func_8011596C.c').read_text() + r'''
static s32 reference(s32 frame){
 int64_t left=16*((int64_t)frame-48),right=16*(156-(int64_t)frame),level=256;
 if(left<level)level=left;if(right<level)level=right;if(level<0)level=0;
 return (s32)level;
}
int main(void){unsigned bits;unsigned positive=0,plateau=0;
 for(bits=0;bits<65536;bits++){
  u16 raw=(u16)bits;s16 frame;s32 actual;memcpy(&frame,&raw,2);
  actual=movie_playback_credits_row_fade(frame);
  assert(actual==reference(frame));assert(actual>=0 && actual<=256 && actual%16==0);
  if(actual>0)positive++;if(actual==256)plateau++;
 }
 assert(positive==107 && plateau==77);
 assert(movie_playback_credits_row_fade(48)==0 && movie_playback_credits_row_fade(64)==256);
 assert(movie_playback_credits_row_fade(140)==256 && movie_playback_credits_row_fade(156)==0);
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
