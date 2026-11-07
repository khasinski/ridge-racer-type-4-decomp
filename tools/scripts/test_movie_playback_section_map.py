"""Check STR text/data separation against the original disc payload."""
from pathlib import Path
import struct
import unittest

from analyze_overlays import BASE, SPANS, padding_spans


class StrSectionMapTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        root = Path(__file__).resolve().parents[2]
        image = (root / 'disc/PAL/R4.BIN').read_bytes()
        cls.image = image
        cls.padding = padding_spans(image).get(668, [])
        sectors = struct.unpack_from('<951I', image, 4)
        cls.module = image[sectors[668] * 2048:sectors[669] * 2048]

    def word(self, address):
        return struct.unpack_from('<I', self.module, address - BASE)[0]

    def test_decoder_table_addressing(self):
        # a2/a3 are table bases, indexed by 13/9 bits at strides 8/4.
        expected = {
            0x80116960: 0x3C068011, 0x80116964: 0x24C67690,
            0x80116968: 0x3C078012, 0x8011696C: 0x24E77690,
            0x80116B3C: 0x000244C2, 0x80116B40: 0x000840C0,
            0x80116B44: 0x01064020, 0x80116B48: 0x8D090000,
            0x80116B7C: 0x000245C2, 0x80116B80: 0x00084080,
            0x80116B84: 0x01074020, 0x80116B88: 0x8D090000,
        }
        for address, instruction in expected.items():
            self.assertEqual(self.word(address), instruction, hex(address))
        self.assertEqual(0x80117690 + (1 << 13) * 8, 0x80127690)
        self.assertEqual(0x80127690 + (1 << 9) * 4, 0x80127E90)
        for base, count, stride, maximum in ((0x80117690, 8192, 8, 13),
                                             (0x80127690, 512, 4, 9)):
            lengths = [self.word(base + i * stride) & 255 for i in range(count)]
            self.assertEqual(min(lengths), 0)
            self.assertEqual(max(lengths), maximum)
            self.assertTrue(all(n <= maximum for n in lengths))

    def test_last_return_and_false_table_returns(self):
        self.assertEqual(BASE + SPANS[668][1], 0x80116CA8)
        self.assertEqual(self.word(0x80116CA0), 0x03E00008)
        self.assertEqual(self.word(0x80116CA4), 0x20020001)
        # These are coefficients/lengths within the second VLC table.
        for address in (0x80127818, 0x8012781C):
            self.assertTrue(0x80127690 <= address < 0x80127E90)
            self.assertEqual(self.word(address), 0x03E00008)

    def test_direct_control_flow_stays_out_of_data(self):
        start, end = SPANS[668]
        for offset in range(start, end, 4):
            address = BASE + offset
            word = self.word(address)
            opcode = word >> 26
            if opcode in (2, 3):
                target = ((address + 4) & 0xF0000000) | ((word & 0x3FFFFFF) << 2)
            elif opcode in (1, 4, 5, 6, 7):
                immediate = word & 0xFFFF
                if immediate & 0x8000:
                    immediate -= 0x10000
                target = address + 4 + immediate * 4
            else:
                continue
            if BASE <= target < BASE + len(self.module):
                self.assertTrue(BASE + start <= target < BASE + end,
                                (hex(address), hex(target)))
                self.assertFalse(any(a <= target < b for a,b in self.padding),
                                 (hex(address), hex(target)))

    def test_padding_is_between_functions(self):
        from verify_overlay_samples import SAMPLES, EXTRA_FUNCTIONS
        self.assertEqual(self.padding, [(0x80116920, 0x80116928)])
        for index, address, size in SAMPLES + EXTRA_FUNCTIONS:
            if index == 668:
                self.assertTrue(all(address + size <= a or b <= address
                                    for a,b in self.padding))

    def test_nonzero_padding_is_rejected(self):
        image = bytearray(self.image)
        sectors = struct.unpack_from('<951I', image, 4)
        offset = sectors[668] * 2048 + self.padding[0][0] - BASE
        struct.pack_into('<I', image, offset, 0x03E00008)
        with self.assertRaises(AssertionError):
            padding_spans(image)


if __name__ == '__main__':
    unittest.main()
