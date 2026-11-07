#!/usr/bin/env python3
"""Build a complete PAL overlay from its C translation units and typed data."""
import argparse
from overlay_names import name as OVN, entry as overlay_entry
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

from elftools.elf.elffile import ELFFile
from levenshtein import levenshtein
from overlay_units import symbol_addresses
from verify_overlay_samples import RODATA_SPANS

ROOT = Path(__file__).resolve().parents[2]


def run(*args):
    subprocess.run([str(arg) for arg in args], cwd=ROOT, check=True, capture_output=True)


def build(index):
    config = json.loads((ROOT / f'configs/PAL/overlays/{OVN(index)}.json').read_text())
    output = ROOT / f'build/overlays/{OVN(index)}-full'
    output.mkdir(parents=True, exist_ok=True)
    objects, sections = [], []
    for unit in config['units']:
        obj = output / (Path(unit['source']).stem + '.o')
        run(ROOT / 'tools/scripts/cc.sh', ROOT / unit['source'], obj)
        objects.append(obj)
        address = unit['address']
        sections.append((address, f'.text_{address:X} 0x{address:X} : SUBALIGN(4) {{ {obj}(.text) }}'))
        for function in config['functions']:
            table = RODATA_SPANS.get((index, function['address']))
            if function['source'] == unit['source'] and table:
                address, size = table
                sections.append((address, f'.table_{address:X} 0x{address:X} : SUBALIGN(4) {{ {obj}(.rodata) }}'))
    data = output / 'data.o'
    run(ROOT / 'tools/scripts/cc.sh', ROOT / f'src/overlays/PAL/{OVN(index)}/data.c', data)
    objects.append(data)
    for row in config['data_sections']:
        section, address = row['section'], row['address']
        sections.append((address, f'{section} 0x{address:X} : SUBALIGN(4) {{ {data}({section}) }}'))
    defined, undefined = set(), set()
    for obj in objects:
        with obj.open('rb') as stream:
            elf = ELFFile(stream)
            for symbol in elf.get_section_by_name('.symtab').iter_symbols():
                if symbol.name:
                    (undefined if symbol['st_shndx'] == 'SHN_UNDEF' else defined).add(symbol.name)
    addresses, definitions = symbol_addresses(), []
    for name in sorted(undefined - defined):
        match = re.fullmatch(r'(?:[a-z][a-z_]*_)?(?:D|func)_([0-9A-Fa-f]{8})', name)
        address = int(match[1], 16) if match else addresses[name]
        definitions.append(f'{name} = 0x{address:X};')
    end = config['load_address'] + config['image_size']
    sections.append((end-1, f'.padding 0x{end-1:X} : {{ BYTE(0); }}'))
    script = output / 'overlay.ld'
    script.write_text('\n'.join(definitions) + '\nSECTIONS {\n' +
                      '\n'.join(section for _, section in sorted(sections)) +
                      '\n/DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) *(.pdr) *(.gnu.attributes) }\n}\n')
    tools = ROOT / 'tools/toolchain/bin'
    elf_path, binary = output / 'overlay.elf', output / 'overlay.bin'
    run(tools / 'mipsel-none-elf-ld', '-EL', '-T', script, '-Map', output / 'overlay.map',
        '-o', elf_path, *objects)
    base, size = config['load_address'], config['image_size']
    run(tools / 'mipsel-none-elf-objcopy', '-O', 'binary', '--gap-fill=0',
        f'--pad-to=0x{base+size:X}', elf_path, binary)
    rebuilt = binary.read_bytes()
    assert len(rebuilt) == size
    blob = (ROOT / 'disc/PAL/R4.BIN').read_bytes()
    sectors = struct.unpack_from('<951I', blob, 4)
    original = blob[sectors[index]*2048:sectors[index+1]*2048]
    functions = []
    with elf_path.open('rb') as stream:
        elf = ELFFile(stream)
        symbols = elf.get_section_by_name('.symtab')
        for row in config['functions']:
            symbol = symbols.get_symbol_by_name(row['symbol'])[0]
            assert symbol['st_value'] == row['address'] and symbol['st_size'] == row['size'], row['symbol']
            start = row['address'] - base
            words = lambda data: list(struct.unpack(f'<{len(data)//4}I', data))
            distance = levenshtein(words(original[start:start+row['size']]),
                                   words(rebuilt[start:start+row['size']]))
            functions.append(dict(symbol=row['symbol'], address=row['address'], levenshtein=distance))
    for row in config['data_sections']:
        with elf_path.open('rb') as stream:
            section = ELFFile(stream).get_section_by_name(row['section'])
            assert section['sh_size'] == row['size'], row['section']
    identity = {str(site): rebuilt == blob[sectors[site]*2048:sectors[site+1]*2048]
                for site in [index] + config['aliases']}
    report = dict(overlay=index, functions=functions, whole_overlay_identical=identity,
                  rebuilt_sha256=hashlib.sha256(rebuilt).hexdigest(),
                  full_exe_identical=(ROOT/'assets/PAL/main.exe').read_bytes() ==
                                     (ROOT/'build/PAL/main.exe').read_bytes())
    (output / 'verified.json').write_text(json.dumps(report, indent=2) + '\n')
    assert all(row['levenshtein'] == 0 for row in functions), report
    assert all(identity.values()), identity
    assert report['full_exe_identical']
    print(f'Overlay {OVN(index)} (R4 entry {index}): {len(functions)} C functions, Levenshtein 0; whole overlay and EXE identical.')
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--overlay', type=overlay_entry, required=True,
                        help='module name (race_results, records, extra_course_select) or R4.BIN entry number')
    parser.add_argument('--require-complete', action='store_true', help='All manifest units are always C.')
    build(parser.parse_args().overlay)


if __name__ == '__main__':
    main()
