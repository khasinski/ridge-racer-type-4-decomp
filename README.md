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

Overlays are called by module name everywhere (directories, configs, symbol prefixes, tests);
the R4.BIN entry number appears only in `configs/PAL/overlays/names.json`:

| Module | R4.BIN entry | Contents |
|---|---:|---|
| `race_results` | 659 (alias 670) | race, results/standings, vehicle preview |
| `time_attack_course_select` | 660 (alias 669) | time attack course selection |
| `versus_select` | 661 (alias 671) | versus selection |
| `garage` | 662 | garage |
| `records` | 663 | records display, time formatting |
| `options` | 665 | audio, screen calibration, visual options |
| `extra_course_select` | 666 | extra course selection |
| `serial_link` | 667 | multiplayer over the link cable |
| `movie_playback` | 668 | STR movie playback |

## Progress

**The main executable matches byte for byte** (`make check` compares the build
against the SHA-1 above). The fit metric used throughout this project is the
Levenshtein distance between MIPS instruction sequences after relocations;
byte or line differences are never reported as progress.

| Scope | Status |
|---|---|
| Main executable | byte-identical to the retail EXE |
| Overlays `race_results`, `records`, `extra_course_select` | Levenshtein 0, whole overlay and EXE identical (`tools/scripts/build_overlay.py`) |
| Other overlays | partial C under `src/overlays/PAL/<module>/`; not yet verified as whole overlays |
| Named symbols | 1,637 of 1,640 resident functions and about 1,210 globals have real names; the rest are BIOS stubs and fields inside named structs |

Matching does not imply complete C decompilation: some units still use inline
assembly or compiler constraints. The policy and the remaining debt are tracked
in [docs/CRUTCH_DEBT.md](docs/CRUTCH_DEBT.md); `make debt` regenerates the
report and `tools/scripts/code_coverage.py` reports C coverage of resident and
overlay text.

## Layout

- `configs/PAL/` - splat config, symbols (`sym.main.txt`), function table.
- `src/main/` - decompiled C translation units for the main executable.
- `src/overlays/PAL/<module>/` - C for overlay modules, one directory per module name.
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

Overlay verification: `.venv/bin/python tools/scripts/build_overlay.py --overlay race_results`
(also `records`, `extra_course_select`) reports the Levenshtein distance and whether the whole
overlay and EXE are identical.

## License

Game code and data remain the property of their respective owners; this
repository contains only clean-room reimplementation source.
