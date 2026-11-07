"""Host behavior of the shared channel-value TU, including overlapping outputs.

The pointer-table page is represented by native pointer slots. The C source's
sizeof-based indexing adapts to the host ABI; fd.py checks the PSX ABI/code.
"""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class Record(C.Structure):
    _fields_ = [('pad00', C.c_uint8 * 0x58), ('value58', C.c_uint16),
                ('value5A', C.c_uint16), ('pad5C', C.c_uint8 * (0x98 - 0x5C)),
                ('mode98', C.c_int32), ('pad9C', C.c_uint8 * (0xB0 - 0x9C))]


class ChannelValuesTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        directory = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        header = (root / 'include/rr/channels.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
#include <string.h>
typedef uint8_t u8; typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
''' + header + '''
RrChannelValueRecordB0 *test_pointer_ram[0x4100];
RrChannelValueRecordB0 test_records[256][256];
s16 SvmLastChannel;
void test_initialize(void) {
    unsigned bank, row;
    memset(test_records, 0xA5, sizeof(test_records));
    memset(test_pointer_ram, 0, sizeof(test_pointer_ram));
    for (bank = 0; bank < 256; bank++) {
        test_pointer_ram[0x4000 - 0xF98 / 4 + bank] = test_records[bank];
        for (row = 0; row < 256; row++) {
            test_records[bank][row].value58 = bank * 257 + row * 73;
            test_records[bank][row].value5A = (bank * 31 + row * 251) ^ 0xFFFF;
        }
    }
    SvmLastChannel = -123;
}
'''
        body = (root / 'tools/scripts/fixtures/legacy/vmchval.c').read_text()
        body = body.replace('#include "rr/channels.h"', '')
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r'"\$[0-9]+", ', '', body)
        body = re.sub(r' : "\$[0-9]+"', '', body)
        body = body.replace('(RrChannelValueRecordB0 **)0x80110000U', '(test_pointer_ram + 0x4000)')
        source += body
        (directory / 'functions.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'functions.c'),
                        '-o', str(directory / 'functions.so')], check=True, capture_output=True)
        cls.library = C.CDLL(str(directory / 'functions.so'))
        cls.records = (Record * (256 * 256)).in_dll(cls.library, 'test_records')
        cls.pointers = (C.c_void_p * 0x4100).in_dll(cls.library, 'test_pointer_ram')
        cls.selected = C.c_int16.in_dll(cls.library, 'SvmLastChannel')
        cls.left = cls.library._SsVmGetSeqLVol
        cls.right = cls.library._SsVmGetSeqRVol
        for function in (cls.left, cls.right):
            function.argtypes = [C.c_int32]
            function.restype = C.c_int32
        cls.pair = cls.library._SsVmGetSeqVol
        cls.pair.argtypes = [C.c_int32, C.POINTER(C.c_uint16), C.POINTER(C.c_uint16)]
        cls.pair.restype = C.c_int32

    def setUp(self):
        self.library.test_initialize()

    def test_all_encoded_banks_and_rows(self):
        before = C.string_at(C.addressof(self.records), C.sizeof(self.records))
        table_before = list(self.pointers)
        left, right = C.c_uint16(), C.c_uint16()
        for encoded in range(0x10000):
            bank, row = encoded & 255, encoded >> 8
            expected_left = (bank * 257 + row * 73) & 0xFFFF
            expected_right = ((bank * 31 + row * 251) ^ 0xFFFF) & 0xFFFF
            self.assertEqual(self.left(encoded), C.c_int16(expected_left).value)
            self.assertEqual(self.right(encoded), C.c_int16(expected_right).value)
            self.assertEqual(self.pair(encoded, C.byref(left), C.byref(right)), C.c_int16(encoded).value)
            self.assertEqual((left.value, right.value), (expected_left, expected_right))
            self.assertEqual(self.selected.value, C.c_int16(encoded).value)
        self.assertEqual(C.string_at(C.addressof(self.records), C.sizeof(self.records)), before)
        self.assertEqual(list(self.pointers), table_before)

    def test_upper_input_bits_do_not_change_the_encoded_selection(self):
        for encoded in (0, 1, 0x7FFF, 0x8000, 0xFFFF):
            expected = (self.left(encoded), self.right(encoded))
            for upper in (1, 0x7FFF, 0x8000, 0xFFFF):
                value = C.c_int32((upper << 16) | encoded).value
                self.assertEqual((self.left(value), self.right(value)), expected)
                self.assertEqual(self.selected.value, C.c_int16(encoded).value)

    def test_pair_return_reloads_selection_after_overlapping_outputs(self):
        encoded = 0x0705
        record = self.records[5 * 256 + 7]
        record.value58, record.value5A = 0x9001, 0x8002
        output = C.c_uint16()
        selected = C.cast(C.byref(self.selected), C.POINTER(C.c_uint16))
        self.assertEqual(self.pair(encoded, selected, C.byref(output)), C.c_int16(0x9001).value)
        self.assertEqual(output.value, 0x8002)
        self.assertEqual(self.pair(encoded, C.byref(output), selected), C.c_int16(0x8002).value)
        self.assertEqual(output.value, 0x9001)
        self.assertEqual(self.pair(encoded, selected, selected), C.c_int16(0x8002).value)

    def test_pair_preserves_the_original_order_when_output_overlaps_record(self):
        encoded = 0x0705
        record = self.records[5 * 256 + 7]
        record.value58, record.value5A = 0x1234, 0xABCD
        right_field = C.cast(C.byref(record, Record.value5A.offset), C.POINTER(C.c_uint16))
        output = C.c_uint16()
        self.assertEqual(self.pair(encoded, right_field, C.byref(output)), encoded)
        self.assertEqual((record.value5A, output.value), (0x1234, 0x1234))
        record.value58, record.value5A = 0x1357, 0x2468
        self.assertEqual(self.pair(encoded, C.byref(output), C.byref(output)), encoded)
        self.assertEqual(output.value, 0x2468)


if __name__ == '__main__':
    unittest.main()
