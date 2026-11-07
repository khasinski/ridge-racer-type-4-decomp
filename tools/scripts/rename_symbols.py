#!/usr/bin/env python3
"""rename_symbols.py: give auto-named functions (func_XXXXXXXX) the real name registered in
configs/PAL/sym.main.txt (`Name = 0xADDR; // type:func`). The name is replaced as a whole word in the sources,
headers, tests, fixtures, configs and docs. The asm is regenerated afterwards (`make split`).

usage: rename_symbols.py [--dry-run]
"""
import re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SKIP_DIRS = {'.git', 'build', '.venv', 'asm', 'linkers', 'assets', 'disc', 'tools/toolchain', 'node_modules'}


def registry():
    """auto name (func_<ADDR>) -> registered name, from `Name = 0xADDR; // type:func` in sym.main.txt"""
    out = {}
    for l in (ROOT / 'configs/PAL/sym.main.txt').read_text().splitlines():
        m = re.match(r'^(\w+)\s*=\s*0x([0-9A-Fa-f]+);\s*//\s*type:func', l)
        if m and not re.fullmatch(r'func_[0-9A-F]{8}', m.group(1)):
            out[f'func_{int(m.group(2), 16):08X}'] = m.group(1)
    for l in (ROOT / 'configs/PAL/sym.main.txt').read_text().splitlines():     # data: D_<ADDR> -> Name
        m = re.match(r'^(\w+)\s*=\s*0x([0-9A-Fa-f]+);\s*//\s*type:(?!func)', l)
        if m and not re.fullmatch(r'D_[0-9A-F]{8}', m.group(1)):
            out[f'D_{int(m.group(2), 16):08X}'] = m.group(1)
    ovl = ROOT / 'configs/PAL/overlays/symbols.txt'     # overlays: <module>_func_<ADDR>/<module>_D_<ADDR> -> name
    if ovl.exists():
        for l in ovl.read_text().splitlines():
            m = re.match(r'^(\w+)\s*=\s*0x([0-9A-Fa-f]+);\s*//\s*overlay:(\w+)\s+type:(\w+)', l)
            if m:
                out[f'{m[3]}_{"func" if m[4] == "func" else "D"}_{int(m[2], 16):08X}'] = m[1]
    return out


def main():
    reg = registry()
    if not reg:
        print('nothing to rename')
        return
    pat = re.compile(r'(?<![\w$])(' + '|'.join(map(re.escape, reg)) + r')(?![\w$])')
    n = 0
    for p in ROOT.rglob('*'):
        rel = p.relative_to(ROOT)
        if not p.is_file() or any(part in SKIP_DIRS for part in rel.parts) or str(rel).startswith('tools/toolchain'):
            continue
        if p.suffix not in {'.c', '.h', '.py', '.txt', '.md', '.json', '.yaml', '.inc', '.s', '.sh'} and p.name != 'Makefile':
            continue
        if p.name in {'rename_symbols.py'}:
            continue
        try:
            t = p.read_text()
        except UnicodeDecodeError:
            continue
        t2 = pat.sub(lambda m: m.group(0) if t[max(0, m.start() - 24):m.start()].endswith(('fixtures/legacy/', 'fixtures/views/'))
                     or t[m.end():m.end() + 3] in ("'.c" , '".c') or t[m.end():m.end() + 2] == ".c" and t[m.end() + 2:m.end() + 3] in ("'", '"')
                     else reg[m.group(1)], t)
        if t2 != t:
            n += 1
            if '--dry-run' not in sys.argv:
                p.write_text(t2)
    if '--dry-run' not in sys.argv:    # fixtures stay address-based; tests may also look one up by its new name
        for sub in ('legacy', 'views'):
            d = ROOT / 'tools/scripts/fixtures' / sub
            for old, new in reg.items():
                src, link = d / (old + '.c'), d / (new + '.c')
                if src.exists() and not link.exists() and not link.is_symlink():
                    link.symlink_to(src.name)
    print('renamed', len(reg), 'functions in', n, 'files (fixture file names stay address-based)')


if __name__ == '__main__':
    main()
