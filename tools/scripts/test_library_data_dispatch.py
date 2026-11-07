"""Actual data dispatcher, callback updates, reentry, and final port-state restore."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

Poll = C.CFUNCTYPE(C.c_int32)
Data = C.CFUNCTYPE(None, C.c_int32, C.c_void_p)


class LibraryDataDispatchTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        directory = Path(cls.temp.name)
        header = (root / 'include/rr/library.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
typedef uint8_t u8; typedef int8_t s8;
typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
''' + header + '''
u8 test_ports[2];
volatile u8 *CD_reg0;
u8 CdIntrStatusBytes[2];
RrLibDataCallback CD_cbsync,CD_cbready;
u8 test_data_page[65536];
s32 (*test_poll)(void);
s32 getintr(void) {return test_poll();}
'''
        body = (root / 'tools/scripts/fixtures/legacy/func_8008DDAC.c').read_text()
        body = body.replace('#include "rr/library.h"', '')
        if body.count('(u8 *)0x80110000U') != 2:
            raise AssertionError('Expected exactly two data-page adapters')
        body = body.replace('(u8 *)0x80110000U', 'test_data_page')
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*', '', body)
        source += body
        (directory / 'dispatch.c').write_text(source)
        result = subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'dispatch.c'),
                                 '-o', str(directory / 'dispatch.so')], capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory / 'dispatch.so'))
        cls.function = cls.lib.CdCommandCallback
        cls.function.argtypes = []
        cls.function.restype = None
        cls.ports = (C.c_uint8 * 2).in_dll(cls.lib, 'test_ports')
        cls.port_pointer = C.c_void_p.in_dll(cls.lib, 'CD_reg0')
        cls.selectors = (C.c_uint8 * 2).in_dll(cls.lib, 'CdIntrStatusBytes')
        cls.page = (C.c_uint8 * 65536).in_dll(cls.lib, 'test_data_page')
        cls.callbacks = {flag: C.c_void_p.in_dll(cls.lib, name)
                         for flag, name in ((4, 'CD_cbready'), (2, 'CD_cbsync'))}
        cls.poll_global = C.c_void_p.in_dll(cls.lib, 'test_poll')

    def setUp(self):
        self.events = []
        self.queue = []
        self.polls_after_end = 0
        self.keep_alive = []
        self.ports[:] = 0xFF, 0xA5
        self.port_pointer.value = C.addressof(self.ports)
        self.selectors[:] = 0x80, 0xFF
        C.memset(C.addressof(self.page), 0xA5, len(self.page))
        for pointer in self.callbacks.values():
            pointer.value = None
        def poll():
            if not self.queue:
                self.polls_after_end += 1
                result = 0
            else:
                result = self.queue.pop(0)
            self.events.append(('poll', result, bytes(self.ports)))
            return result
        self.poll = Poll(poll)
        self.poll_global.value = C.cast(self.poll, C.c_void_p).value

    def install(self, flag, label, action=None):
        def callback(value, pointer):
            self.events.append((label, value, pointer, bytes(self.ports)))
            if action:
                action(value, pointer)
        hook = Data(callback)
        self.keep_alive.append(hook)
        self.callbacks[flag].value = C.cast(hook, C.c_void_p).value
        return self.callbacks[flag].value

    def test_every_initial_port_byte_is_masked_after_terminating_poll(self):
        for byte in range(256):
            self.ports[:] = byte, 0xA5
            self.events.clear()
            self.queue = [0]
            self.function()
            self.assertEqual(self.events, [('poll', 0, bytes([byte, 0xA5]))])
            self.assertEqual(list(self.ports), [byte & 3, 0xA5])
        self.assertEqual(self.polls_after_end, 0)

    def test_flags_null_callbacks_unsigned_selectors_and_data_pointer_routing(self):
        for enabled in ((), (4,), (2,), (4, 2)):
            self.setUp()
            for flag in enabled:
                self.install(flag, str(flag))
            results = [4, 2, 6, 8, -1, 0]
            self.queue = list(results)
            before = bytes(self.page)
            self.function()
            expected = []
            for result in results:
                expected.append(('poll', result, b'\xff\xa5'))
                if result:
                    for flag, selector, offset in ((4, 255, 0x320), (2, 128, 0x318)):
                        if result & flag and flag in enabled:
                            expected.append((str(flag), selector, C.addressof(self.page)+offset, b'\xff\xa5'))
            self.assertEqual(self.events, expected)
            self.assertEqual(list(self.ports), [3, 0xA5])
            self.assertEqual(bytes(self.page), before)
            self.assertEqual(self.polls_after_end, 0)

    def test_first_callback_updates_second_callback_selectors_and_final_port_pointer(self):
        def second(value, pointer):
            self.port_pointer.value = C.addressof(self.ports)+1
            self.ports[1] = 0xFA
        def first(value, pointer):
            self.callbacks[4].value = None
            self.selectors[0] = 7
            self.install(2, 'new_second', second)
            self.ports[0] = 0xF0
            C.c_uint8.from_address(pointer).value = 0x42
        self.install(4, 'first', first)
        self.queue = [6, 6, 0]
        self.function()
        self.assertEqual(self.events, [
            ('poll', 6, b'\xff\xa5'),
            ('first', 255, C.addressof(self.page)+0x320, b'\xff\xa5'),
            ('new_second', 7, C.addressof(self.page)+0x318, b'\xf0\xa5'),
            ('poll', 6, b'\xf0\xfa'),
            ('new_second', 7, C.addressof(self.page)+0x318, b'\xf0\xfa'),
            ('poll', 0, b'\xf0\xfa')])
        self.assertEqual(list(self.ports), [0xF0, 3])
        self.assertIsNone(self.callbacks[4].value)
        self.assertEqual(self.page[0x320], 0x42)
        expected = bytes([0xA5])*0x320 + b'\x42' + bytes([0xA5])*(len(self.page)-0x321)
        self.assertEqual(bytes(self.page), expected)
        self.assertEqual(self.polls_after_end, 0)

    def test_reentrant_dispatch_preserves_each_invocations_saved_state(self):
        def first(value, pointer):
            self.ports[0] = 0xF2
            self.function()
        def second(value, pointer):
            self.ports[0] = 0xF1
        self.install(4, 'first', first)
        self.install(2, 'second', second)
        self.queue = [4, 2, 0, 0]
        self.function()
        self.assertEqual([event[:2] for event in self.events],
                         [('poll', 4), ('first', 255), ('poll', 2), ('second', 128), ('poll', 0), ('poll', 0)])
        self.assertEqual(self.events[-1], ('poll', 0, b'\x02\xa5'))
        self.assertEqual(list(self.ports), [3, 0xA5])
        self.assertEqual(self.polls_after_end, 0)


if __name__ == '__main__':
    unittest.main()
