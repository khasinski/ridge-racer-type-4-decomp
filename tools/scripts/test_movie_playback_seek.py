"""CD seek retry ordering with real local mode storage and simulated services."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class Overlay668SeekTest(unittest.TestCase):
    def test_retry_schedules(self):
        root = Path(__file__).resolve().parents[2]
        body = (root/'src/overlays/PAL/movie_playback/func_801153F0.c').read_text()
        body = '\n'.join(line for line in body.splitlines() if not line.startswith('#include'))
        source = r'''
#include <stdint.h>
#include <assert.h>
#include <string.h>
typedef uint8_t u8;
typedef int32_t s32;
static u8 location[4], before[4], *modeAddress;
static unsigned scenario,roundIndex,stage,attempt,events;
static u8 expectedMode;
static unsigned retries(unsigned which){return (scenario>>(which*2))&3;}
s32 CdControl(s32 command,const u8 *parameters,u8 *result){
 assert(result==0);assert(stage<2);events++;
 if(stage==0){assert(command==2);assert(parameters==location);}
 else{assert(command==14);assert(parameters!=location);assert(*parameters==expectedMode);
  if(modeAddress)assert(parameters==modeAddress);modeAddress=(u8*)parameters;
  /* Mutable command parameter persists across retries and outer rounds. */
  expectedMode=(u8)(expectedMode+17);*modeAddress=expectedMode;
 }
 if(attempt++<retries(stage))return 0;
 attempt=0;stage++;return 1;
}
s32 VSync(s32 kind){assert(stage==2);assert(kind==3);stage=3;events++;return -123;}
s32 CdRead2(s32 timeout){assert(stage==3);assert(timeout==480);stage=0;events++;
 return roundIndex++==retries(2)?-7:0;
}
''' + body + r'''
int main(void){
 for(scenario=0;scenario<65536;scenario++){
  unsigned i,expected;
  for(i=0;i<4;i++)location[i]=(u8)(scenario>>(i*3));
  memcpy(before,location,4);roundIndex=stage=attempt=events=0;modeAddress=0;expectedMode=128;
  movie_playback_kick_cd(location);
  expected=(retries(2)+1)*(retries(0)+retries(1)+4);
  assert(events==expected);assert(stage==0);assert(roundIndex==retries(2)+1);
  assert(memcmp(before,location,4)==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all',str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__ == '__main__':
    unittest.main()
