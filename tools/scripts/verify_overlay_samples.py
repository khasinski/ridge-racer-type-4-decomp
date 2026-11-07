#!/usr/bin/env python3
"""Compile, relocate and verify the implemented PAL overlay functions.

Linking resolves every relocation before instruction Levenshtein comparison.
Overlay reconstructions replace only their declared spans in a disc-image copy;
whole-image identity also checks the four duplicate overlay sites.
"""
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

from elftools.elf.elffile import ELFFile
from analyze_overlays import padding_spans
from levenshtein import levenshtein
from overlay_units import function_owner, link_unit

ROOT = Path(__file__).resolve().parents[2]
SAMPLES = [(659, 0x801148CC, 60), (660, 0x80114910, 16),
           (661, 0x80117340, 16), (662, 0x8011E7C0, 28),
           (663, 0x80115BA0, 148), (665, 0x80115C8C, 112),
           (666, 0x801166D0, 96), (667, 0x80115048, 16),
           (668, 0x8011688C, 24)]
# Additional completed owners, including the second function of race_results.
# Duplicate disc sites share one source owner and count once.
EXTRA_FUNCTIONS = [
    (667, 0x801161A8, 1904),
    (662, 0x801154C8, 2556),
    (661, 0x80114B00, 0xA98),
    (661, 0x80117430, 0x848),
    (660, 0x8011718C, 0x694),
    (668, 0x80115F80, 0x318),
    (667, 0x80116918, 0x29C),
    (665, 0x80115820, 0x46C),
    (667, 0x80117FA8, 0x68C),
    (660, 0x80114D38, 0x7A0),
    (665, 0x801151A4, 0x5F4),
    (666, 0x80114CCC, 0x47C),
    (659, 0x801159E8, 0x2FC),
    (663, 0x80114B98, 0x490),
    (662, 0x8011E070, 0x2FC),
    (662, 0x80119E00, 0x614),
    (662, 0x80118A30, 0x43C),
    (662, 0x80121074, 0x4F0),
    (662, 0x80120B68, 0x50C),
    (662, 0x8011E824, 0x4FC),
    (662, 0x80116C34, 0x1DC),
    (662, 0x80116960, 0x2D4),
    (662, 0x801246E4, 0x290),
    (662, 0x80123CC8, 0x2C8),
    (662, 0x80123F90, 0x2C8),
    (662, 0x80119898, 0x2AC),
    (662, 0x8011C37C, 0x294),
    (662, 0x8011EFFC, 0x294),
    (662, 0x8011ED20, 0x2DC),
    (665, 0x80116AA4, 620),
    (662, 0x8011E36C, 596),
    (662, 0x80124974, 328),
    (662, 0x80124E4C, 396),
    (662, 0x8011CD18, 628),
    (662, 0x8011A518, 624),
    (662, 0x801161FC, 1072),
    (662, 0x80117F54, 740),
    (662, 0x80117854, 592),
    (662, 0x801207C8, 508),
    (663, 0x80115C34, 692),
    (663, 0x8011602C, 908),
    (663, 0x801163B8, 908),
    (668, 0x80115164, 504),
    (665, 0x80118EB8, 352),
    (662, 0x80119B44, 624),
    (662, 0x8011C610, 556),
    (663, 0x80115964, 572),
    (661, 0x80115598, 2080),
    (665, 0x80116070, 0x22C),
    (665, 0x80115CFC, 0x374),
    (665, 0x80114A24, 0x228),
    (665, 0x80114C4C, 0x38C),
    (660, 0x80114920, 0x320),
    (662, 0x8011512C, 0x284),
    (665, 0x8011643C, 0x580),
    (667, 0x80114E34, 0x214),
    (667, 0x80114AF8, 0x218),
    (667, 0x801151E0, 0x210),
    (662, 0x80125DE0, 0x180),
    (662, 0x80116690, 0x178),
    (662, 0x8011CF8C, 0x134),
    (662, 0x8011AE2C, 0x98),
    (663, 0x80115EE8, 0x144),
    (662, 0x80114998, 0x204),
    (666, 0x801148D4, 0x240),
    (663, 0x80115028, 0x190),
    (667, 0x80117EF8, 0xB0),
    (661, 0x80117350, 0xE0),
    (663, 0x801148D8, 0x188),
    (665, 0x80114FD8, 0x1CC),
    (662, 0x80116020, 0x128),
    (666, 0x80115148, 0x168),
    (666, 0x801152B0, 0xD8),
    (662, 0x8011BF04, 0x108),
    (662, 0x8011B2D4, 0x12C),
    (662, 0x8011B8B4, 0x8C),
    (662, 0x8011B400, 0x84),
    (662, 0x8011AC94, 0xA8),
    (662, 0x8011B034, 0x98),
    (662, 0x8011D338, 0x254),
    (662, 0x8011D664, 0xCC),
    (662, 0x8011D730, 0x1B4),
    (662, 0x8011D8E4, 0x1AC),
    (662, 0x8011D58C, 0xD8),
    (662, 0x8011DCDC, 0xFC),
    (667, 0x80114A40, 0xB8),
    (662, 0x8011883C, 0xF4),
    (660, 0x801162D8, 0xEC),
    (665, 0x8011629C, 0xBC),
    (667, 0x80115494, 0x74),
    (667, 0x801155E0, 0x60),
    (667, 0x801153F0, 0xA4),
    (660, 0x80117084, 0xA8),
    (662, 0x80116148, 0x68),
    (662, 0x801161B0, 0x4C),
    (660, 0x801156A8, 0x84),
    (661, 0x80117E10, 0x80),
    (666, 0x80115510, 0x80),
    (667, 0x801187DC, 0x80),
    (662, 0x8011662C, 0x64),
    (668, 0x80114DF4, 0x54),
    (662, 0x8011A414, 0x104),
    (662, 0x8011A788, 0x110),
    (662, 0x8011D278, 0xC0),
    (662, 0x8011D0C0, 0x60),
    (662, 0x801256D4, 0x48),
    (662, 0x8011D120, 0xE8),
    (662, 0x8011D208, 0x70),
    (662, 0x8011DDD8, 0x98),
    (662, 0x8011DC74, 0x68),
    (668, 0x8011535C, 0x94),
    (662, 0x80125F60, 0xE4),
    (668, 0x80116648, 0x90),
    (668, 0x801166D8, 0x8C),
    (668, 0x80116764, 0x94),
    (668, 0x801167F8, 0x94),
    (659, 0x801158B4, 0x134),
    (662, 0x8011E694, 0x34),
    (662, 0x8011E6C8, 0x6C),
    (662, 0x8011E734, 0x8C),
    (662, 0x8011E7DC, 0x48),
    (668, 0x80114E48, 0x48),
    (668, 0x80114E90, 0x58),
    (668, 0x80116298, 0x34),
    (668, 0x801162CC, 0x8C),
    (668, 0x80116358, 0x98),
    (668, 0x801163F0, 0xC),
    (668, 0x80116534, 0x24),
    (668, 0x801163FC, 0x7C),
    (668, 0x80116478, 0x20),
    (668, 0x80116498, 0x3C),
    (668, 0x801164D4, 0x3C),
    (668, 0x80116510, 0x24),
    (659, 0x801150CC, 0x158),
    (660, 0x80114C40, 0xF8),
    (660, 0x801163C4, 0x16C),
    (660, 0x8011712C, 0x60),
    (661, 0x80114AA0, 0x60),
    (661, 0x80115F8C, 0x164),
    (662, 0x80114B9C, 0xB0),
    (662, 0x801153B0, 0xC4),
    (662, 0x80115474, 0x54),
    (662, 0x80115EC4, 0x15C),
    (662, 0x80116808, 0x158),
    (662, 0x80118930, 0x100),
    (662, 0x80118E6C, 0x6C),
    (662, 0x80119DB4, 0x4C),
    (662, 0x8011A898, 0xF0),
    (662, 0x8011A988, 0xFC),
    (662, 0x8011AA84, 0x70),
    (662, 0x8011AAF4, 0xC0),
    (662, 0x8011ABB4, 0xE0),
    (662, 0x8011B0CC, 0x100),
    (662, 0x8011B940, 0x7C),
    (662, 0x8011DBD8, 0x9C),
    (662, 0x8011E5C0, 0xD4),
    (662, 0x8011F290, 0x230),
    (662, 0x8011F4C0, 0x1B8),
    (662, 0x8011F678, 0x210),
    (662, 0x8011F888, 0x2A4),
    (662, 0x8011FB2C, 0x1B4),
    (662, 0x8011FCE0, 0x1A4),
    (662, 0x8011FE84, 0x180),
    (662, 0x80120004, 0x198),
    (662, 0x8012019C, 0x180),
    (662, 0x8012031C, 0x180),
    (662, 0x8012049C, 0x1AC),
    (662, 0x80120648, 0x180),
    (662, 0x801209C4, 0x1A4),
    (662, 0x80121564, 0x174),
    (662, 0x801216D8, 0x174),
    (662, 0x8012184C, 0x180),
    (662, 0x801219CC, 0x188),
    (662, 0x80121B54, 0x188),
    (662, 0x80121CDC, 0x190),
    (662, 0x80121E6C, 0x188),
    (662, 0x80121FF4, 0x194),
    (662, 0x80122188, 0x188),
    (662, 0x80122310, 0x19C),
    (662, 0x801224AC, 0x18C),
    (662, 0x80122638, 0x18C),
    (662, 0x801227C4, 0x190),
    (662, 0x80122954, 0x198),
    (662, 0x80122AEC, 0x188),
    (662, 0x80122C74, 0x1A0),
    (662, 0x80122E14, 0x1A0),
    (662, 0x80122FB4, 0x110),
    (662, 0x801230C4, 0x164),
    (662, 0x80123228, 0x110),
    (662, 0x80123338, 0x164),
    (662, 0x8012349C, 0x158),
    (662, 0x801235F4, 0x15C),
    (662, 0x80123750, 0x15C),
    (662, 0x801238AC, 0x160),
    (662, 0x80123A0C, 0x15C),
    (662, 0x80123B68, 0x160),
    (662, 0x80124258, 0x184),
    (662, 0x801243DC, 0x184),
    (662, 0x80124560, 0x184),
    (662, 0x80124ABC, 0x1D8),
    (662, 0x80124C94, 0x1B8),
    (662, 0x80124FD8, 0x1D4),
    (662, 0x801251AC, 0x1C4),
    (662, 0x80125500, 0x1D4),
    (663, 0x80114A60, 0x138),
    (663, 0x801157B8, 0xFC),
    (663, 0x801158B4, 0xB0),
    (663, 0x80116744, 0x124),
    (665, 0x80114918, 0x10C),
    (665, 0x80115798, 0x88),
    (665, 0x80116358, 0xE4),
    (665, 0x801169BC, 0xE8),
    (666, 0x8011656C, 0x164),
    (667, 0x8011492C, 0x24),
    (667, 0x80114950, 0x38),
    (667, 0x80114988, 0xB8),
    (667, 0x80114D10, 0x20),
    (667, 0x80114D30, 0x104),
    (667, 0x80115058, 0x10),
    (667, 0x80115508, 0xD8),
    (667, 0x80117EE8, 0x10),
    (668, 0x80114910, 0xD8),
    (668, 0x801149E8, 0xC0),
    (668, 0x80114AA8, 0x84),
    (668, 0x80114B2C, 0x110),
    (668, 0x80114EE8, 0x60),
    (668, 0x801150D8, 0x8C),
    (668, 0x801153F0, 0x70),
    (668, 0x80115460, 0xF4),
    (668, 0x8011596C, 0x54),
    (668, 0x80116558, 0xF0),
    (668, 0x801168A4, 0x7C),
    (662, 0x8011BCC0, 0x108),
    (667, 0x80115068, 0x178),
    (667, 0x80116BB4, 0x164),
    (667, 0x80118634, 0x1A8),
    (668, 0x80114F48, 0x190),
    (668, 0x80115850, 0x11C),
    (668, 0x80115DFC, 0x184),
    (660, 0x801154D8, 0x1D0),
    (661, 0x80114924, 0x17C),
    (661, 0x80115DB8, 0x1D4),
    (661, 0x80117C78, 0x198),
    (662, 0x80117654, 0x200),
    (662, 0x80117AA4, 0x1F4),
    (662, 0x80117C98, 0x164),
    (662, 0x80117DFC, 0x158),
    (662, 0x80118238, 0x188),
    (662, 0x80118ED8, 0x190),
    (662, 0x8011AD3C, 0xF0),
    (662, 0x8011AEC4, 0x170),
    (662, 0x8011B1CC, 0x108),
    (662, 0x8011BDC8, 0x13C),
    (662, 0x8011C00C, 0x168),
    (662, 0x8011C174, 0x208),
    (662, 0x8011DA90, 0x148),
    (662, 0x80125370, 0x190),
    (662, 0x80126044, 0x1D0),
    (666, 0x80114B14, 0x1B8),
    (666, 0x80115388, 0x188),
    (666, 0x80115A88, 0x1D4),
    (668, 0x80114C3C, 0x1B8),
    (663, 0x801151B8, 0x600),
    (666, 0x80115590, 0x4F8),
    (666, 0x80115C5C, 0x910),
    (666, 0x80116730, 0x7B0),
    (659, 0x80114908, 0x7C4),
    (659, 0x80115224, 0x690),
    (659, 0x80115CE4, 0x84C),
    (660, 0x8011572C, 0x87C),
    (660, 0x80115FA8, 0x330),
    (660, 0x80116530, 0xB54),
    (661, 0x801160F0, 0x1250),
    (662, 0x80114C4C, 0x4E0),
    (662, 0x80116E10, 0x844),
    (662, 0x801183C0, 0x47C),
    (662, 0x80119068, 0x830),
    (662, 0x8011B484, 0x430),
    (662, 0x8011B9BC, 0x304),
    (662, 0x8011C83C, 0x4DC),
    (662, 0x8011DE70, 0x200),
    (662, 0x8012571C, 0x2E0),
    (662, 0x801259FC, 0x3E4),
    (665, 0x80116D10, 0x21A8),
    (667, 0x80115640, 0xB68),
    (667, 0x80116D18, 0x11D0),
    (667, 0x8011885C, 0x5A4),
    (668, 0x801159C0, 0x210),
    (668, 0x80115BD0, 0x22C),
    (661, 0x80117E90, 0x4C8),
    (668, 0x80115554, 0x1A8),
    (668, 0x801156FC, 0x154)]
