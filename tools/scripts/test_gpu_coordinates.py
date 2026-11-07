"""Actual E3/E4 coordinate TU, with an independent clamp oracle and read hooks."""
import ctypes as C
from pathlib import Path
import random
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class GpuCoordinatesTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=TemporaryDirectory();cls.addClassCleanup(cls.temp.cleanup)
        root=Path(__file__).resolve().parents[2];d=Path(cls.temp.name)
        header=(root/'include/rr/gpu.h').read_text()
        header=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',header,flags=re.M)
        body=(root/'tools/scripts/fixtures/legacy/gpu_coordinates.c').read_text().replace('#include "rr/gpu.h"','')
        body=re.sub(r'\s+__asm__\("\$[0-9]+"\)','',body)
        body=re.sub(r' : "\$[0-9]+"','',body)
        observed=body.replace('get_cs','observed_top_left').replace('get_ce','observed_bottom_right')
        for axis,name in enumerate(('GpuVramWidth','GpuVramHeight')):
            assert observed.count(f'c = {name} - 1')==2
            assert observed.count(f't = *(u16 *)&{name};')==2
            observed=observed.replace(f'c = {name} - 1',f'c = read_signed({axis}) - 1').replace(f't = *(u16 *)&{name};',f't = read_unsigned({axis});')
        code='''#include <stdint.h>
typedef uint8_t u8; typedef int8_t s8; typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
'''+header+'''
s16 GpuVramWidth,GpuVramHeight,after[2];
s32 reads,log_entries[8];
s32 read_signed(s32 axis) {
    s16 *p=axis ? &GpuVramHeight : &GpuVramWidth;
    s32 result=*p;
    log_entries[reads++]=axis*2;
    *p=after[axis];return result;
}
u16 read_unsigned(s32 axis) {
    log_entries[reads++]=axis*2+1;
    return (u16)(axis ? GpuVramHeight : GpuVramWidth);
}
'''+body+observed
        (d/'f.c').write_text(code)
        subprocess.run(['cc','-shared','-fPIC','-O2',str(d/'f.c'),'-o',str(d/'f.so')],check=True,capture_output=True)
        cls.lib=C.CDLL(str(d/'f.so'))
        cls.functions=[cls.lib.get_cs,cls.lib.get_ce]
        cls.observed=[cls.lib.observed_top_left,cls.lib.observed_bottom_right]
        for f in cls.functions+cls.observed:f.argtypes=[C.c_int32,C.c_int32];f.restype=C.c_uint32
        cls.width=C.c_int16.in_dll(cls.lib,'GpuVramWidth');cls.height=C.c_int16.in_dll(cls.lib,'GpuVramHeight')
        cls.after=(C.c_int16*2).in_dll(cls.lib,'after');cls.reads=C.c_int32.in_dll(cls.lib,'reads')
        cls.log=(C.c_int32*8).in_dll(cls.lib,'log_entries')

    @staticmethod
    def axis(value,first,last=None):
        signed=C.c_int16(value&65535).value
        if signed<0:return 0
        if first-1<signed:return ((first if last is None else last)&65535)-1
        return value

    def expected(self,x,y,width,height,prefix,last=None):
        a=self.axis(x,width,None if last is None else last[0])
        b=self.axis(y,height,None if last is None else last[1])
        return prefix|((b&1023)<<10)|(a&1023)

    def check(self,x,y,width,height):
        self.width.value=width;self.height.value=height
        for f,prefix in zip(self.functions,(0xE3000000,0xE4000000)):
            self.assertEqual(f(x,y),self.expected(x,y,width,height,prefix))
            self.assertEqual((self.width.value,self.height.value),(width,height))

    def test_every_low_halfword_and_high_word_patterns(self):
        self.width.value=320;self.height.value=240
        for upper in (0,0x1234,0xFFFF):
            for low in range(65536):
                x=C.c_int32((upper<<16)|low).value
                y=C.c_int32(((upper^0xFFFF)<<16)|(65535-low)).value
                for f,prefix in zip(self.functions,(0xE3000000,0xE4000000)):
                    self.assertEqual(f(x,y),self.expected(x,y,320,240,prefix))
        self.assertEqual((self.width.value,self.height.value),(320,240))

    def test_dimension_limits_and_coordinate_edges(self):
        dimensions=(-32768,-1,0,1,2,240,320,1024,32767)
        for width in dimensions:
            for height in dimensions:
                xs=(-32769,-32768,-1,0,1,32767,32768,width-1,width,width+1)
                ys=(-32769,-32768,-1,0,1,32767,32768,height-1,height,height+1)
                for x in xs:
                    for y in ys:self.check(x,y,width,height)

    def test_random_full_word_inputs_and_signed_dimensions(self):
        rng=random.Random(0x94198)
        for _ in range(10000):
            self.check(rng.randint(-2147483648,2147483647),rng.randint(-2147483648,2147483647),rng.randint(-32768,32767),rng.randint(-32768,32767))

    def test_limit_reload_uses_fresh_unsigned_value_after_signed_comparison(self):
        cases=[(10,20,0,32767,(777,0)),(100,100,32767,0,(-1,1024)),(-1,10,320,0,(1,240)),(10,-1,0,240,(320,1)),(-1,-32768,0,0,(-1,-1))]
        for f,prefix in zip(self.observed,(0xE3000000,0xE4000000)):
            for x,y,width,height,last in cases:
                self.width.value=width;self.height.value=height;self.after[:]=last;self.reads.value=0
                self.assertEqual(f(x,y),self.expected(x,y,width,height,prefix,last))
                expected_log=[]
                if C.c_int16(x&65535).value>=0:expected_log += [0,1]
                if C.c_int16(y&65535).value>=0:expected_log += [2,3]
                self.assertEqual(list(self.log)[:self.reads.value],expected_log)
                self.assertEqual((self.width.value,self.height.value),(last[0] if 0 in expected_log else width,last[1] if 2 in expected_log else height))
