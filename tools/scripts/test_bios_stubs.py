"""Named BIOS dispatch macros preserve the original ABI and instruction sequence."""
from pathlib import Path
import json
import re
import struct
import unittest

from elftools.elf.elffile import ELFFile
from levenshtein import levenshtein
from normalize_bios_stubs import normalize, render_macros, services

ROOT = Path(__file__).resolve().parents[2]


class BiosStubsTest(unittest.TestCase):
    def test_normalize_preserves_padding_and_uses_service_name(self):
        source = '''glabel FlushCache
    /* file VA word */ addiu $t2, $zero, 0xA0
    /* file VA word */ jr $t2
    /* file VA word */ addiu $t1, $zero, 0x44
endlabel FlushCache
    nop
'''
        updated, count = normalize(source)
        self.assertEqual(count, 1)
        self.assertEqual(updated, 'BIOS_FLUSH_CACHE FlushCache\n    nop\n')
        self.assertEqual(normalize(updated), (updated, 0))

    def test_named_macros_and_complete_exe(self):
        self.assertEqual((ROOT/'include/bios_calls.inc').read_text(), render_macros())
        names = set(services().values())
        original = (ROOT/'assets/PAL/main.exe').read_bytes()
        built = (ROOT/'build/PAL/main.exe').read_bytes()
        entries = []
        with (ROOT/'build/PAL/main.elf').open('rb') as stream:
            symbols = ELFFile(stream).get_section_by_name('.symtab')
            for source in (ROOT/'asm/PAL/main').glob('*.s'):
                for macro, name in re.findall(r'^(BIOS_\w+) ([A-Za-z_]\w*)$', source.read_text(), re.M):
                    self.assertIn(macro, names)
                    address = __import__('c_functions').address_of(name)
                    symbol = symbols.get_symbol_by_name(name)[0]
                    offset = address - 0x80010000 + 0x800
                    left = struct.unpack('<3I', original[offset:offset+12])
                    right = struct.unpack('<3I', built[offset:offset+12])
                    self.assertEqual(levenshtein(left, right), 0, name)
                    self.assertEqual(symbol['st_value'], address)
                    self.assertEqual(symbol['st_size'], 12)
                    entries.append(dict(symbol=name, service=macro, levenshtein=0))
        self.assertEqual(len(entries), 52)
        self.assertEqual(built, original)
        output = ROOT/'build/smallest10/bios_verified.json'
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps(dict(functions=entries, full_exe_identical=True), indent=2)+'\n')


if __name__ == '__main__':
    unittest.main()
