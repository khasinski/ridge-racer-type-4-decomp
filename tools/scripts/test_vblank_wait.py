"""Real bounded wait body, BIOS call order, and a separately advancing clock adapter."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class VBlankWaitTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        directory = Path(cls.temp.name)
        header = (root / 'include/rr/timing.h').read_text().replace('#include "types.h"', '')
        source = '''#include <stdint.h>
#include <stdarg.h>
#include <string.h>
typedef uint32_t u32; typedef int32_t s32;
''' + header + '''
volatile u32 Vcount;
char test_message_page[65536];
u32 test_calls, test_clock_reads, test_advance_at, test_initial_clock, test_final_clock;
uintptr_t test_events[3][3];
s32 PsyqPuts(const char *message, ...) {
 u32 i=test_calls++;if(i<3) {test_events[i][0]=1;test_events[i][1]=(uintptr_t)message;}
 return -37;
}
s32 ChangeClearPAD(s32 value) {
 u32 i=test_calls++;if(i<3) {test_events[i][0]=2;test_events[i][1]=(u32)value;}
 return 99;
}
s32 ChangeClearRCnt(s32 first,s32 second) {
 u32 i=test_calls++;if(i<3) {test_events[i][0]=3;test_events[i][1]=(u32)first;test_events[i][2]=(u32)second;}
 return -1;
}
u32 test_read_clock(void) {
 return test_clock_reads++ >= test_advance_at ? test_final_clock : test_initial_clock;
}
void test_reset(void) {test_calls=test_clock_reads=0;memset(test_events,0,sizeof(test_events));}
'''
        body = (root / 'tools/scripts/fixtures/views/func_8008B2CC.c').read_text()
        body = body.replace('#include "rr/timing.h"', '')
        if body.count('(char *)0x80010000U') != 1:
            raise AssertionError('Expected exactly one fixed message page')
        body = body.replace('(char *)0x80010000U', 'test_message_page')
        source += body
        if body.count('(s32)Vcount') != 2:
            raise AssertionError('Clock adapter must cover both actual volatile read sites')
        adapted = body.replace('v_wait', 'test_advancing_wait')
        adapted = adapted.replace('(s32)Vcount', '(s32)test_read_clock()')
        source += adapted
        (directory / 'wait.c').write_text(source)
        result = subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'wait.c'),
                                 '-o', str(directory / 'wait.so')], capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory / 'wait.so'))
        cls.clock = C.c_uint32.in_dll(cls.lib, 'Vcount')
        cls.calls = C.c_uint32.in_dll(cls.lib, 'test_calls')
        cls.reads = C.c_uint32.in_dll(cls.lib, 'test_clock_reads')
        cls.advance = C.c_uint32.in_dll(cls.lib, 'test_advance_at')
        cls.initial = C.c_uint32.in_dll(cls.lib, 'test_initial_clock')
        cls.final = C.c_uint32.in_dll(cls.lib, 'test_final_clock')
        cls.events = (C.c_size_t * 9).in_dll(cls.lib, 'test_events')
        cls.message = (C.c_char * 65536).in_dll(cls.lib, 'test_message_page')
        cls.wait = cls.lib.v_wait
        cls.advancing_wait = cls.lib.test_advancing_wait
        for function in (cls.wait, cls.advancing_wait):
            function.argtypes = [C.c_int32, C.c_int32]
            function.restype = None

    def assert_timeout(self):
        self.assertEqual(self.calls.value, 3)
        self.assertEqual(list(self.events), [1, C.addressof(self.message)+0x17F8, 0,
                                             2, 0, 0, 3, 3, 0])

    def test_already_reached_uses_signed_clock_comparison_and_no_bios_calls(self):
        for clock, target in ((0, 0), (1, 0), (0xFFFFFFFF, -1),
                              (0x80000000, -2147483648), (0x7FFFFFFF, -1)):
            for budget in (-2147483648, -1, 0, 1, 2147483647):
                self.lib.test_reset()
                self.clock.value = clock
                self.wait(target, budget)
                self.assertEqual(self.calls.value, 0)
                self.assertEqual(self.clock.value, clock)

    def test_actual_timeout_path_wraps_budget_and_preserves_clock(self):
        for budget in (-2147483648, 0, 1, 2, 0x20000, 0x20001, 0x40002):
            self.lib.test_reset()
            self.clock.value = 0xFFFFFFFF
            self.wait(0, budget)
            self.assert_timeout()
            self.assertEqual(self.clock.value, 0xFFFFFFFF)

    def test_advancing_clock_exits_before_timeout_for_large_and_negative_budgets(self):
        for budget in (-1, 1, 2, 32767, 2147483647):
            for advance_at in (0, 1, 2, 7, 100):
                self.lib.test_reset()
                self.initial.value = 4
                self.final.value = 5
                self.advance.value = advance_at
                self.advancing_wait(5, budget)
                self.assertEqual(self.reads.value, advance_at+1)
                self.assertEqual(self.calls.value, 0)

    def test_budget_expiry_precedes_next_clock_read(self):
        for budget in (0, 1, 2, 0x20000, 0x20001):
            initial_count = (budget << 15) & 0xFFFFFFFF
            self.lib.test_reset()
            self.initial.value = 0
            self.final.value = 1
            self.advance.value = initial_count+1
            self.advancing_wait(1, budget)
            self.assert_timeout()
            self.assertEqual(self.reads.value, initial_count+1)
            # Advancing one read sooner reaches the target after the last
            # non-expiring decrement (except a zero budget, which expires first).
            if initial_count:
                self.lib.test_reset()
                self.advance.value = initial_count
                self.advancing_wait(1, budget)
                self.assertEqual(self.calls.value, 0)
                self.assertEqual(self.reads.value, initial_count+1)


if __name__ == '__main__':
    unittest.main()
