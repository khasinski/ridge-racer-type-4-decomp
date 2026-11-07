# PAL overlays: static analysis

Analysis date: 2026-10-28. Entry indices below are **zero-based**.

## Container and address map

`disc/PAL/R4.BIN` has 950 entries: a little-endian u32 count followed by
951 u32 sector offsets (2048-byte sectors). First payload sector: 2; terminal
sector: 28776, exactly the file end (58,933,248 bytes). Entries are heterogeneous:
**not every entry is a nested container**. Executable overlays are flat images
with a module ID, strings/jump tables, code, data, and sector padding.

The overlay destination is **0x80114878**, not the formerly suspected 0x80110000.
Evidence:

- Main EXE word at VA `0x800102F0` (file offset `0xAF0`) is `0x80114878`.
- Startup at `0x8007D718` clears BSS through `0x80114878` (exclusive).
  The `0x80110000` region contains resident library state, not overlay code.
- `LoadModeMenuAssetsStep` resets `LoadCursor` to `CdDirectoryBuffer`, then requests
  `CdFileReadStart(GetR4FileIndex(1, mode + 0xB), LoadCursor)`.
  The category-1 mapper adds `0x292`, so this selects entries 669..673 for
  mode 0..4. These include copies of the executable menu modules.
- `LoadSelectedModeFileStep` likewise requests entries `659 + ModeMenuCursorSaved` into that buffer.
- `LoadSplashAssetsStep` and `LoadIntroDemoAssetsStep` explicitly load entry 668
  (`0x292 + 10`) into the reset buffer.
- Absolute jump tables and intra-overlay calls agree with this base.
  For example, entry 660 has a function prologue at file offset `0xA8`,
  hence VA `0x80114920`, called by resident `MenuStepExitToNextMode`.

The loader reuses the ordinary asynchronous CD state machine:
`LoadR4Directory` converts the R4 directory into `CdFileTable` entries;
`CdFileReadStart` selects destination/location/sector count;
`CdFileStateStartRead` starts the read through `CdRead`;
`CdFileBytesLoaded` reports completed bytes. The callers advance their allocation
cursor by the full sector-rounded size. Static code contains absolute resident
and overlay addresses; there is no evidence here of a relocation pass.
Runtime cache behavior has not been verified in an emulator.

## Executable inventory

The spans below run from the first identified instruction to the final observed
return delay slot, with an exclusive end. They are **candidate text spans**,
not a finalized linker section map or a verified function partition.
Role names are inferred from embedded strings and callers; the registry is `configs/PAL/overlays/names.json` (aliases: 669=660, 670=659, 671=661, 672=667).

| Entry | Name | Module ID | Sector | Stored bytes | Candidate text offsets | Text span bytes | Interpretation |
|---|---|---|---|---:|---|---:|---|
| 659 | `race_results` | 10 | 19512 | 8192 | 0x54..0x1CB8 | 7268 | Race results, standings and preview (`race_results_rank_standings`) |
| 660 | `time_attack_course_select` | 11 | 19516 | 14336 | 0x98..0x2FA8 | 12048 | Time attack course selection (`job_ta_crs_info0`) |
| 661 | `versus_select` | 8 | 19523 | 16384 | 0xAC..0x3AE0 | 14900 | Versus selection (`PLAYER 2`, `job_vs_car_info0`) |
| 662 | `garage` | 12 | 19531 | 77824 | 0x120..0x1199C | 71804 | Garage (`garage0`, `job_garage_car_info0`) |
| 663 | `records` | 13 | 19569 | 10240 | 0x60..0x1FF0 | 8080 | Records (`recode0`) |
| 665 | `options` | 14 | 19582 | 20480 | 0xA0..0x47A0 | 18176 | Options: audio mono/stereo, screen calibration |
| 666 | `extra_course_select` | 15 | 19592 | 10240 | 0x5C..0x2668 | 9740 | Extra course selection (`job_ex_crs_info0`) |
| 667 | `serial_link` | 9 | 19597 | 18432 | 0xB4..0x4588 | 17620 | Multiplayer / serial (`sio:`, `PLAYER 2/3/4`) |
| 668 | `movie_playback` | 6 | 19606 | 79872 | 0x98..0x2430 | 9112 | STR playback (`strNext`, timeout diagnostics); decoder tables excluded |

Exact whole-entry duplicate pairs: **669 = 660**, **670 = 659**,
**671 = 661**, **672 = 667**. Thus 13 executable entries represent 9 unique
images. Their unique candidate text spans total **168,748 bytes**.
Duplicates should share source/build products and be checked at both disc sites.

Entries 658, 664 and 673 are not executable overlays on the evidence examined:
658/673 have asset container headers; 664 begins with the `SC` resource signature
and Shift-JIS text. Their sizes must not be added to a code denominator.

A full aligned-word scan of all 950 entries found return-instruction patterns
only in the 13 listed overlays and entry **879**. Entry 879 is a structured
resource container: its apparent instructions occur in repeated 44-byte records
with constants 1000/400 and zeroed operands, rather than contiguous executable
MIPS. Treat this as **unclassified MIPS-pattern data**, not a tenth confirmed
overlay. Its resource semantics still need investigation. This scan does not
prove absence of compressed code or code without standard returns elsewhere.

