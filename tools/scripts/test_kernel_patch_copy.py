"""Actual fixed code-span copier with native buffers replacing only PAL addresses."""
import ctypes as C
from pathlib import Path
import random
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class KernelPatchCopyTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        directory = Path(cls.temp.name)
        header = (root / 'include/rr/memory.h').read_text().replace('#include "types.h"', '')
        source = '''#include <stdint.h>
typedef uint8_t u8; typedef uint32_t u32; typedef int32_t s32;
''' + header + '''
u32 test_source[34];
u32 test_destination[34];
'''
        body = (root / 'tools/scripts/fixtures/views/func_80095FA0.c').read_text()
        body = body.replace('#include "rr/memory.h"', '')
        replacements = {
            '0xDF80U': '(unsigned long)(test_destination+2)',
            '(unsigned long)StopCARD2+0x10': '(unsigned long)(test_source+4)',
            '(unsigned long)_copy_memcard_patch_end': '(unsigned long)(test_source+32)',
        }
        for old, new in replacements.items():
            if body.count(old) != 1:
                raise AssertionError('Kernel address adapter no longer matches exactly one site: ' + old)
            body = body.replace(old, new)
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        source += body
        (directory / 'copy.c').write_text(source)
        result = subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'copy.c'),
                                 '-o', str(directory / 'copy.so')], capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory / 'copy.so'))
        cls.source = (C.c_uint32 * 34).in_dll(cls.lib, 'test_source')
        cls.destination = (C.c_uint32 * 34).in_dll(cls.lib, 'test_destination')
        cls.function = cls.lib._copy_memcard_patch
        cls.function.argtypes = []
        cls.function.restype = None

    def check_copy(self, source, destination):
        self.source[:] = source
        self.destination[:] = destination
        before_source = bytes(self.source)
        expected = bytearray(self.destination)
        expected[8:120] = before_source[16:128]
        self.function()
        self.assertEqual(bytes(self.destination), bytes(expected))
        self.assertEqual(bytes(self.source), before_source)

    def test_fixed_span_copies_each_word_and_preserves_both_guards(self):
        cases = [[0]*34, [0xFFFFFFFF]*34,
                 [(0x1234567*i) & 0xFFFFFFFF for i in range(34)],
                 [1 << (i % 32) for i in range(34)]]
        for source in cases:
            self.check_copy(source, [0xA5A5A5A5]*34)

    def test_random_code_words_and_destination_canaries(self):
        rng = random.Random(0x95FA0)
        for _ in range(500):
            self.check_copy([rng.getrandbits(32) for _ in range(34)],
                            [rng.getrandbits(32) for _ in range(34)])

    def test_repeated_calls_observe_new_payload_but_ignore_header_and_endpoint(self):
        self.source[:] = list(range(34))
        self.destination[:] = [0xA5A5A5A5]*34
        self.function()
        before = bytes(self.destination)
        for index in (0, 1, 2, 3, 32, 33):
            self.source[index] ^= 0xFFFFFFFF
        self.function()
        self.assertEqual(bytes(self.destination), before)
        self.source[4], self.source[31] = 0xFFFFFFFF, 0x80000000
        expected = list(self.destination)
        expected[2], expected[29] = self.source[4], self.source[31]
        self.function()
        self.assertEqual(list(self.destination), expected)


if __name__ == '__main__':
    unittest.main()
