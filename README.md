# Ridge Racer Type 4 (PAL) — Decompilation

A decompilation of the PAL release of **Ridge Racer Type 4** (`SCES_017.06`) for the PlayStation (PS1) into readable, accurately typed C.

## Matching Criteria

- **The sole matching metric** for functions and translation units is the **Levenshtein distance between relocated MIPS instruction sequences** (target: `0`).
- Reaching distance `0` must be confirmed by verifying full binary identity of the rebuilt main executable (`build/PAL/main.exe`) against the original, as well as verifying the reconstructed overlay modules (`R4.BIN`).

## Repository Layout

```text
.
├── assets/PAL/            # Original main executable (main.exe / SCES_017.06)
├── disc/PAL/              # Original disc files (R4.BIN, FILE.DAT, SCES_017.06, SYSTEM.CNF)
├── asm/PAL/               # Extracted assembly (data, handwritten ASM, segments)
├── configs/PAL/           # Splat configuration, symbol maps, overlay manifests
├── include/               # Public unit headers and shared type headers (rr/)
├── linkers/PAL/           # Linker scripts and auto-generated symbol lists
├── src/
│   ├── main/PAL/          # Resident main executable source code
│   │   ├── game/          # Namco game code (camera, car, course, data, demo, effect, gfx, math, menu, race, sound, system, ui)
│   │   └── psyq/          # Sony PsyQ SDK libraries (libapi, libc, libcard, libcd, libetc, libgpu, libgte, libmath, libpad, libsio, libsnd, libspu)
│   └── overlays/PAL/      # Dynamically loaded overlay modules from R4.BIN (loaded at 0x80114878)
│       ├── grand_prix/    # Overlay 659 (alias 670) — Grand Prix mode
│       ├── time_attack/   # Overlay 660 (alias 669) — Time Attack mode
│       ├── versus/        # Overlay 661 (alias 671) — Versus mode (2-player split-screen)
│       ├── garage/        # Overlay 662 — Garage & emblem editor
│       ├── records/       # Overlay 663 — Record tables
│       ├── options/       # Overlay 665 — Audio & display options
│       ├── extra_trial/   # Overlay 666 — Extra Trial mode
│       ├── serial_link/   # Overlay 667 (alias 672) — Link Cable multiplayer mode
│       └── movie_playback/# Overlay 668 — STR video player & MDEC decoder
├── tools/                 # Toolchain (PSX GCC, maspsx, binutils) and verification/test scripts
└── docs/                  # Technical documentation
```

## Prerequisites & Setup

1. **Python environment:**
   ```sh
   python3 -m venv .venv
   .venv/bin/pip install -r requirements.txt
   ```

2. **Original game assets (PAL, `SCES_017.06`):**
   - `assets/PAL/main.exe` (SHA-1: `f607c67df8f2a08d1bcd7f8d630cd16d92c3c2bc`)
   - `disc/PAL/R4.BIN` (required for analyzing, building, and verifying overlay modules)

3. **Compilers & toolchain (`tools/toolchain/`):**
   The project uses stock PSX GCC compilers (`gcc-2.8.1`, `gcc-2.7.2` for specific SDK units, and the `gcc-970318` snapshot), the `maspsx` assembly preprocessor, and `mipsel-none-elf` binutils (`as`, `ld`, `objcopy`).
   Host compiler build recipes are located in:
   - `tools/gcc-psx/build.sh` (`2.7.2` and `2.8.1`)
   - `tools/gcc-970318/build.sh`

## Building & Verification

### Main Executable (`main.exe`)

- **Split the binary (`splat`):**
  ```sh
  make split
  ```
- **Build and verify the complete executable checksum:**
  ```sh
  make -j8 check
  ```
- **Verify a single translation unit (TU) with full relocation:**
  ```sh
  .venv/bin/python tools/scripts/verify_c_unit.py src/main/PAL/game/path/unit.c --output build/unit_check
  ```
- **Fast single-function iteration (masks relocations — not a substitute for full TU verification):**
  ```sh
  .venv/bin/python tools/scripts/fd.py src/main/PAL/game/path/unit.c
  ```

### Overlay Modules (`R4.BIN`)

- **Verify all implemented overlay functions and reconstructed `R4.BIN`:**
  ```sh
  .venv/bin/python tools/scripts/verify_overlay_samples.py
  ```
- **Build a complete overlay image from C:**
  ```sh
  .venv/bin/python tools/scripts/build_overlay.py --overlay 659
  ```

### Native Tests & Code Quality Audits

- **Run the native and regression test suite:**
  ```sh
  make test
  ```
  or run specific test modules:
  ```sh
  .venv/bin/python tools/scripts/run_tests.py <test_paths> --jobs 4 --output build/tests/check
  ```
- **Audit reconstruction debt (ASM barriers, register pins, casts):**
  ```sh
  make debt
  make debt-check
  ```

## Documentation

Detailed documentation on the architecture and workflow is available in `docs/`:

- [docs/DECOMP_GUIDE.md](docs/DECOMP_GUIDE.md) — type reconstruction guidelines, compiler configurations, and verification workflow.
- [docs/SOURCE_LAYOUT.md](docs/SOURCE_LAYOUT.md) — translation unit organization, header hierarchy, and shared data owners.
- [docs/overlays.md](docs/overlays.md) — `R4.BIN` container analysis, memory map, and overlay module layout.
- [docs/BARRIERS.md](docs/BARRIERS.md) — policy for documenting and retiring `ASM_BARRIER` constraints and register pins.
- [docs/CRUTCH_DEBT.md](docs/CRUTCH_DEBT.md) — reconstruction debt audit summary.
- [FOUND_NAMES.md](FOUND_NAMES.md) — original source filenames, `$Id:` revision strings, and diagnostic labels preserved in the PAL binaries.
- [unused_content.md](unused_content.md) — catalog of unused assets, hidden strings, and debug leftovers.
