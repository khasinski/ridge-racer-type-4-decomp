"""Actual named-table scan with a native BIOS strcmp stand-in and PAL page view."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class Entry(C.Structure):
    _fields_ = [('key', C.c_int32), ('value', C.c_int32), ('name', C.c_char * 0x24)]


class Page(C.Structure):
    _fields_ = [('prefix', C.c_uint8 * 0x954), ('entries', Entry * 128),
                ('suffix', C.c_uint8 * (0x10000 - 0x954 - 128 * 0x2C))]


class NamedLibraryLookupTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        directory = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        header = (root / 'include/rr/library.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
#include <string.h>
typedef uint8_t u8; typedef int8_t s8;
typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
''' + header + '''
struct { u8 prefix[0x954]; RrLibNamedEntry entries[128];
         u8 suffix[0x10000 - 0x954 - 128 * 0x2C]; } test_page;
#define CdPathTblName test_page.entries[0].name
s32 test_comparisons, test_mutate_next;
const char *test_last_name;
s32 PsyqStrcmp(const char *name, const char *entry_name) {
    test_comparisons++;
    test_last_name = entry_name;
    if (test_mutate_next) test_page.entries[1].key = 0;
    return strcmp(name, entry_name);
}
'''
        body = (root / 'tools/scripts/fixtures/views/func_8008E440.c').read_text()
        body = body.replace('#include "rr/library.h"', '')
        body = body.replace('extern char CdPathTblName[];', '')
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r'"\$[0-9]+", ', '', body)
        body = re.sub(r' : "\$[0-9]+"', '', body)
        body = body.replace('(u8 *)0x80110000U', '((u8 *)&test_page)')
        source += body
        (directory / 'function.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'function.c'),
                        '-o', str(directory / 'function.so')], check=True, capture_output=True)
        cls.library = C.CDLL(str(directory / 'function.so'))
        cls.page = Page.in_dll(cls.library, 'test_page')
        cls.comparisons = C.c_int32.in_dll(cls.library, 'test_comparisons')
        cls.mutate = C.c_int32.in_dll(cls.library, 'test_mutate_next')
        cls.last_name = C.c_void_p.in_dll(cls.library, 'test_last_name')
        cls.function = cls.library.DS_searchdir
        cls.function.argtypes = [C.c_int32, C.c_char_p]
        cls.function.restype = C.c_int32

    def setUp(self):
        C.memset(C.addressof(self.page), 0xA5, C.sizeof(self.page))
        C.memset(C.addressof(self.page.entries), 0, C.sizeof(self.page.entries))
        self.comparisons.value = self.mutate.value = 0

    def fill(self):
        for index, entry in enumerate(self.page.entries):
            entry.key = index + 1
            entry.value = -index
            entry.name = ('name-%d' % index).encode()

    def test_zero_key_terminates_without_dereferencing_query_name(self):
        before = bytes(self.page)
        for key in (0, 1, -1, -(1 << 31), (1 << 31) - 1):
            self.assertEqual(self.function(key, None), -1)
            self.assertEqual(self.comparisons.value, 0)
            self.assertEqual(bytes(self.page), before)

    def test_every_index_including_last_returns_one_based_position(self):
        self.fill()
        before = bytes(self.page)
        for index in range(128):
            self.comparisons.value = 0
            self.assertEqual(self.function(index + 1, ('name-%d' % index).encode()), index + 1)
            self.assertEqual(self.comparisons.value, 1)
            self.assertEqual(self.last_name.value, C.addressof(self.page.entries) + index * 0x2C + 8)
            self.assertEqual(bytes(self.page), before)

    def test_duplicates_use_first_matching_name_and_skip_other_keys(self):
        self.fill()
        for index in (2, 5, 8):
            self.page.entries[index].key = -123
            self.page.entries[index].name = b'match' if index >= 5 else b'wrong'
        before = bytes(self.page)
        self.assertEqual(self.function(-123, b'match'), 6)
        self.assertEqual(self.comparisons.value, 2)
        self.assertEqual(bytes(self.page), before)

    def test_missing_entry_and_name_respect_limit_and_early_terminator(self):
        self.fill()
        for entry in self.page.entries:
            entry.key = 7
        before = bytes(self.page)
        self.assertEqual(self.function(7, b'absent'), -1)
        self.assertEqual(self.comparisons.value, 128)
        self.assertEqual(bytes(self.page), before)
        self.comparisons.value = 0
        self.page.entries[3].key = 0
        self.page.entries[4].name = b'absent'
        before = bytes(self.page)
        self.assertEqual(self.function(7, b'absent'), -1)
        self.assertEqual(self.comparisons.value, 3)
        self.assertEqual(bytes(self.page), before)

    def test_next_key_is_read_after_comparison_side_effect(self):
        self.fill()
        self.page.entries[0].key = self.page.entries[1].key = 7
        self.page.entries[1].name = b'match'
        self.mutate.value = 1
        before = Page.from_buffer_copy(self.page)
        before.entries[1].key = 0
        self.assertEqual(self.function(7, b'match'), -1)
        self.assertEqual(self.comparisons.value, 1)
        self.assertEqual(bytes(self.page), bytes(before))


if __name__ == '__main__':
    unittest.main()