ALIASES = {660: [669], 659: [670], 661: [671], 667: [672]}


# Explicit auxiliary section ownership. Entries are added only with a completed
# function; arbitrary or undeclared object data remains rejected.
# (overlay, function VA): (read-only data VA, exact size)
RODATA_SPANS = {
    (662, 0x801154C8): (0x801148A4, 36),
    (661, 0x80117430): (0x801148C8, 36),
    (668, 0x80115F80): (0x801148A8, 24),
    (660, 0x80114D38): (0x80114894, 44),
    (665, 0x801151A4): (0x8011488C, 40),
    (663, 0x80114B98): (0x8011488C, 36),
    (663, 0x801151B8): (0x801148B4, 36),
    (662, 0x80120B68): (0x8011496C, 20),
    (662, 0x8011C37C): (0x80114928, 20),
    (662, 0x80124974): (0x80114984, 20),
    (662, 0x8011E36C): (0x8011493C, 20),
    (662, 0x801161FC): (0x801148CC, 36),
    (662, 0x801207C8): (0x80114954, 20),
    (665, 0x80118EB8): (0x801148FC, 28),
    (666, 0x80115590): (0x80114894, 36),
    (666, 0x80115C5C): (0x801148B8, 28),
    (659, 0x80115224): (0x8011487C, 36),
    (659, 0x80115CE4): (0x801148A0, 44),
    (660, 0x8011572C): (0x801148CC, 0x24),
    (660, 0x80116530): (0x801148F0, 0x20),
    (661, 0x801160F0): (0x8011489C, 0x2C),
    (662, 0x8011B484): (0x80114910, 0x14),
    (665, 0x80116D10): (0x801148CC, 0x2C),
    (667, 0x80116D18): (0x801148B4, 0x2C),
    (667, 0x8011885C): (0x801148E0, 0x4C),
    (668, 0x801159C0): (0x80114898, 0x10),
    (661, 0x80117E90): (0x80114900, 0x24),
}


