"""Real device-table search, native pointer layout, and handler replacement ordering."""
import ctypes as C
from pathlib import Path
import re
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

Handler = C.CFUNCTYPE(C.c_int32, C.POINTER(C.c_int32), C.c_int32, C.c_int32)
Compare = C.CFUNCTYPE(C.c_int32, C.c_void_p, C.c_void_p)


class BiosDeviceHandlerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        directory = Path(cls.temp.name)
        header = (root / 'include/rr/bios_devices.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
#include <stddef.h>
#include <string.h>
typedef uint8_t u8; typedef int8_t s8; typedef uint32_t u32; typedef int32_t s32;
''' + header + '''
RrBiosDevice test_devices[8],test_other_devices[8];
RrBiosDevice *test_kernel_table;
u32 test_kernel_bytes;
char test_name_page[65536];
#define CardFirstFileDeviceName (test_name_page+0x43C0)
char *test_requested_path;
void *test_directory;
u32 test_first_calls;
void *firstfile2(char *path,void *directory) {
 test_requested_path=path;test_directory=directory;test_first_calls++;return directory;
}
RrBiosDeviceHandler CardFirstFileHandler;
s32 (*test_compare)(const char *,const char *);
s32 PsyqStrcmp(const char *first,const char *second) {return test_compare(first,second);}
s32 test_prior_handler(s32 *first,s32 second,s32 third) {return -1;}
u32 test_layout[]={sizeof(RrBiosDevice),offsetof(RrBiosDevice,name),offsetof(RrBiosDevice,handler)};
const char *test_names[]={0,"other","tty","last"};
void test_set_name(u32 index,u32 which) {test_devices[index].name=test_names[which];}
void test_set_other_name(u32 index,u32 which) {test_other_devices[index].name=test_names[which];}
void test_initialize(u32 bytes) {
 u32 i;memset(test_devices,0xA5,sizeof(test_devices));
 memset(test_other_devices,0x69,sizeof(test_other_devices));
 for(i=0;i<8;i++) {test_devices[i].name=0;test_devices[i].handler=test_prior_handler;}
 test_kernel_table=test_devices;test_kernel_bytes=bytes;test_first_calls=0;
 memcpy(test_name_page+0x43C0,"tty",4);
}
void test_replace_kernel_table(void) {test_kernel_table=test_other_devices;test_kernel_bytes=0;}
void test_select_other_table(u32 count) {test_kernel_table=test_other_devices;test_kernel_bytes=count*sizeof(RrBiosDevice);}
'''
        body = (root / 'tools/scripts/fixtures/views/func_800972B0.c').read_text()
        body = body.replace('#include "rr/bios_devices.h"', '')
        sites = ('*(RrBiosDevice **)0x150', '*(u32 *)0x154', '(char *)0x80110000U')
        for site in sites:
            if body.count(site) != 1:
                raise AssertionError('Unexpected kernel/page adapter site count: ' + site)
        body = body.replace(sites[0], 'test_kernel_table').replace(sites[1], 'test_kernel_bytes')
        body = body.replace(sites[2], 'test_name_page')
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*', '', body)
        source += body
        first = (root / 'tools/scripts/fixtures/views/func_80097114.c').read_text()
        first = first.replace('#include "rr/bios_devices.h"', '')
        for site, replacement in zip(sites[:2], ('test_kernel_table', 'test_kernel_bytes')):
            if first.count(site) != 2:
                raise AssertionError('Unexpected first-file kernel adapter count: ' + site)
            first = first.replace(site, replacement)
        source += first
        (directory / 'devices.c').write_text(source)
        result = subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'devices.c'),
                                 '-o', str(directory / 'devices.so')], capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory / 'devices.so'))
        cls.size, cls.name_offset, cls.handler_offset = (C.c_uint32 * 3).in_dll(cls.lib, 'test_layout')
        cls.records = (C.c_ubyte * (cls.size * 8)).in_dll(cls.lib, 'test_devices')
        cls.other = (C.c_ubyte * (cls.size * 8)).in_dll(cls.lib, 'test_other_devices')
        cls.page = (C.c_char * 65536).in_dll(cls.lib, 'test_name_page')
        cls.function = cls.lib._firstfile_hook
        cls.function.argtypes = [C.POINTER(C.c_int32), C.c_int32, C.c_int32]
        cls.function.restype = C.c_int32
        cls.first = cls.lib.firstfile
        cls.first.argtypes = [C.c_char_p, C.c_void_p]
        cls.first.restype = C.c_void_p
        cls.first_calls = C.c_uint32.in_dll(cls.lib, 'test_first_calls')
        cls.requested_path = C.c_void_p.in_dll(cls.lib, 'test_requested_path')
        cls.directory = C.c_void_p.in_dll(cls.lib, 'test_directory')
        cls.handler_global = C.c_void_p.in_dll(cls.lib, 'CardFirstFileHandler')
        cls.compare_global = C.c_void_p.in_dll(cls.lib, 'test_compare')

    def setUp(self):
        self.events = []
        self.handlers = []
        for label in ('A', 'B'):
            def handle(context, second, third, label=label):
                self.events.append((label, C.addressof(context.contents), context[0], second, third))
                return -77
            self.handlers.append(Handler(handle))
        self.original = C.cast(self.handlers[0], C.c_void_p).value
        self.replacement = C.cast(self.handlers[1], C.c_void_p).value
        self.handler_global.value = self.original
        self.compare_hook = Compare(self.compare_names)
        self.compare_global.value = C.cast(self.compare_hook, C.c_void_p).value

    def compare_names(self, first, second):
        self.events.append(('compare', C.string_at(first), C.string_at(second), second))
        return 0 if C.string_at(first) == C.string_at(second) else 1

    def expected(self, before, index, handler):
        result = bytearray(before)
        if index is not None:
            struct.pack_into('P', result, index*self.size+self.handler_offset, handler)
        return bytes(result)

    def test_empty_partial_null_names_duplicates_and_first_match(self):
        patterns = ([0]*8, [1]*8, [2]*8, [0, 1, 0, 2, 1, 2, 0, 3], [1]*7+[2])
        for count in range(9):
            for extra in (0, 1, self.size-1):
                for pattern in patterns:
                    for initial in (0, -1, -2147483648):
                        self.lib.test_initialize(count*self.size+extra)
                        self.events.clear()
                        for index, which in enumerate(pattern):
                            self.lib.test_set_name(index, which)
                        context = C.c_int32(initial)
                        before, other = bytes(self.records), bytes(self.other)
                        match = next((i for i in range(count) if pattern[i] == 2), None)
                        end = match+1 if match is not None else count
                        self.assertEqual(self.function(C.byref(context), -2147483648, 0x12345678), -77)
                        expected_events = [('compare', (b'other' if pattern[i] == 1 else b'tty' if pattern[i] == 2 else b'last'),
                                            b'tty', C.addressof(self.page)+0x43C0)
                                           for i in range(end) if pattern[i]]
                        expected_events.append(('A', C.addressof(context), initial or 1, -2147483648, 0x12345678))
                        self.assertEqual(self.events, expected_events)
                        self.assertEqual(context.value, initial or 1)
                        self.assertEqual(bytes(self.records), self.expected(before, match, self.original))
                        self.assertEqual(bytes(self.other), other)

    def test_compare_replaces_global_handler_and_kernel_table_but_keeps_captured_search(self):
        self.lib.test_initialize(5*self.size)
        for index, which in enumerate((1, 0, 2, 2, 1)):
            self.lib.test_set_name(index, which)
        before, other = bytes(self.records), bytes(self.other)
        context = C.c_int32(0)
        observed_context = []
        def compare(first, second):
            observed_context.append(context.value)
            result = self.compare_names(first, second)
            self.handler_global.value = self.replacement
            self.lib.test_replace_kernel_table()
            return result
        self.compare_hook = Compare(compare)
        self.compare_global.value = C.cast(self.compare_hook, C.c_void_p).value
        self.function(C.byref(context), -7, 99)
        self.assertEqual(observed_context, [1, 1])
        self.assertEqual([event[0] for event in self.events], ['compare', 'compare', 'B'])
        self.assertEqual(self.events[-1], ('B', C.addressof(context), 1, -7, 99))
        self.assertEqual(bytes(self.records), self.expected(before, 2, self.original))
        self.assertEqual(bytes(self.other), other)

    def test_name_changes_during_compare_affect_later_entries(self):
        self.lib.test_initialize(4*self.size)
        for i in range(4):
            self.lib.test_set_name(i, 1)
        context = C.c_int32(77)
        def compare(first, second):
            result = self.compare_names(first, second)
            self.lib.test_set_name(2, 2)
            return result
        self.compare_hook = Compare(compare)
        self.compare_global.value = C.cast(self.compare_hook, C.c_void_p).value
        self.function(C.byref(context), 0, -1)
        self.assertEqual([event[1] for event in self.events[:-1]], [b'other', b'other', b'tty'])
        selected = C.c_void_p.from_buffer(self.records, 2*self.size+self.handler_offset).value
        self.assertEqual(selected, self.original)
        self.assertEqual(self.events[-1], ('A', C.addressof(context), 77, 0, -1))

    def test_first_file_installs_once_and_forwards_original_arguments(self):
        self.lib.test_initialize(4*self.size)
        for index, which in enumerate((0, 1, 2, 2)):
            self.lib.test_set_name(index, which)
        before = bytes(self.records)
        prior = C.c_void_p.from_buffer(self.records, 2*self.size+self.handler_offset).value
        path = C.create_string_buffer(b'tty:entry')
        directory = C.create_string_buffer(40)
        self.assertEqual(self.first(path, C.addressof(directory)), C.addressof(directory))
        patch = C.cast(self.function, C.c_void_p).value
        self.assertEqual(bytes(self.records), self.expected(before, 2, patch))
        self.assertEqual(self.handler_global.value, prior)
        self.assertEqual(C.string_at(C.addressof(self.page)+0x43C0), b'tty')
        self.assertEqual([event[1] for event in self.events], [b'other', b'tty']*2)
        self.assertEqual(self.first_calls.value, 1)
        self.assertEqual(self.requested_path.value, C.addressof(path))
        self.assertEqual(self.directory.value, C.addressof(directory))
        self.assertEqual(path.value, b'tty:entry')

    def test_first_file_missing_device_skips_bios_and_keeps_handler(self):
        for path in (b'tty:entry', b'tty0:entry', b'\xff:entry', b':entry'):
            with self.subTest(path=path):
                self.events.clear()
                self.lib.test_initialize(self.size-1)
                before = bytes(self.records)
                self.assertIsNone(self.first(path, 123))
                self.assertEqual(bytes(self.records), before)
                self.assertEqual(self.handler_global.value, self.original)
                self.assertEqual(self.first_calls.value, 0)
                self.assertEqual(self.events, [])
                prefix = b'tty' if path.startswith(b'tty') else b''
                self.assertEqual(C.string_at(C.addressof(self.page)+0x43C0), prefix)

    def test_first_file_refreshes_kernel_descriptor_between_searches(self):
        self.lib.test_initialize(2*self.size)
        self.lib.test_set_name(0, 2)
        self.lib.test_set_other_name(1, 2)
        before, other = bytes(self.records), bytes(self.other)
        def compare(first, second):
            result = self.compare_names(first, second)
            self.lib.test_select_other_table(2)
            return result
        # The other table's first entry must have a valid empty name.
        self.lib.test_set_other_name(0, 0)
        other = bytes(self.other)
        self.compare_hook = Compare(compare)
        self.compare_global.value = C.cast(self.compare_hook, C.c_void_p).value
        self.assertEqual(self.first(b'tty:entry', 123), 123)
        self.assertEqual(bytes(self.records), before)
        patch = C.cast(self.function, C.c_void_p).value
        self.assertEqual(bytes(self.other), self.expected(other, 1, patch))
        self.assertEqual(self.first_calls.value, 1)


if __name__ == '__main__':
    unittest.main()
