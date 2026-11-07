"""Check typed channel callback installation and the real readiness callback."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class ChannelStatus(C.Structure):
    _fields_ = [('pad00', C.c_uint8 * 4), ('entries04', C.c_void_p), ('pad08', C.c_uint8 * 4), ('reset0C', C.c_int32), ('owner10', C.c_void_p), ('update14', C.c_void_p),
                ('pad18', C.c_uint8 * 12), ('argument24', C.c_uint8), ('pad25', C.c_uint8 * 3), ('data28', C.c_void_p),
                ('data2C', C.c_void_p), ('input30', C.c_void_p), ('length34', C.c_uint8), ('pad35', C.c_uint8), ('length36', C.c_uint8),
                ('command37', C.c_uint8), ('pad38', C.c_uint8 * 4), ('packet', C.c_void_p),
                ('buffer40', C.c_void_p), ('pad44', C.c_uint8 * 2), ('phase', C.c_uint8), ('state47', C.c_uint8),
                ('pad48', C.c_uint8), ('state49', C.c_uint8), ('pad4A', C.c_uint8 * (0x57-0x4A)), ('gates57', C.c_uint8 * 6), ('payload5D', C.c_uint8 * 6), ('pad63', C.c_uint8 * (0xE4-0x63)),
                ('argumentE4', C.c_uint8), ('padE5', C.c_uint8), ('pending', C.c_uint16)]


class ChannelRecord(C.Structure):
    _fields_ = [('status', ChannelStatus), ('responseE8', C.c_uint8), ('entry_countE9', C.c_uint8), ('padEA', C.c_uint8 * 6)]


class ChannelCallbacksTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        directory = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        header = (root / 'include/rr/channels.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        header = header.replace('extern RrChannelUpdateCallback PadPacketDoneCallback;', '')
        source = '''#include <stdint.h>
typedef uint8_t u8; typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
''' + header + '''
RrChannelStatusCallback PadChannelDispatchPtr, PadChannelBusyFn;
RrChannelUpdateCallback test_slots[0x10000 / sizeof(RrChannelUpdateCallback)];
#define PadPacketDoneCallback test_slots[0xBC88 / sizeof(RrChannelUpdateCallback)]
void *test_last_channel;
s32 _padChannelDispatch(void *channel) { return 101; }
void _padPacketDone(void *channel) { test_last_channel = channel; }
s32 test_first_status(void *channel) { return PadChannelDispatchPtr(channel); }
s32 test_second_status(void *channel) { return PadChannelBusyFn(channel); }
void test_update(void *channel) { PadPacketDoneCallback(channel); }
'''
        for name in ('_padInitDirSeq', '_padChannelBusy'):
            body = (root / 'tools/scripts/fixtures/views' / (name + '.c')).read_text()
            body = body.replace('#include "rr/channels.h"', '')
            body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
            body = re.sub(r'"\$[0-9]+", ', '', body)
            body = re.sub(r' : "\$[0-9]+"', '', body)
            body = body.replace('(u8 *)0x800B0000U', '((u8 *)test_slots + 0x10000)')
            source += body
        (directory / 'functions.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'functions.c'),
                        '-o', str(directory / 'functions.so')], check=True, capture_output=True)
        cls.library = C.CDLL(str(directory / 'functions.so'))
        cls.install = cls.library._padInitDirSeq
        cls.install.argtypes = []
        cls.install.restype = None
        cls.slots = (C.c_void_p * (0x10000 // C.sizeof(C.c_void_p))).in_dll(cls.library, 'test_slots')
        for name in ('test_first_status', 'test_second_status'):
            function = getattr(cls.library, name)
            function.argtypes = [C.c_void_p]
            function.restype = C.c_int32
        cls.library.test_update.argtypes = [C.c_void_p]
        cls.library.test_update.restype = None

    def test_installation_binds_three_correct_functions_and_preserves_other_slots(self):
        first = C.cast(self.library._padChannelDispatch, C.c_void_p).value
        second = C.cast(self.library._padChannelBusy, C.c_void_p).value
        update = C.cast(self.library._padPacketDone, C.c_void_p).value
        C.c_void_p.in_dll(self.library, 'PadChannelDispatchPtr').value = second
        C.c_void_p.in_dll(self.library, 'PadChannelBusyFn').value = first
        index = 0xBC88 // C.sizeof(C.c_void_p)
        self.slots[index - 1:index + 2] = [update, None, update]
        expected = list(self.slots)
        expected[index] = update
        for _ in range(2):
            self.install()
            self.assertEqual(C.c_void_p.in_dll(self.library, 'PadChannelDispatchPtr').value, first)
            self.assertEqual(C.c_void_p.in_dll(self.library, 'PadChannelBusyFn').value, second)
            self.assertEqual(list(self.slots), expected)
        state = ChannelStatus()
        self.assertEqual(self.library.test_first_status(C.byref(state)), 101)
        self.library.test_update(C.byref(state))
        self.assertEqual(C.c_void_p.in_dll(self.library, 'test_last_channel').value, C.addressof(state))

    def test_readiness_callback_uses_pending_count_and_phase(self):
        self.install()
        state = ChannelStatus()
        for phase in range(256):
            state.phase = phase
            for pending in (0, 1, 0x7FFF, 0x8000, 0xFFFF):
                state.pending = pending
                before = C.string_at(C.addressof(state), C.sizeof(state))
                expected = int(pending == 0 or phase != 0xFF)
                self.assertEqual(self.library.test_second_status(C.byref(state)), expected)
                self.assertEqual(C.string_at(C.addressof(state), C.sizeof(state)), before)


if __name__ == '__main__':
    unittest.main()
