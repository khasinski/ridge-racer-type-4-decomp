"""Execute the real alternating-state callback with one shared native RAM page."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

Callback = C.CFUNCTYPE(None)


class Page(C.Structure):
    _fields_ = [('prefix', C.c_uint8 * 0x4D40), ('callback', C.c_void_p),
                ('gap', C.c_uint8 * (0xC - C.sizeof(C.c_void_p))), ('state', C.c_int32),
                ('suffix', C.c_uint8 * (0x10000 - 0x4D50))]


class ChannelToggleCallbackTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        directory = Path(cls.temp.name)
        body = (root / 'tools/scripts/fixtures/legacy/func_80082D00.c').read_text()
        body = body.replace('#include "types.h"', '')
        body = body.replace('extern s32 SndSeqTickEnvDividedPhase;', '').replace('extern void (*SndSeqTickEnvCallback)(void);', '')
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r'"\$[0-9]+", ', '', body)
        body = body.replace('(u8 *)0x800A0000U', '((u8 *)&test_page)')
        source = '''#include <stdint.h>
typedef uint8_t u8; typedef int32_t s32;
struct { u8 prefix[0x4D40]; void (*callback)(void);
 u8 gap[0xC - sizeof(void (*)(void))]; s32 state;
 u8 suffix[0x10000 - 0x4D50]; } test_page;
#define SndSeqTickEnvDividedPhase test_page.state
#define SndSeqTickEnvCallback test_page.callback
''' + body
        (directory / 'function.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'function.c'),
                        '-o', str(directory / 'function.so')], check=True, capture_output=True)
        cls.lib = C.CDLL(str(directory / 'function.so'))
        cls.page = Page.in_dll(cls.lib, 'test_page')
        cls.function = cls.lib._SsTickCallbackDivided
        cls.function.argtypes = []
        cls.function.restype = None

    def setUp(self):
        C.memset(C.addressof(self.page), 0xA5, C.sizeof(self.page))
        self.events = []
        self.callback = Callback(lambda: self.events.append(bytes(self.page)))
        self.page.callback = C.cast(self.callback, C.c_void_p).value

    def test_zero_sets_one_without_callback_or_other_writes(self):
        for callback in (None, C.cast(self.callback, C.c_void_p).value):
            self.page.state = 0
            self.page.callback = callback
            expected = Page.from_buffer_copy(self.page)
            expected.state = 1
            self.function()
            self.assertEqual(bytes(self.page), bytes(expected))
            self.assertEqual(self.events, [])

    def test_every_nonzero_state_publishes_zero_before_callback(self):
        for state in (1, 2, 127, -1, -0x80000000, 0x7FFFFFFF):
            self.events.clear()
            self.page.state = state
            expected = Page.from_buffer_copy(self.page)
            expected.state = 0
            self.function()
            self.assertEqual(self.events, [bytes(expected)])
            self.assertEqual(bytes(self.page), bytes(expected))

    def test_callback_state_and_pointer_mutations_survive(self):
        replacement = Callback(lambda: self.events.append(b'replacement'))
        def mutate():
            self.events.append(self.page.state)
            self.page.state = -42
            self.page.callback = C.cast(replacement, C.c_void_p).value
        self.callback = Callback(mutate)
        self.page.callback = C.cast(self.callback, C.c_void_p).value
        self.page.state = 1
        self.function()
        self.assertEqual(self.events, [0])
        self.assertEqual(self.page.state, -42)
        self.assertEqual(self.page.callback, C.cast(replacement, C.c_void_p).value)
        self.function()
        self.assertEqual(self.events, [0, b'replacement'])
        self.assertEqual(self.page.state, 0)

    def test_reentrant_call_sees_zero_and_arms_next_invocation(self):
        def reenter():
            self.events.append(self.page.state)
            self.function()
            self.events.append(self.page.state)
        self.callback = Callback(reenter)
        self.page.callback = C.cast(self.callback, C.c_void_p).value
        self.page.state = 1
        self.function()
        self.assertEqual(self.events, [0, 1])
        self.assertEqual(self.page.state, 1)


if __name__ == '__main__':
    unittest.main()
