"""Actual sine TU and cosine function against PAL data and independent oracles."""
import ctypes as C
import math
from pathlib import Path
import random
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class TrigLookupTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=TemporaryDirectory();cls.addClassCleanup(cls.temp.cleanup)
        root=Path(__file__).resolve().parents[2];d=Path(cls.temp.name)
        header=(root/'include/rr/trig.h').read_text()
        header=re.sub(r'^#include[^\n]*\n','',header,flags=re.M)
        code='''#include <stdint.h>
typedef int16_t s16; typedef int32_t s32; typedef uint32_t u32;
'''+header+'''
s16 test_rom[4097];
#define SinTableBiasedBaseC00 test_rom
#define SinTableBiasedBase800 (test_rom+0x400)
#define SinTableBiasedBase400 (test_rom+0x800)
#define SinTable (test_rom+0xC00)
'''
        for source in ('trig_sine.c','func_800904F4.c'):
            body=(root/'tools/scripts/fixtures/views'/source).read_text()
            body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
            code+=body+'\n'
        (d/'f.c').write_text(code)
        subprocess.run(['cc','-shared','-fPIC','-O2',str(d/'f.c'),'-o',str(d/'f.so')],check=True,capture_output=True)
        cls.lib=C.CDLL(str(d/'f.so'))
        cls.sin=cls.lib.rsin;cls.cos=cls.lib.rcos;cls.lookup=cls.lib.sin_1
        for f in (cls.sin,cls.cos,cls.lookup):f.argtypes=[C.c_int32];f.restype=C.c_int32
        cls.rom=(C.c_int16*4097).in_dll(cls.lib,'test_rom')
        exe=(root/'assets/PAL/main.exe').read_bytes();offset=0x800+0x800A50C0-0x80010000
        cls.original=exe[offset:offset+C.sizeof(cls.rom)]
        assert len(cls.original)==C.sizeof(cls.rom)

    def setUp(self):C.memmove(C.addressof(self.rom),self.original,len(self.original))

    def check_math(self,angle):
        phase=angle%4096
        self.assertLessEqual(abs(self.sin(angle)-4096*math.sin(phase*math.tau/4096)),0.500001)
        self.assertLessEqual(abs(self.cos(angle)-4096*math.cos(phase*math.tau/4096)),0.500001)

    def test_every_phase_and_all_quadrant_boundaries(self):
        for angle in range(-4097,4098):self.check_math(angle)
        self.assertEqual([(self.sin(a),self.cos(a)) for a in (0,1024,2048,3072)],[(0,4096),(4096,0),(0,-4096),(-4096,0)])
        for phase in range(4096):self.assertEqual(self.lookup(phase),self.sin(phase))
        self.assertEqual(bytes(self.rom),self.original)

    def test_every_signed_halfword(self):
        for angle in range(-32768,32768):self.check_math(angle)
        self.assertEqual(bytes(self.rom),self.original)

    def test_full_signed_word_range_and_minimum_integer(self):
        rng=random.Random(0x904F4)
        values=[-2147483648,2147483647,-2147483647,0,-1,1]
        values += [C.c_int32((high<<12)+low).value for high in (0,1,0x7FFFF,0x80000,0xFFFFF) for low in (0,1,1023,1024,1025,2047,2048,2049,3071,3072,3073,4095)]
        values += [rng.randint(-2147483648,2147483647) for _ in range(10000)]
        for angle in values:
            self.check_math(angle)
            if angle!=-2147483648:
                self.assertEqual(self.sin(-angle),-self.sin(angle))
                self.assertEqual(self.cos(-angle),self.cos(angle))
        self.assertEqual((self.sin(-2147483648),self.cos(-2147483648)),(0,4096))
        self.assertEqual(bytes(self.rom),self.original)

    def test_signed_table_values_are_negated_without_halfword_truncation(self):
        rng=random.Random(0x68C0)
        quarter=[0]+[rng.choice((-32768,-1,0,1,32767)) for _ in range(1023)]+[4096]
        for i,value in enumerate(quarter):self.rom[0xC00+i]=value
        before=bytes(self.rom)
        def expected_sine(phase):
            phase%=4096
            quadrant,index=divmod(phase,1024)
            if quadrant==0:return quarter[index]
            if quadrant==1:return quarter[1024-index]
            if quadrant==2:return -quarter[index]
            return -quarter[1024-index]
        saw_widened=False
        for phase in range(4096):
            sine=expected_sine(phase);cosine=expected_sine(phase+1024)
            self.assertEqual(self.sin(phase),sine)
            self.assertEqual(self.lookup(phase),sine)
            self.assertEqual(self.cos(phase),cosine)
            saw_widened |= sine==32768 or cosine==32768
        self.assertTrue(saw_widened)
        self.assertEqual(bytes(self.rom),before)