def validate_object_sections(parsed, source, rodata_span):
    for section in parsed.iter_sections():
        if section.name in ('.data', '.sdata', '.rodata', '.bss', '.sbss') and section['sh_size']:
            if section.name == '.rodata' and rodata_span and section['sh_size'] == rodata_span[1]:
                continue
            raise ValueError(f'Unexpected data section in {source}')
    if rodata_span:
        section = parsed.get_section_by_name('.rodata')
        if section is None or section['sh_size'] != rodata_span[1]:
            raise ValueError(f'Missing or wrong-sized declared read-only section in {source}')


def read_linked_rodata(parsed, span, module, load_address=0x80114878):
    section = parsed.get_section_by_name('.rodata')
    if span is None:
        if section is not None and section['sh_size']:
            raise ValueError('Undeclared linked read-only section')
        return b''
    address, size = span
    offset = address - load_address
    if size <= 0 or size % 4 or address % 4 or offset < 0 or offset + size > len(module):
        raise ValueError('Declared read-only span is outside the module or misaligned')
    if section is None or section['sh_addr'] != address or section['sh_size'] != size:
        raise ValueError('Linked read-only section does not match its declared span')
    data = section.data()
    if data != module[offset:offset + size]:
        raise ValueError('Relocated read-only data does not match the original module')
    return data


