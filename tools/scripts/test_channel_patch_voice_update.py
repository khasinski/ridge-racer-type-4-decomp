"""Real patch/voice/decoder chain with cached slot and late state reloads."""
import ctypes as C
from pathlib import Path
import re
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

Patch = C.CFUNCTYPE(C.c_int32, C.c_int32, C.c_int32, C.c_int32)
Voice = C.CFUNCTYPE(None, C.c_int32, C.c_int32, C.c_int32, C.c_uint32, C.c_uint32)
FIELDS = ('cursor', 'slot', 'device', 'previous', 'channel', 'pitch', 'accumulator', 'counter',
          'mode', 'control_value', 'enabled', 'repeat', 'pending', 'serial', 'saved_cursor')


class ChannelVoiceFixture:
    handler = "800805D4"
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        directory = Path(cls.temp.name)
        layout = (root / 'include/rr/layout.h').read_text().replace('#include "types.h"', '')
        asm_header = (root / 'include/rr_asm.h').read_text()
        header = (root / 'include/rr/library.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
#include <stddef.h>
#include <string.h>
typedef uint8_t u8; typedef int8_t s8;
typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
''' + layout + header + asm_header + '''
s32 test_commands[32];
s32 (*test_command_sink)(s32 *);
s32 SpuSetReverbModeParam(s32 *command) { return test_command_sink(command); }
s16 test_mode_page[32768];
#define _svm_damper test_mode_page[0xA0/2]
RrLibControllerCallback test_loop_callbacks[0x200];
RrLibStateB0 *test_table[0x4100];
RrLibStateB0 test_records[9][17];
u8 test_streams[9][17][32];
u32 test_layout[] = {sizeof(RrLibStateB0), offsetof(RrLibStateB0,c0),
 offsetof(RrLibStateB0,b17), offsetof(RrLibStateB0,b26), offsetof(RrLibStateB0,b27),
 offsetof(RrLibStateB0,b37), offsetof(RrLibStateB0,h60), offsetof(RrLibStateB0,w88),
 offsetof(RrLibStateB0,w90), offsetof(RrLibStateB0,b1b),
 offsetof(RrLibStateB0,b1a), offsetof(RrLibStateB0,b15), offsetof(RrLibStateB0,b1d),
 offsetof(RrLibStateB0,b1c), offsetof(RrLibStateB0,b1f), offsetof(RrLibStateB0,c8)};
RrLibControllerCallback SsCtrlHandlerCC6,SsCtrlHandlerCC7,SsCtrlHandlerCC10,SsCtrlHandlerCC11,SsCtrlHandlerCC64;
RrLibControllerCallback SsCtrlHandlerCC98,SsCtrlHandlerCC99,SsCtrlHandlerCC100,SsCtrlHandlerCC101,SsCtrlHandlerCC91;
RrLibControllerResetCallback SsCtrlHandlerCC121;
void test_bind_controller(void) { SsCtrlHandlerCC11=_SsContExpression; }
s32 (*test_patch)(s32,s32,s32);
void (*test_voice)(s32,s32,s32,u32,u32);
s32 _SsVmSetProgVol(s16 device,s16 channel,s32 value) { return test_patch(device,channel,value); }
void _SsVmSetVol(s32 packed,s32 device,s32 channel,u32 pitch,u32 previous) {
 test_voice(packed,device,channel,pitch,previous);
}
void test_initialize(void) {
 s32 bank;
 memset(test_records,0xA5,sizeof(test_records));
 memset(test_streams,0x5A,sizeof(test_streams));
 memset(test_table,0,sizeof(test_table));
 memset(test_loop_callbacks,0,sizeof(test_loop_callbacks));
 for(bank=-4;bank<=4;bank++) test_table[0x4000-0xF98/4+bank]=&test_records[bank+4][8];
}
void test_prepare(s32 bank,s32 row,const u8 *input,u32 length,u32 accumulator) {
 u8 *stream=test_streams[bank+4][row+8];
 memcpy(stream,input,length);
 test_records[bank+4][row+8].c0=stream;
 test_records[bank+4][row+8].w88=accumulator;
}
'''
        source = source.replace('SsCtrlHandlerCC11=_SsContExpression',
                                {'80080434': 'SsCtrlHandlerCC7=_SsContMainVol',
                                 '800806C4': 'SsCtrlHandlerCC64=_SsContDamper',
                                 '80080774': 'SsCtrlHandlerCC91=_SsContExternal',
                                 '80080804': 'SsCtrlHandlerCC98=_SsContNrpn1',
                                 '80080904': 'SsCtrlHandlerCC99=_SsContNrpn2'}.get(
                                     cls.handler, 'SsCtrlHandlerCC11=_SsContExpression'))
        for name in ('80080434', '800805D4', '800806C4', '80080774', '80080804', '80080904', '80081E04', '800818B4',
                     '80087044', '80087054', '80086554'):
            body = (root / ('tools/scripts/fixtures/views/func_' + name + '.c')).read_text()
            body = body.replace('#include "rr_asm.h"', '')
            body = body.replace('#include "rr/library.h"', '').replace('#include "types.h"', '')
            body = body.replace('extern short _svm_damper;', '')
            body = body.replace('extern int _svm_rattr[];', '')
            body = body.replace('_svm_rattr', 'test_commands')
            body = body.replace('(s16 *)0x80110000U', 'test_mode_page')
            body = body.replace('extern RrLibStateB0 *_ss_score[];', '')
            body = body.replace('_SsMarkCallback', '(test_loop_callbacks + 0x100)')
            body = body.replace('_ss_score', '(test_table + 0x4000 - 0xF98 / 4)')
            body = body.replace('(RrLibStateB0 **)0x80110000U', '(test_table + 0x4000)')
            body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
            body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*', '', body)
            source += body
        (directory / 'function.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'function.c'),
                        '-o', str(directory / 'function.so')], check=True, capture_output=True)
        cls.lib = C.CDLL(str(directory / 'function.so'))
        values = list((C.c_uint32 * (len(FIELDS) + 1)).in_dll(cls.lib, 'test_layout'))
        cls.stride, cls.offsets = values[0], dict(zip(FIELDS, values[1:]))
        cls.records = (C.c_uint8 * (9 * 17 * cls.stride)).in_dll(cls.lib, 'test_records')
        cls.streams = (C.c_uint8 * (9 * 17 * 32)).in_dll(cls.lib, 'test_streams')
        cls.table = (C.c_void_p * 0x4100).in_dll(cls.lib, 'test_table')
        cls.prepare = cls.lib.test_prepare
        cls.prepare.argtypes = [C.c_int32, C.c_int32, C.c_void_p, C.c_uint32, C.c_uint32]
        cls.function = getattr(cls.lib, __import__('c_functions').name_at(cls.handler))
        cls.function.argtypes = [C.c_int32] * 3
        cls.function.restype = None
        cls.controller = cls.lib._SsSetControlChange
        cls.controller.argtypes = [C.c_int32] * 3
        cls.controller.restype = None

    def setUp(self):
        self.lib.test_initialize()
        self.lib.test_bind_controller()
        self.events = []
        self.callbacks = []
        def patch(*args):
            self.events.append(('patch', args, bytes(self.records)))
            return -1
        self.hook('test_patch', Patch, patch)
        self.hook('test_voice', Voice, lambda *args: self.events.append(('voice', args, bytes(self.records))))

    def hook(self, name, kind, function):
        value = kind(function)
        self.callbacks.append(value)
        C.c_void_p.in_dll(self.lib, name).value = C.cast(value, C.c_void_p).value

    def location(self, bank, row):
        index = (bank + 4) * 17 + row + 8
        return index * self.stride, C.addressof(self.streams) + index * 32

    def field(self, offset, name, kind, index=0):
        return kind.from_buffer(self.records, offset + self.offsets[name] + index * C.sizeof(kind))

    def prepare_record(self, bank, row, slot, data, accumulator):
        buffer = (C.c_uint8 * len(data))(*data)
        self.prepare(bank, row, buffer, len(data), accumulator)
        offset, stream = self.location(bank, row)
        self.field(offset, 'slot', C.c_uint8).value = slot
        device = [-128, -1, 0, 127][slot % 4]
        self.field(offset, 'device', C.c_int8).value = device
        for i in range(16):
            self.field(offset, 'channel', C.c_uint8, i).value = (i * 17 + 7) & 255
            self.field(offset, 'previous', C.c_uint8, i).value = (i * 11 + 19) & 255
            self.field(offset, 'pitch', C.c_uint16, i).value = (i * 0x1234 + 0xFADE) & 65535
        return offset, stream, device, (slot * 17 + 7) & 255, (slot * 11 + 19) & 255, (slot * 0x1234 + 0xFADE) & 65535

    def write_expected(self, expected, offset, name, data, index=0):
        start = offset + self.offsets[name] + index * len(data)
        expected[start:start + len(data)] = data


class ChannelPatchVoiceUpdateTest(ChannelVoiceFixture, unittest.TestCase):
    def test_all_slots_raw_values_and_signed_selectors_with_real_decoder(self):
        table_before = bytes(self.table)
        for bank, row in [(-4, -8), (0, 0), (4, 8)]:
            raw_bank, raw_row = 0x12340000 | (bank & 65535), 0x43210000 | (row & 65535)
            encoded = C.c_int16(raw_bank | (raw_row << 8)).value
            for slot in range(16):
                for value in (-0x80000000, -1, 0, 127, 128, 255, 256, 0x7FFFFFFF):
                    self.events.clear()
                    offset, stream, device, channel, previous, pitch = self.prepare_record(bank, row, slot, [0x81, 0], 0xFFFFFFFD)
                    before = bytes(self.records)
                    expected = bytearray(before)
                    self.write_expected(expected, offset, 'cursor', struct.pack('P', stream + 2))
                    self.write_expected(expected, offset, 'accumulator', struct.pack('<I', (0xFFFFFFFD + 1280) & 0xFFFFFFFF))
                    self.write_expected(expected, offset, 'counter', struct.pack('<i', 1280))
                    streams_before = bytes(self.streams)
                    self.function(raw_bank, raw_row, value)
                    self.assertEqual(self.events, [('patch', (device, channel, value & 255), before),
                                                   ('voice', (encoded, device, channel, pitch, previous), before)])
                    self.assertEqual(bytes(self.records), bytes(expected))
                    self.assertEqual(bytes(self.streams), streams_before)
        self.assertEqual(bytes(self.table), table_before)

    def test_first_callback_data_changes_are_reloaded_for_the_cached_slot(self):
        offset, stream, device, channel, previous, pitch = self.prepare_record(0, 0, 5, [0x7E, 0x81, 0], 99)
        initial = bytes(self.records)
        def patch(*args):
            self.events.append(('patch', args, bytes(self.records)))
            self.field(offset, 'slot', C.c_uint8).value = 9
            self.field(offset, 'device', C.c_int8).value = -77
            self.field(offset, 'channel', C.c_uint8, 5).value = 222
            self.field(offset, 'previous', C.c_uint8, 5).value = 201
            self.field(offset, 'pitch', C.c_uint16, 5).value = 0xFEDC
            self.field(offset, 'counter', C.c_int32).value = -666
            return -1
        def voice(*args):
            self.events.append(('voice', args, bytes(self.records)))
            self.field(offset, 'cursor', C.c_void_p).value = stream + 1
            self.field(offset, 'accumulator', C.c_uint32).value = 0xFFFFFFFC
            self.field(offset, 'previous', C.c_uint8, 5).value = 12
        self.hook('test_patch', Patch, patch)
        self.hook('test_voice', Voice, voice)
        expected = bytearray(initial)
        self.write_expected(expected, offset, 'slot', bytes([9]))
        self.write_expected(expected, offset, 'device', struct.pack('b', -77))
        self.write_expected(expected, offset, 'channel', bytes([222]), 5)
        self.write_expected(expected, offset, 'previous', bytes([201]), 5)
        self.write_expected(expected, offset, 'pitch', struct.pack('<H', 0xFEDC), 5)
        self.write_expected(expected, offset, 'counter', struct.pack('<i', -666))
        voice_snapshot = bytes(expected)
        self.write_expected(expected, offset, 'cursor', struct.pack('P', stream + 3))
        self.write_expected(expected, offset, 'accumulator', struct.pack('<I', (0xFFFFFFFC + 1280) & 0xFFFFFFFF))
        self.write_expected(expected, offset, 'previous', bytes([12]), 5)
        self.write_expected(expected, offset, 'counter', struct.pack('<i', 1280))
        self.function(0, 0, 0x123456AB)
        self.assertEqual(self.events, [('patch', (device, channel, 0xAB), initial),
                                       ('voice', (0, -77, 222, 0xFEDC, 201), voice_snapshot)])
        self.assertEqual(bytes(self.records), bytes(expected))

    def check_table_replacement(self, phase):
        offset, stream, device, channel, previous, pitch = self.prepare_record(0, 0, 0, [0], 55)
        other, other_stream, *_ = self.prepare_record(1, 0, 1, [0x81, 0], 100)
        replacement = C.addressof(self.records) + other
        initial = bytes(self.records)
        def patch(*args):
            self.events.append(('patch', args, bytes(self.records)))
            if phase == 'patch':
                self.table[0x4000 - 0xF98 // 4] = replacement
            return -1
        def voice(*args):
            self.events.append(('voice', args, bytes(self.records)))
            if phase == 'voice':
                self.table[0x4000 - 0xF98 // 4] = replacement
        self.hook('test_patch', Patch, patch)
        self.hook('test_voice', Voice, voice)
        expected = bytearray(initial)
        self.write_expected(expected, other, 'cursor', struct.pack('P', other_stream + 2))
        self.write_expected(expected, other, 'accumulator', struct.pack('<I', 1380))
        self.write_expected(expected, offset, 'counter', struct.pack('<i', 1280))
        self.function(0, 0, 255)
        self.assertEqual(self.events, [('patch', (device, channel, 255), initial),
                                       ('voice', (0, device, channel, pitch, previous), initial)])
        self.assertEqual(bytes(self.records), bytes(expected))
        self.assertEqual(self.table[0x4000 - 0xF98 // 4], replacement)

    def test_patch_table_replacement_redirects_decoder_but_not_voice_or_output(self):
        self.check_table_replacement('patch')

    def test_voice_table_replacement_is_seen_by_the_later_decoder(self):
        self.check_table_replacement('voice')


    def test_real_controller_calls_typed_patch_handler_then_real_decoder_once(self):
        bank, row = 1, 2
        offset, stream, device, channel, previous, pitch = self.prepare_record(bank, row, 5, [0xAB, 0x81, 0], 0xFFFFFFFD)
        published = bytearray(self.records)
        self.write_expected(published, offset, 'cursor', struct.pack('P', stream + 1))
        expected = bytearray(published)
        self.write_expected(expected, offset, 'cursor', struct.pack('P', stream + 3))
        self.write_expected(expected, offset, 'accumulator', struct.pack('<I', (0xFFFFFFFD + 1280) & 0xFFFFFFFF))
        self.write_expected(expected, offset, 'counter', struct.pack('<i', 1280))
        self.controller(0x12340001, 0x43210002, 0x1234000B)
        self.assertEqual(self.events, [('patch', (device, channel, 0xAB), bytes(published)),
                                       ('voice', (0x201, device, channel, pitch, previous), bytes(published))])
        self.assertEqual(bytes(self.records), bytes(expected))


if __name__ == '__main__':
    unittest.main()
