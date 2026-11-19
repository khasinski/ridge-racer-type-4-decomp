# ridge-racer-type-4-decomp

Matching decompilation of the European PlayStation release of Ridge Racer Type 4,
target `SCES-017.06`.

This repository is source-only. It does not contain disc images, extracted game
assets, FMV, memory cards, Sony PSYQ SDK files, compiler binaries, or generated
split output. Contributors must provide their own legally obtained copy of the
game and local toolchains.

## Target

| Field | Value |
|---|---|
| Game | Ridge Racer Type 4 (PAL) |
| Executable | `SCES_017.06` |
| Target path | `assets/PAL/main.exe` |
| SHA-1 | `f607c67df8f2a08d1bcd7f8d630cd16d92c3c2bc` |
| Entry PC | `0x8007D718` |
| Resident VRAM | `0x80010000..0x800AC000` |
| Overlay window | `0x80114878` (`disc/PAL/R4.BIN`) |

## Matching Policy & Quality

The matching metric for functions and translation units is the Levenshtein
distance between relocated MIPS instruction sequences. Distance `0` must be
confirmed by complete binary identity of the linked executable (`make check`)
and reconstructed overlay modules in `R4.BIN`.

Binary matching is only the baseline. A translation unit is complete when its
symbols are named, its data owners and interfaces are accurately typed with
full-width native pointers, and it is placed in its proper subsystem; see
[docs/DECOMP_GUIDE.md](docs/DECOMP_GUIDE.md) and
[docs/SOURCE_LAYOUT.md](docs/SOURCE_LAYOUT.md).

The matching policy is conservative: decompiled code must match through the
stock compiler and assembler pipeline. Register pins and empty `asm` barriers
are treated as reconstruction debt, tracked via `make debt`, and retired as
types and shared layouts are recovered; see
[docs/BARRIERS.md](docs/BARRIERS.md) and
[docs/CRUTCH_DEBT.md](docs/CRUTCH_DEBT.md). Inline instructions are restricted
to named hardware/GTE macros in `include/rr_asm.h` or functions proven to be
handwritten assembly (`configs/PAL/hasm.txt`). Compiler postpasses, instruction
patching, and build hacks are forbidden.

## How the game is built

`SCES_017.06` is the resident executable (`0x80010000..0x800AC000`): boot,
the main loop, vehicle physics and AI, track geometry, rendering, HUD/UI,
audio management, CD streaming, and the Sony PsyQ SDK libraries
(`0x8007D718..0x8009DF14`). Startup clears BSS up to `0x80114878`, which serves
as the load address for executable overlays streamed from the 950-entry
`R4.BIN` container on disc.

Nine unique executable overlays (stored across 13 disc entries, with four
on-disc duplicate pairs: `669=660`, `670=659`, `671=661`, `672=667`) share the
RAM window at `0x80114878`:

- `grand_prix` (`659` / `670`) — Grand Prix race, standings, results, and vehicle preview
- `time_attack` (`660` / `669`) — Time Attack course selection, race, and results
- `versus` (`661` / `671`) — 2-player split-screen selection, race, and results
- `garage` (`662`) — Garage and emblem editor
- `records` (`663`) — Record tables and time formatting
- `options` (`665`) — Audio and screen calibration options
- `extra_trial` (`666`) — Extra Trial mode
- `serial_link` (`667` / `672`) — Link Cable multiplayer mode
- `movie_playback` (`668`) — STR video playback and MDEC decoding

The container format, memory map, and overlay verification pipeline are
documented in [docs/overlays.md](docs/overlays.md). Original source filenames
and diagnostic identifiers preserved in the retail binaries are cataloged in
[FOUND_NAMES.md](FOUND_NAMES.md), and unused code/assets in
[unused_content.md](unused_content.md).

## Layout

- `src/main/PAL/game/<subsystem>/` — resident Namco game code (`camera`, `car`,
  `course`, `data`, `demo`, `effect`, `gfx`, `math`, `menu`, `race`, `sound`,
  `system`, `ui`).
- `src/main/PAL/psyq/lib*/` — Sony PsyQ SDK translation units with their SDK
  library names.
- `src/overlays/PAL/<overlay>/` — one directory per unique overlay module.
- `include/game/`, `include/psyq/`, `include/rr/` — public unit headers (one
  owner per declaration) and shared type headers.
- `configs/PAL/` — splat configuration, symbol maps, handwritten ASM registry,
  and overlay manifests (`configs/PAL/overlays/`).
- `tools/scripts/` — build, verification, debt audit, and native test harnesses.
- `docs/` — architecture, layout, and decompilation guides.

Generated directories such as `asm/`, `linkers/`, `build/`, `assets/`, and
`disc/` are intentionally ignored, along with local toolchains and scratch
notes.

## Toolchain

Assembling and linking use standard GNU binutils (`mipsel-none-elf-as`,
`mipsel-none-elf-ld`, `mipsel-none-elf-objcopy`) under `tools/toolchain/bin/`.

Compiling matching C via `tools/scripts/cc.sh` uses stock PSX GCC `cc1`
binaries paired with [maspsx](https://github.com/mkst/maspsx) (`--aspsx-version=2.79`
by default, with `-O2 -G0 -mips1 -mcpu=3000 -funsigned-char`):

- **GCC 2.8.1** (`tools/toolchain/gcc-2.8.1/cc1`) — default compiler for Namco
  game code, overlays, and late PsyQ units.
- **GCC 2.7.2** (`tools/toolchain/gcc-2.7.2/cc1`) — selected per unit via
  `/* GCC_VERSION: 2.7.2 */` for older PsyQ SDK modules.
- **GCC 970318** (`tools/toolchain/gcc-970318/cc1`) — 1997-03-18 snapshot
  selected via `/* GCC_VERSION: 970318 */` for PsyQ 4.5+ library objects.

Build the local host compilers with:

```sh
bash tools/gcc-psx/build.sh 2.7.2
bash tools/gcc-psx/build.sh 2.8.1
bash tools/gcc-970318/build.sh
```

Environment overrides supported by `tools/scripts/cc.sh`: `CPP`, `CC1`, `AS`,
`PYTHON`, `RR_ASPSX_VERSION`, and `RR_MASPSX_FLAGS`.

## Quick Start

```sh
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt

mkdir -p assets/PAL disc/PAL
cp /path/to/your/SCES_017.06 assets/PAL/main.exe
cp /path/to/your/R4.BIN disc/PAL/R4.BIN

make split
make -j8 check
```

`make split` generates the splat configuration, splits `assets/PAL/main.exe`
into `asm/` and `linkers/`, and normalizes assembly symbols and BIOS stubs.
`make build` compiles and links `build/PAL/main.exe`, and `make check` verifies
its SHA-1 against retail.

Useful verification and test targets:

```sh
# Verify a single translation unit with full relocation resolution
.venv/bin/python tools/scripts/verify_c_unit.py src/main/PAL/game/car/carmove1.c --output build/unit_check

# Verify all overlay functions, duplicate disc sites, and reconstructed R4.BIN
.venv/bin/python tools/scripts/verify_overlay_samples.py

# Build a complete overlay binary from C (e.g. 659, 663, 666)
.venv/bin/python tools/scripts/build_overlay.py --overlay 659

# Run native C / MIPS regression tests and source-debt checks
make test
make debt-check
```
