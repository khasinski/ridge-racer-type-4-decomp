"""Ownership and complete-image regressions for the cleaned overlay modules."""
from pathlib import Path
from overlay_names import name as OVN
import re
import unittest

from build_overlay import build
from overlay_units import function_extent, function_owner, manifest, read_c_source

ROOT = Path(__file__).resolve().parents[2]


class OverlayUnitsTest(unittest.TestCase):
    def test_complete_c_images(self):
        for index in (659, 663, 666):
            with self.subTest(overlay=index):
                report = build(index)
                self.assertTrue(all(report['whole_overlay_identical'].values()))
                self.assertTrue(report['full_exe_identical'])

    def test_unique_owners_and_clean_c(self):
        for index in (659, 663, 666):
            config = manifest(index)
            sources = {ROOT / row['source'] for row in config['units']}
            self.assertEqual(len(config['functions']), len({r['symbol'] for r in config['functions']}))
            for row in config['functions']:
                source, symbol = function_owner(index, row['address'])
                self.assertIn(source, sources)
                text = source.read_text()
                start, end = function_extent(text, symbol)
                fixture = ROOT / f"src/overlays/PAL/{OVN(index)}/func_{row['address']:08X}.c"
                self.assertEqual(read_c_source(fixture), text[start:end] + '\n')
            for source in sources:
                text = source.read_text()
                self.assertIsNone(re.search(r'\b(?:register|volatile|__asm__|asm)\b|GCC_VERSION|CC1_FLAGS', text))
                self.assertIsNone(re.search(r'\b(?:temp|var|block)_\w+|\(unsigned long\)', text))

    def test_definition_boundaries_ignore_literals_and_comments(self):
        source = 'void first(void) { puts("}"); /* { */ }\nvoid second(void) {}\n'
        start, end = function_extent(source, 'first')
        self.assertEqual(source[start:end], 'void first(void) { puts("}"); /* { */ }')


if __name__ == '__main__':
    unittest.main()
