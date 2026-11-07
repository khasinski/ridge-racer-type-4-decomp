"""Actual queue completion body: packed copy, publication and callback ordering."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

Completion = C.CFUNCTYPE(None)


class Record(C.Structure):
    _fields_ = [('state', C.c_uint16), ('pad02', C.c_uint8 * 6),
                ('value08', C.c_int32), ('pad0C', C.c_uint8 * 16), ('word1C', C.c_uint32)]


class Page(C.Structure):
    _fields_ = [('prefix', C.c_uint8 * 0x2EC), ('busy', C.c_int32),
                ('pad2F0', C.c_uint8 * 4), ('next', C.c_int32), ('current', C.c_int32),
                ('pad2FC', C.c_uint8 * 0x14), ('records', C.c_void_p),
                ('pad314', C.c_uint8 * (0x2750 - 0x310 - C.sizeof(C.c_void_p))),
                ('word', C.c_uint32), ('value', C.c_int32), ('pad2758', C.c_uint8 * 8),
                ('callback', C.c_void_p),
                ('suffix', C.c_uint8 * (0x10000 - 0x2760 - C.sizeof(C.c_void_p)))]


class LibraryQueueCompletionTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        directory = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        header = (root / 'include/rr/library.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
#include <stddef.h>
typedef uint8_t u8; typedef int8_t s8;
typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
''' + header + '''
RrLibQueueRecord32 test_records[16];
struct {
    u8 prefix[0x2EC]; s32 busy; u8 pad2F0[4]; s32 next, current;
    u8 pad2FC[0x14]; RrLibQueueRecord32 *records;
    u8 pad314[0x2750 - 0x310 - sizeof(RrLibQueueRecord32 *)];
    RrLibPackedWord word; s32 value; u8 pad2758[8];
    void (*callback)(void); u8 suffix[0x10000 - 0x2760 - sizeof(void (*)(void))];
} test_page;
_Static_assert(sizeof(RrLibQueueRecord32) == 0x20, "record stride");
_Static_assert(offsetof(__typeof__(test_page), current) == 0x2F8, "index offset");
_Static_assert(offsetof(__typeof__(test_page), word) == 0x2750, "packed word offset");
_Static_assert(offsetof(__typeof__(test_page), callback) == 0x2760, "callback offset");
#define StDataReadyPending test_page.busy
#define StRingWriteIndex test_page.next
#define StRingStartIndex test_page.current
#define StRingHeaders test_page.records
#define StLastReadyLoc test_page.word
#define StLastReadyFrame test_page.value
#define StDataReadyFunc test_page.callback
'''
        body = (root / 'tools/scripts/fixtures/legacy/func_8008F5E4.c').read_text()
        body = body.replace('#include "rr/library.h"', '')
        body = re.sub(r'^extern[^\n]*\n', '', body, flags=re.M)
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r'"\$[0-9]+", ', '', body)
        body = re.sub(r' : "\$[0-9]+"', '', body)
        body = body.replace('(u8 *)0x80110000U', '((u8 *)&test_page)')
        source += body
        (directory / 'function.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'function.c'),
                        '-o', str(directory / 'function.so')], check=True, capture_output=True)
        cls.library = C.CDLL(str(directory / 'function.so'))
        cls.records = (Record * 16).in_dll(cls.library, 'test_records')
        cls.page = Page.in_dll(cls.library, 'test_page')
        cls.function = cls.library.data_ready_callback
        cls.function.argtypes = []
        cls.function.restype = None

    def setUp(self):
        C.memset(C.addressof(self.page), 0xA5, C.sizeof(self.page))
        C.memset(C.addressof(self.records), 0x5A, C.sizeof(self.records))
        self.page.records = C.addressof(self.records)
        self.events = []
        self.callback = Completion(lambda: self.events.append((bytes(self.page), bytes(self.records))))
        self.page.callback = C.cast(self.callback, C.c_void_p).value

    def expected_before_callback(self, selected):
        page = Page.from_buffer_copy(self.page)
        records = (Record * 16).from_buffer_copy(self.records)
        records[selected].state = 2
        page.word = records[selected].word1C
        page.value = records[selected].value08
        page.current = page.next
        return page, records

    def test_all_record_positions_and_values_with_optional_callback(self):
        for selected in range(16):
            for word in (0, 0x12345678, 0x80000000, 0xFFFFFFFF):
                for value in (-2147483648, -1, 0, 2147483647):
                    for enabled in (False, True):
                        self.page.busy = -123
                        self.page.current = selected
                        self.page.next = -456
                        self.records[selected].state = 0xFFFF
                        self.records[selected].word1C = word
                        self.records[selected].value08 = value
                        self.page.callback = C.cast(self.callback, C.c_void_p).value if enabled else None
                        self.events.clear()
                        expected_page, expected_records = self.expected_before_callback(selected)
                        callback_snapshot = bytes(expected_page)
                        self.function()
                        self.assertEqual(self.events, [(callback_snapshot, bytes(expected_records))] if enabled else [])
                        expected_page.busy = 0
                        self.assertEqual(bytes(self.page), bytes(expected_page))
                        self.assertEqual(bytes(self.records), bytes(expected_records))

    def test_callback_mutations_survive_except_final_busy_clear(self):
        self.page.busy = 77
        self.page.current = 3
        self.page.next = 8
        self.records[3].word1C = 0xFEDCBA98
        self.records[3].value08 = -12345
        def callback():
            self.events.append((bytes(self.page), bytes(self.records)))
            self.page.busy = 999
            self.page.current = 12
            self.page.next = 13
            self.page.word = 0x10203040
            self.page.value = 5678
            self.records[3].state = 6
        self.callback = Completion(callback)
        self.page.callback = C.cast(self.callback, C.c_void_p).value
        expected_page, expected_records = self.expected_before_callback(3)
        callback_snapshot = bytes(expected_page), bytes(expected_records)
        self.function()
        self.assertEqual(self.events, [callback_snapshot])
        expected_page.busy = 0
        expected_page.current = 12
        expected_page.next = 13
        expected_page.word = 0x10203040
        expected_page.value = 5678
        expected_records[3].state = 6
        self.assertEqual(bytes(self.page), bytes(expected_page))
        self.assertEqual(bytes(self.records), bytes(expected_records))


if __name__ == '__main__':
    unittest.main()
