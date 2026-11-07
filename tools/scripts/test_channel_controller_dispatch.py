"""Real controller routing and VLQ decoder with typed two/three-argument hooks."""
import ctypes as C
from pathlib import Path
import re
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

Control = C.CFUNCTYPE(None, C.c_int32, C.c_int32, C.c_int32)
Reset = C.CFUNCTYPE(None, C.c_int32, C.c_int32)
ROUTES = {6: 'SsCtrlHandlerCC6', 7: 'SsCtrlHandlerCC7', 10: 'SsCtrlHandlerCC10',
          11: 'SsCtrlHandlerCC11', 64: 'SsCtrlHandlerCC64', 91: 'SsCtrlHandlerCC91',
          98: 'SsCtrlHandlerCC98', 99: 'SsCtrlHandlerCC99', 100: 'SsCtrlHandlerCC100', 101: 'SsCtrlHandlerCC101'}


class ChannelControllerDispatchTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        directory = Path(cls.temp.name)
        asm_header = (root / 'include/rr_asm.h').read_text()
        header = (root / 'include/rr/library.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
#include <stddef.h>
#include <string.h>
typedef uint8_t u8; typedef int8_t s8;
typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
''' + header + asm_header + '''
RrLibStateB0 *test_table[0x4100];
RrLibStateB0 test_records[9][17];
u8 test_streams[9][17][32];
u32 test_layout[] = {sizeof(RrLibStateB0), offsetof(RrLibStateB0,b26),
                    offsetof(RrLibStateB0,w88), offsetof(RrLibStateB0,w90)};
RrLibControllerResetCallback SsCtrlHandlerCC121;
void test_initialize(void) {
 s32 bank;
 memset(test_records,0xA5,sizeof(test_records));
 memset(test_streams,0x5A,sizeof(test_streams));
 memset(test_table,0,sizeof(test_table));
 for(bank=-4;bank<=4;bank++)
  test_table[0x4000-0xF98/4+bank]=&test_records[bank+4][8];
}
void test_prepare(s32 bank,s32 row,const u8 *bytes,u32 count,u32 accumulator) {
 u8 *stream=test_streams[bank+4][row+8];
 memcpy(stream,bytes,count);
 test_records[bank+4][row+8].c0=stream;
 test_records[bank+4][row+8].w88=accumulator;
}
'''
        source += '\n'.join('RrLibControllerCallback ' + name + ';' for name in ROUTES.values()) + '\n'
        for name in ('800818B4', '80081E04'):
            body = (root / ('tools/scripts/fixtures/views/func_' + name + '.c')).read_text()
            body = body.replace('#include "rr_asm.h"', '')
            body = body.replace('#include "rr/library.h"', '')
            body = body.replace('extern RrLibStateB0 *_ss_score[];', '')
            body = body.replace('_ss_score', '(test_table + 0x4000 - 0xF98 / 4)')
            body = body.replace('(RrLibStateB0 **)0x80110000U', '(test_table + 0x4000)')
            body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
            body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*', '', body)
            source += body
        (directory / 'function.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'function.c'),
                        '-o', str(directory / 'function.so')], check=True, capture_output=True)
        cls.lib = C.CDLL(str(directory / 'function.so'))
        cls.stride, cls.byte_offset, cls.accumulator_offset, cls.counter_offset = (C.c_uint32 * 4).in_dll(cls.lib, 'test_layout')
        cls.records = (C.c_uint8 * (9 * 17 * cls.stride)).in_dll(cls.lib, 'test_records')
        cls.streams = (C.c_uint8 * (9 * 17 * 32)).in_dll(cls.lib, 'test_streams')
        cls.table = (C.c_void_p * 0x4100).in_dll(cls.lib, 'test_table')
        cls.prepare = cls.lib.test_prepare
        cls.prepare.argtypes = [C.c_int32, C.c_int32, C.c_void_p, C.c_uint32, C.c_uint32]
        cls.function = cls.lib._SsSetControlChange
        cls.function.argtypes = [C.c_int32] * 3
        cls.function.restype = None
        cls.decoder = cls.lib._SsReadDeltaValue
        cls.decoder.argtypes = [C.c_int32] * 2
        cls.decoder.restype = C.c_int32

    def setUp(self):
        self.lib.test_initialize()
        self.events = []
        self.hooks = []
        for code, name in ROUTES.items():
            self.hook(name, Control, lambda *args, code=code: self.events.append((code, args, bytes(self.records))))
        self.hook('SsCtrlHandlerCC121', Reset, lambda *args: self.events.append((121, args, bytes(self.records))))

    def hook(self, name, kind, function):
        value = kind(function)
        self.hooks.append(value)
        C.c_void_p.in_dll(self.lib, name).value = C.cast(value, C.c_void_p).value

    def location(self, bank, row):
        index = (bank + 4) * 17 + row + 8
        return index * self.stride, C.addressof(self.streams) + index * 32

    def check_control(self, code, value, bank, row, high):
        self.events.clear()
        data = (C.c_uint8 * 3)(value, 0x81, 0)
        self.prepare(bank, row, data, 3, 0xFFFFFFFE)
        offset, stream = self.location(bank, row)
        published = bytearray(self.records)
        published[offset:offset + C.sizeof(C.c_void_p)] = struct.pack('P', stream + 1)
        expected = bytearray(published)
        if code in ROUTES:
            events = [(code, (bank, row, value), bytes(published))]
        elif code == 121:
            events = [(121, (bank, row), bytes(published))]
        else:
            events = []
            if code == 0:
                expected[offset + self.byte_offset] = value
            expected[offset:offset + C.sizeof(C.c_void_p)] = struct.pack('P', stream + 3)
            expected[offset + self.accumulator_offset:offset + self.accumulator_offset + 4] = struct.pack('<I', (0xFFFFFFFE + 1280) & 0xFFFFFFFF)
            expected[offset + self.counter_offset:offset + self.counter_offset + 4] = struct.pack('<I', 1280)
        streams_before = bytes(self.streams)
        raw_bank = 0x12340000 | (bank & 65535)
        raw_row = 0x43210000 | (row & 65535)
        self.function(raw_bank, raw_row, high | code)
        self.assertEqual(self.events, events)
        self.assertEqual(bytes(self.records), bytes(expected))
        self.assertEqual(bytes(self.streams), streams_before)

    def test_all_control_codes_and_raw_argument_high_bits(self):
        table_before = bytes(self.table)
        for code in range(256):
            bank, row = [(-4, -8), (0, 0), (4, 8)][code % 3]
            for high, value in [(0, 0), (0x12340000, 127), (0x80000000, 128), (0xFFFFFF00, 255)]:
                self.check_control(code, value, bank, row, high)
        self.assertEqual(bytes(self.table), table_before)

    def test_every_payload_byte_for_recognized_routes_and_metadata(self):
        for code in (0, *ROUTES, 121):
            for value in range(256):
                self.check_control(code, value, 0, 0, 0)

    def test_callback_changes_survive_without_extra_delta_decode(self):
        data = (C.c_uint8 * 3)(0xF1, 0x81, 0)
        self.prepare(-4, -8, data, 3, 10)
        offset, stream = self.location(-4, -8)
        def mutate(bank, row, value):
            self.events.append((bank, row, value, C.c_void_p.from_buffer(self.records, offset).value))
            C.c_void_p.from_buffer(self.records, offset).value = stream + 2
            C.c_int32.from_buffer(self.records, offset + self.counter_offset).value = -77
            C.c_int32.from_buffer(self.records, offset + self.accumulator_offset).value = 123
            self.records[offset + self.byte_offset] = 99
        self.hook(ROUTES[7], Control, mutate)
        expected = bytearray(self.records)
        expected[offset:offset + C.sizeof(C.c_void_p)] = struct.pack('P', stream + 2)
        expected[offset + self.counter_offset:offset + self.counter_offset + 4] = struct.pack('<i', -77)
        expected[offset + self.accumulator_offset:offset + self.accumulator_offset + 4] = struct.pack('<i', 123)
        expected[offset + self.byte_offset] = 99
        self.function(-4, -8, 7)
        self.assertEqual(self.events, [(-4, -8, 0xF1, stream + 1)])
        self.assertEqual(bytes(self.records), bytes(expected))

    def test_reset_callback_takes_two_arguments_and_preserves_its_mutations(self):
        data = (C.c_uint8 * 3)(0xA7, 0x81, 0)
        self.prepare(0, 0, data, 3, 10)
        offset, stream = self.location(0, 0)
        replacement = C.addressof(self.records) + self.location(1, 0)[0]
        def reset(bank, row):
            self.events.append((bank, row, C.c_void_p.from_buffer(self.records, offset).value))
            self.table[0x4000 - 0xF98 // 4] = replacement
            C.c_int32.from_buffer(self.records, offset + self.counter_offset).value = -99
        self.hook('SsCtrlHandlerCC121', Reset, reset)
        expected = bytearray(self.records)
        expected[offset:offset + C.sizeof(C.c_void_p)] = struct.pack('P', stream + 1)
        expected[offset + self.counter_offset:offset + self.counter_offset + 4] = struct.pack('<i', -99)
        self.function(0, 0, -135)  # low byte 121
        self.assertEqual(self.events, [(0, 0, stream + 1)])
        self.assertEqual(bytes(self.records), bytes(expected))
        self.assertEqual(self.table[0x4000 - 0xF98 // 4], replacement)

    def test_decoder_public_register_arguments_narrow_before_record_lookup(self):
        for bank, row in [(-4, -8), (0, 0), (4, 8)]:
            data = (C.c_uint8 * 2)(0x81, 0)
            self.prepare(bank, row, data, 2, 0xFFFFFFFF)
            offset, stream = self.location(bank, row)
            expected = bytearray(self.records)
            expected[offset:offset + C.sizeof(C.c_void_p)] = struct.pack('P', stream + 2)
            expected[offset + self.accumulator_offset:offset + self.accumulator_offset + 4] = struct.pack('<I', 1279)
            self.assertEqual(self.decoder(0x12340000 | (bank & 65535), 0x43210000 | (row & 65535)), 1280)
            self.assertEqual(bytes(self.records), bytes(expected))
        self.assertEqual(self.events, [])


if __name__ == '__main__':
    unittest.main()
