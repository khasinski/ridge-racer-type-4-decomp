"""Actual control-row reset and mask propagation for valid channels 0..31."""
import ctypes as C
from pathlib import Path
import random
import re
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest


class ChannelControlResetTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        directory = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        header = (root / 'include/rr/channels.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
typedef uint8_t u8; typedef int8_t s8; typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
''' + header + '''
RrChannelControlRow36 test_rows[32];
u16 test_masks[0x10010 / 2];
u16 SvmVoiceIndex;
#define _svm_voice test_rows
#define SvmVoicePitch ((u8 *)&test_rows + 4)
#define SvmVoiceKeyStatus ((u8 *)&test_rows + 0x1D)
#define _svm_okon1 test_masks[0xF8F8 / 2]
#define _svm_okon2 test_masks[0xF8FA / 2]
#define _svm_okof1 test_masks[0x10000 / 2]
#define _svm_okof2 test_masks[0x10002 / 2]
'''
        body = (root / 'tools/scripts/fixtures/views/func_80088DA4.c').read_text()
        body = body.replace('#include "rr/channels.h"', '')
        body = re.sub(r'^extern[^\n]*\n', '', body, flags=re.M)
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r'"\$[0-9]+", ', '', body)
        body = re.sub(r' : "\$[0-9]+"', '', body)
        body = body.replace('(u16 *)0x80110000U', '(test_masks + 0x10000 / 2)')
        source += body
        (directory / 'function.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'function.c'),
                        '-o', str(directory / 'function.so')], check=True, capture_output=True)
        cls.library = C.CDLL(str(directory / 'function.so'))
        cls.rows = (C.c_uint8 * (32 * 0x36)).in_dll(cls.library, 'test_rows')
        cls.masks = (C.c_uint16 * (0x10010 // 2)).in_dll(cls.library, 'test_masks')
        cls.selected = C.c_uint16.in_dll(cls.library, 'SvmVoiceIndex')
        cls.function = cls.library._SsVmKeyOffNow
        cls.function.argtypes = [C.c_int32]
        cls.function.restype = None

    def check(self, selected, low, high, pending_low, pending_high):
        C.memset(C.addressof(self.rows), 0xA5, C.sizeof(self.rows))
        C.memset(C.addressof(self.masks), 0x5A, C.sizeof(self.masks))
        self.selected.value = selected
        for offset, value in ((0x10000, low), (0x10002, high),
                              (0xF8F8, pending_low), (0xF8FA, pending_high)):
            self.masks[offset // 2] = value
        rows_before = bytearray(self.rows)
        masks_before = bytearray(self.masks)
        rows_before[selected * 0x36:selected * 0x36 + 2] = b'\0\0'
        rows_before[selected * 0x36 + 4:selected * 0x36 + 6] = b'\0\0'
        rows_before[selected * 0x36 + 0x1D] = 0
        low |= (1 << selected) if selected < 16 else 0
        high |= (1 << (selected - 16)) if selected >= 16 else 0
        for offset, value in ((0x10000, low), (0x10002, high),
                              (0xF8F8, pending_low & ~low), (0xF8FA, pending_high & ~high)):
            masks_before[offset:offset + 2] = struct.pack('<H', value)
        self.function(0)
        self.assertEqual(bytes(self.rows), bytes(rows_before))
        self.assertEqual(bytes(self.masks), bytes(masks_before))
        self.assertEqual(self.selected.value, selected)

    def test_every_channel_with_boundary_and_preexisting_masks(self):
        for selected in range(32):
            for low, high, pending_low, pending_high in (
                (0, 0, 0xFFFF, 0xFFFF), (0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF),
                (0xAAAA, 0x5555, 0xFFFF, 0xFFFF), (0xFFFF, 0xFFFF, 0, 0),
                (0, 0, 0, 0), (1, 0x8000, 0xAAAA, 0x5555)):
                self.check(selected, low, high, pending_low, pending_high)

    def test_random_masks_preserve_other_rows_and_memory(self):
        rng = random.Random(0x88DA4)
        for _ in range(1000):
            self.check(rng.randrange(32), *(rng.getrandbits(16) for _ in range(4)))


if __name__ == '__main__':
    unittest.main()