## Effect on coverage

### NOP alignment (2026-11-03)

`gen_config.py` emits `pad` subsegments in `main.yaml` for gaps between known
functions only when the entire original range is zero. Instructions within
functions, including NOP delay slots, remain part of those functions.
`configs/PAL/code_layout.yaml` records zero-filled resident data that must remain
outside padding (the kernel descriptor reset image at `0x80096044..0x80096050`)
and the equivalent overlay padding ranges as
half-open VRAM intervals: module 668 has alignment at `0x80116920..0x80116928`
before `DecDCTvlcSize`. Coverage excludes these declared padding ranges alongside
hand-written assembly. Coverage and overlay verification validate the original
padding contents; padding must not overlap implemented functions.

### STR section-map correction (2026-11-02)

The original return-word scan overestimated module 668's text extent. Its last
executable return and delay slot are at `0x80116CA0` and `0x80116CA4`.
Data starts at `0x80116CA8`: initial state, the twenty-word camera lookup at
`0x80116CC0`, quantization commands at `0x80116D10`, and the matrix command at
`0x80116D94`. The command layouts are independently used by the implemented
helpers and declared in `include/rr/overlay668_decode.h`.

The decoder at `0x80116958` loads two data bases, `0x80117690` and
`0x80127690`, into `a2` and `a3`. At `0x80116B3C` it indexes the first with
the upper thirteen input bits times eight: 8192 records, ending exactly at the
second base. At `0x80116B7C` it indexes the second with the upper nine bits
times four: 512 records, ending at `0x80127E90`. Lookup results are consumed
as bit lengths and coefficient values; they are not control-flow targets.
The apparent returns near `0x80127818` lie inside that second table.
The decoder also reads two 256-record, four-byte tables immediately before
the first base, at `0x80116E90` and `0x80117290`.

Thus the old `0x12FAC` end included decoder data. `analyze_overlays.SPANS`
now uses the actual exclusive code end `0x2430`, without adding any completed
functions. The combined resident/overlay code pool is now 741,432 bytes.
Coverage ledgers must be recalculated against this corrected map; historical
figures below describe the earlier provisional estimate.

The resident EXE has 572,684 text bytes; C definitions cover 233,480 bytes.
Adding the nine unique candidate overlay text spans yields an estimated code
pool of **809,908 bytes**, hence approximately **28.83%** recovered in C.
This is a provisional whole-program estimate: candidate section boundaries,
entry 879 and possible compressed payloads remain to be resolved. The previous
**40.77%** remains the exact stated coverage of the main EXE text only.
No overlay functions have been added to the 1200 completed C functions.
Do not interpret `jr ra` counts as function counts: functions can have multiple
returns, tail calls, or no standard return.

## Reproduction and next work

Run:

```sh
.venv/bin/python tools/scripts/analyze_overlays.py
```

This validates the directory, checks duplicate identities, emits a full
950-entry inventory to `build/overlays/inventory.json`, extracts the 13 images,
and writes nine candidate-text disassemblies. It does not modify main EXE
configuration, symbols, source ownership, or completion statistics.

Use overlay identity **and** address in future symbols: the same RAM address
has different code across images. A useful layout is `src/overlays/PAL/<id>/`
and a separate split/link map per unique image. Establish code/data/BSS sections
and function partitions before enabling exact coverage accounting. Start with
659 (smallest candidate text span), then 660 (known resident call target and
recognizable course-selection structures). Garage 662 is a useful larger TU
for shared menu structures. STR module 668 likely contains substantial library
code and should be tracked separately when distinguishing game/library code.

## Compiler (verified on three small functions: ResetCarSlotBuffers, BackupSaveData, AngleDiffAbs)

gcc **2.8.1** PSX cc1 reproduces the original codegen (hoisted `lui/addiu` base pointers, loop delay slots);
2.7.2 folds the symbols into the load/store and does not match. Pipeline in `tools/scripts/cc.sh`:
cpp -> gcc 2.8.1 cc1 `-O2 -G0 -mips1 -mcpu=3000 -funsigned-char` -> maspsx -> GNU as.
Binaries are copied (not committed) into `tools/toolchain/` from ~/Projects/parasite-pc and ~/Projects/rage-pc.
ASPSX version (`--aspsx-version=2.79`) and gas backend are unverified guesses.


## First C samples (2026-10-28)

One complete function from every unique image is now in
`src/overlays/PAL/<entry>/`. Symbols include the image number to avoid collisions
between mutually exclusive RAM images. These are separate reconnaissance units;
they are not included in the resident EXE's 1200-function ownership count.

