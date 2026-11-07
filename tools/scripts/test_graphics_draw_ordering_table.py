"""DrawOTag: debug routing, fresh driver lookup, and real GPU register writer."""
import ctypes as C
from pathlib import Path
import re
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

Debug = C.CFUNCTYPE(C.c_int32, C.c_char_p, C.c_int32)
Dispatch = C.CFUNCTYPE(C.c_int32, C.c_void_p, C.c_int32, C.c_int32, C.c_int32)
Draw = C.CFUNCTYPE(None, C.c_ulong)


class GraphicsDrawOrderingTableTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        directory = Path(cls.temp.name)
        header = (root / 'include/rr/library.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
typedef uint8_t u8; typedef int8_t s8;
typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
''' + header + '''
const char DrawOTagFmt[]="DrawOTag(%08x)...\\n";
RrGpuDebugSettings D_800ABAC2;
s32 (*test_debug)(const char *,s32);
s32 test_printf(const char *format,...) {
 va_list args; s32 value; va_start(args,format); value=va_arg(args,s32); va_end(args);
 return test_debug(format,value);
}
s32 (*GPU_printf)(const char *,...) = test_printf;
RrLibGraphicsDriverPrefix test_drivers[2];
RrLibGraphicsDriverPrefix *GpuFuncTable;
u32 test_driver_size=sizeof(RrLibGraphicsDriverPrefix);
s32 test_gpu[4];
s32 *GPU_STATUS=&test_gpu[0], *DMA2_MADR=&test_gpu[1];
s32 *DMA2_BCR=&test_gpu[2], *DMA2_CHCR=&test_gpu[3];
void _cwc(unsigned long);
void test_configure(s32 index,s32 (*dispatch)(RrLibDrawOrderingTable,s32,s32,s32),
                    RrLibDrawOrderingTable draw) {
 test_drivers[index].dispatch=dispatch;
 test_drivers[index].draw_ordering_table=draw;
}
void test_select(s32 index) { GpuFuncTable=&test_drivers[index]; }
void test_initialize(void) {
 memset(test_drivers,0xA5,sizeof(test_drivers));
 memset(test_gpu,0xA5,sizeof(test_gpu));
 GpuFuncTable=&test_drivers[0];
}
'''
        for name in ('80093344', '80094B28'):
            body = (root / ('tools/scripts/fixtures/legacy/func_' + name + '.c')).read_text()
            body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
            body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
            body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*', '', body)
            source += body
        (directory / 'function.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'function.c'),
                        '-o', str(directory / 'function.so')], check=True, capture_output=True)
        cls.lib = C.CDLL(str(directory / 'function.so'))
        cls.function = cls.lib.DrawOTag
        cls.function.argtypes = [C.c_int32]
        cls.function.restype = None
        cls.configure = cls.lib.test_configure
        cls.configure.argtypes = [C.c_int32, C.c_void_p, C.c_void_p]
        cls.select = cls.lib.test_select
        cls.select.argtypes = [C.c_int32]
        cls.driver_size = C.c_uint32.in_dll(cls.lib, 'test_driver_size').value
        cls.driver_bytes = (C.c_uint8 * (2 * cls.driver_size)).in_dll(cls.lib, 'test_drivers')
        cls.gpu = (C.c_uint32 * 4).in_dll(cls.lib, 'test_gpu')
        cls.level = C.c_uint8.in_dll(cls.lib, 'D_800ABAC2')
        cls.draw = cls.lib._cwc
        cls.draw.argtypes = [C.c_ulong]
        cls.draw.restype = None
        cls.draw_address = C.cast(cls.draw, C.c_void_p).value

    def setUp(self):
        self.lib.test_initialize()
        self.events = []
        self.callbacks = []
        self.level.value = 0
        def dispatch(command, value, third, fourth):
            self.events.append(('dispatch', command, value, third, fourth))
            Draw(command)(value)
            return -1
        self.dispatch = self.keep(Dispatch, dispatch)
        self.configure(0, C.cast(self.dispatch, C.c_void_p), self.draw_address)
        def debug(format, value):
            self.events.append(('debug', format, value))
            return -1
        self.debug = self.keep(Debug, debug)
        C.c_void_p.in_dll(self.lib, 'test_debug').value = C.cast(self.debug, C.c_void_p).value

    def keep(self, kind, function):
        if kind is Draw:
            original = function
            function = lambda value: original(C.c_int32(value).value)
        callback = kind(function)
        self.callbacks.append(callback)
        return callback

    def test_all_debug_levels_and_raw_addresses_with_real_gpu_writer(self):
        before = bytes(self.driver_bytes)
        for level in range(256):
            for value in (-0x80000000, -1, 0, 1, 0x12345678, 0x7FFFFFFF):
                self.events.clear()
                self.level.value = level
                self.function(value)
                expected = [('debug', b'DrawOTag(%08x)...\n', value)] if level >= 2 else []
                expected.append(('dispatch', self.draw_address, value, 0, 0))
                self.assertEqual(self.events, expected)
                self.assertEqual(list(self.gpu), [0x04000002, value & 0xFFFFFFFF, 0, 0x01000401])
                self.assertEqual(bytes(self.driver_bytes), before)
                self.assertEqual(self.level.value, level)

    def test_debug_can_replace_driver_before_dispatch_and_preserves_raw_value(self):
        self.level.value = 2
        replacement_draw = self.keep(Draw, lambda value: self.events.append(('replacement-draw', value)))
        def replacement_dispatch(command, value, third, fourth):
            self.events.append(('replacement-dispatch', command, value, third, fourth))
            Draw(command)(value)
            return -77
        replacement = self.keep(Dispatch, replacement_dispatch)
        draw_address = C.cast(replacement_draw, C.c_void_p).value
        self.configure(1, C.cast(replacement, C.c_void_p), draw_address)
        def debug(format, value):
            self.events.append(('debug', format, value))
            self.select(1)
            self.level.value = 0
            return 0
        self.debug = self.keep(Debug, debug)
        C.c_void_p.in_dll(self.lib, 'test_debug').value = C.cast(self.debug, C.c_void_p).value
        gpu_before = bytes(self.gpu)
        self.function(-0x76543210)
        self.assertEqual(self.events, [('debug', b'DrawOTag(%08x)...\n', -0x76543210),
                                      ('replacement-dispatch', draw_address, -0x76543210, 0, 0),
                                      ('replacement-draw', -0x76543210)])
        self.assertEqual(bytes(self.gpu), gpu_before)
        self.assertEqual(self.level.value, 0)

    def test_debug_can_replace_callback_fields_on_the_same_driver(self):
        self.level.value = 255
        replacement_draw = self.keep(Draw, lambda value: self.events.append(('draw', value)))
        draw_address = C.cast(replacement_draw, C.c_void_p).value
        def debug(format, value):
            self.events.append(('debug', format, value))
            self.configure(0, C.cast(self.dispatch, C.c_void_p), draw_address)
            return 1
        self.debug = self.keep(Debug, debug)
        C.c_void_p.in_dll(self.lib, 'test_debug').value = C.cast(self.debug, C.c_void_p).value
        self.function(-1)
        self.assertEqual(self.events, [('debug', b'DrawOTag(%08x)...\n', -1),
                                      ('dispatch', draw_address, -1, 0, 0), ('draw', -1)])

    def test_nested_draw_during_debug_keeps_each_ordering_table(self):
        self.level.value = 2
        def debug(format, value):
            self.events.append(('debug', format, value))
            self.level.value = 0
            self.function(0x12345678)
            return -1
        self.debug = self.keep(Debug, debug)
        C.c_void_p.in_dll(self.lib, 'test_debug').value = C.cast(self.debug, C.c_void_p).value
        self.function(-1)
        self.assertEqual(self.events, [('debug', b'DrawOTag(%08x)...\n', -1),
                                      ('dispatch', self.draw_address, 0x12345678, 0, 0),
                                      ('dispatch', self.draw_address, -1, 0, 0)])
        self.assertEqual(list(self.gpu), [0x04000002, 0xFFFFFFFF, 0, 0x01000401])


if __name__ == '__main__':
    unittest.main()
