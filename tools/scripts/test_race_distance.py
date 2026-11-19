"""Actual distance selection with wrapped sums, signed comparison and real pointers."""
import ctypes as C
from itertools import product
from pathlib import Path
import random
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest
from c_functions import source_owners, definition_source


class DistanceControl(C.Structure):
    _fields_ = [('prefix00', C.c_uint8*0x18), ('record', C.c_void_p), ('handling', C.c_void_p),
                ('reserved20', C.c_uint8*(0x44-0x20)), ('boostTimer44', C.c_int16),
                ('reserved46', C.c_uint8*(0x118-0x46)), ('maximum118', C.c_int16),
                ('span11A', C.c_int16), ('divisor11C', C.c_int16),
                ('tail', C.c_uint8*14)]


class Distance(C.Structure):
    _fields_ = [('pad', C.c_uint8*0x184), ('first', C.c_int32), ('second', C.c_int32),
                ('middle', C.c_uint8*(0x1D8-0x18C)), ('input', C.c_int16),
                ('pad1DA', C.c_uint8*(0x1F4-0x1DA)), ('control', DistanceControl)]


class StandingEntry(C.Structure):
    _fields_ = [('id', C.c_int32), ('distance', C.c_int32), ('vehicle', C.c_void_p)]


class Result(C.Structure):
    _fields_ = [('pad', C.c_uint8*8), ('first', C.c_int32), ('second', C.c_int32),
                ('selected', C.c_int32), ('gap', C.c_int32), ('entries', StandingEntry*8)]


class Context(C.Structure):
    _fields_ = [('pad', C.c_uint8*0x14), ('mode', C.c_int32),
                ('middle', C.c_uint8*(0xE8-0x18)), ('result', C.POINTER(Result)),
                ('native_tail', C.c_uint8*(0x5A8-0xF0))]


class NativeContext(C.Structure):
    # Keep the historical Context/model schema above literal. Bind the current
    # native owner separately, including the five full pointer cells.
    _fields_ = Context._fields_[:-1] + [
        ('native_scalar_tail', C.c_uint8*(0x550-0xF0)),
        ('start_grid', C.c_void_p), ('car_specs', C.c_void_p),
        ('spec_targets', C.c_void_p*2), ('lane_offsets', C.c_void_p),
        ('native_resource_tail', C.c_uint8*(0x5C0-0x578)),
    ]


class GuardedResult(C.Structure):
    _fields_ = [('before', C.c_uint32*4), ('result', Result), ('after', C.c_uint32*4)]


def signed(value):
    return C.c_int32(value).value


def magnitude(value):
    return signed(-value) if value < 0 else value


class RaceDistanceTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        owner = source_owners(root/'src/main/PAL')['CalcDistanceToPlayer']
        body = definition_source(owner.read_text(), 'CalcDistanceToPlayer')
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r'"\$[0-9]+",\s*', '', body)
        body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*(?=\))', '', body)
        source = '#include <stdlib.h>\n#include "rr/race.h"\n' + r'''
static __inline__ s32 WrappingDiff(u32 a0, u32 a1, u32 b0, u32 b1) {
    return (a0 + a1) - (b0 + b1);
}
static __inline__ s32 WrappingMagnitude(s32 value) {
    u32 u = value;
    return value < 0 ? 0U - u : u;
}
RaceVehicle PlayerCars[2];
SceneRaceContext RaceContext;
unsigned long test_context_size(void) {return sizeof(SceneRaceContext);}
unsigned long test_record_size(void) {return sizeof(RaceVehicle);}
unsigned long test_table_size(void) {return sizeof(RaceStandingsTable);}
unsigned long test_entries_offset(void) {return __builtin_offsetof(RaceStandingsTable,entries18);}
''' + body
        directory = Path(cls.temp.name)
        (directory/'function.c').write_text(source)
        result = subprocess.run(['cc', '-shared', '-fPIC', '-O2', '-I', str(root/'include'), str(directory/'function.c'),
                                 '-o', str(directory/'function.so')], capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory/'function.so'))
        cls.references = (Distance*2).in_dll(cls.lib, 'PlayerCars')
        cls.context = NativeContext.in_dll(cls.lib, 'RaceContext')
        cls.calculate = cls.lib.CalcDistanceToPlayer
        cls.calculate.argtypes = [C.POINTER(Distance)]
        cls.calculate.restype = None
        for name in ('test_context_size', 'test_record_size', 'test_table_size', 'test_entries_offset'):
            getattr(cls.lib, name).restype = C.c_ulong

    def setUp(self):
        self.output = GuardedResult()
        self.current = Distance()
        for obj in (self.output, self.current, self.references, self.context):
            C.memset(C.addressof(obj), 0xA5, C.sizeof(obj))
        self.context.result = C.pointer(self.output.result)

    def check(self, mode, values, current=None):
        self.context.mode = mode
        for record, pair in zip(self.references, (values[:2],values[2:4])):
            record.first, record.second = pair
        self.current.first, self.current.second = values[4:]
        if current is None:
            current = self.current
        before_refs = bytes(self.references)
        before_current = bytes(current)
        before_context = bytes(self.context)
        before_output = bytearray(bytes(self.output))
        self.calculate(C.byref(current))
        own = current.first + current.second
        first = signed(self.references[0].first+self.references[0].second-own)
        second = signed(self.references[1].first+self.references[1].second-own)
        expected = (first, 0x7FFFFFFF, 0) if mode == 1 else (
            (first, second, int(magnitude(second) < magnitude(first))) if mode == 2 else (0,0,0))
        self.assertEqual((self.output.result.first,self.output.result.second,self.output.result.selected), expected)
        result_offset = GuardedResult.result.offset
        for offset, value in zip((8,12,16), expected):
            before_output[result_offset+offset:result_offset+offset+4] = C.c_uint32(value).value.to_bytes(4,'little')
        self.assertEqual(bytes(self.output), bytes(before_output))
        self.assertEqual(bytes(self.context), before_context)
        self.assertEqual(bytes(self.references), before_refs)
        self.assertEqual(bytes(current), before_current)

    def test_native_layout_full_pointer_and_signed_boundaries(self):
        self.assertEqual(self.lib.test_context_size(), C.sizeof(NativeContext))
        self.assertEqual(self.lib.test_record_size(), C.sizeof(Distance))
        self.assertEqual(self.lib.test_table_size(), C.sizeof(Result))
        self.assertEqual(self.lib.test_entries_offset(), Result.entries.offset)
        self.assertGreater(C.addressof(self.output.result), 0xFFFFFFFF)
        self.assertEqual(Context.result.offset, 0xE8)
        self.assertEqual(C.addressof(self.context.result.contents), C.addressof(self.output.result))
        boundaries = (-0x80000000,-0x7FFFFFFF,-1,0,1,0x7FFFFFFE,0x7FFFFFFF)
        for first, second, own in product(boundaries, repeat=3):
            for mode in (1,2):
                self.check(mode, (first,second,second,first,own,own))
        cases = ((-0x80000000,0, 0,0, 0,0),
                 (0,0,-0x80000000,0,0,0),
                 (-0x80000000,0,-0x80000000,0,0,0),
                 (3,0,-3,0,0,0), (0x7FFFFFFF,1,0,-1,0,0))
        for values in cases:
            self.check(2, values)

    def test_independent_components_random_wrap_and_reference_aliases(self):
        randomizer = random.Random(0x4A080)
        for _ in range(5000):
            values = tuple(signed(randomizer.getrandbits(32)) for _ in range(6))
            self.check(randomizer.choice((0,1,2,3,-1)), values)
        for index in (0,1):
            for mode in (1,2):
                for _ in range(100):
                    values = tuple(signed(randomizer.getrandbits(32)) for _ in range(6))
                    self.check(mode, values, self.references[index])

    def test_other_modes_allow_null_current_and_only_clear_result_fields(self):
        for mode in (-0x80000000,-1,0,3,4,0x7FFFFFFF):
            self.context.mode = mode
            before = bytearray(bytes(self.output))
            offset = GuardedResult.result.offset+8
            before[offset:offset+12] = b'\0'*12
            self.calculate(None)
            self.assertEqual(bytes(self.output), bytes(before))
            self.assertEqual(self.context.mode, mode)
