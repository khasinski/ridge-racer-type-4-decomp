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
