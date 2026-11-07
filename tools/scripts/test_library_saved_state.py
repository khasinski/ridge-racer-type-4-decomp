"""Recovered save/restore pair: callback order, shared layout and aliasing."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class State(C.Structure):
    _fields_ = [('active', C.c_uint16), ('pad', C.c_uint8 * 2), ('callbacks', C.c_void_p * 11),
                ('enabled_mask', C.c_uint16),
                ('register16', C.c_uint16), ('register32', C.c_int32),
                ('callback_data', C.c_uint8 * 4)]


Begin = C.CFUNCTYPE(None, C.c_void_p)
Event = C.CFUNCTYPE(None)


class SavedStateTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        directory = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        header = (root / 'include/rr/library.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
typedef uint8_t u8; typedef int8_t s8;
typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
''' + header + '''
RrLibSavedState IntrEnv;
u16 *I_MASK, *I_STAT;
s32 *DPCR_Intr;
void (*test_begin)(void *);
void (*test_end)(void);
void (*test_save_begin)(void);
void (*test_save_end)(void);
void HookEntryInt(void *data) { test_begin(data); }
void ExitCriticalSection(void) { test_end(); }
void EnterCriticalSection(void) { test_save_begin(); }
void ResetEntryInt(void) { test_save_end(); }
'''
        for name in ('8008B988', '8008B8E8'):
            body = (root / ('tools/scripts/fixtures/legacy/func_' + name + '.c')).read_text()
            body = body.replace('#include "rr/library.h"', '')
            body = body.replace('__asm__ volatile("nop");', '')
            source += body
        (directory / 'functions.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'functions.c'),
                        '-o', str(directory / 'functions.so')], check=True, capture_output=True)
        cls.library = C.CDLL(str(directory / 'functions.so'))
        cls.state = State.in_dll(cls.library, 'IntrEnv')
        cls.restore = cls.library.restartIntr
        cls.save = cls.library.stopIntr
        for function in (cls.restore, cls.save):
            function.argtypes = []
            function.restype = C.c_void_p

    def setUp(self):
        C.memset(C.addressof(self.state), 0xA5, C.sizeof(self.state))
        self.port16 = C.c_uint16(0x1234)
        self.port32 = C.c_int32(0x12345678)
        self.ack = C.c_uint16(0xBEEF)
        for name, value in [('I_MASK', self.port16), ('DPCR_Intr', self.port32),
                            ('I_STAT', self.ack)]:
            C.c_void_p.in_dll(self.library, name).value = C.addressof(value)
        self.events = []
        self.callbacks = []
        for name, kind in [('test_begin', Begin), ('test_end', Event),
                           ('test_save_begin', Event), ('test_save_end', Event)]:
            self.callback(name, kind, lambda *args, name=name: self.events.append(name))

    def callback(self, name, kind, function):
        callback = kind(function)
        self.callbacks.append(callback)
        C.c_void_p.in_dll(self.library, name).value = C.cast(callback, C.c_void_p).value

    def test_restore_reads_callback_changes_and_publishes_before_end(self):
        self.state.active = 0
        observed = []
        def begin(data):
            observed.append((data, self.state.active, self.port16.value, self.port32.value))
            self.state.register16 = 0xFEDC
            self.state.register32 = -1234567
        def end():
            observed.append((self.state.active, self.port16.value, self.port32.value))
        self.callback('test_begin', Begin, begin)
        self.callback('test_end', Event, end)
        before = bytes(self.state)
        self.assertEqual(self.restore(), C.addressof(self.state))
        self.assertEqual(observed, [(C.addressof(self.state) + State.callback_data.offset, 0, 0x1234, 0x12345678),
                                    (1, 0xFEDC, -1234567)])
        self.assertEqual(bytes(self.state)[2:0x32], before[2:0x32])
        self.assertEqual(bytes(self.state)[State.callback_data.offset:], before[State.callback_data.offset:])

    def test_restore_nonzero_gate_and_save_zero_gate_have_no_effects(self):
        for active in (1, 2, 255, 256, 65535):
            self.state.active = active
            before = bytes(self.state)
            self.assertIsNone(self.restore())
            self.assertEqual(bytes(self.state), before)
        self.state.active = 0
        before = bytes(self.state)
        self.assertIsNone(self.save())
        self.assertEqual(bytes(self.state), before)
        self.assertEqual(self.events, [])
        self.assertEqual((self.port16.value, self.port32.value, self.ack.value),
                         (0x1234, 0x12345678, 0xBEEF))

    def test_first_output_alias_changes_subsequent_word_read(self):
        self.state.active = 0
        self.state.register16 = 0xABCD
        self.state.register32 = 0x12345678
        C.c_void_p.in_dll(self.library, 'I_MASK').value = C.addressof(self.state) + State.register32.offset
        self.assertEqual(self.restore(), C.addressof(self.state))
        self.assertEqual(self.port32.value, 0x1234ABCD)
        self.assertEqual(self.events, ['test_begin', 'test_end'])

    def test_save_captures_after_begin_and_clears_active_after_end(self):
        self.state.active = 0x1234
        observed = []
        def begin():
            self.port16.value = 0xFEDC
            self.port32.value = C.c_int32(0xA5B6C7D8).value
        def end():
            observed.append((self.state.active, self.state.register16, self.state.register32,
                             self.port16.value, self.port32.value, self.ack.value))
        self.callback('test_save_begin', Event, begin)
        self.callback('test_save_end', Event, end)
        self.assertEqual(self.save(), C.addressof(self.state))
        self.assertEqual(observed, [(0x1234, 0xFEDC, C.c_int32(0xA5B6C7D8).value,
                                    0, 0x25364750, 0)])
        self.assertEqual(self.state.active, 0)


if __name__ == '__main__':
    unittest.main()
