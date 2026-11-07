"""Find game function definitions, including old-style K&R parameters.

Require an opening body brace so prototypes and calls are never counted.
"""
import re
from pathlib import Path

_NONCODE = re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S)
_DEFINITION = re.compile(
    r"^[ \t]*(?:[A-Za-z_]\w*[ \t\n*]+)+"
    r"([A-Za-z_]\w*)\s*\(([^(){};]*)\)\s*"
    r"((?:[A-Za-z_][^(){};]*;\s*)*)\{", re.M)


_KNOWN = None


def known_functions():
    """every function the game defines: auto names (func_XXXXXXXX) and names registered in sym.main.txt"""
    global _KNOWN
    if _KNOWN is None:
        listing = Path(__file__).resolve().parents[2] / 'configs/PAL/functions.txt'
        _KNOWN = {line.split()[0] for line in listing.read_text().splitlines() if line.strip()}
    return _KNOWN


def function_definitions(source):
    source = _NONCODE.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), source)
    names = []
    for match in _DEFINITION.finditer(source):
        if match[1] not in known_functions() and not re.fullmatch(r'func_[0-9A-F]{8}', match[1]):
            continue
        # Declarations between ')' and '{' are valid only for K&R name lists.
        if match[3] and not re.fullmatch(r"\s*[A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*)*\s*", match[2]):
            continue
        names.append(match[1])
    return names


def source_owners(directory):
    """Index functions by their owning TU, independently of the filename."""
    owners = {}
    for source in sorted(Path(directory).rglob("*.c")):
        for name in function_definitions(source.read_text()):
            if name in owners:
                raise ValueError(f"{name}: defined in both {owners[name]} and {source}")
            owners[name] = source
    return owners


def address_of(name):
    """address of a function by auto name (func_XXXXXXXX) or registered name; lets tests keep working when a name is registered later"""
    import re
    if re.fullmatch(r'func_[0-9A-Fa-f]{8}', name):
        return int(name[-8:], 16)
    for line in (Path(__file__).resolve().parents[2] / 'configs/PAL/functions.txt').read_text().splitlines():
        p = line.split()
        if p and p[0] == name:
            return int(p[1], 16)
    if re.fullmatch(r'(?:[a-z][a-z_]*_)?(?:func|D)_[0-9A-Fa-f]{8}', name):
        return int(name[-8:], 16)   # overlay names such as race_results_func_80123456
    for line in (Path(__file__).resolve().parents[2] / 'configs/PAL/sym.main.txt').read_text().splitlines():
        m = re.match(r'^(\w+)\s*=\s*0x([0-9A-Fa-f]+);', line)   # registered data names (CarTable, ScreenFrame)
        if m and m.group(1) == name:
            return int(m.group(2), 16)
    raise KeyError(name)


def name_at(address):
    """current name of the function at an address (hex string or int): the registered name, else func_XXXXXXXX"""
    import re
    a = int(address, 16) if isinstance(address, str) else address
    for line in (Path(__file__).resolve().parents[2] / 'configs/PAL/sym.main.txt').read_text().splitlines():
        m = re.match(r'^(\w+)\s*=\s*0x([0-9A-Fa-f]+);\s*//\s*type:func', line)
        if m and int(m.group(2), 16) == a and not re.fullmatch(r'func_[0-9A-F]{8}', m.group(1)):
            return m.group(1)
    return 'func_%08X' % a
