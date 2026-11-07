# Ridge Racer Type 4 Decompilation

A byte-exact source reconstruction of the main executable of the PAL
PlayStation release of Ridge Racer Type 4 (European release: English, French,
German, Spanish, Italian), plus matching C for a growing number of the overlay
modules loaded from `R4.BIN`.

This repository is source-only. It does not contain disc images, extracted game
assets, CDDA/XA data, Sony PSYQ SDK files, compiler binaries, generated split
output, or decompilation scratch work. Contributors must provide their own
legally obtained copy of the game and a local toolchain.

## Target

| Version | Executable | SHA-1 | Entry PC | Loads at |
|---|---|---|---:|---:|
| PAL | `main.exe` (PS-X EXE) | `f607c67df8f2a08d1bcd7f8d630cd16d92c3c2bc` | `0x8007D718` | `0x80010000` |

The overlay area starts at `0x80114878` (startup clears BSS up to that address);
overlays are loaded from `R4.BIN` (950 entries). See
[docs/overlays.md](docs/overlays.md) for the container format and address map.

## Progress

**The main executable matches byte for byte** (`make check` compares the build
against the SHA-1 above). The fit metric used throughout this project is the
Levenshtein distance between MIPS instruction sequences after relocations;
byte or line differences are never reported as progress.

| Scope | Status |
|---|---|
| Main executable | byte-identical to the retail EXE |
| Overlays 659, 663, 666 | Levenshtein 0, whole overlay and EXE identical (`tools/scripts/build_overlay.py`) |
| Other overlays (660-662, 665, 667, 668) | partial C under `src/overlays/PAL/<id>/`; not yet verified as whole overlays |
| Named symbols | about 1,430 of 1,671 functions and about 690 globals have real names |

Matching does not imply complete C decompilation: some units still use inline
assembly or compiler constraints. The policy and the remaining debt are tracked
in [docs/CRUTCH_DEBT.md](docs/CRUTCH_DEBT.md); `make debt` regenerates the
report and `tools/scripts/code_coverage.py` reports C coverage of resident and
overlay text.

## Layout

- `configs/PAL/` - splat config, symbols (`sym.main.txt`), function table.
- `src/main/` - decompiled C translation units for the main executable.
- `src/overlays/PAL/<id>/` - C for overlay modules.
- `include/` - project headers and local PSYQ-compatible declarations.
- `tools/scripts/` - build, verification and analysis helpers, plus their tests.
- `docs/` - overlay analysis, source layout, decompilation guide, crutch debt.
- `FOUND_NAMES.md` - names preserved in the original files (source file names,
  `$Id:` strings, assertions, diagnostics).

Functions and globals are named in `configs/PAL/sym.main.txt`
(`Name = 0xADDR; // type:func`). `tools/scripts/rename_symbols.py` applies a
registered name across sources, headers, tests and docs, and
`tools/scripts/sync_test_names.py` keeps the tests in step. Names come from
evidence (callers, data flow, PsyQ library structure, emulator traces), not
guesses.

Generated directories such as `asm/`, `linkers/`, `build/`, `assets/` and
`disc/` are intentionally ignored.

## Toolchain

Linking and assembling use GNU binutils for `mipsel-none-elf`. Compiling matching
C uses an old PlayStation GCC (`tools/gcc-970318`) plus
[maspsx](https://github.com/mkst/maspsx) to translate Psy-Q style assembly to GNU
assembler input. The repository publishes neither compiler binaries nor SDK
material.

Expects `assets/PAL/main.exe` locally (and `disc/PAL/` for the overlay work).

## Quick Start

```sh
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt

mkdir -p assets/PAL
cp /path/to/your/main.exe assets/PAL/main.exe

make split
make build
make check          # main.exe: OK means byte-identical
make test           # regression tests, prints FAIL lines only
```

Overlay verification: `.venv/bin/python tools/scripts/build_overlay.py --overlay 659`
(also `663`, `666`) reports the Levenshtein distance and whether the whole
overlay and EXE are identical.

## License

Game code and data remain the property of their respective owners; this
repository contains only clean-room reimplementation source.