| Entry | Function VA | Behavior |
|---|---|---|
| 659 | 0x801148CC | Calls resident cleanup and sets the next menu state |
| 660 | 0x80114910 | Writes transition code 0x131 |
| 661 | 0x80117340 | Writes transition code 0x137 |
| 662 | 0x8011E7C0 | Stores four low-halfword arguments in overlay-local state |
| 663 | 0x80115BA0 | Initializes records display state, scratchpad and resident renderer |
| 665 | 0x80115C8C | Initializes a menu panel and publishes its state |
| 666 | 0x801166D0 | Cleans up course selection and updates the display |
| 667 | 0x80115048 | Reads an overlay-local state word |
| 668 | 0x8011688C | Reads a word through an overlay-local pointer |

All nine compile with the existing default **GCC 2.8.1 / MASPSX** pipeline,
without compiler overrides, register bindings or assembly barriers. All have
**instruction Levenshtein distance 0 after full relocation resolution**.
This supports the load address and toolchain choice for these functions; it
does not establish compiler identity for every function or library in each image.

Reproduce:

```sh
.venv/bin/python tools/scripts/verify_overlay_samples.py
.venv/bin/python -m unittest tools/scripts/test_overlay_samples.py -v
```

The verifier reads original disc entries directly, compiles each source, links
it at its original VA with every external address resolved, and compares the
resulting instruction sequence. `SUBALIGN(4)` preserves the original four-byte
placement even where it differs from the object's default section alignment.
It also replaces the nine function spans and their four duplicate sites in an
in-memory copy of R4.BIN, confirms whole-image identity, and independently checks
the currently built main EXE against its original. This is a **partial rebuild
using original bytes for untouched regions**, not a complete overlay link.
The machine-readable result is `build/overlays/samples/verified.json`.

The native test compiles the actual nine source bodies with UBSan. Its only
address adaptation replaces the records routine's literal scratchpad base with
a host array. Resident calls use assertions for arguments, call order and live
state; callbacks change pointers before subsequent stores to check fresh loads.
It covers every low-halfword value in the garage setter, surrounding storage,
word getters and changing full-width host pointers. These are unit-level checks,
not an emulator playthrough. Existing resident source files and shared headers
were left untouched to allow concurrent debt cleanup.

## Complete C translation units: 659/670, 663, 666

These three unique overlays are now built entirely from C, including typed local
data. No reference assembly is linked into their images. Overlay 670 is the
on-disc alias of 659.

| Overlay | Functions | Code TUs | Responsibilities |
| --- | ---: | ---: | --- |
| 659 / 670 | 7 | 3 | Race, results/standings, vehicle preview |
| 663 | 14 | 4 | Menu, selection, previews, record rendering/time formatting |
| 666 | 13 | 4 | Course menu, results, preview, race/checkpoints |

The manifests in `configs/PAL/overlays/` assign each function to exactly one TU
and fix the original addresses of code, generated jump tables and typed data.
`include/rr/overlay659.h`, `overlay663.h` and `overlay666.h` expose module APIs;
`menu_overlay_runtime.h` gives these modules one consistent resident interface.
Local globals have module names and real C definitions. The shared selection
buffer has explicit choice, selection-list and entry views in one union.

The C controllers use ordinary array indexing and structured state transitions.
They contain no register bindings, inline assembly, fake volatile locals,
compiler-version overrides or integer conversions of software pointers. The RGB
argument uses its real three channels and aligned ABI view; padding is ignored
by the renderer. The scratchpad remains a genuine hardware address.

```sh
make -j8 check
.venv/bin/python tools/scripts/build_overlay.py --overlay race_results
.venv/bin/python tools/scripts/build_overlay.py --overlay records
.venv/bin/python tools/scripts/build_overlay.py --overlay extra_course_select
.venv/bin/python tools/scripts/verify_overlay_samples.py
```

Each complete build checks relocated instruction Levenshtein distance 0 for
all module functions, verifies symbol addresses and sizes, then checks the entire
overlay image (including alias 670) and the full main EXE against the originals.
Reports are saved under `build/overlays/{entry}-full/verified.json`. The sample
verifier also handles multiple functions per TU, compiling each TU only once.
Its reconstructed R4.BIN still uses original bytes outside verified functions;
the three complete module builders do not use that partial-rebuild mechanism.

Historical per-function headers forward to the canonical module interface.
Native fixtures select the actual definition from its owning TU, rather than
keeping duplicate C bodies. Native regressions use UBSan and compare behavior
with independent state oracles or original MIPS service traces.

## Overlay 660/669 translation units

All fourteen matched functions now belong to four contiguous code translation
units: `course_menu.c`, `results.c`, `preview.c`, and `race.c`. The 660 manifest
records function ownership and local-global addresses. It is a code-layout
manifest; complete-image builders remain available for 659, 663 and 666.

`rr/overlay660.h` is the module interface. Historical per-function headers
forward to it. Resident objects use the shared menu runtime types, including
full frame storage, course records, selection storage and race vehicles.
Module-local declarations have descriptive names and one declaration per
symbol. The retry selector retains the caller's explicit, unused zero argument.

GPU scratch pointer types live in `gpu_scratch_types.h`; the common race
scratch prefix is defined once in `race_frame_scratch.h` and shared by 659 and
660. The race-state gate reads the existing steering `count22` field through
its control-record type instead of a raw byte offset. The course controller
uses one ordinary loop counter in place of four forced argument registers.
