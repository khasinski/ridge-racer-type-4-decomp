"""Command response writers, wrapped 16-bit assembly, and typed constructors."""
import ctypes as C
from pathlib import Path
import re
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

Write = C.CFUNCTYPE(C.c_int32, C.c_int32)
SCALARS = ('CardTxCount', 'CardCommandRequest', 'CardReceiveIndex', 'CardRxCount',
           'CardCmdSector', 'CardVarRxLength', 'CardBytesRemaining', 'CardCurrentCommand')
POINTERS = ('CardCmdBuffer', 'CardCmdResultPtrA', 'CardCmdResultPtrB', 'CardCommandReader', 'CardWriteHandler')


class CommandResponseWritersTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        directory = Path(cls.temp.name)
        header = (root / 'include/rr/commands.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        header = header.replace('extern s32 CardCommandStatus,', 'extern s32')
        source = '''#include <stdint.h>
#include <string.h>
typedef uint8_t u8; typedef uint32_t u32; typedef int32_t s32;
s32 test_ram[0x4000];
#define CardCommandStatus test_ram[0xBDE4/4]
''' + header + '''
u8 *CardCmdBuffer;
void *CardCmdResultPtrA;
u32 *CardCmdResultPtrB;
RrCommandParameter CardCmdValue;
RrCommandReadCallback CardCommandReader;
RrCommandWriteCallback CardWriteHandler;
s32 _cardCmd59Tx(void) { return 0; }
s32 _cardCmd58Tx(void) { return 0; }
s32 _cardGetIdTx(void) { return 0; }
''' + '\n'.join('s32 '+name+';' for name in SCALARS) + '''
u32 test_results[2][16];
void test_initialize(void) {
 memset(test_ram,0xA5,sizeof(test_ram));
 memset(test_results,0xA5,sizeof(test_results));
 CardCmdBuffer=(u8 *)&test_results[0][1];
 CardCmdResultPtrA=&test_results[0][3]; CardCmdResultPtrB=&test_results[1][11];
 CardCmdValue.buffer=(u8 *)&test_results[0][2];
 CardCommandReader=_cardCmd59Tx; CardWriteHandler=0;
}
'''
        for name in ('8009C400', '8009D680', '8009C310', '8009D5B4', '8009DE54', '8009DD88'):
            body = (root / ('tools/scripts/fixtures/views/func_' + name + '.c')).read_text()
            body = body.replace('#include "rr/commands.h"', '')
            body = body.replace('#include "rr_asm.h"', '')
            body = body.replace('RR_NOP_DEP_ON(word);', '(void)word;')
            body = body.replace('(s32 *)0x800B0000U', '(test_ram + 0x4000)')
            body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
            body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*(?:,\s*"memory")?', '', body)
            source += body
        (directory / 'function.c').write_text(source)
        compiled = subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'function.c'),
                        '-o', str(directory / 'function.so')], check=False, capture_output=True)
        if compiled.returncode:
            raise AssertionError(compiled.stderr.decode())
        cls.lib = C.CDLL(str(directory / 'function.so'))
        cls.results = (C.c_uint32 * 32).in_dll(cls.lib, 'test_results')
        cls.ram = (C.c_uint8 * 65536).in_dll(cls.lib, 'test_ram')
        cls.scalars = {name: C.c_int32.in_dll(cls.lib, name) for name in SCALARS}
        cls.pointers = {name: C.c_void_p.in_dll(cls.lib, name) for name in POINTERS}
        cls.writers = []
        for name in ('8009C400', '8009D680'):
            function = getattr(cls.lib, __import__('c_functions').name_at(int(name,16)|0x80000000))
            function.argtypes = [C.c_int32]
            function.restype = C.c_int32
            cls.writers.append(function)
        cls.response101 = cls.lib._cardGetIdRx
        cls.response101.argtypes = [C.c_int32]
        cls.response101.restype = C.c_int32
        cls.start_word = cls.lib._cardCmd59Begin
        cls.start_word.argtypes = [C.c_int32, C.c_void_p]
        cls.start_word.restype = C.c_int32
        cls.start_pair = cls.lib._cardCmd58Begin
        cls.start_pair.argtypes = [C.c_int32, C.c_void_p, C.c_void_p]
        cls.start_pair.restype = C.c_int32

    def setUp(self):
        self.lib.test_initialize()
        for index, value in enumerate(self.scalars.values()):
            value.value = 0x12345670 + index

    def snapshot(self):
        return (bytes(self.results), bytes(self.ram),
                {name: value.value for name, value in self.scalars.items()},
                {name: value.value for name, value in self.pointers.items()})

    def test_all_bytes_raw_high_bits_response_phases_and_word_wrap(self):
        for writer_index, writer in enumerate(self.writers):
            for phase in (-0x80000000, -1, 0, 1, 2, 3, 4, 5, 6, 255, 0x7FFFFFFF):
                for byte in range(256):
                    for pattern in (0, 0x12345600, 0xFFFFFF00):
                        self.scalars['CardRxCount'].value = phase
                        self.results[3] = 0xFFFFF100
                        before, ram, scalars, pointers = self.snapshot()
                        expected = bytearray(before)
                        if phase == 3:
                            scalars['CardBytesRemaining'] = byte
                        elif phase == 4:
                            struct.pack_into('<I', expected, 3 * 4, byte)
                        elif phase == 5:
                            if writer_index == 0:
                                struct.pack_into('<I', expected, 3 * 4, (0xFFFFF100 + (byte << 8)) & 0xFFFFFFFF)
                            else:
                                struct.pack_into('<I', expected, 27 * 4, byte)
                        self.assertEqual(writer(C.c_int32(pattern | byte).value), 0)
                        self.assertEqual(self.snapshot(), (bytes(expected), ram, scalars, pointers))

    def test_nonwriting_phases_accept_null_result_pointers(self):
        self.pointers['CardCmdResultPtrA'].value = None
        self.pointers['CardCmdResultPtrB'].value = None
        for writer in self.writers:
            for phase in (-1, 0, 2, 3, 6, 0x7FFFFFFF):
                self.scalars['CardRxCount'].value = phase
                before, ram, scalars, pointers = self.snapshot()
                if phase == 3:
                    scalars['CardBytesRemaining'] = 0xAB
                self.assertEqual(writer(0x123456AB), 0)
                self.assertEqual(self.snapshot(), (before, ram, scalars, pointers))

    def test_pair_result_pointers_may_alias(self):
        self.pointers['CardCmdResultPtrB'].value = self.pointers['CardCmdResultPtrA'].value
        before, ram, scalars, pointers = self.snapshot()
        expected = bytearray(before)
        for phase, byte in [(4, 0x12), (5, 0xAB)]:
            self.scalars['CardRxCount'].value = phase
            scalars['CardRxCount'] = phase
            struct.pack_into('<I', expected, 12, byte)
            self.assertEqual(self.writers[1](byte), 0)
            self.assertEqual(self.snapshot(), (bytes(expected), ram, scalars, pointers))

    def test_real_word_constructor_installs_writer_and_assembles_result(self):
        C.c_int32.from_buffer(self.ram, 0xBDE4).value = 1
        pointer = C.addressof(self.results) + 12
        self.assertEqual(self.start_word(0x12345678, pointer), 1)
        self.assertEqual(self.pointers['CardCmdResultPtrA'].value, pointer)
        self.assertEqual(self.pointers['CardWriteHandler'].value, C.cast(self.writers[0], C.c_void_p).value)
        self.assertEqual(self.scalars['CardCurrentCommand'].value, 1)
        self.assertEqual(self.scalars['CardCmdSector'].value, -2)
        callback = Write(self.pointers['CardWriteHandler'].value)
        for phase, value in [(3, 0xFEDCBA55), (4, -2), (5, 0x1234567F)]:
            self.scalars['CardRxCount'].value = phase
            self.assertEqual(callback(C.c_int32(value).value), 0)
        self.assertEqual(self.results[3], 0x7FFE)
        self.assertEqual(self.scalars['CardBytesRemaining'].value, 0x55)

    def test_real_pair_constructor_installs_writer_and_routes_two_results(self):
        C.c_int32.from_buffer(self.ram, 0xBDE4).value = 1
        first, second = C.addressof(self.results) + 12, C.addressof(self.results) + 108
        self.assertEqual(self.start_pair(-1, first, second), 1)
        self.assertEqual(self.pointers['CardCmdResultPtrA'].value, first)
        self.assertEqual(self.pointers['CardCmdResultPtrB'].value, second)
        self.assertEqual(self.pointers['CardWriteHandler'].value, C.cast(self.writers[1], C.c_void_p).value)
        self.assertEqual(self.scalars['CardCurrentCommand'].value, 100)
        callback = Write(self.pointers['CardWriteHandler'].value)
        for phase, value in [(3, 0x12345680), (4, -1), (5, 0x1234567F)]:
            self.scalars['CardRxCount'].value = phase
            self.assertEqual(callback(C.c_int32(value).value), 0)
        self.assertEqual(self.results[3], 255)
        self.assertEqual(self.results[27], 127)
        self.assertEqual(self.scalars['CardBytesRemaining'].value, 128)

    def test_command101_all_bytes_phases_and_wrapped_accumulation(self):
        phases = [-0x80000000, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 255, 0x7FFFFFFF]
        for phase in phases:
            for byte in range(256):
                for pattern in (0, 0x12345600, 0xFFFFFF00):
                    self.scalars['CardRxCount'].value = phase
                    self.results[3] = 0xFFFFF123
                    self.results[27] = 0x8000FE45
                    before, ram, scalars, pointers = self.snapshot()
                    expected = bytearray(before)
                    if phase == 2:
                        scalars['CardBytesRemaining'] = 8
                    elif phase in (7, 8, 9, 10):
                        index = 3 if phase in (7, 8) else 27
                        old = self.results[index]
                        value = byte if phase in (7, 9) else ((old << 8) + byte) & 0xFFFFFFFF
                        struct.pack_into('<I', expected, index * 4, value)
                    result = -1 if phase == 3 and byte != 0x5A else 0
                    self.assertEqual(self.response101(C.c_int32(pattern | byte).value), result)
                    self.assertEqual(self.snapshot(), (bytes(expected), ram, scalars, pointers))

    def test_command101_null_unused_outputs_and_aliased_word_sequence(self):
        self.pointers['CardCmdResultPtrA'].value = None
        self.pointers['CardCmdResultPtrB'].value = None
        for phase in (-0x80000000, -1, 0, 1, 2, 3, 4, 5, 6, 11, 0x7FFFFFFF):
            self.scalars['CardRxCount'].value = phase
            self.assertEqual(self.response101(0x5A), 0)
        address = C.addressof(self.results) + 3 * 4
        self.pointers['CardCmdResultPtrA'].value = address
        self.pointers['CardCmdResultPtrB'].value = address
        expected = bytes(self.results)
        for phase, byte, value in [(7, 0xFE, 0xFE), (8, 0xDC, 0xFEDC),
                                    (9, 0x98, 0x98), (10, 0x76, 0x9876)]:
            self.scalars['CardRxCount'].value = phase
            self.assertEqual(self.response101(byte), 0)
            self.assertEqual(self.results[3], value)
            self.assertEqual(bytes(self.results)[:12], expected[:12])
            self.assertEqual(bytes(self.results)[16:], expected[16:])

    def test_command101_constructor_installs_real_callback(self):
        C.c_int32.from_buffer(self.ram, 0xBDE4).value = 1
        first = C.addressof(self.results) + 12
        second = C.addressof(self.results) + 108
        start = self.lib._cardGetIdBegin
        start.argtypes = [C.c_int32, C.c_void_p, C.c_void_p]
        start.restype = C.c_int32
        self.assertEqual(start(0x1234, first, second), 1)
        self.assertEqual(self.scalars['CardCurrentCommand'].value, 101)
        self.assertEqual(self.pointers['CardWriteHandler'].value, C.cast(self.response101, C.c_void_p).value)
        callback = Write(self.pointers['CardWriteHandler'].value)
        for phase, byte in [(2, 0), (3, 0x5A), (7, 0xFE), (8, 0xDC), (9, 0xBA), (10, 0x98)]:
            self.scalars['CardRxCount'].value = phase
            self.assertEqual(callback(byte), 0)
        self.assertEqual((self.results[3], self.results[27]), (0xFEDC, 0xBA98))
        self.assertEqual(self.scalars['CardBytesRemaining'].value, 8)


if __name__ == '__main__':
    unittest.main()
