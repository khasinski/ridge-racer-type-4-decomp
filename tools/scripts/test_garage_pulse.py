"""Garage pulse with the actual sine TU, PAL ROM table and defined phase wrap."""
from pathlib import Path
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayPulseTest(unittest.TestCase):
    def test_pulse_with_real_sine_and_full_word_phases(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/garage/func_8011D208.c').read_text()
        trig = (root / 'tools/scripts/fixtures/legacy/trig_sine.c').read_text()
        exe = (root / 'assets/PAL/main.exe').read_bytes()
        offset = 0x800 + 0x800A50C0 - 0x80010000
        table = struct.unpack_from('<4097h', exe, offset)
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/overlay662_garage.h"
#include "rr/trig.h"
static struct World {u32 before[8];RrGarageSelectionPrefix state;u32 after[8];} world,expected;
#define garage_D_801276A8 world.state
static s16 rom[4097]={''' + ','.join(map(str, table)) + r'''};
static s16 original_rom[4097];
#define D_800A50C0 rom
#define D_800A58C0 (rom+0x400)
#define D_800A60C0 (rom+0x800)
#define SinTable (rom+0xc00)
''' + trig + body + r'''
static u32 seed=0x1d208;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static int sine_model(int64_t angle){
 int phase=(int)(angle%4096),index,quadrant;
 if(phase<0)phase+=4096;
 quadrant=phase/1024;index=phase%1024;
 switch(quadrant){
 case 0:return rom[0xc00+index];
 case 1:return rom[0xc00+1024-index];
 case 2:return -rom[0xc00+index];
 default:return -rom[0xc00+1024-index];
 }
}
int main(void){unsigned k,i;const u32 edges[]={0,4095,0x7fffffffU,0x7fffffa0U,0x80000000U,0xffffff9fU,0xffffffa0U,0xffffffffU};
 memcpy(original_rom,rom,sizeof(rom));
 for(k=0;k<65544;k++){
  u32 raw=k<65536?(next()&0xffff0000U)|k:edges[k-65536];
  int64_t angle=raw,updated=(int64_t)(((uint64_t)raw+96)&UINT32_MAX);
  for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
  world.state.phaseCC=raw;expected=world;
  if(angle>=INT64_C(2147483648))angle-=INT64_C(4294967296);
  if(updated>=INT64_C(2147483648))updated-=INT64_C(4294967296);
  expected.state.shadeD0=(u8)(sine_model(angle)/64-65);
  expected.state.phaseCC=(u32)(updated%4096);
  garage_func_8011D208();
  assert(memcmp(&world,&expected,sizeof(world))==0);
  assert((uintptr_t)&world.state>UINT32_MAX);
 }
 assert(memcmp(rom,original_rom,sizeof(rom))==0);
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            path = Path(temp)
            (path / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(path / 'test.c'), '-o', str(path / 'test')], check=True)
            subprocess.run([str(path / 'test')], check=True)


if __name__ == '__main__':
    unittest.main()
