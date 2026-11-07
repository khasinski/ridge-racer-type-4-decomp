"""Host checks for PAL indexed RAM accessors, with target addresses remapped.

fd.py checks the register annotations and actual target instructions.
"""
import ctypes as C
from pathlib import Path
import re
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest


class IndexedStateTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        directory = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        header = (root / 'include/rr/channels.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
typedef uint8_t u8; typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
u32 test_history_words[0x20000 / 4];
#define test_history ((u8 *)test_history_words)
u32 test_ram[0x14000];
u32 *I_STAT_RCnt;
u32 *GPU_STATUS;
''' + header + '\nRrChannelControlRow36 test_rows[320];\n'
        for name in ('_getctl', 'StopRCnt', '_ctl', 'vmNoiseOff'):
            body = (root / 'tools/scripts/fixtures/legacy' / (name + '.c')).read_text()
            body = body.replace('#include "types.h"', '')
            body = body.replace('#include "rr/channels.h"', '')
            body = body.replace('extern RrChannelControlRow36 _svm_voice[];', '#define _svm_voice test_rows')
            body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
            body = re.sub(r'"\$[0-9]+", ', '', body)
            body = re.sub(r' : "\$[0-9]+"', '', body)
            page = '((u8 *)&test_rows + 0x6F8)' if name == 'vmNoiseOff' else '(test_history + 0x10000)'
            body = body.replace('(u8 *)0x80110000U', page)
            body = body.replace('(u8 *)0x800B0000U', '((u8 *)test_ram + 0x10000)')
            source += body
        (directory / 'functions.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'functions.c'),
                        '-o', str(directory / 'functions.so')], check=True, capture_output=True)
        cls.library = C.CDLL(str(directory / 'functions.so'))
        cls.history = (C.c_uint8 * 0x20000).in_dll(cls.library, 'test_history_words')
        cls.rows = (C.c_uint8 * (320 * 0x36)).in_dll(cls.library, 'test_rows')
        cls.ram = (C.c_uint32 * 0x14000).in_dll(cls.library, 'test_ram')
        cls.get = cls.library._getctl
        cls.get.argtypes = [C.c_int32]
        cls.get.restype = C.c_int32
        cls.clear = cls.library.StopRCnt
        cls.clear.argtypes = [C.c_uint32]
        cls.clear.restype = C.c_int32
        cls.write = cls.library._ctl
        cls.write.argtypes = [C.c_uint32]
        cls.write.restype = None
        cls.clear_row = cls.library.vmNoiseOff
        cls.clear_row.argtypes = [C.c_uint8]
        cls.clear_row.restype = None

    def test_history_returns_unsigned_bytes_without_writes(self):
        for index in range(len(self.history)):
            self.history[index] = (index * 73 + index // 256) & 255
        before = bytes(self.history)
        for index in (*range(256), -0x27D0, -1, 0xD82F):
            self.assertEqual(self.get(index), self.history[0x127D0 + index])
        self.assertEqual(bytes(self.history), before)

    def test_mask_lookup_covers_every_halfword_index(self):
        for index in range(len(self.ram)):
            self.ram[index] = (index * 0x9E3779B9) & 0xFFFFFFFF
        before = bytes(self.ram)
        registers = (C.c_uint32 * 4)(0x12345678, 0, 0x9ABCDEF0, 0xDEADBEEF)
        pointer = C.c_void_p.in_dll(self.library, 'I_STAT_RCnt')
        pointer.value = C.addressof(registers)
        for index in range(0x10000):
            word = (index * 0x10204081 + 0xA55AA55A) & 0xFFFFFFFF
            registers[1] = word
            mask = self.ram[0xBC38 // 4 + index]
            self.assertEqual(self.clear(index), 1)
            self.assertEqual(list(registers),
                             [0x12345678, word & (~mask & 0xFFFFFFFF), 0x9ABCDEF0, 0xDEADBEEF])
        self.assertEqual(pointer.value, C.addressof(registers))
        self.assertEqual(bytes(self.ram), before)

    def test_only_low_halfword_selects_the_mask(self):
        registers = (C.c_uint32 * 2)(0xA5A5A5A5, 0)
        C.c_void_p.in_dll(self.library, 'I_STAT_RCnt').value = C.addressof(registers)
        for index in (0, 1, 2, 5, 0x7FFF, 0xFFFF):
            self.ram[0xBC38 // 4 + index] = 0x80000001 ^ index
            for upper in (0, 1, 0x7FFF, 0xFFFF):
                registers[1] = 0xFFFFFFFF
                self.assertEqual(self.clear((upper << 16) | index), 1)
                self.assertEqual(registers[1], (~(0x80000001 ^ index)) & 0xFFFFFFFF)
                self.assertEqual(registers[0], 0xA5A5A5A5)

    def test_command_write_records_low_byte_under_the_command_index(self):
        port = (C.c_uint32 * 3)(0x12345678, 0, 0x9ABCDEF0)
        C.c_void_p.in_dll(self.library, 'GPU_STATUS').value = C.addressof(port) + 4
        C.memset(C.addressof(self.history), 0xA5, C.sizeof(self.history))
        for index in range(256):
            for byte in (0, 1, 0x7F, 0x80, 0xFF):
                command = (index << 24) | 0x123400 | byte
                before = bytes(self.history)
                self.write(command)
                self.assertEqual(list(port), [0x12345678, command, 0x9ABCDEF0])
                offset = 0x127D0 + index
                self.assertEqual(bytes(self.history), before[:offset] + bytes([byte]) + before[offset + 1:])
                self.assertEqual(self.get(index), byte)

    def test_command_port_write_precedes_an_overlapping_history_byte(self):
        for index in range(1, 256, 4):
            C.memset(C.addressof(self.history), 0xA5, C.sizeof(self.history))
            offset = 0x127D0 + index
            C.c_void_p.in_dll(self.library, 'GPU_STATUS').value = C.addressof(self.history) + offset - 1
            expected = bytearray(bytes(self.history))
            command = (index << 24) | 0x12AB34
            struct.pack_into('<I', expected, offset - 1, command)
            expected[offset] = 0x34
            self.write(command)
            self.assertEqual(bytes(self.history), expected)

    def test_row_reset_changes_only_three_fields_of_the_selected_row(self):
        for index in range(256):
            C.memset(C.addressof(self.rows), 0xA5, C.sizeof(self.rows))
            expected = bytearray(bytes(self.rows))
            offset = index * 0x36
            expected[offset:offset + 2] = b'\0\0'
            expected[offset + 4:offset + 6] = b'\0\0'
            expected[offset + 0x1D] = 0
            self.clear_row(index)
            self.assertEqual(bytes(self.rows), expected)


if __name__ == '__main__':
    unittest.main()