def run(*args):
    subprocess.run([str(a) for a in args], check=True, capture_output=True)


def main():
    blob = (ROOT / 'disc/PAL/R4.BIN').read_bytes()
    padding = padding_spans(blob)
    for index, address, size in SAMPLES + EXTRA_FUNCTIONS:
        assert all(address + size <= a or b <= address
                   for a, b in padding.get(index, [])), (index, hex(address))
    sectors = struct.unpack_from('<951I', blob, 4)
    reconstructed = bytearray(blob)
    results = []
    output = ROOT / 'build/overlays/samples'
    output.mkdir(parents=True, exist_ok=True)
    compiled_units = {}
    for index, address, size in SAMPLES + EXTRA_FUNCTIONS:
        source, name = function_owner(index, address, ROOT)
        rodata_span = RODATA_SPANS.get((index, address))
        if rodata_span:
            data_address, data_size = rodata_span
            if any(other_index == index and data_address < other_address + other_size
                   and other_address < data_address + data_size
                   for other_index, other_address, other_size in SAMPLES + EXTRA_FUNCTIONS):
                raise ValueError('Read-only data overlaps an implemented function')
        key = (index, source)
        if key not in compiled_units:
            compiled_units[key] = link_unit(
                index, source, SAMPLES + EXTRA_FUNCTIONS, RODATA_SPANS,
                output, validate_object_sections, ROOT)
        elf, unit_rodata_span = compiled_units[key]
        with elf.open('rb') as stream:
            parsed = ELFFile(stream)
            text = parsed.get_section_by_name('.text')
            symbol = parsed.get_section_by_name('.symtab').get_symbol_by_name(name)[0]
            assert symbol['st_value'] == address
            offset_in_unit = symbol['st_value'] - text['sh_addr']
            code = text.data()[offset_in_unit:offset_in_unit + symbol['st_size']]
            assert len(code) == symbol['st_size']
            module = blob[sectors[index] * 2048:sectors[index + 1] * 2048]
            unit_rodata = read_linked_rodata(parsed, unit_rodata_span, module)
            rodata = unit_rodata if rodata_span else b''
        offset = sectors[index] * 2048 + address - 0x80114878
        original = blob[offset:offset + size]
        words = lambda data: list(struct.unpack(f'<{len(data) // 4}I', data))
        distance = levenshtein(words(original), words(code))
        row = dict(overlay=index, symbol=name, address=address,
                   original_size=size, compiled_size=len(code), levenshtein=distance,
                   source=str(source.relative_to(ROOT)))
        if rodata_span:
            row['rodata'] = dict(address=rodata_span[0], size=len(rodata), identical=True)
        results.append(row)
        print(f'{index}: {name}: Levenshtein {distance}')
        if distance == 0:
            for site in [index] + ALIASES.get(index, []):
                start = sectors[site] * 2048 + address - 0x80114878
                assert blob[start:start + size] == original
                reconstructed[start:start + size] = code
                if rodata_span:
                    data_start = sectors[site] * 2048 + rodata_span[0] - 0x80114878
                    assert blob[data_start:data_start + len(rodata)] == rodata
                    reconstructed[data_start:data_start + len(rodata)] = rodata
    report = dict(functions=results, reconstructed_r4_identical=bytes(reconstructed) == blob,
                  original_r4_sha256=hashlib.sha256(blob).hexdigest(),
                  reconstructed_r4_sha256=hashlib.sha256(reconstructed).hexdigest())
    original_exe = (ROOT / 'assets/PAL/main.exe').read_bytes()
    built_exe = (ROOT / 'build/PAL/main.exe').read_bytes()
    report['full_exe_identical'] = original_exe == built_exe
    (output / 'verified.json').write_text(json.dumps(report, indent=2) + '\n')
    if any(row['levenshtein'] for row in results):
        raise SystemExit(1)
    assert report['reconstructed_r4_identical'] and report['full_exe_identical']
    print('All implemented functions and duplicate sites verified; reconstructed R4.BIN and full EXE identical.')


if __name__ == '__main__':
    main()
