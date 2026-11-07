"""Overlay module names. Overlays are R4.BIN entries; the entry number is only the disc index,
everywhere else (directories, configs, symbol prefixes, tests) an overlay is called by its module name."""
import json
from pathlib import Path

_ROOT = Path(__file__).resolve().parents[2]
_TABLE = {int(k): v['name'] for k, v in json.loads((_ROOT / 'configs/PAL/overlays/names.json').read_text()).items()}
_ENTRY = {v: k for k, v in _TABLE.items()}


def name(overlay):
    """module name for an R4.BIN entry number (int or digit string) or a name"""
    if isinstance(overlay, str) and not overlay.isdigit():
        if overlay not in _ENTRY:
            raise KeyError(overlay)
        return overlay
    return _TABLE[int(overlay)]


def entry(overlay):
    """R4.BIN entry number for a module name or number"""
    return _ENTRY[name(overlay)]


def names():
    return dict(_TABLE)


def symbols():
    """registered overlay symbols: {(module, address): (name, kind)} from configs/PAL/overlays/symbols.txt,
    lines `name = 0xADDR; // overlay:<module> type:<func|data>`"""
    import re
    out = {}
    path = _ROOT / 'configs/PAL/overlays/symbols.txt'
    if path.exists():
        for line in path.read_text().splitlines():
            m = re.match(r'^(\w+)\s*=\s*0x([0-9A-Fa-f]+);\s*//\s*overlay:(\w+)\s+type:(\w+)', line)
            if m:
                out[(m[3], int(m[2], 16))] = (m[1], m[4])
    return out


def symbol_name(overlay, address, kind='func'):
    """current name of an overlay function/data symbol: registered name, else <module>_func_/_D_<ADDR>"""
    module = name(overlay)
    hit = symbols().get((module, address))
    if hit:
        return hit[0]
    return f'{module}_{"func" if kind == "func" else "D"}_{address:08X}'
