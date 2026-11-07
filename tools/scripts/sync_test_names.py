#!/usr/bin/env python3
"""sync_test_names.py: native tests find the globals and functions a game body uses with regexes such as
`D_[0-9A-F]{8}|CarTable|...` and `func_[0-9A-F]{8}|PlaySfx|...`. Names registered in configs/PAL/sym.main.txt are not
matched by the address patterns, so this rewrites those alternations to list every registered name.
Idempotent. usage: sync_test_names.py"""
import re
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
funcs, data = [], []
for l in (ROOT / 'configs/PAL/sym.main.txt').read_text().splitlines():
    m = re.match(r'^(\w+)\s*=\s*0x[0-9A-Fa-f]+;\s*//\s*type:(\w+)', l)
    if not m or re.fullmatch(r'(func|D)_[0-9A-F]{8}', m[1]): continue
    (funcs if m[2] == 'func' else data).append(m[1])
_ovl = ROOT / 'configs/PAL/overlays/symbols.txt'
if _ovl.exists():
    for l in _ovl.read_text().splitlines():
        m = re.match(r'^(\w+)\s*=\s*0x[0-9A-Fa-f]+;\s*//\s*overlay:\w+\s+type:(\w+)', l)
        if m: (funcs if m[2] == 'func' else data).append(m[1])
funcs_set, data_set = set(funcs), set(data)
D_ALT = re.compile(r'(D_\[(?:0-9A-F|A-Fa-f0-9)\](?:\{8\}|\+))((?:\|[A-Za-z0-9_]+(?![\[A-Za-z0-9_]))*)')
F_ALT = re.compile(r'(func_\[0-9A-F\]\{8\})((?:\|[A-Za-z0-9_]+(?![\[A-Za-z0-9_]))*)')
def merge(extra, existing, known):
    keep = [t for t in existing.split('|') if t and t not in known]
    return ''.join('|' + t for t in keep + sorted(known, key=lambda s: (-len(s), s)))
n = 0
for p in sorted((ROOT / 'tools/scripts').glob('test_*.py')):
    out = []
    for line in p.read_text().split('\n'):
        if 're.' in line and ('D_[' in line or 'func_[0-9A-F]{8}' in line):
            line = D_ALT.sub(lambda m: m[1] + merge(None, m[2], data_set), line)
            if 'findall' in line: line = F_ALT.sub(lambda m: m[1] + merge(None, m[2], funcs_set), line)
        out.append(line)
    new = '\n'.join(out)
    if new != p.read_text(): p.write_text(new); n += 1
print('updated', n, 'tests')
