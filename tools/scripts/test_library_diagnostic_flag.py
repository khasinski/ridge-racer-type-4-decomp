"""Check byte truncation, adjacent context and callback timing of the flag setter."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

Callback = C.CFUNCTYPE(None, C.c_void_p, C.c_int32, C.c_int32, C.c_int32)


class DiagnosticFlagTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        directory = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        body = (root / 'tools/scripts/fixtures/legacy/func_80092B48.c').read_text()
        body = body.replace('#include "types.h"', '''#include <stdint.h>
typedef uint8_t u8; typedef int32_t s32;
u8 test_state[64];
char test_format[0x10000];
#define GpuDebugLevel (test_state + 2)
''')
        body = body.replace('extern u8 GpuDebugLevel[];', '')
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r'"\$[0-9]+", ', '', body)
        body = re.sub(r' : "\$[0-9]+"', '', body)
        body = body.replace('(const char *)0x80010000U', 'test_format')
        body += '\nvoid (*GPU_printf)(const char *, s32, s32, s32);\n'
        (directory / 'function.c').write_text(body)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'function.c'),
                        '-o', str(directory / 'function.so')], check=True, capture_output=True)
        cls.library = C.CDLL(str(directory / 'function.so'))
        cls.state = (C.c_uint8 * 64).in_dll(cls.library, 'test_state')
        cls.format = (C.c_char * 0x10000).in_dll(cls.library, 'test_format')
        cls.function = cls.library.SetGraphDebug
        cls.function.argtypes = [C.c_int32]
        cls.function.restype = C.c_int32

    def setUp(self):
        self.events = []
        def callback(format, enabled, second, third):
            self.events.append((format, enabled, second, third, self.state[2]))
        self.callback = Callback(callback)
        C.c_void_p.in_dll(self.library, 'GPU_printf').value = C.cast(self.callback, C.c_void_p).value

    def test_every_new_and_previous_byte(self):
        self.state[:] = bytes([0xA5]) * 64
        for previous in range(256):
            for enabled in range(256):
                self.state[2] = previous
                self.events.clear()
                before = bytearray(self.state)
                self.assertEqual(self.function(enabled), previous)
                before[2] = enabled
                self.assertEqual(bytes(self.state), bytes(before))
                expected = [(C.addressof(self.format) + 0x1E24, enabled, 0xA5, 0xA5, enabled)] if enabled else []
                self.assertEqual(self.events, expected)

    def test_upper_bits_do_not_change_gate_or_stored_byte(self):
        for value in (-2147483648, -65536, -256, -1, 0, 256, 65536, 2147483647):
            self.state[:] = bytes(range(64))
            self.events.clear()
            self.assertEqual(self.function(value), 2)
            self.assertEqual(self.state[2], value & 255)
            self.assertEqual(len(self.events), bool(value & 255))
            if self.events:
                self.assertEqual(self.events[0][1:], (value & 255, 0, 3, value & 255))

    def test_callback_changes_do_not_replace_saved_return_value(self):
        self.state[2] = 231
        self.callback = Callback(lambda *args: self.state.__setitem__(2, 17))
        C.c_void_p.in_dll(self.library, 'GPU_printf').value = C.cast(self.callback, C.c_void_p).value
        self.assertEqual(self.function(5), 231)
        self.assertEqual(self.state[2], 17)


if __name__ == '__main__':
    unittest.main()
