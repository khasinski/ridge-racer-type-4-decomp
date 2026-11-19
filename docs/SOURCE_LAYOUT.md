# Source layout

`src/main/PAL/` is organised as the original program was: game code and the Sony PsyQ library
are separate trees, and each `.c` file is a translation unit (TU) holding several functions.

```
src/main/PAL/
  game/        Namco game code            (addresses below 0x8007D000)
    system/    boot, main loop, interrupt/timer glue, pad
    gfx/       ordering tables, primitives, sprites, fonts, renderers
    ui/        HUD, text, windows        menu/   front-end screens
    sound/     game-side audio manager   data/   CD/file loading, compression, save
    course/    track data and geometry   car/    vehicle state, physics, AI
    race/      race flow, rules, replay  camera/ effect/ math/ demo/
  psyq/        Sony PsyQ SDK (compiled with the library compiler, see DECOMP_GUIDE "Third compiler")
    libapi/ libc/ libcard/ libcd/ libetc/ libgpu/ libgte/ libmath/ libpad/ libsio/ libsnd/ libspu/ other/
  func_XXXXXXXX.c   (top level only) a freshly matched function that has not been placed yet
```

Overlay code stays in `src/overlays/PAL/<n>/` and is described in `docs/overlays.md`.

## File naming
PsyQ files take the name of the real library member. Three real source names are preserved
verbatim in `$Id:` strings of the executable (see `FOUND_NAMES.md`): `intr.c` (libetc), `bios.c`
(libcd) and `sys.c` (libgpu). Hence `libetc/intr*`, `libcd/bios_*` and `libgpu/sys_*` (`sys_reset`,
`sys_queue`, `sys_drain`, `sys_sync`, `sys_timer`, `sys_load`, `sys_store`, `sys_clear`) are slices of
those members. The slices are separate units because the verified compiler configuration differs
between them (gcc 2.8.1, 970318, 2.7.2), so they cannot yet be proven to be one translation unit; the
suffix after `sys_` is a descriptive guess. `libgpu/gpudisp` (display environment) is not tied to
`sys.c`. Other PsyQ names come from `configs/PAL/psyq_hints.parasite.txt` against the member lists of
https://github.com/software-factory-vibe-city/psyq-decompilation.

Game files use Namco/PsyQ-era style: lower case, at most 8 characters, a noun or noun+verb abbreviation
(`carmove1.c`, `cdread.c`, `sprite.c`, `vmchval.c`). Where a PsyQ library file name is known
(`utgpa`, `ccadsr`, `midi*`) it is used. Many names are descriptive guesses; they carry no
information about the real original file names. A trailing digit (`menu1`, `raceinit`) only orders
units of one topic that sit next to each other in the address space; it is not an original name.

Names chosen from code content alone (guesses): `gfxmtx` (3x3 short matrices), `menuwork` (menu work
area), `menustat` (menu state machine), `menuslot` (0x24-byte menu slots), `barsel0/1` (bar selection
variants), `uicode` (code table lookup), `badgeprim`/`badgespr`, `menuicon`, `raceinit`, `trktext`.

Current organization: 462 translation units (`game/` and `psyq/`). `make check` verifies the
complete resident EXE; the relocated instruction audit and native tests are separate checks.
Many game file names are still descriptive guesses.

## How the TUs were formed (reconstructed, not recovered)
The original TU boundaries are not stored in the executable. The current grouping is a
reconstruction that obeys everything that is *hard evidence*:
* one compiler configuration per TU (`GCC_VERSION`, `CC1_FLAGS`, `MASPSX_FLAGS` markers);
* functions of a TU form one address-contiguous run (checked by `gen_config.py`);
* the `.rodata` blocks of a TU are one contiguous run;
* hand-written assembly (`configs/PAL/hasm.txt`) is never inside a C TU;
* every TU compiles as a whole and every function stays at relocated Levenshtein distance 0.

Conflicting global declarations in reconstructed units are evidence that their
types still need investigation; they do not establish incompatible declarations
in the original source. Recover the shared owner and its pointer/scalar fields,
then update its public declaration and consumers together. The merge tool's
historical alias mode wrapped conflicting views in scoped cast macros labelled
`legacy typing of shared globals`. The current resident/overlay audit finds no
such blocks; do not reintroduce them to conceal unresolved layouts.
`tools/scripts/tu_coalesce.py` repeats the greedy merge of adjacent units in one directory.

Between those constraints functions were grouped thematically (shared callees and data,
TextTiling over the call/data reference profile), then each group was cut at the first point
where the merged text no longer compiled or matched. Treat the boundaries as editable.

Overlay 662 keeps `editor_selection.c` and `editor_hit.c` as adjacent units.
Selection uses its existing stock `-fno-schedule-insns` profile; hit testing and
clip stores use the default profile. Combining all six functions under either
current profile loses the relocated match. The two-unit arrangement verifies
all six functions at their original addresses, with unchanged C bodies and no
new compiler constraints. These are verified reconstruction boundaries, not
recovered original source-file boundaries.

## Tools
* `tools/scripts/gen_config.py` + `make split` – segment names are the paths below `src/main/PAL`
  (`game/car/carmove1`), so moving or renaming a file is just a `git mv` followed by `make split`.
* `tools/scripts/tumerge.py` – parser/merger used to build the TUs (dedupes includes, typedefs and
  externs, renames colliding local types, emits prototypes for functions defined in the TU).
* `tools/scripts/fixtures/legacy/` freezes ~100 pre-merge single-function sources for the few
  unit tests that depend on their exact include layout.
* `make views` generates the single-function views used by older native fixtures. New fixtures
  use `c_functions.source_owners` and `definition_source` to select the actual current body and
  supply its types and service adapters explicitly. A merged TU's unrelated functions and scoped
  alias macros do not belong in a single-function native harness.
* `make test` calls `tools/scripts/run_tests.py`. The runner invokes TestCase modules through
  `unittest`, runs standalone script tests normally, rejects zero-test unittest runs and returns
  failure if any module fails. Per-module logs and incremental results are in `build/tests/run/`.
  Running a TestCase file directly is insufficient: many modules have no `unittest.main()` guard.
* Overlay 662 fixtures keep their historical address paths as function identifiers,
  but use `overlay_units.read_c_source` to extract the current definition from its
  owning TU. Their native harnesses, models, cases and PRNG streams are unchanged.
  The controller and fade trace tools resolve the actual TU for dependency hashes;
  the controller selects its named definition explicitly instead of dropping the
  first line of a former single-function file. Frozen fixture files are not edited.
* New matches: write `src/main/PAL/func_XXXXXXXX.c`, use `fd.py` for iteration, then move it into the
  TU it belongs to (same compiler markers, address-contiguous with its neighbours) or leave it at
  top level until a neighbour is matched. `verify_c_unit.py` compiles and fully relocates the TU,
  checks every registered/address-named function and its address, text extent and rodata.
  `fd.py` masks relocations and is not proof of a relocated match. Finish with `make check`.

## Headers
Every translation unit that exports a function or a global gets its own header, mirrored under
`include/` (`src/main/PAL/game/car/carmove1.c` -> `include/game/car/carmove1.h`). There is no
catch-all header: a caller includes the header of the unit that defines the symbol, so the include
graph shows which units depend on each other (and therefore whether a unit boundary is right).

* Functions: the owner is the unit that defines them.
* Globals (`D_…`): the data sections follow the order of the units, so a shared global is assigned to
  the unit whose rank matches its address (interpolated from globals used by only one unit). The owner
  only decides which header carries the declaration; it does not affect the bytes.
* `tools/scripts/unit_headers.py` tries existing type spellings in callers and
  requires their relocated Levenshtein distance to remain 0. That selects
  matching candidates; it does not prove the chosen layout or interface is
  correct. Check actual definitions, callers and data accesses, including full
  native pointer widths, before treating a type as recovered.
* Remaining local declarations are reconstruction debt. A failed attempt to unify
  one declaration constrains the current reconstructed caller; it does not establish that the
  original source used an incompatible declaration. Return types, argument counts, promotions and
  pointee layouts need to be checked together. `proto_conflicts.py` lists the current candidates,
  including registered names; it does not prove that a listed declaration cannot be unified.
* Forward prototypes of a unit's own functions stay where the definition order needs them.
* Tools: `split_decls.py` (one declaration per symbol), `unit_headers.py` (create headers, rewrite
  callers), `own_includes.py` (units include their own header), `dedupe_rr.py` (drop local copies of
  symbols that an `rr/` header already declares). `unit_headers.fd_ok` uses `verify_c_unit.py`;
  check a tool's verifier before relying on an older cleanup script's success report.

Work list for the remaining declarations: `tools/scripts/proto_conflicts.py [dir]` prints every function that
still has local cross-unit declarations, the spellings in use and the units involved.

A remaining local declaration may disagree with an older `include/rr/*.h` prototype. Review the real
definition, all call slots, promotions and shared layouts together. A currently matching K&R spelling
does not establish the original declaration. Native harnesses must also use the real public interface.

Tried and rejected: respelling the `rr/` prototypes to the callers' K&R form (`tools/scripts/rr_respell.py`).
It gave 116 matching respellings in the EXE, but it broke `make check` in one unit and 72 host/overlay tests,
because those tests compile the `rr/` headers on the host against their own definitions. Those failures
require investigating both the candidate interface and the fixtures; they do not prove the declarations
cannot be unified. Do not run the tool without extending verification to the real native test suite.

Splitting the `rr/` headers (`tools/scripts/split_rr.py`): prototypes are moved from an `rr/` header into the header
of the unit that defines the function, and the `rr/` header includes it, so host tests and overlays see the same
declarations. In that pass, only 24 prototypes moved safely. A prototype stayed in `rr/` when (a) it disagreed with the
definition (return type or parameter count), (b) some unit declares the symbol differently, or (c) the unit that
defines the function includes that `rr/` header itself and would then see the unit-header spellings that suit the
callers. These are unresolved reconstruction constraints. Further cleanup must reconcile the interfaces and
native fixtures while preserving the relocated instruction match and complete EXE identity.

## Shared types and constants

`SceneRaceContext` now covers the complete `0x57C` PAL allocation cleared by
`SoundSystemInit`. Its resource tail names the camera-node root, workspaces,
light-preset array, sound-payload pointer and event directory. The constructor
clears `sizeof(RaceContext)` through the shared declaration, including complete
native pointer cells. `TrackCameraNodes` (`+0x550`), `TrackLightPresets` (`+0x558`),
`TrackEventTables` (`+0x568`) and `SceneOverlayEnabled` (`+0xD4`) are historical
PAL labels for fields of this owner; their active C consumers no longer declare
separate objects. The light-preset producer and consumer share the same typed
four-entry array, and the enable setter writes the same field as scene reset.

The same owner now names `playerCount14` and the full standings pointer at
`+0xE8`. `RaceStandings` is a historical PAL label for this pointer cell, not
a separate C global. Its four resident consumers use `RaceContext.standings`.
One `RaceStandingsTable` contains two player speeds, two distances from the
current car, the closest-player index, the overtake gap and eight standings
rows. This replaces the former distance-result prefix and rubber-band array
projection. The overtake caller indexes the actual row array instead of
advancing a native pointer by a hardcoded twelve-byte PAL stride.

`test_race_standings_owner.py` checks the actual standings producer followed by
four distance calculations in 1,024 scenarios on native C and original/linked
MIPS. Independent models check stable ranking, signed speeds, wrapped distances
and player aliases. Both complete standings buffers, thirteen vehicles, the
complete context, immutable inputs, guards, MIPS stores and register/stack ABI
are checked. There are no substituted services or production-body rewrites.
`test_car_overtake_owner.py` adds 8,192 native cases for the successful selected-row
lane-target branch, with controlled search/score boundaries that can change
the published standings root, selected entry and gap. This fixture does not
cover the algorithms behind those services or the fallback projection branch.

The E8 pointer uses the old native scalar/padding cell without moving the
inline camera: the context is `0x57C` on PAL and, with the typed grid/spec/lane cells,
`0x5C0` on LP64. Frozen
camera inputs retain their original tokens, schemas, PRNG streams and cases;
the test codec binds the E8 token to a guarded complete native standings owner
and serializes its saved logical token separately. It never reconstructs a
host pointer from token bits. The existing distance and front-search fixtures
also use the complete context and standings owners.

`RaceSoundConfig` names the four scalar configuration words at `+0x56C`:
SFX VAB selection, two car variants and the computer-car bank. Race setup,
the staged sound loader and slot startup use this same nested record.
`PendingTrackSfxVabId`, `ComSoundBankIndex` and `TrackSoundPayloadPtr` remain
historical PAL labels, with no separate active C declarations. The loaders
now publish the sound-payload and camera-track roots through full native
pointer members. `DualPlayerSoundEnabled` likewise names `SoundState.split10`,
and `SoundSeqVariantIndex` names `SoundState.sequenceVariant19CC`; queue
selection consumes the same field written by race setup. `SlotStart` indexes
the real slot array instead of casting an interior address to `AudioState`.

`rr/race_audio_resources.h` describes five actual wire-header layouts used by
track audio, trophy, attract audio and the menu-car loaders. Their signed
section offsets remain file-relative words; address addition retains the live
native base. The menu loader's `model560` assignment still belongs to the
legacy integer mesh API and needs the separate model/renderer migration.
These controller changes do not supply a native decoder for a raw PAL mesh,
VAB or TIM payload.

`test_race_audio_owner.py` executes nine actual C functions in 1,024 scenarios
on native C and original/linked MIPS, from race configuration through staged
bank loading, slot key-on and queue selection. Independent models check both
complete owners, selection/program tables, return values and every observed
service boundary. Controlled I/O and key-on services can change configuration,
loader roots, slot fields and queue variant; the caller must retain captured
arguments while re-reading later live fields. Native pointer cells, alignment
gaps, exterior guards, MIPS stores and stack/callee-save state are checked.
Only the queue's existing target register annotation is elided for native
compilation; no production calculation is rewritten.

`test_track_audio_resources.py` checks three actual resource controllers plus
the real alignment helper in 3,072 native cases. It covers pending/default
states, section arguments, live arena switches, full-width published pointers,
all surrounding state and read-only resource guards. Its relocation tables are
empty and its bank/image/reset services are controlled boundaries. The two
existing fixtures changed only their owner lvalues; their decision rules,
case generation and PRNG streams are preserved.

`rr/track_lookup_types.h` owns one `TrackEventDirectory`: three pair-table counts,
four threshold-table counts and seven references. The former `LookupTables16`
and `LookupTables32` described overlapping parts of this same record. Each
reference now has a signed-offset member before relocation and a typed pointer
member afterwards. Address relocation retains the native pointer width and
supports references to sections before and after the descriptor. The dispatcher
has a defined zero result for selectors outside 0..6, matching the original
MIPS range-check result; no warning suppression or compiler constraint is used.
The node search also imports the full shared camera record and a typed vehicle
parameter instead of a private `TrackEntry44` projection.

`test_track_resource_owner.py` executes the actual relocation, workspace setup,
node queries, all seven event queries, light-preset caller and scene setters in
2,048 scenarios on native C and original/linked MIPS. It covers all 256 light
selectors, negative/positive resource offsets, counts 0..8, circular node search,
signed keys and invalid selectors with an unpublished directory. There are no
opaque resource services or host-only body rewrites. Complete owners and full
resource/context/slot snapshots, native pointers above 4 GiB, exterior guards,
MIPS stores, stack and callee-save state are checked. This is a test of these
producers and consumers, not of importing an entire raw track file on a host.

`scene_context_wire.h` preserves the old camera trace format while binding its
opaque resource tokens to real native node, buffer and preset owners. Tokens
never become host pointer bits. New physical-tail guards follow `0x57C`; the
historical `0x564` trace prefix and all prior case matrices remain. Camera-track
and render-state fixtures generate their original frozen LP64 input records,
then decode them into the current owner. Compiled schema audits preserve every
input member offset and PRNG byte count. The render oracle moves its enable and
palette accesses to their actual scene fields; its decision rules remain.
The audio constructor fixture likewise checks a complete typed native context.

The resource and start-grid passes use full native pointers for their migrated
consumers. Other private `RaceContext` projections remain, including the
trigger-table consumer. These paths still need their own producer and consumer
migration; the shared owner does not yet establish one declaration for every
user of the context.

`RaceVehicle.worldPosition10` owns the complete 16-byte SDK world-position
vector at `0x10`, including its fourth reserved word. Vehicle/render consumers
share this word/vector union rather than treating three words and the following
slot as an unrelated private structure. The vehicle body-position copy now uses
the actual vector member. Layout checks for `worldPosition10` and `position20`
live with `RaceVehicle`; their duplicate declarations in `rr/race.h` are removed.

`CarLocalToWorld` accepts `RaceVehicle *`, `const Vec3s8 *` and `Vector *` through
one public declaration. All six call sites in the two camera TUs pass actual
packed-offset and full-vector members. The helper reads the vehicle's published
position after its matrix services, copies all four words, and adds the three
transformed components with explicit 32-bit wrapping. Its old private `V` copy
and integer-carried input pointer are removed. The camera caller's private `V4`
is also replaced by the common SDK vector. Their other `RaceContext` projections
remain separate reconstruction debt.

`test_car_local_world.py` executes the current body without host rewriting in
4,096 native scenarios and on original/linked MIPS. Six explicit matrix-service
adapters check argument identities, live angle reads, initialized rotation bytes,
input components and scalar-owner snapshots. All six output owners and three
input owners are exercised, including output equal to the vehicle's source
vector, all 64 service-mutation masks, signed limits and wrapping in both
directions. Native pointers stay above 4 GiB; unchanged owner tails, exterior
guards, stack bounds and callee-save state are checked. GTE matrix arithmetic is
outside this service-boundary test. The helper's pre-existing two spare matrix
words remain documented debt; removing them is not claimed by this cleanup.

`SceneRaceContext` now names the camera-follow prefix through `0x4C`.
`ComputeCameraNodeDist` imports this shared owner rather than declaring a private
48-byte view. `CarTrackProgressDelta` (`RaceContext + 0x40`) and
`CarTrackDirectionSign` (`RaceContext + 0x44`) are accessed as the same owner's
previous-distance and limit fields, including the final store after the look-ahead
service. Their historical PAL address labels remain in the symbol map. The
function uses a three-component delta and three-word look-ahead output, explicit
32-bit wrapping, a typed public vehicle/node interface and the resident SDK
square-root declaration. Its pre-existing second assembler name for `RaceContext`
remains a documented compiler constraint; this change does not retire it.

`test_track_camera_distance.py` checks 2,048 cases on the current native C and
original/linked MIPS. It compares the scalar context prefix, vehicle/node inputs,
service-entry traces and final stores, and checks the rest of the declared native
owners and exterior guards. Both explicit service adapters may replace live
scalar state; square-root and interpolation internals are outside this test.
Native pointers stay above 4 GiB. The one host normalization maps the existing
assembler alias to the same native object. Both outcomes of all six tested
predicates are required. Other `RaceContext` views and remaining resource aliases
still require a shared-owner migration; the prefix cleanup does not establish
one complete declaration for every consumer.

`game/car/carsp.h` owns one `CarTrackWork` shared by track search, wheel
contact and suspension. The old search view omitted the leading vehicle pointer:
on LP64 it therefore disagreed with the contact view's offsets. Both caller and
callee now use the same complete owner with a full native pointer. Layout
assertions account for that width. Packed NCLIP coordinates have explicit
halfword/word union views; the search no longer reads halfword fields through
cast word pointers. Four shared `Vector` positions replace private `T16` formats.
The `wheelState150` union separates track indices from suspension height offsets.

`rr/track_sample_types.h` owns `TrackRoadSample34`, the centre-line/surface
subrecord at byte 8 of `TrackSample3C`. The resource and scratch copies share this
exact type; private `Node`/`Z52` projections and their casts are removed. Known
coordinates, heights, widths and angles have semantic names. Uninterpreted fields
remain explicitly unknown. The translation and wheel-position stores also use
`CarTrackWork`, replacing the separate `ScratchCarPad` view whose fixed native
offsets could disagree with the pointer-bearing owner.

`UpdateWheelBumps` uses this owner instead of the private `T` view, whose
three-row declaration was indexed for all four wheels. Wheel state, velocity
and base use actual vehicle arrays instead of a vehicle pointer shifted by two
bytes. The low heading halfword uses integer conversion rather than pointer
punning. `test_wheel_bump_owner.py` checks 768 bounded-arithmetic scenarios against
original/linked MIPS, including four wheels, both height banks, successful and
declined triggers and landings. Trigger, impact and angle services are explicit
stateful adapters. The body's one register binding and four empty barriers remain
debt and are removed only for host compilation.

`test_track_road_format.py` runs the current contact definition through all four
subrecord copies, stopping at its first sine service boundary. A 512-case sweep
replaces track roots during explicit search adapters; another 256 cases execute
the actual axle search and model NCLIP on native C and original/linked MIPS.
The latter includes searches requiring zero through three retries. Complete
owners, all resource bytes, service/GTE boundaries and exterior guards are
checked with native pointers above 4 GiB. The actual search's three old register
annotations are removed only on the host. This test does not execute the later
rotation/contact arithmetic. Existing track consumers retain their test cases,
PRNG streams and oracles; only accesses to the shared record fields are migrated.

`psyq/libgpu/gpudisp.h` owns the area, offset, texture-window and drawing-mode
packet interfaces. Rectangle/window arguments share `GpuRectangleFields *`;
`SetTexWindow` no longer transports its window pointer through a signed word.
`rr/draw_environment.h` and the overlay area header import those declarations.
The particle caller also drops its conflicting integer window argument.

`SetupViewport` uses the complete `RroSceneScratchPrefix`, including real packet
and ordering pointers. Its seven rectangles per draw-buffer bank share the SDK
rectangle type. Passing the actual rectangle to `SetDrawArea` replaces the old
one-argument declaration/call, allowing both viewport register bindings to be
removed. The duplicated mirror predicate and address-named private structs are
removed. Mirror-coordinate additions retain explicit unsigned word wrapping.
The UI/sky callers use actual rectangle locals; `AddDrawAreaPrim` reads the two
`FrameStorage` display descriptors rather than a private `char[]` global view.

The overlay662 clip entry has one minimal public header. It accepts an aligned
word packet, full-word coordinates/dimensions and returns a full native byte
cursor. The constructor explicitly narrows its dimensions to the rectangle's
low16 fields. Former private byte-pointer/halfword declarations are removed;
all callers and service adapters use the same entry type. The original 458,752
geometry/link cases remain, and an additional 458,752 cases pass nonzero upper
dimension words. Two caller fixtures now compile the current SDK mode/window
helpers instead of frozen copies; their existing models and case streams remain.

The overlay662 area constructor uses the same resident declaration and the
native DMA address backend. Its framebuffer row offset is unsigned bank*256;
this preserves the original whole-word load without the old volatile cast,
under the unchanged compiler and assembler configuration.

`test_viewport_draw_area.py` executes the current viewport, area constructor,
coordinate encoders and DMA linker on complete native owners and original/linked
MIPS for 336 scenarios. Geometry/GTE setup calls are explicit mutable service
adapters. Calls, actual SDK helper results, owner snapshots, stack/callee-save
state and both CPU address aliases are checked. Native pointers remain above
4 GiB; the viewport needs no host-only source changes. The DMA linker's existing
single register annotation is removed only for host compilation.
`test_draw_area_primitive.py` checks 8,192 UI/SDK scenarios against an independent
byte oracle, including full-word coordinates, both framebuffer layouts, signed
limits, null/real texture windows and offset packets. The overlay fixture retains
its 458,752 cases, original PRNG and geometry/linking model; it now compiles the
current resident constructor/helpers and uses registered CPU-address transport.
`get_tw` now names its four encoded offset/mask fields and builds the command
through used horizontal/vertical intermediates. Its assignment-only `new_var`,
numeric local array and byte-pointer field casts are removed. All four local
fields participate in the returned command; their storage is not frame padding.
The mode, drawing-offset and mask builders also use semantic argument names
and fixed-width scalar types. Mode commands now return `u32`, matching their
encoded GPU-word consumers. The helper/environment declarations live in
`psyq/libgpu/gpudisp.h`; production consumers include that owner directly.
`rr/draw_environment.h` remains a compatibility include for older fixtures.
The two environment fixtures update only their mode service's return type and
remove the signed result cast; their cases, PRNG streams and byte/protocol models
are otherwise unchanged, including mutable owners and NULL windows.

`EmitPolyFT3` now uses the complete `GpuPolyFT3Packet` wire layout in `rr/gpu.h`
instead of the private `struct S32` byte array. XY, UV, color, CLUT and texture
page stores name their fields; the reserved final halfword remains untouched.
The emitting TU owns the declaration in `game/gfx/sprefct.h`, with read-only
four-byte coordinate/UV arrays and full-word material arguments narrowed inside
the emitter. `rr/textured_triangle.h` is a compatibility include. `DrawImageRow`
imports the same declaration and threads a full native scratch cursor through
the triangle and sprite helpers, replacing its integer return declaration and
integer scratch access. The pause-menu flat-triangle interface remains separate
debt, as described below.

`test_textured_triangle.py` extracts the current emitter and SDK setters. Its
262,144 original packet/ordering-alias cases, model and PRNG are retained;
another 262,144 cases pass nonzero upper material words. The image-fade fixture
updates its service signature without changing its mutations or case stream.
The sprite/page fixture also selects the current mode/window helpers, preserving
its original packet oracle and generator rather than editing frozen sources.

`AddSpriteWithDrawMode` shares `SpritePacket` rather than a private copy of the
same packet. Its ordering, cursor, coordinates, UVs and material parameters have
semantic names; packet casts occur at the word-link and draw-mode boundaries.
`test_image_row_pipeline.py` executes the actual image-row caller, both emitters,
SDK command setters, CLUT encoder and DMA linker in 2,048 native scenarios.
Complete frame banks, image rows, scratch and packet arena are compared against
an independent byte/link oracle, covering both banks, both CPU aliases, sprite
and triangle styles, no-op styles, signed geometry, shade and transparency.
The DMA backend registers the real native buffers above 4 GiB. All selected C
bodies execute literally, with layout assertions enabled. This is a memory and
command-stream test, not GPU hardware execution.

`TriggerWheelBump` indexes the vehicle's actual four-wheel arrays rather than
forming a shifted `RaceVehicle *`. Its lock update uses the declared halfword,
and event multiplication explicitly wraps as a PAL word before signed division.
`test_wheel_bump_trigger.py` compares 1,552 cases on native C and original/linked
MIPS, including all four wheels, the input threshold, absent events, signed/full
word event values and a lookup service that changes live input. Complete vehicle
prefixes, service snapshots, exterior guards and saved registers are checked;
the native owner remains above 4 GiB and the body needs no host rewrites. The
lookup implementation itself is an instrumented service in this fixture.

`game/gfx/spremit.h` now owns `GameQueueSprite`. It uses the shared unsigned
CLUT `SpritePacket` layout and full native packet/ordering pointers; the former
signed-CLUT view and address-like local names are removed from this emitter.
Its pause and HUD callers agree with the public sprite declaration. The pause
renderer threads a real scratch cursor and uses shared `Vec2s` corners with
named loop/geometry locals. Its triangle return declaration now preserves a
pointer, but the old aggregate-coordinate call contract remains unresolved:
the current emitter takes three scalar words, while this caller passes pairs
of shorts. Do not hide that difference with aliases or ABI wrappers. Native
execution of the complete pause renderer is not established by these changes.

`game/gfx/scene1.h` owns the scene initializer and both state setters, including
the enable word's declaration. Color channels arrive as complete argument
words and narrow only at the three stored bytes. `TrackLightPreset8` and its
palette pointer belong to `game/gfx/dispenv.h`, replacing the caller's private
record. `ApplyTrackLightPreset` decodes its selector once, uses an ordinary
signed comparison value and no longer needs its empty-asm barrier. Its ignored
second enable argument and redundant loop-counter assignments are removed.

The title, opening and camera consumers now import the defining headers for
view setup, CD playback/fade, opening glyph/icon emission, track interpolation
and font strings. The opening's icon fade retains its explicit byte conversion;
font input uses the existing encoded-byte interface. The chase-camera caller
keeps its signed-halfword angle interpretation explicit with the shared
word-returning declaration. `ResetCarSlotBuffers` has one no-argument public
declaration in `game/data/resinit.h`; its callers drop ignored arguments, and
the former single-iteration reset loop is a direct call.

`test_render_state_owners.py` executes the actual state setters, light-preset
caller and slot reset on complete `SceneRaceContext` and 16 native slot owners.
An independent byte oracle checks 8,192 scenarios, all 256 light selectors,
full-word colors/modes, both enable outcomes, every slot and exterior guards.
Layout assertions remain enabled, pointers stay above 4 GiB, and no production
body needs host rewriting. The packet-emitter fixture now extracts the current
sprite body; its cases, service mutations and protocol model are retained.
The two HUD service fixtures change only their sprite packet type spelling.
The player/reverse-heading fixture's reset adapter now has the actual
no-argument signature. Its historical flag payload comes from the selected
vehicle's already-published flags, and an additional MIPS assertion checks the
incoming register against that same owner. The event layout, all prior
assertions, case matrices and PRNG streams are retained.

`psyq/libsnd/vab_format.h` owns the complete VAB header, program and tone
formats. `VagAtr` has named tone attributes and the original reserved fields;
private `Tone`, `SpuTone20` and `SpuTone32` projections have been removed from
the voice consumers. Resource cursors use `VagAtrBuffer *` and derive field
positions from this format, retaining byte-addressable aliases and halfword
alignment without converting native pointers to PAL integers. Getter/setter
copy the fourteen byte attributes and four halfwords, preserving offsets 14/15
and the reserved tail. Each byte observes the live cursor; the halfwords share
one explicitly captured cursor. They do not require a complete 32-byte object
at the 24-byte transfer buffer.

The tone copy fixtures execute the current definitions with the complete
`SvmCur` owner. Their original exhaustive cases, overlap models and pointer-cell
aliases remain. The SDK integration fixture additionally runs the real parser,
transfer, selector, tone copy and pitch helpers on original/linked MIPS and
native allocations above 4 GiB, checking complete resources and owner guards.
`note2pitch2` declares the promoted word arguments and explicitly uses their
signed low halfwords; its cursor retains the native pointer width. Pitch
integration exercises MIDI note/fine values 0..127; the older variable-shift
expressions still require valid C shift counts and do not establish arbitrary
raw-note support. Existing voice-allocation pins/pages remain reconstruction
debt. The controller imports the public tone-copy declarations and explicitly
normalizes its tone numbers to signed halfwords.

NRPN handlers now share `SvmToneControl`: three signed-halfword selectors,
one complete `VagAtr` by value, the selected attribute word and one data byte.
The dispatcher actually passes both trailing arguments. The previous 36-byte
`LibBlock24`/`LibBlock24Fields` carriers absorbed the attribute word into a
fake tone tail; five reverb handlers instead reached for the data byte through
another parameter's stack address. Both reconstructions are removed. All 20
handlers and their public declarations use the same concrete signature. The
mode handler returns `void`, matching its callers' use of the callback.

`SpuAdsrParameters` names the nine decoded fields (18 bytes). Resolve/build and
their callers share this type and explicit prototypes. The former 12-halfword
locals had three unused elements; the real object retains the match without
adding padding. Combining the attack-mode/rate expression also retires one
of the builder's three register bindings. Two bindings remain debt.

`test_spu_tone_control.py` compiles the current handlers, selector, tone copies,
dispatcher and delta decoder with complete native owners and pointers above
4 GiB, and compares them with original/linked MIPS. It exercises every data byte,
raw transport words, a typed 20-entry callback table, complete resources/guards,
all 65536 ADSR words on the native codec, and output aliases. The original SDK
fixture cases and seeds are inherited unchanged. The runner invokes this
indirect `TestCase` subclass through its explicit `unittest.main()` entry.

Known original defects remain explicit: handlers 11..14 do not initialize all
ADSR fields before calling the builder; the dispatcher passes an indeterminate
incoming tone scratch value. Native compiler warnings remain visible in the
new fixture's log. Tests of 11..14 observe only the fields actually initialized
at the builder boundary, then supply deterministic builder outputs. They do
not establish a fully defined native chain for those four handlers. Real
resolve/build execution is checked separately and on the fully initialized
4..10 paths. Dispatcher tests cover active NRPN banks, RPN tone counts 0/1/16/255,
loop-start priority and idle decode. The real getter fills every consumed tone
field. The fixture sets the program-table limit explicitly, exercising both
successful selection and the deliberately ignored selection failures.
`SsSeqOpen`'s mixed callback-table `void **` view still requires unification.

`cc_6.c` imports the actual public getter/setter interfaces. Its entry point
accepts three promoted words, matching `LibControllerCallback`, and explicitly
normalizes bank/row to signed halfwords and data to a byte. Its verified stock
GCC 970318 profile disables CSE block skipping (`-fno-cse-skip-blocks`), preserving
the original allocation with the correct argument types and signed-halfword
tone transport. This is a matching reconstruction profile, not evidence that
the original build used that exact option. No incompatible local prototypes
or new register bindings are used to retain the match.

Coordinate sums/differences and packed status shifts use unsigned 32-bit operands
before the operation where PAL wrapping is observable. Signed division and
arithmetic right shifts explicitly receive the wrapped signed result. Native
fixtures retain their extreme-word cases. An already-unsigned subtraction by
`1U` and the bounded texture remainder `y & 255` need no redundant conversion.

Word arithmetic that wraps on PAL needs unsigned operands before the operation
in C. Converting only the result does not prevent signed overflow or an invalid
left shift. The native fixtures exercise extreme values as well as normal game
inputs; removing these conversions can preserve every MIPS instruction while
making the C execution undefined. Already-unsigned expressions need no redundant
conversion. `SetVoiceVolumeScaled` expresses its two products as `u32` values
with a `u32` scale, then performs the logical shift and signed-halfword transport.

`SoundSequenceState` is the real forty-byte owner at `AudioState.sequence19D0`.
Enqueue, stereo playback and clearing share its active flag, count, read index,
first-program interruption flag, left/right volumes and eight program slots.
The word at relative +0x0C is cleared with the read index; its meaning remains
unknown. Native/PAL size and field-offset assertions preserve the original
0x19D0..0x19F8 layout without legacy aliases or additional padding.

`SoundQueueEnqueue` indexes that actual program array. A named reset helper
replaces the side-effect conditional. All addresses derive from `SoundState`;
the physical page constant, empty asm and one old register binding are removed.
The native byte-pointer result preserves the existing base-plus-count ABI,
which all known callers ignore. One previous return-register binding remains
reconstruction debt. `game/sound/sndque.h` owns the one public declaration;
resident and overlay callers import it instead of declaring different return
types locally. The runtime/context headers forward that declaration.

`test_sequence_queue.py` compiles the current enqueue body and its actual reset
helper. Its full-page oracle, cases, inputs and assertions are preserved after
exact field-path renames. The obsolete physical-page substitution and empty-asm
erasure are removed; no fake shifted `AudioState` object or low-address mapping
is needed. The historical `SoundQueueEnqueue` view receives only the same field-path
adapter. The queue consumer's old register binding remains reconstruction debt.

`SpuGetKeyStatus(s32 voiceMask)` has one public argument, matching the actual
register inputs used by its body. `SoundQueueUpdate` imports that prototype and
passes only the voice mask. The SDK body uses a structured voice scan and
separate mask, voice pointer, software key state and volatile ENVX value.
An explicit MMIO byte-address conversion through `unsigned long` remains a
reconstruction boundary; a size assertion prevents native pointer truncation.
It is not evidence that the original source used integer pointer arithmetic.
`test_spu_key_status.py` checks all ENVX values, all voices and exact MIPS reads
against original and linked code, using complete native register banks above
4 GiB. `test_sound_queue_playback.py` exercises the actual consumer and SDK
together, including callback changes to the live bank/index and captured
program/volumes. The existing consumer register annotation alone is omitted
when compiling that consumer for the host; the SDK body is used unchanged.

SPU initialization and key commands share the named register fields in
`SpuRegisterBank`: main/reverb volumes, KON/KOF, PMON/NON/EON, transfer control
and the 24 voice records. Their halfword offsets follow the
[SPU register map](https://psx-spx.consoledev.net/soundprocessingunitspu/)
and are asserted in the header; the complete storage owner remains 512 bytes.
`SpuSetKey` retains the hardware fields' `volatile` qualification in both direct
branches. Its previous key-on branch assigned a volatile halfword pointer to an
ordinary pointer. No new compiler constraints are used to retain the match.
`test_spu_initialization.py` and `test_spu_key.py` now extract the current
definitions instead of frozen projections. Their original models, cases,
inputs and assertions are preserved. The reset test retains its existing
device-status injection at the STATUS macro and also executes the direct
volatile-read variant.

The body, spark, glow-fan and shadow native harnesses also link the real GPU address
backend. Their complete native buffers are registered at CPU address
`0x80180000`; no software pointer is shortened or rebuilt from address bits.
The old `0x81000000 | nativeLow24` fixture address was outside the backend's
24-bit physical DMA domain. Case seeds, numerical models, alias offsets and
assertions are retained; the body and shadow oracles' link fields use their independent
owner-to-CPU-address adapter instead of native pointer bits. This address
transport adapter is separate from the production encoder under test.

Overlay 662's packed image rotation uses a real `u32 *` destination cursor.
The eight-word row advance occurs only when another row will be written, so
the final unused advance cannot form a pointer beyond the 512-word image.
The original pixel/service-order model and its complete-owner assertions are
unchanged; the relocated instruction sequence still matches.

Several units used to carry private copies of the same struct. Where one view is a superset of the others, or the
definitions are byte-identical, the definition lives in one header that every user includes:

* `include/rr/race_types.h` – `RrRaceVehicle`, its real `RrRaceVehicleControl` owner,
  shared vectors and steering/player state; `game/car/car.h`
  and `game/course/control.h` are forwarding headers.
* `include/game/car/carsp.h` – scratchpad work area of the car physics (`SP`, `SP_1`, `T16_1`, `Z52`)
* `include/game/car/grid.h`, `include/game/course/control.h`, `include/game/race/objrec.h`,
  `include/game/data/savetype.h`, `include/game/system/modeslot.h`
* `include/psyq/libgte/libgte.h`, `libspu/libspu.h`, `libsnd/libsnd.h` – PsyQ types (`VECTOR`, `SVECTOR`, `Spu*`, `VagAtr`, `ProgAtr`),
  named after the SDK headers; `tools/scripts/psyq_types.py` moves further byte-identical ones
* `include/scratchpad.h` – `SCRATCHPAD`, `SCRATCH_ROT_MATRIX`, `SCRATCH_CAR_WORK` instead of bare `0x1F8xxxxx`

`__adddf3` imports the shared declarations from `rr/softfloat.h`. Its signed
shift helper returns `Wide64` by value, matching the actual `_dbl_shift`
definition. Caller-side result-pointer declarations and their artificial
destination aliases are removed. Mantissas, exponent, sign and rounding values
replace stack-offset variable names. Exponent serialization shifts an unsigned
word, preserving PAL wrapping when cancellation produces a negative exponent.
The existing register bindings and empty barriers remain reconstruction debt;
this change adds none. Other software-arithmetic callers still need interface
reconciliation.

`test_softfloat_addition.py` compiles the current addition and its actual
addition/negation/shift helpers against public headers. An independent integer
oracle covers 590,160 native cases under UBSan, including all encoded exponents
and signs, alignment gates, cancellation, rounding carry, overflow diagnostics
and negative exponent serialization. Another 194 flows execute the original
MIPS helpers and check native results, error calls, stack and saved-register
guards. These preserve the original encoding semantics for NaNs and subnormals;
they do not substitute host floating-point arithmetic.

The PAL vehicle layout is 0x320 bytes, supported by the stride of `CarTable` in the running game.
Physics, race logic, drawing, AI drive/RPM and checkpoint handlers now use `RrRaceVehicle`.
The old `Car`, `Control`, checkpoint prefix and GNU aliasable drive views have been removed.
The embedded control owner begins at +0x1F4 on PAL and holds real pointers to the selected
car record (+0x18) and its handling data (+0x1C). `InitPlayerCar` selects handling at
record+0x1C8. The known handling prefix describes steering parameters at +8, +A
and +1C and the slide threshold at +16. Car records share seven 0x3C gear slots
beginning at +4; their highest active index is read live from the leading word.
`rr/car_record_types.h` owns the full `RrCarRecord` layout: all 640 PAL car
resources carry 0x218 bytes between their record and bounds offsets. Named fields
include two target-frame counts at1E8, top speed at1F0, stage at1F3 and a ten-byte
terminated name at1F5. Unidentified parts retain reserved byte spans; the original
field names and the meaning of every reserved field have not been recovered.
`game/data/bankrec.h` declares the one `RrCarRecord *CarRecordPtrs[]` table and the
typed `SetCarRecordPtr` interface. Loading, physics, initialization, menu labels
and result gating share them; the old scalar/int/char/local-prefix declarations
have been removed. The loader's encoded resource relocation still needs cleanup.

`InitPlayerCar` now initializes the actual vehicle/control owner and the selected
record through these types. It clears `sizeof(*vehicle)`, retains both full native
pointers and calls the grid-placement function with its two consumed arguments.
The control region at +94 explicitly overlays player dynamics (slide threshold,
top-gear speed and motion state) and AI steering. The initializer preserves the
MIPS wrapped product when deriving gear rates without a signed-overflow or
negative-left-shift dependency.

Word/halfword views that the executable really uses have explicit unions: engine state and
RPM, the steering target, a preview sample and contact bytes. Position copies include the
fourth reserved word, while the coordinate array has three elements. The old `Q4` copy type
is replaced by the shared vector type.

Native 64-bit vehicles retain both full pointers and their natural alignment (currently 816
bytes). Host tests use their actual stride/offsets. `vehicle_wire.h` and `vehicle_native.py`
are test-only PAL serializers: console addresses resolve to real native record/handling
owners, and serialized bytes are compared with original MIPS at its fixed 800-byte stride.
The fixtures check inserted native padding and outer guards separately. Overlay661 now
indexes the actual vehicle array rather than spelling the 800-byte multiplication manually;
its camera branches retain the match with one fewer register binding and two fewer gotos.
The migrated track projection, lane target/overtaking, lap timing and rubber-band
specification paths now take `&vehicle->control`; they no longer use the PAL-only
1F4 offset to address a pointer-bearing native control record. Other private
vehicle views and raw-offset initializers still need migration.

`rr/vehicle_table.h` declares the single eight-slot `CarTable` and `CarSnapshots`
pointer tables. Race initialization, ranking, collisions, replay copies and the
minimap use complete `RrRaceVehicle` owners. Replay capture copies the whole native
owner, including both full record/handling pointers. `InitRaceCars` uses pointer
slots and `sizeof` the real player vehicle when clearing it. The PAL address
`D_800AC3FC` aliases `PlayerCars[1].flags1E4`; initialization now writes that field
instead of declaring independent storage. The shared vehicle also names the four
wheel positions, four planar collision corners, position histories and collision
flags used by these consumers. These changes do not migrate every player-physics
view or remove the remaining register bindings in the race TUs.

`rr/replay_types.h` describes the single-player, two-player and link work layouts:
packed four-byte input frames followed by complete vehicles. PAL offsets and the
maximum 0x60E8 payload are asserted; native offsets/strides use actual pointer
alignment. `rr/work_arena.h` declares the shared `SharedWorkArena` backing as bytes,
because loaded assets, menu rows and replay storage occupy it at different times.
Menu initializers select their row view explicitly. Bank setup derives payload
offsets and lengths from the actual layouts, but still contains legacy integer
address arithmetic, a hardcoded count-page view and compiler barriers.

`test_race_vehicle_tables.py` selects the real current `InitRaceCars` and
`SaveCarSnapshots` bodies. It compares 4096 original-MIPS/native cases, complete
owners and service-entry traces, with mutable table roots, live counts, self and
chained snapshot copies, guards and ABI checks. Native-only cases also cover a
snapshot destination aliasing the live count. The initializer test models the
real second-player flag alias. The minimap fixture serializes complete native
vehicles and checks all original 4096 MIPS/service-trace cases. These are direct
checks of those bodies, not evidence that every race/input function is native-safe.

The result handler uses the canonical `RrMenuSettings2C`: per-stage results at0,
phase at17, current result at19 and mode at1A. `MenuSubmodePendingId` is the existing alias
of `SelectionState.phase17`, not independent storage. Its old local prefix and
reinterpretation macro are gone from `raceai2.c`; other callers of the address
alias remain to be unified. Layout assertions live beside the shared type.

The car renderer now uses `RrRaceVehicle` for preview, player and opponent drawing, instead of
three local partial vehicle structs. Model dimensions have one `CarBounds` definition in
`include/game/car/bounds.h`, shared by the renderer and bounds setup. `RrVector` describes the
body position copy. The target-node word also has a signed-half view because rendering reads its
high half for an alternate drag response. Public drawing prototypes preserve all argument slots
used by callers, including ignored preview color/flags and player view mode.

Camera dispatch, initialization, stepping and marker rendering share `rr/presentation_types.h`.
The initializer uses typed key-table rows (three linear keys or two cyclic keys), and model
rendering shares `rr/lighting_types.h` with the overlays. Neither module needs a byte-array
alias of `RaceContext`; the initializer's old volatile integer pointer reads are gone.
One pre-existing object-pointer register binding still remains.

`RrPresentationController` now owns the camera, panel, scroll and moving-model tracks together.
`MotionController` at `0x800ACA28` aliases `RaceContext` (`0x800AC650`) plus `0x3D8`; these modules
access that real pointer field as `RaceContext.controller3D8`. `RrPresentationContextPrefix`
contains the inline controller at `+0x250`, its pointer slot and the model handle at `+0x564`.
Initialization clears `sizeof` the actual owner. All four stepping APIs share `RrMotionClock20`;
camera arithmetic explicitly reads its timing word as signed, while scrolling/panel timing uses
the unsigned interpretation. Native tests retain real pointers and derive scroll offsets from C.
Other parts of `RaceContext` still have local views and remain to be reconciled.

`SetGteObjectTransform` has one public interface in `game/camera/viewcam.h`.
Its output is the shared `ObjectTransform38`, used by the resident renderers
and overlay665. It names the relative short vector, rotated word vector and
full matrix whose translation begins at output+2C. `MatrixWords32` now exposes
the actual nine rotation halfwords and three translation words alongside its
eight-word hardware view. The car renderer and model renderer use it instead
of private `Mat8`/`MATRIX` copies. Course-object, menu-grid and lighting preparation
also use it instead of private matrix structs or padded rotation arrays. Other
matrix service declarations remain to be reconciled; these shared records do not
by themselves unify the whole libgte API.
`DrawRroModel` also accepts the real vector and matrix pointers instead of two
integer addresses. `rr/rro_model.h` now owns its `RroModelScratch` and the shared
`RroRenderWork` passed to all three RRO emitters. `RroSceneScratchPrefix`, shared
with the effect renderer, holds the canonical camera prefix and matrix, real
command-table/vertex/normal pointers and renderer settings. A tail union receives
the complete object transform first, then the emission context replaces it.
Native pointer cells retain their
full width and natural alignment; the PAL compiler checks the original offsets.
The dispatcher traverses the selected command stream through a real pointer,
uses the typed ordering table and passes the same typed context to each emitter.
The unknown-command format is an actual character array, not an integer whose
address is passed as text. All three emitters stage vertices, projected XY
pairs, areas and depth through the named context fields, retaining full native
pointer width. Their register constraints and mixed numeric address/value
lifetimes remain reconstruction debt. Other rendering kernels and shared frame
interfaces still need migration.

`test_rro_model_dispatch.py` extracts the current dispatcher body with its real
headers. Its 8192 cases compare native execution with original or compiled MIPS:
all command kinds, signed counts, four object selections, full native pointers,
texture and scratch-root replacement during transform, emitter-returned command
cursors, later root replacement, error returns, GTE RGB writes, service protocol,
complete native owners, ABI and guards under UBSan/bounds. Transform and emitter
services are instrumented; this test does not execute the primitive emitters or
validate GTE lighting/projection numerics.

`rr/rro_effects.h` owns the effect workspace's known pointer/scalar fields
and complete scene/work owner. `rr/car_render_types.h` supplies its shared
projected XY/depth records. PAL assertions keep the
work at scratchpad+7C and the complete owner within the 1 KiB hardware scratchpad;
native offsets follow the full pointer cells. `RroProjectionWorkspace` starts
at work+188 and gives the body pipeline one actual array of 63 XY/depth pairs:
three initial cursor positions followed by sixty possible results. Its cursor
advances before each triple store, staying in that array. The overlaid fan view
starts at work+198 and supplies its 21-word array, including the two initial
cursor words and unused final projection; trails use XY/depth pairs at the
centre's address. Shadow XY words have another view of the same owner.
The PAL circle table contains seventeen positions, including its repeated closing
point; the sixth triple load also reads the adjacent geometry word. That last
projection is stored but does not become a fan triangle.

`DrawGlowFan` takes the canonical car-slot and effect-work owners, retains full
packet/ordering pointers, builds typed 32-byte E1/POLY_G3 packets and packs color
through unsigned arithmetic. Its one remaining integer address calculation uses
the full pointer width; an ordinary pointer spelling changed instruction
selection and remains cleanup work. Actual GTE transfers are named hardware
adapters. The enclosing renderer now includes the canonical `CarSlotBuffers`
declaration and selects slots by their real array stride, instead of a conflicting
byte-array declaration. Private projected-vertex and short-vector copy types use
the shared types, preserving their original alignment. The primitive kernels
and wheel/trail renderer still contain raw accesses, integer address temporaries
and compiler constraints.

`test_rro_glow_fan.py` extracts the current body and compiles its real public
headers under UBSan/bounds. Its 4096 original-or-compiled MIPS/native cases compare
every GTE transfer, complete GPU buffers, native owner write extents, signed
depth/clipping boundaries, full-width color packing, hardware-adapter root/color
mutations, ordering-head/packet aliases, DMA low24 links, guards and ABI. The GTE
model scripts projection results; it does not establish numerical GTE accuracy,
flags/timing or native correctness of the enclosing wheel/trail renderer.

`rr/car_render_types.h` also defines the complete observed `CarModelHeader`,
its vertex/count/command mesh records, sixteen-entry `CarSlotBuffers` array
element, trail pointer/state pairs and sixteen-byte spark records. Model,
vertex, command, trail and spark pointers retain their full native width;
PAL assertions check the original pointer-dependent sizes and offsets.
The car allocator selects four actual slots, captures the two vertex counts,
allocates the three trail rings through a byte cursor and publishes the real
pointer. Spark allocation advances the fresh global cursor after each clear
service, which may replace that cursor. Reset and the track-lighting preset
select named fields through the slot array, without assuming its PAL stride.

`LoadCursor` has one `void *` declaration in `rr/load_cursor.h`, shared by
resident and overlay users. Consumers select the required byte or payload view;
the car, scene-chunk and frame loaders publish pointers without truncating
them to integers. This does not make every resource loader native-safe:
legacy relocation paths still operate on encoded 32-bit PAL address words,
and other raw payload/scratch views remain. The CD request entry points and
their shared destination root also transport real buffer pointers. Sector
offset addition and loaded-byte calculation express the original unsigned
32-bit wrapping explicitly.

`TransformRroVertices` takes the canonical slot and effect workspace, reads
the actual vehicle, walks the six-color table and output array, dispatches
sparks through their pointer, and selects the frame's body mesh and shadow mesh
from the real model header. Translation jitter wraps as unsigned 32-bit
arithmetic. The spark/body/shadow kernels now share typed public interfaces;
their internal legacy addressing and register constraints still need migration.

Three tests extract the current C bodies with the real public headers and
compare native execution with original or compiled MIPS under UBSan/bounds:
`test_car_render_allocation.py` covers 8192 allocation/reset cases, all valid
four-slot groups, complete native buffers and callback-time cursor replacement;
`test_cd_file_requests.py` covers 6912 request/finish cases, full-width or null
destinations, busy states, mutable conversion services and wrapped counts;
`test_car_effect_preparation.py` covers 4096 preparation cases, complete native
owners, mutable model/vehicle/spark roots, frame selection, clamp/speed, extreme
jitter, six color transfers and exact service arguments. These tests also check
the MIPS calling convention and write extents. The preparation test instruments
the three render kernels and scripts GTE colors; it does not validate those
kernels' native internals, numerical GTE behavior or the entire wheel renderer.

The shadow emitter reads the observed `RroTexturedQuadPrefix` command prefix and projects
through complete native command/vertex pointers. Its four packed XY results
have their own view in `RroProjectionWorkspace`; workspace depth, areas, winding
and palette reads select named fields. `GpuPolyFT4` provides word and byte
views of the same complete forty-byte hardware packet. One of the emitter's
two register bindings has been removed. Its legacy ordering-table calculation
now retains full native address width; signed bias multiplication avoids
shifting negative values. The table link is stored as a 32-bit hardware word,
then its high byte is cleared, preserving the DMA low24 protocol without a
native pointer-sized store into the table. One scalar register binding and
numeric address arithmetic remain reconstruction debt; an ordinary pointer
spelling has not yet retained the matching instruction selection. The command
prefix's size is not evidence for a complete command's stream stride; its
unobserved tail is not reconstructed.

`test_rro_shadow_projection.py` extracts this current body and compares 4096
native/original-or-compiled MIPS cases under UBSan/bounds. It exercises the
nonpositive-depth, far-depth and backface returns, all four full-width vertex
addresses, live fourth-index reads after a GTE store, captured command/vertex
roots, exact transfer/store traces, complete native owners and the MIPS ABI.
Every native case exits before packet emission. This test excludes the
ordering-table path and numerical GTE behavior.

`test_rro_shadow_emission.py` separately runs the current complete emitter in
8192 native/original-or-compiled MIPS cases under UBSan/bounds and strict native
compiler warnings. It retains the projection test and adds packet emission,
all 256 signed byte biases, all 32 valid depth shifts, signed UV boundary words,
live fourth index reads, full-width roots/cursors and all ten word-aligned packet/head
overlaps. An independent wire protocol checks complete native owners and both
GPU arenas, including alias-sensitive tag/color/XY/UV writes and cursor
advancement; exact GTE transfer traces, MIPS write extents and ABI are also
checked. GTE arithmetic is scripted, and the command/work owners are separate
from the GPU arenas as in the renderer. This does not validate the other car
effect emitters, the entire wheel renderer, numerical GTE behavior or timing.

`EmitCarBodyQuads` stages three real `Vec3s8` records and projects through the
shared workspace array. Its input header has explicit command/count halfwords,
and `RroBodyQuad` describes the observed twenty-byte vertex/color-index stride.
`RroBodyPacket` is a complete forty-byte E1/POLY_G4 owner. Model, command, scene,
input, ordering and packet addresses retain full native width; projected and
palette byte views select offsets derived from the actual C layout. Depth
accumulation wraps at 32 bits and DMA links use word stores, with the tag's high
byte cleared. Two of the six register bindings have been removed. Four bindings,
empty barriers, generic temporary lifetimes and full-width numeric address
calculations remain reconstruction debt.

`test_rro_body_emission.py` extracts the current complete body and its real
public types. Its 4096 native/original-or-compiled MIPS cases cover vertex counts
0..60, quad counts 0..8, all 28 valid depth-shift settings before the emitter's
additional four-bit shift, both ordering biases, and all ten word-aligned
packet/head overlaps. The input backing includes the extra triple read by the
original final prefetch; those words are not additional emitted vertices.
Scripted GTE operations replace model roots after capture, alter the pending
prefetch data and switch the command root before it is read. Later clip hooks
change indices, palette entries, header counts and public GPU roots, checking
which values must be captured and which must be read live. An independent wire
oracle verifies packet construction, linked-list aliases and cursor advancement;
complete native owners, both GPU arenas, exact GTE transfer traces, MIPS write
extents and ABI are checked under UBSan/bounds and strict compiler warnings.
GTE numerical behavior and timing are not modeled, command/work storage is
separate from GPU storage, and this does not validate the spark kernel or entire
wheel/trail renderer.

`DrawSparkLines` uses the actual sixteen-byte `RroSpark` record. Its motion
union names position and all three velocity halfwords, while retaining the three
packed words used by the spawn stores. Cached ordering, shift, draw mode, spawn
gate, launch velocity and input pointer have their own local fields. The scene,
work, input and twenty-byte `RroSparkPacket` owners retain full native pointers;
DMA links remain hardware words. Motion shifts, timer subtraction and launch
speed express the original 32-bit wrapping. One empty barrier and an unrelated
asm input dependency have been removed. Seven register bindings, three empty
barriers and mixed full-width address/scalar temporaries remain debt.

`test_rro_spark_emission.py` compares 4096 complete native/original-or-compiled
MIPS executions with a structured wire oracle. It covers both spark banks,
spawn gating and random-service mutations, signed motion/timer/speed boundaries,
all thirty valid cached shift settings, captured GPU roots, five packet/head
word aliases and color writes to a packet subsequently rejected by depth. The
actual public owners, local pointer cells, complete GPU arenas, guards, GTE
transfer traces and MIPS ABI are checked under UBSan/bounds and strict native
warnings. Projection and color math are scripted; the test does not validate
numerical GTE accuracy or the enclosing wheel/trail renderer.

`rr/track_object_render.h` describes the textured track-object renderer's
thirty-two-byte quad records, translation prefetch, depth, flashing flag and
four packed projected positions. `DrawTrackObjectModel` shares the real
`RroSceneScratchPrefix`, `RroEffectMesh` table entries, vectors and forty-byte
`GpuPolyFT4` packets. Native table strides and workspace offsets follow the
actual types; scene, model, vertex, command, packet and ordering addresses retain
full width. Flash-frame addition and signed ordering bias use defined unsigned
wrapping. Five register bindings, ten empty barriers and the three volatile
depth reads have been removed. Six bindings, sixteen empty barriers and mixed
numeric address/scalar lifetimes still need reconstruction. The lit companion and its triangle emitter share a separate typed workspace,
described below.

`test_track_object_model.py` compares 8192 complete native/original-or-compiled
MIPS cases against an independent wire oracle. It exercises four model entries,
zero through six quads, the initial and final extra translation prefetch,
all thirty valid scene shifts, nonpositive/far depth, screen clipping and
partially written rejected packets. Emitted packets cover all signed byte
biases, signed UV high halves, frame overflow, flashing phases and all ten
word-aligned packet/head overlaps. Complete native owners, both GPU arenas,
GTE transfer/control traces, write extents and the MIPS ABI are checked under
UBSan/bounds and strict compiler warnings. GTE math is scripted; geometry,
command and workspace owners are separate from GPU storage, and hardware
flags/timing and the other track-object renderers are outside this test.

`DrawTrackObjectLitModel` and `EmitModelTrisGT3` share `TrackObjectLitWork`:
a rotation matrix, prefetched short-vector input, full vertex/command pointers
and raw GTE depth. The model table uses `RroEffectMesh` strides, placement
records have their real eight-byte extent and the twenty-byte triangle records
name packed indices and three color words. Both public interfaces transport
actual pointers, including the returned GPU cursor. `GpuPolyG3` describes each
twenty-eight-byte Gouraud triangle; `GpuCommandPacket` describes the following
eight-byte draw-mode packet. The emitter still executes its first triangle when
the encoded count is zero, as the original do-loop does. Rejected triangles
leave partial color/XY writes before that final draw-mode packet is written.

Four of the lit caller's five register bindings and eight of its twelve empty
barriers have been removed. Its matrix argument is the real workspace field,
replacing the old scratchpad-address OR. Two false asm output dependencies were
retired while retaining their existing input uses, allowing the compiler to
reuse the known address. One caller binding, four barriers, full-width numeric
address arithmetic and a reused scene-address/shift temporary remain debt.
The triangle emitter retains one counter binding and no empty barriers. GPU
links use 32-bit hardware stores; signed XY extraction explicitly wraps the
left shifts. Ordinary short casts have not yet retained the instruction match.

`psyq/libgte/rotmatz.h` declares the real `RotMatrixZ` interface: a matrix pointer
returned unchanged. Its two callers now use `MatrixWords32`, including the sky's
stack matrix. The PAL label `GteProjectionScaleTable` aliases
`CameraViewMatrix.fields.translation14`, rather than independent storage.
The billboard reads that field directly. The lit caller still needs the named
symbol view to preserve its address-load instructions; its header documents the
alias, and native tests use the actual shared owner.

`test_track_object_lit_pipeline.py` runs both current rendering bodies together
in 8192 native/original-or-compiled MIPS cases, with strict native warnings and
UBSan/bounds. A separate wire oracle checks complete native owners, the initial
matrix/service entry, zero through four placements, zero through five encoded
triangles, all 32 valid shifts, both depth limits and initial/final translation
prefetches. Rotation-service mutations replace public model and GPU roots,
counts, camera/translation storage and input padding. The checks distinguish
captured geometry from live placement/scene data. Emission covers all signed
byte ordering biases, seven triangle/head word overlaps, both draw-mode/head
word overlaps and partial rejected packets. Exact GTE transfers, complete GPU
arenas, callee-saved registers and return/stack state are checked. Rotation and
GTE arithmetic are scripted; numerical accuracy, flags/timing, other renderers
and geometry/workspace aliases with GPU storage are outside this test.

`DrawTrackObjectBillboard` and `DrawTrackObjectFlashQuads` now use complete
scene/work owners, `RroEffectMesh` table strides, real vertex pointers and
`GpuPolyFT4` packets. Four private record/workspace definitions have been
removed. Billboard quads retain their thirty-two-byte record with translation
prefetch. Flash quads advance by the twenty-eight-byte `RroTexturedQuadPrefix`,
shared with the shadow reader. Its phase byte and reserved final halfword are
explicit; this newly established flash stride does not establish the shadow
command's unobserved tail or stream stride.

The billboard workspace names its rotation matrix, depth, staged vector and
four projected positions; flash depth, area and projected positions have their
own workspace. Scratch matrix/translation reads use the actual scene fields,
including native pointer-dependent offsets. Both ordering paths use actual
`u32` arrays and signed bias indexing rather than integer address arithmetic.
GPU links truncate only when stored to a 32-bit hardware word, then clear the
length byte. The billboard phase offset uses unsigned cyclic arithmetic;
flash-frame addition wraps explicitly and winding selects a signed sign-bit
mask without a signed left shift. Three redundant billboard asm inputs have
been retired. Its one register binding/one barrier, the flash kernel's two
bindings/three barriers remain reconstruction debt.

`rr/model_table.h` describes the counted table published by
`RelocateModelTable`: a signed count followed by three-word mesh entries. The
scene stores the entries pointer, immediately after that header. Billboard and
flash renderers recover the real `RroModelTable` owner using its field offset
and index `table->entries[modelIndex]`; their numeric entry-address temporaries
have been removed. The native owner retains full-width mesh pointers and its
natural header alignment. PAL assertions preserve the four-byte header.

The encoded resource has a separate `RroModelTableWire` view. Its flexible
word array names the vertex/command fields and entry stride. Relocation uses
unsigned addition modulo 32 bits, a single cursor that stops at the end of the
entries, and the canonical scene pointer field. `SceneLoadStep` retains the
actual table pointer across this call and narrows only its stored PAL address
word. Other encoded-address lifetimes in that loader and the native resource
decoder remain unported; the wire table is not a decoded native mesh array.

`test_track_object_quads.py` compares 8192 complete native/original-or-compiled
MIPS cases per renderer (16384 per MIPS image) with a separate wire oracle.
The initial 4096 cases are retained, with another 4096 covering independent
phase values. Both kernels cover zero through six quads, thirty valid scene
shift settings, all signed byte phases/biases, signed UV high halves, frame
boundaries, screen clipping and all ten word-aligned packet/head overlaps.
GTE-store hooks change model/GPU roots, render flags, matrices, current indices,
UVs, phase, bias, frame and palette. The checks distinguish captured geometry,
GPU roots, winding and billboard fourth indices from live flash fourth indices
and color fields. Billboard tests also check initial/final extra translation
prefetch and a next-record mutation after that vector has already been staged.
Negative/zero/far depth, reversed/zero winding, partial rejected packets,
complete native owners/GPU arenas, guards, exact GTE transfers and the MIPS ABI
are checked under UBSan/bounds and strict native warnings. GTE numerical
accuracy, flags/timing, other renderers and geometry/workspace aliases with GPU
storage are outside this test.

The two model banks in that fixture now include their real count headers.
Native banks are allocated as actual flexible-array owners; complete allocation
comparisons cover both headers, alignment padding and outer guards. The original
cases and independent oracle are retained. `test_model_table_relocation.py`
adds 4096 original/compiled-MIPS versus current native-C cases for signed counts,
wrapping wire offsets, exact store order, untouched table words and scratchpad
storage, full native published roots, allocation guards and the MIPS ABI.

The spotlight's migrated hardware calls preserve its original two asm blocks:
`RR_GTE_LWC2_V012_3P` loads the three inputs and `RR_GTE_RTPT_NOP2` keeps the
existing two load-delay instructions with the RTPT command. A three-block
substitution changed scheduling; the corrected sequence retains the relocated
instruction match. This does not make that renderer's remaining legacy pointer
arithmetic native-safe.

`EmitRroRawTexturedQuads` replaces the incorrect inferred name
`EmitRroLitTexTris`. Its four staged vertices, AVSZ4 command, forty-byte FT4
packet and fixed `0x2F` GPU command establish raw-textured quads; this is not a
recovered original symbol name. `rr/car_render_types.h` owns its twenty-four-byte
`RroRawTexturedQuad` stream record: packed vertex indices, three UV words and a
signed ordering bias. The fourth UV word sign-extends the high half of `uv10`.

The raw, lit and environment emitters decode the lower index in each packed
vertex/normal word as unsigned and the upper index as signed. Their array roots
can therefore denote an interior origin. Signed upper indices multiply by the
signed `RRO_VECTOR_STRIDE` enumeration instead of left-shifting a negative value;
the explicit halfword conversion recovers the packed index width. A bare
`sizeof(Vec3s8)` multiplier would instead make a negative offset unsigned.
The lower indices remain masked before their nonnegative shifts.

`test_rro_vertex_offsets.py` adds 4822 prefetch cases per emitter and MIPS image
(original and linked), with a separate byte oracle and actual LP64 C owners
under UBSan/bounds. It checks both signed-halfword extremes, unsigned indices
through 65535, nonpositive signed-header counts, complete work/scene state,
unchanged geometry/commands, guards and saved registers. Projection and emission
are deliberately rejected in this fixture; the existing three emission fixtures
remain unchanged. This removes signed-index shift UB, while the mixed numeric
address/value temporaries and existing register constraints remain debt.

The emitter uses the canonical scene/work owners, actual `GpuPolyFT4` packet
fields and named command fields. Its retained length-byte and bias-byte cursors
recover their enclosing records through field offsets. Native address lifetimes
retain LP64 pointer width; DMA links narrow only at their hardware-word store.
Signed ordering bias uses multiplication rather than a negative left shift.
Semantic local names replace the register-shaped names. The cached vertex base
is now an actual `const Vec3s8 *`, preserving the selected owner even when GTE
store hooks change the public root. Five register bindings, two empty barriers
and a combined word/address temporary still need reconstruction; that temporary
converts the typed vertex base at each address-use phase. The shared byte cursor
also retains its command-pointer then ordering-pointer lifetimes. The rest of
`gtepoly1.c` has not been migrated by this change.

`test_rro_raw_textured_quads.py` compares 8448 complete native C/original-or-
compiled MIPS cases with a separate wire oracle. It checks initial staging even
for negative/zero counts and the extra prefetched record after the last quad.
GTE-store hooks mutate indices after they were captured, geometry between the
first two and last two prefetched vertex loads, cached scene roots/shift and
live ordering, UV and bias fields. The cases include all signed-byte biases,
all thirty-two shift settings, zero through six quads, signed command counts,
depth/winding rejection, clipping at 320 by 512 (including mixed corners),
partial rejected packets and all ten word-aligned packet/head overlaps. Complete
native owners, padding, guards, exact GTE transfers, returned stream cursors and
the MIPS ABI are checked under UBSan/bounds and strict native warnings. Numerical
GTE accuracy, timing/flags and geometry/workspace aliases with GPU storage are
outside this scripted test.

`EmitRroLitGouraudQuads` replaces the incorrect inferred name
`EmitRroLitTexQuads`, including the actual dispatcher call and its public
prototype. The command has four vertex indices, four normal indices, a source
RGB/code word and a signed ordering bias. It emits a Gouraud G4 primitive between
two E6 mask commands; it does not read UV coordinates. This is a descriptive
reconstruction name, not an original recovered symbol.

`RroQuadIndices` shares the packed-word/halfword views of four indices between
the raw-textured and lit commands. `RroLitGouraudQuad` owns the complete
twenty-four-byte lit record. `GpuMaskedG4Packet` owns its forty-four-byte chain
entry: one DMA tag, mask setup, four color/XY pairs and mask restoration. Both
structures assert the known offsets and sizes on native and PAL builds.

The lit emitter uses canonical scene/work fields, typed command/packet records
and full-width cached vertex/normal roots. Genuine GTE calls receive actual
field addresses instead of numeric address temporaries where their removal
preserves the match. The signed ordering bias multiplies by four rather than
left-shifting a negative value; DMA links narrow only at their `u32` wire store.
Four retained barriers now only read their operands: six false output
dependencies were removed. Six register bindings, five empty barriers (including
the remaining two output dependencies), mixed full-width numeric temporaries
and its short context-pointer/word lifetime still need reconstruction.

`test_rro_lit_gouraud_quads.py` checks 8448 native-C/original-or-compiled MIPS
cases with a separate wire oracle. It covers initial/final vertex prefetch,
partial vertex-word capture, current normal-root selection, capture of the four
normal indices and source RGB, late fourth-normal reads, the staged fourth
normal and the GTE RGB FIFO. Instrumented stores/lighting operations change
the command, geometry, normal root/data, ordering root, depth, bias and mask.
The oracle checks the exact order of packet tag, mask and color stores through
all eleven word-aligned packet/head overlaps, including color writes after the
DMA head is linked. Every signed-byte bias, all seven mask samples, both normal
banks, all thirty-two shifts, negative/zero counts and zero through six quads
are exercised. Complete native owners/padding/guards, returned command cursors,
GTE transfers and the MIPS ABI are checked with strict native warnings and
UBSan/bounds. Projection/lighting arithmetic and flags/timing are scripted;
geometry/workspace aliases with GPU storage are outside this test.

`StepMenuCameraTransition` uses actual, read-only `ImageRow20` and `BadgeRow1C`
array cursors. Its old image-root lifetime through an `int` parameter and manual
twenty-byte increments have been removed. The public interface takes explicit
group flags; `MENU_TRANSITION_BADGES`, `MENU_TRANSITION_IMAGES` and
`MENU_TRANSITION_ALL` describe both the requested groups and returned readiness
bits. Its readiness mask is a `u16`, so ordinary bit clears produce the original
sixteen-bit masks without literal `0xFFFE`/`0xFFFD` values. The actual menu caller
uses the shared selector. Unselected groups and nonpositive counts are ready;
selected groups stop at the first state other than zero or their settled state
(badge eleven, image five).

`test_menu_transition_scan.py` executes the actual body with complete native
row arrays, high pointer roots, count/root cells, padding and guards. Its 8448
original/compiled-MIPS versus native-C cases cover all group flags/results,
zero/negative counts (including signed minimum), every start row, null inactive
roots and all 256 image-state bytes actually reached by the scan. A separate
array oracle verifies the exact MIPS early-exit read sequence and checks that
the arrays, roots and counts stay untouched. Native execution uses UBSan/bounds
and strict warnings; both MIPS images also check callee-saved and stack/return
state. The existing course-light test retains its negative-index and aliased-row
cases after removing redundant signed `sizeof` casts in `SetLightColorsFromRow`;
its integer address arithmetic still needs reconstruction.

The transform reads the camera through `ProjectionScratchPrefix`; native
coordinate offsets follow its two full pointer cells. Its calls to the GTE
services retain their original order. Ring scratch explicitly overlays the
transform and later ring geometry, instead of passing a four-halfword origin
array to a service which writes a complete 0x38-byte transform. Position inputs
still select the signed low half of three word coordinates; legacy callers
retain explicit views of their existing vector/matrix storage. Those private
input owners and other raw scratchpad accesses still need reconstruction.

`test_object_transform.py` compiles the real current transform body and compares
4096 original-MIPS/native service traces, complete owners, full scratch pointers,
extreme low-half coordinates, input/output and matrix/output aliases, mutable
services, outer guards and ABI preservation under UBSan/bounds. Its GTE services
are instrumented; it does not establish the numerical correctness of the hardware
matrix routines or direct native correctness of the complete car/ring renderers.

The SDK headers now declare `MulMatrix0`, `MulMatrix`, `MulMatrix2` and
`MulRotMatrix0` with real matrix pointers and their actual returned owner.
`MulMatrix` writes its first argument; `MulMatrix2` writes its second. All four
write five rotation words, including the padding halfword, and retain the three
translation words. Packing temporaries use unsigned words to make the shifts
defined. The existing hardware register bindings and return-copy barriers remain;
they have not been replaced by additional compiler constraints. `SetLightMatrix`
takes the same matrix owner. Its header also gives `SetBackColor` its actual void,
three-argument interface. The old ignored fourth arguments in lighting preparation
were removed while retaining the relocated match.

`LightMatrix` has one declaration in `rr/matrix_workspace.h`. The former scalar,
halfword-array, private-struct and scoped macro views now select its real matrix
or halfword members. `rr/track_lighting.h` owns the separate `RrTrackLighting2C`
resource rows: flags, three RGB/direction records and ambient color. Physics,
object lighting, color setup and the typed `SetLightTable` setter share that
pointer. The upstream scene loader still relocates encoded resource addresses
through integers; a typed setter does not make that loader native-safe.

The two color setters build complete local matrices, replacing unused padding
variables and the old fourteen-halfword buffer. Row selection still retains two
legacy integer address calculations, widened to full native pointer width.
Ordinary pointer/array spellings changed instruction selection in this TU; those
calculations remain reconstruction debt. Its temporary color cursor uses real
pointer arithmetic and has no 32-bit address truncation.

`test_gte_matrix_products.py` selects the actual four SDK product bodies and
`SetLightMatrix`, replacing only hardware transfers with adapters. It compares
20480 original-MIPS/native cases, all complete-owner argument combinations among
eight matrices, returned pointers, complete records, translation preservation and the
transfer/store protocol. An independent saturated Q12 product checks the packed
results. This model omits GTE FLAG, timing and undocumented behavior.
`test_track_lighting.py` compares 8192 original-MIPS/native cases for the real
table setter and both color consumers, including high pointers, valid negative
indices, row/matrix aliases, mutable callbacks, full owners, guards and ABI.
These checks use UBSan/bounds; they do not cover native execution of the complete
car/object renderers or the remaining legacy matrix-array callers.

`rr/camera_matrices.h` declares the six shared camera matrices as complete
`MatrixWords32` owners. Consumers select the word or unsigned-halfword view
explicitly, including the model renderer; no competing scalar, array or small
prefix declaration remains for those globals. `ProjectionScratchPrefix` names
the packed position and pitch/yaw/roll fields after two full native pointer
cells. `ViewMatrixScratch48` owns that prefix, the reserved word and the current
matrix. View setup, camera building, rear-view save/restore and the intro camera
row adjustment now use those owners. The intro adjustment doubles the matrix's
second rotation row with unsigned-halfword wraparound. Other renderer scratch
regions and encoded model/packet addresses still need reconstruction.
Intro lighting's temporary angles, direction and matrix also share one complete
local scratch record; its three transform calls select the actual typed children
instead of repeating raw offsets. The scene buffer uses the same `IntroSceneState`
in its initializer, script and renderers. Both leading matrices are complete
`MatrixWords32` owners; the cue pointer retains its native width and is followed
by the script fields and eight object yaw cells. Allocation advances by the real
owner's `sizeof`. The initializer clears the named script fields and the actual
yaw array, preserving the cue pointer, two leading counters and final padding.
Object rendering uses that array and a scratch union whose matrix-setup view
later gives way to the complete `ObjectTransform38`; its local product is also a
complete matrix. The script's old `GS` type macro has been removed. Fixed
scratchpad copy offsets, other script/renderer views and legacy GPU interfaces
still require migration before the entire intro can be considered native-safe.

`AllocMenuGridRecords` takes the caller's real buffer pointer and derives its
byte count from `sizeof(MenuGrid38)`, publishes the pointer, then invokes the
clear service. Its callers share the typed root and byte-backed work arena.
`test_menu_grid_allocation.py` extracts the current allocator with its public
headers and compares 4096 native/original-or-compiled MIPS cases: every valid
start/count combination in four eight-record arenas, complete clear-service
entry state, a service that may replace the published root, complete buffers,
return size, full native pointers, ABI and guards under UBSan/bounds. This test
also checks the pointer-sensitive scene offsets; it does not execute the entire
intro initializer or script natively.
The grid initializer has a full five-argument prototype. Its captured RAM page
uses `MenuGridPointerPage.records3A28`, replacing an array index that assumed
four-byte pointer cells. Native page offsets now select the actual full pointer.
The existing full-width integer address calculations and compiler constraints
remain debt. `test_menu_grid.py` extracts the current body and includes its real
public header, with only the fixed RAM page and hardware/compiler adapters
substituted. Its original 32768 cases and independent memory oracle are retained,
including negative valid indices, signed halfword narrowing and overlapping,
unaligned layout rows.

`ApplyMatrix`, `ApplyMatrixSV`, `ApplyMatrixLV` and `ApplyRotMatrix` have shared
SDK interfaces and return their actual destination pointers. Packed short inputs
use a word-aligned, eight-byte `Vec3s8`; full-word inputs/outputs use the actual
twelve-byte `Vec3i`. `ApplyMatrix` writes full MAC words, `ApplyMatrixSV` writes
three saturated IR halfwords, and `ApplyRotMatrix` writes sign-extended saturated
IR words. `ApplyMatrixLV` retains explicit unsigned wrapping during signed
high/low decomposition and result accumulation. Existing register constraints
and return-copy barriers remain reconstruction debt.

`test_gte_vector_transforms.py` extracts all four current C bodies and adapts only
hardware operations and compiler constraints. Its 16384 cases compare native
execution with original or compiled MIPS, complete argument-owner alias sets,
exact transfer/store traces, returned pointers, complete surrounding owners and
an independent arithmetic specification, including saturation and INT_MIN.
`test_intro_camera_matrix.py` extracts the actual intro camera body and checks
all 65536 halfword patterns in each adjusted cell, service entry, untouched
storage, native pointer width/padding, guards and ABI against original or compiled
MIPS. Both use UBSan/bounds. The vector model omits GTE FLAG and timing; the intro
test instruments `BuildCameraMatrices` rather than validating its trigonometry.

The GPU driver has one complete sixteen-pointer `GpuDriver40` definition in
`rr/library.h`; reset, display, image and queue callers select its named slots.
The former graphics prefix spelling is an alias of this same complete owner.
Image APIs take a real `GpuRectangle` and separate const upload / mutable download
pixel pointers. Their callers use the rectangle union, retaining field-only
copies where the original alignment requires them. Queue records transport the
first parameter as `void *` and the second as `unsigned long`, preserving complete
native pointers. Both queue indices have consistent interrupt-visible volatile
declarations. The status callback and `_status` return `u32`, including the high
status bit consumed by `GetODE`.

`test_gpu_image_dispatch.py` checks the four current wrappers against original or
compiled MIPS in 16384 cases, complete native driver/rectangle/pixel owners,
diagnostic-time callback/root mutation, RGB byte packing and the fill flag.
`test_gpu_queue_transport.py` exercises the current enqueue bodies in 16384 native
cases against an independent model, including both entry points, ring wrap/full
states, draining/timeouts, non-word lengths, overlapping payloads and high pointers.
Their GPU/interrupt services are scripted. The queue's unspecified-argument
`GpuOpFunc` callback ABI and casts of differently typed backend operations still
need reconciliation; the queue test uses compatible callback signatures and does
not establish native correctness of every backend operation or IRQ interleaving.

Remaining local views include `S_ECB4` (objsim reads `u16 f1C..f22`, objdraw reads
`s16 rot.vx..vz`), `RrSlot620` (sndchan/sndsys use different field types), `UiElem24` and
`RrParticleSys`. Reconcile their real access semantics before selecting one common layout.

## Symbol names
Auto names (`func_XXXXXXXX`) are replaced by real names when the name is certain. `configs/PAL/sym.main.txt`
is the registry (`Name = 0xADDR; // type:func`); `tools/scripts/rename_symbols.py` applies it to sources, headers,
tests and configs, and `make split` regenerates the asm with the same names. 76 PsyQ library functions have their
real names (`DrawSync`, `SetDispMask`, `CdControl`, `SetDrawEnv`, ...): a name is registered only if it exists as a
symbol in the reference PsyQ decompilation and the hint list has exactly one address for it. Names that the hint list
repeats at several addresses (`InterruptCallback`, `Menu_RebuildSelectableMask`, ...) or that are tool-made labels
(`Gpu_*`, `CardObj_*`) are not registered. Fixture files keep their address-based file names (tests build the paths
from the address); only their contents use the new names.

### Where names come from
* PsyQ: the reference PsyQ decompilation (unique match by code and library order).
* Game, graphics layer: 14 functions have a byte-equivalent in the Rage Racer decompilation (`~/Projects/rage-pc`);
  they carry its names (`SubmitModelFaces`, `EmitPolyF4`, `TransformCourseModel`, `GameQueueSprite`, `BuildRotMatrixX/Y/Z`, ...).
  Those are reconstructions of the same kind as ours, not names from Namco's source.
* Fields: `tools/scripts/emu_probe.py` drives the emulator; the old `Car.posX/posY/posZ`
  (now `RrRaceVehicle.posScaled00`) were identified by moving the car through memory
  (the car and camera jump), and the position at +0x10 by the code refreshing it with `>> 6`.
* Globals: `CarTable` (was `CarTable`) is the table of pointers to the race vehicle records. Seen in the running game:
  eight consecutive pointers with a constant 0x320 stride, the first one equal to the vehicle that `FindWheelTrackNodes` receives.
  `rename_symbols.py` also registers data names (`Name = 0xADDR; // type:u32` renames `D_ADDR`).

`EmitRroEnvMapQuads` uses the complete thirty-two-byte `RroEnvMapQuad` command,
canonical scene/work fields and actual vertex/normal roots. `GpuGT4Body` holds
four RGB/XY/UV groups. `GpuEnvMapBlock` describes both the direct packet starting
at its second word and the masked E6/GT4 packet starting at its first word.
`GpuEnvMapEmission` owns both blocks and the optional mask-restoration node.
Secondary XY values are written before clipping and mode selection, so even a
direct emission advancing by fifty-six bytes requires the complete 120-byte
lookahead footprint. The fourth primary UV sign-extends the high half of uv18.
UV bit packing and signed ordering-bias scaling avoid negative left shifts;
the primary UV/flags addition preserves the original 32-bit wrapping.
Four false empty-asm output dependencies were retired. Eleven register bindings,
seven empty barriers, mixed numeric address/value lifetimes and two retained
packed-XY word stores still need reconstruction. Those two stores use offsets
of the actual native owner; they do not assume the PAL pointer-prefix size.

`rr/gpu_address.h` separates native pointers from encoded PSX DMA addresses.
All three RRO emitters encode their ordering links through this boundary;
envmap also resolves the previous node before reading its first payload byte.
The original reads encoded address+4, retaining low tag bits: it does not test
the GPU opcode at byte7. The PAL adapter preserves those actual loads/stores.
`src/native/gpu_address.c` maps registered real buffers, validates complete spans,
supports physical/KSEG0/KSEG1 aliases and rejects overlapping or invalid maps
without replacing the active map. It never reconstructs a host pointer from
truncated bits. Native callers must install descriptors/buffers that stay alive
through command processing; a full native application still needs that setup.
The raw-textured and lit fixtures now exercise this real backend as well.

`test_rro_env_map_quads.py` compares 8448 complete native-C/original-or-compiled
MIPS cases with an independent wire oracle. It covers direct and masked modes,
restore-node insertion/reuse, tagged previous-node addresses, all fourteen
direct packet/head word overlaps and all seven masked overlaps whose previous
node remains a valid mapped address at the peek. The fixture mutates geometry
between prefetch loads, normals between capture and the fourth-normal load,
UVs/flags around lighting, ordering/depth/bias/control after primary colors,
and CLUT/page at their actual read boundaries. Initial frame height, depth
shift and restore mask remain captured despite later public-root replacement.
It checks full native owners/padding/guards, lookahead writes on rejected/direct
quads, signed UV/IR boundaries, every signed-byte bias, all thirty-two depth
shifts, negative/zero counts, clipping/winding/depth rejection, exact GTE
transfers, RGB FIFO and the MIPS ABI under UBSan/bounds and strict warnings.
Projection/lighting arithmetic and timing/flags are scripted; geometry/workspace
aliases with GPU storage and invalid unmapped DMA inputs are outside this
emitter test. `test_gpu_address.py` separately checks the production native
mapper's bounds, aliases, rejected-map rollback and unmapped-boundary aborts.

`FrameStorage` now exposes its complete `FrameBuffer`: the environment pair,
two 704-word ordering tables and the packet arena beginning at1670. The display
suffix is an actual `DisplayEnvironment` child at `FrameEnvironment.display5C`.
Frame initialization and the calibration/playback/draw-area consumers select
that child through the same shared layout. `D_800D0268` and `D_800D02C4` name
addresses inside bank1, rather than independent C globals; initialization now
uses `DrawBuffers[1]` and its display child, and their old externs are removed.
The shared SDK header declares the actual typed `SetDefDispEnv` interface.

`MainLoop` selects `&frames[bank]` from the real `DrawBuffers` array. Its ordering
roots, packet cursor, draw/display environments and final table entries use
named fields of that owner. Scratchpad publication uses `GpuScratchPointers`
with real pointer cells. The reconstructed800B page and its numeric pointer
multiplication are gone, as are both empty barriers and one of its two register
bindings. One arena-root register binding and the retained800F page addressing
of `DrawBufIndex` still need reconstruction. The public `CurrentDrawBuf` views
remain inconsistent across other modules; the loop still publishes the byte
view and selects the actual owner explicitly at each later read, preserving
callback-time root replacement. This is not a completed global unification.

`SetupRenderer` has one prototype in its owner header, `game/data/cdfile2.h`.
Resident callers and overlay interfaces include it rather than supplying
private `int`/unspecified-argument declarations. Its source definition and all
callers agree on `void SetupRenderer(s32)`. The renderer-initializer, frame-loop
and frame-environment tests now select the actual current definitions, instead
of historical single-function views. All previous input matrices and memory
specifications remain. The loop test also checks complete frame arenas,
scratch-pointer owner guards, untouched bank-page bytes and full-width roots.
The environment test runs the actual draw/display SDK constructors with their
typed interfaces, retaining empty-asm operand dependencies while stripping
only physical MIPS bindings/clobbers on the native host. One false flags output
dependency was retired from frame initialization. Its other bindings/barriers
remain debt. These native tests cover helper protocols and CPU state, not GPU
execution, vblank timing or interrupt concurrency.

`rr/gpu_environment_types.h` separates the complete92-byte `DrawEnvironment`
from the20-byte `DisplayEnvironment`. `FrameEnvironment` owns both as `draw00`
and `display5C`; SDK drawing functions receive just the drawing child. The
sixteen-word command backing belongs to DRAWENV, including its unused words.
`SetDefDrawEnv`, `SetDrawEnv`, `SetDrawEnv2`, `PutDrawEnv`, `DrawOTagEnv` and
`GetDrawEnv` use that shared type. The queue's remembered drawing/display
records are the actual `GpuResetState.draw10` and `display6C` fields. Reset,
submission and readback use their sizes; readback no longer returns an integer
address or declares the draw-cache alias as a separate byte array.

The old drawing-byte name `rgb24` was incorrect: the builders use byte18 to
emit a background clear. It is now `background`, while the display descriptor's
byte11 is `rgb24` (previously called `high_res`). The behavior in the PAL
builders/display-mode command establishes the distinction; the field meanings
also agree with the [PSn00bSDK descriptor definitions](https://github.com/Lameguy64/PSn00bSDK/blob/master/libpsn00b/include/psxgpu.h#L508-L523).
That SDK is a semantic cross-check, not the source of this executable's PAL
sizes or command backing. `FRAME_CLEAR_BACKGROUND`, `FRAME_INTERLACED` and
`FRAME_RGB24` name the game setup masks1/2/4. RGB24 uses480 VRAM words for a
320-pixel display; it is not a wider display resolution. Existing flag values,
input matrices and byte-level expected results are preserved.

The STR frame-packet emitter uses the same standalone DRAWENV, full frame and
scratch-pointer owners, `GpuTilePacket` and explicit `GpuDmaTag` fields. Its
private descriptor/frame/scratch/tag copies and generic tag macros are gone.
Both tag bitfields and byte/word views describe the actual low24 link/high8
length protocol. CPU pointers encode through the production DMA adapter at
wire stores; the cursor remains full-width. Unsigned multiplication preserves
the original bank*256 wrapping without shifting a negative signed value.

`test_draw_environment.py` and `test_draw_fill_environment.py` now select
current builder/helper bodies, preserving empty-asm operands on the host while
normalizing physical register bindings. Their original524288 cases each and
independent byte/service oracles remain, including embedded command output and
untouched display suffixes in complete frame-prefix owners.
`test_draw_environment_transport.py` adds8192 cases for the actual constructor,
submission, tagged submission and readback bodies. It checks all debug bytes,
real native driver/root/descriptor pointers, callback-time mutations, cache
self-aliases, all sixteen embedded ordering-address choices, complete owners,
service-entry traces and92-byte copies with the real native DMA mapper.
The queue operation pointer retains its existing cross-signature legacy ABI;
the fixture compares/transports it but never executes GPU operations through it.
Other native compiler diagnostics remain errors.

`test_overlay668_frame_packets.py` adds4096 cases with actual STR, initializer,
builder and coordinate/mode/window helpers. An independent byte oracle checks
all four modes plus invalid-mode no-ops, full signed bank boundaries, sixteen
packet/head overlaps per mode, callback-time frame/cursor replacement, complete
frame arenas and scratch guards under UBSan/bounds. Draw-mode insertion is an
instrumented service; hardware execution, GPU queue concurrency and native
application initialization remain outside these fixtures. Other shared `CurrentDrawBuf` views and the GPU queue operation-pointer ABI
still need reconstruction.

`PutDispEnv` and `GetDispEnv` now share the actual `DisplayEnvironment`
interface in `psyq/libgpu/gpudisp.h`, including the main-loop caller. Readback
returns a full native pointer and copies the actual twenty-byte owner. The
private DISPENV and rectangle-layout copies are removed; primitive rectangle
setters use `GpuRectangleFields` too. Horizontal timing has one four-byte
`GpuHorizontalRange` record for the2x5 table, declared by its owning header.

The descriptor tail has explicit byte and packed-word views in `DisplayFlags`.
Submission compares all four bytes, not just interlace/RGB24. Its mode byte
receives the video standard and temporarily holds8 to force timing rebuilding.
The word view also supplies the four-byte alignment required by the actual
loads, while descriptor size and every PAL field offset stay unchanged.
Frame/SDK constructors and their native oracles select the same typed fields.

The cached display and screen rectangles, packed flags and final copy now
select `GpuResetState.display6C` directly. The old D_800ABB2C/D_800ABB34/D_800ABAC3
views no longer declare independent objects in this unit. The debug read uses
the actual `GpuDebugSettings` child. Eight pre-existing volatile cache loads are
retained through a pair of pointers to qualified `GpuRectangleFields`; no new
volatile access was added. Three register bindings and the mode-byte reload's
empty memory barrier remain reconstruction debt. Removing those constraints
has not yet preserved the complete relocated instruction sequence.

`test_display_environment_transport.py` executes the actual submission and
readback bodies in16384 native cases under strict warnings and UBSan/bounds.
An independent byte/protocol oracle checks debug/video/control/copy service
entries, complete owners and guards, full native return pointers, separate and
self-aliased cache/input descriptors, all five width classes and both standards.
Callbacks replace the live driver and mutate input/cache/flags/timing tables;
the vertical command retains the calculated window after the horizontal
callback, while the final copy reads the subsequently changed descriptor.
The cases cover mode rebuild, timing-only rebuild and unchanged settings,
zero-height defaults, signed coordinate/dimension boundaries and every debug
byte. The existing memory barrier and volatile cache views remain in native
execution; only physical register bindings are normalized. GPU command
execution, asynchronous interrupts and the queue's separate legacy callback
ABI remain outside this fixture.

The GPU reset/queue state now owns a named `pending08` word and a real
`GpuCompletionCallback` (`void (*)(void)`) in `completion0C`. `DrawSyncCallback`
accepts and returns that pointer type; it no longer finds a callback through
integer arithmetic on the debug-byte alias. Logging still precedes capturing
the previous registration. Queue publication and draining use the same fields;
the separate `D_800ABACC` integer declaration and integer-to-function-pointer
conversion are removed. Draining restores interrupts, checks queue/DMA state,
clears the pending word, then calls the captured function. The existing volatile
clear remains; user code can replace the registration and rearm a later pass.

PAL retains the exact0x80-byte owner, callback at0xC and caches at0x10/0x6C.
The native64-bit owner is136 bytes: natural callback alignment places it at16,
followed by the unchanged92/20-byte descriptors at24/116. `ResetGraph` clears
`sizeof(*state)`, and `SetDispMask` invalidates the actual display child using
its size. Neither operation depends on an offset past the old debug prefix.
The PAL offsets have compile-time assertions; native fixtures independently
assert the complete pointer, cache and state layouts.

The existing reset262144, queue16384, draw8192 and display16384 cases remain.
Their native byte oracles use the corresponding native cache offsets and full
owner extents. The reset fixture preserves its original128-byte RNG stream
and mutation positions by mapping scalar/cache bytes into the native owner;
former callback bytes select actual functions. The queue fixture uses a real
registered function instead of integer1 as a callback sentinel. The draw and
display fixtures also seed valid callback representations.

`test_gpu_completion.py` adds4096 cases executing the current registration,
display-mask and drain bodies. Each case checks registration, mask submission
and two drain passes against an independent byte/service model. Full service
entry snapshots include the complete state, all64 queue records and their native
padding, payloads, callback/root changes and interrupt state; owner guards are
compared too. Cases cover every ring position, all debug bytes, depths0..3,
null/two real callbacks, four pending-word boundaries, DMA-busy exits, delayed
draining and callback rearming. Registered functions and payload/parameter
pointers retain native addresses above32 bits. Hardware readiness and interrupt
services are scripted; asynchronous execution is outside this fixture. The
queue operation's separate `GpuOpFunc int(*)()` ABI is still unresolved: its
deprecated-prototype diagnostic remains visible in the native log while other
diagnostics are errors. No target compiler flags or allocation constraints
were added. The drain's constant-mask scheduling expressions remain debt.

`CurrentDrawBuf` now has one declaration, `FrameStorage *`, in
`rr/frame_storage_types.h`. Main-loop selection publishes the full bank owner,
and consumers select its draw/display environments, primary/secondary ordering
words or packet bytes. The byte-pointer, rectangle-pointer and independent
icon/fade/overlay context declarations are removed. Direct fixed ordering
offsets now select the corresponding array elements, including the final
primary head and the first secondary head. The menu's byte18 write names the
DRAWENV background flag.

The private `IconSpriteContext`, `FadeSpriteContext`, `PanelContext`,
`StrCreditContext`, `Ovl665Frame`, ripple `DrawBuf98` and divider `struct O`
layouts are retired. Their fields select the actual frame owner, including
heads0/1/10 and the secondary table. Ripple/brightness emission explicitly
captures a pointer to its ordering word; repeated loads and packet/head overlap
semantics remain. Existing register constraints and volatile stores have not
been supplemented. The common environment records now live in
`rr/frame_environment_types.h`; importing storage types does not import the
settings global or setup API. Consumers still needing that API include its
header independently.

Native frame consumers now allocate real `FrameStorage` owners. Icon, fade,
badge-pattern, text, panel, tile, sprite-effect and glyph fixtures select the
current production definitions; icon rows also select their actual shared body
macro. Existing input matrices, service protocols and byte/alias oracles remain.
The icon pointer-slot redirection fixture keeps its complete enlarged allocation
inside one24-bit window and independently asserts that property; its appended
storage permits the redirected full frame span. Screen-tile aliases likewise
have full owner backing, and whole frame guards/tails are checked.

The screen-exit fixture retains its old RNG stream by randomizing each B70-byte
prefix separately; the added packet tails hold canaries and participate in the
complete owner comparisons. The1,441,792-case audio-options fixture previously
kept every frame byte at A6. Its full frame arenas are now mapped read-only with
that same content. Any write to a frame fails at the memory protection boundary;
service snapshots still compare all mutable state, arguments and live roots.
This avoids copying immutable packet storage into every event without reducing
cases or the previous byte oracle. The RRO environment fixture serializes each
frame's original156-byte prefix separately into its unchanged wire/oracle image;
its full native owners and untouched tails are still compared.

Other private scratch/packet views, integer-address kernels and register/barrier
constraints remain reconstruction debt. A common global declaration does not make those remaining local interfaces
or all rendering code portable. GPU execution and asynchronous interrupt timing
remain outside the scripted native fixtures.

Ordering storage now has one `FrameOrderingTable` union:704 SDK words and704
`GpuDmaTag` records describe the same memory. `FrameBuffer` and the shared
ordering prefix own these tables. SDK/renderer arguments select `.words`, while
link updates select `.tags` and their low24/high8 fields. The old `DRAWBUF`
macro, `DrawBuf`/`DrawBufTag` prefix copies and `game/gfx/drawbuf.h` are removed.
The frame union's separate padded secondary-table view is gone too; consumers
select the real secondary table. Table/environment/packet offsets and bank size
are unchanged on PAL and the native host.

Scene overlay emission uses the shared sprite, single-command, three-word
draw-mode and rectangle records. Its private `Sprt`, `DrTPage`, `DrStp`, `Rect`
and generic link macro are gone. Packet insertions name the actual tag/head
fields and encode native packet addresses through the production DMA registry.
Camera/motion emission also uses the shared draw-mode and tile packets; its
private packet/tag copies are removed. Sub-order rendering uses `GpuDmaTag`
instead of a second local tag type. Its remaining scratch/integer-address
kernel has not been ported by these frame-view changes.

`SetDrawStp` takes the real `GpuDrawModePacket *` backing: byte3 becomes2,
command0 is E6000000/E6000001, and command1 is zero. The mode emitter has the
same explicit three-word input shape and an opaque next-cursor result in its
public interface and definition; its former private three-int `T` is removed.
Typed declarations/service doubles select this shared packet too. The rank
renderer reads a full native scratch cursor, drops its ignored fourth helper
argument, and encodes packet links at the wire stores. Its zero high-bit mask
on an already24-bit field is retired. `QueueDrawStp` names the full packet,
selects primary word703 after construction, links it, advances once and
publishes the cursor. Its dead result/cursor aliases are gone. The atlas address
calculation still uses explicit address arithmetic to retain the PAL expression;
it now retains complete native pointer width rather than narrowing to `s32`.

`test_scene_packet_submission.py` adds4096 cases executing current scene,
rank, mask-submission, mask-construction and mode-emitter bodies under strict
warnings and UBSan/bounds. All4096 cases compare complete frame/GPU/input owners,
service-entry snapshots, normalized full native scratch pointers and guards
against an independent byte/protocol oracle. Cases cover both banks, both GPU
address maps, mutable service boundaries, separate and three overlapping
packet/head placements, RGB/packet overlap and signed flag boundaries. Native
addresses exceed32 bits and differ from the configured DMA addresses. Existing
ordering-table native matrices retain their original cases and oracle values;
their member selection now names the `.words` representation.

Mode building and GPU linking are instrumented services in the new fixture;
mask construction and mode-emitter call/advance behavior use actual current C.
GPU execution, concurrency, remaining legacy callback declarations and integer
scratch/address kernels are outside this scoped verification. Existing register
bindings/barriers and target toolchain settings have not been supplemented.

The65,536-case HUD-number fixture now selects the current `DrawHudNumber5` and
`EmitHudGlyph12` definitions instead of stale generated single-function views.
Its input matrix, helper-boundary mutations, packet/head alias cases and complete
byte oracle are unchanged. The glyph's existing test-only full-width address
normalization remains; its production integer-address kernel still needs work.

`ScaleMatrixRow1` has one public declaration in its defining unit's
`game/car/carbound.h`: it takes a complete `MatrixWords32 *`. All resident and
overlay callers use this declaration; the old integer/unspecified parameter
forms and overlay-local prototype are removed. Hardware scratch calls select
the matrix at28 explicitly, while overlay665 selects its real matrix member.
The implementation names the second rotation row and uses an unsigned factor
so the retained low16 product is defined even for extreme signed scale inputs.
No register bindings, barriers or compiler settings are added.

`test_matrix_row_scale.py` exercises the current definition and public header:
1,114,112 native cases exhaust every halfword pattern for16 scale boundaries
and add random full-word scales. An independent byte oracle compares the whole
matrix and both guards. Another65,536 cases compare native execution with the
original MIPS, checking all state, exactly three halfword stores and the
callee-saved registers. The optional compiled-ELF path checks the linked current
target function too. The existing overlay665 renderer fixture retains its
original matrix/scenario/service oracle and now selects this current scaler
definition instead of the stale generated view.

`ProjectionScratchPrefix` now owns two complete SDK vectors after its two CPU
pointer cells. The fourth angle word at PAL+0x24 belongs to that vector; the
separate `reserved24` fields in `ViewMatrixScratch48` and `PreviewRenderScratch`
are removed. Their PAL matrix offset remains+0x28 and the complete PAL layouts
are unchanged. Native matrices follow the actual full-width pointer/vector
prefix, without a second reserved word or the old intervening alignment gap.
Component-only users select `pose08.angles10.xyz`; full camera copies retain all
four words through `pose08.angles10.vector`.

`StepCameraScript` uses the common `RroSceneScratchPrefix` instead of its private
`Scratch5B304` layout. Its cursor and ordering table are actual full CPU pointers;
the old `s32 p04` address cell is gone. Packet constructors select their shared
packet type at cursor boundaries. Sub-table links use `GpuDmaTag` and encode the
last ordering word through the production DMA registry. The local scratch and
packet variables now describe their roles. Existing register/barrier constraints
and toolchain settings have not been supplemented.

Existing native angle consumers retain their input matrices, cases and byte
oracles. The intro-camera fixture still serializes its original72-byte wire
image, including the same fourth angle word; that word is now an explicit vector
member and is independently checked against the input. The transform fixture
continues to check that the unused fourth word remains untouched.

`test_camera_script_submission.py` compares4096 current native camera executions
against an independent script/byte model. Complete owners and every service-entry
snapshot retain real native pointers. The fixture covers both clocks, timer and
duration boundaries, sentinel wrap, all operation paths, viewport/draw/stencil
gates, helper-driven controller/frame/cursor changes, four packet/head placements
and two production DMA maps. Coverage assertions require the reserved fourth word,
timer wrap, sentinel and every alias placement to be exercised.

Another512 cases compare complete serialized owners and all service boundaries
with original MIPS; the optional linked-ELF path checks the current target body.
Stores are constrained to the actual controller fields, scratch, GPU owners and
callee stack frame, with outer stack guards and saved-register checks. Native
mask construction and row scaling execute their current definitions; the model
uses independent byte arithmetic. Motion, geometry and model services remain
instrumented; GPU hardware, trigonometry and asynchronous execution are outside
this fixture. Existing private global views and other integer-address kernels
remain reconstruction debt.

`DrawPreviewCar` now has one public declaration in `game/car/carwheel.h`.
Resident preview callers and overlay controller headers import it instead of
declaring their own integer, unspecified or duplicate aggregate interfaces.
Four overlay-local color unions are replaced by the shared `PreviewColor`.
Main preview and reflection callers likewise select its named RGB channels and
aligned argument; the black preview color uses a normal `PreviewRgb3`
initializer. The argument's fourth byte remains ABI padding, not a color channel.

`MenuArenas` has one declaration in `rr/menu_arena_state.h`, re-exported by the
menu row owner's header. All resident/overlay consumers use the same two-pointer
`MenuArenas8` record. Old scalar/word-array declarations select `.first` and
`.second` explicitly. The garage captures `LoadCursor` as a complete pointer
before publishing it; it no longer reuses an integer UI-row variable for an
address. Other legacy address globals in the arena initializer/loaders still
need migration; this shared declaration does not establish their native safety.

The garage state machine selects the live phase with a normal `switch` and
retains its selected case across helper calls. Its labels, dead assignments,
`W7C0_U` and private `MenuSelectionState` prefix are removed. Settings copies use
the actual `MenuSettings2C`/`MenuChoice4` records. The showcase selector names
that same owner's `phase17`, the real alias of `MenuSubmodePendingId`. Frame increments
and angle updates use unsigned arithmetic before signed serialization, retaining
the target wrap without signed-overflow dependencies. Existing target compiler
settings and register/barrier constraints are unchanged.

`test_main_preview_controllers.py` executes both current main entry points through
the public headers. Its independent transition model compares 32,768 native
cases, every complete service-entry state and all final owners. Cases cover
the loader and menu phases, full arena pointers, real vehicle/control pointers,
mutable UI/image/badge roots, signed state boundaries, frame/angle wrap and all
256 image timers. Coverage assertions require every service path and timer.
Current matrix scaling and counter reset bodies execute in the native path;
reference effects use independent scalar/byte operations.

A further 1024 cases compare complete wire states, call arguments and boundaries
with original MIPS; the linked-ELF option checks the current target entries too.
The selector's address alias is asserted, native pointers are serialized through
real owned regions, and saved registers, store bounds and stack guards are
checked. RGB comparison covers exactly three defined bytes. Preview drawing,
loading and other geometry/UI services are instrumented; the complete renderer,
reflection integer-address kernel and asynchronous execution remain outside
this fixture. Existing overlay matrices and byte oracles are retained; the
overlay662 stub generator now accepts named parameters from the owner header.

`SpuVoiceRegisters` names all eight actual channel MMIO halfwords, including
pitch, sample/loop addresses and both envelope registers. Their volatility is
hardware semantics. Layout assertions cover the complete 16-byte channel and
all member offsets; the complete bank remains a 512-byte `SpuRegisterStorage`.
`SpuSetVoiceAttr` accesses channel fields through that bank rather than indexing
beyond a scalar `left` member. Its halfword register index is also the interface
to the address-writing helper. The bank root remains live across helper calls.
All resident callers import the public declaration from `psyq/libsnd/vmvinit.h`.

`test_spu_voice_attributes.py` executes the current body in 65,536 native cases
with UBSan and full native pointers. An independent numbered-register model
compares both complete bank owners, attributes, center-note cache, guards,
every service-entry state and the final state. Cases cover all 24 voices,
individual/combined/zero masks, volume/envelope boundaries and helper-driven
changes to the live bank, attributes and cache. Another 1024 cases check
original MIPS service traces and final owners; the linked-ELF option executes
the current target body. Store bounds, stack guards and saved registers are
checked. Note-to-pitch and address conversion are instrumented services in this
fixture; existing register pins and the legacy delay loop still need cleanup.

`scene1`, `camview`, `camfx`, `carfx` and `meshquad` share `SceneCameraState160` and the corresponding
`SceneRaceContext` declaration. Five actual 24-byte poses name full word-component
positions and SDK short-angle vectors. The lighting block reuses
`PreviewLighting40`; the three former private camera layouts and the private
rotation/light input records are removed. The native context tail follows its
full-width camera pointer. Unresolved words remain explicit fields. Other
`RaceContext` views elsewhere still need recovery; the legacy `model560` slot
must be migrated together with `DrawMeshPrimList` and the reflection packet path.

`SetupObjectLightMatrices` and `TransformPoint` have typed public interfaces in
`game/gfx/scene1.h`. Their reflection caller uses actual matrix/vector owners.
The fifth point-transform workspace call slot is preserved and explicitly
unused by the implementation. The transform owns a complete, aligned `Vec3s8`;
only its three defined components are consumed. Point additions and showcase
controller calculations retain MIPS wrapping through unsigned arithmetic.
No target register constraints or compiler settings were added.

`PlayerCars` has one declaration in `rr/player_cars.h`, re-exported by its five
existing public consumers. `InitCameraState` reads the real bounds pointer
array's first record and the player's `halfSpan10E` member, the physical alias
previously declared as `D_800AC006`. Camera curves likewise have one declaration
in `rr/camera_curve_tables.h`: four three-point arrays and the thirteen-point
steering array, including their negative-time sentinels. The old scalar view
of `ShowcaseCamDepthCurve` and its cast to a curve pointer are gone. The scale vector uses
the shared `VectorWords16` rather than a private four-word record.

Three current-body fixtures check 65,536 native point transforms, 32,768 native
initializer/controller runs and 8192 native lighting setups with UBSan. They
compare complete owners, every service-entry state, callback changes, live
camera-root replacement, pointer-array records, output aliases and final states.
The point fixture exhausts all input halfwords. Separate 1024/1024/512 cases
check original MIPS; their linked-ELF options check the current target bodies.
Store bounds, stack guards, saved registers and the curve/vehicle address
aliases are checked. Trigonometry, matrix/GTE operations, curve interpolation
and the old model-page call slots are instrumented services in these fixtures.
The lighting test seeds complete helper matrix results and consumes only the
nine defined color-matrix halfwords. The reflection renderer's remaining byte
workspace, integer GPU kernel, old rotation/buffer interfaces and other native
address kernels remain outside these fixtures and still need cleanup.

The showcase track consumer now uses the actual pointer at camera+0x138 to
arrays of 24 complete `SceneCameraKeyframe2C` records. Each record contains a
36-byte `SceneCameraView24`, a time and a restart value. The root's view at
+0x60 uses the same type as `ApplyCameraState`; the old private `Node`, camera
and root views in `carfx`/`meshquad` have been removed. Record short-angle
vectors retain their natural two-byte alignment, while local GTE input vectors
are word-aligned `Vec3s8` owners. On native hosts the track pointer widens and
the containing camera/root follow their natural alignment; PAL layout checks
remain target-specific. No host pointer bits are stored in 32-bit address cells.

Track interpolation uses an inline C displacement helper with unsigned word
subtraction/multiplication, signed division and unsigned base addition. Keyframe
time conversion, fallback positions, scene distance, counters and applied camera
translations also preserve MIPS wrapping without signed-overflow dependencies.
The original unused local in the track function and unrelated packet kernels
remain reconstruction debt; no new stack padding or register constraints were
introduced.

`test_scene_camera_tracks.py` runs the three actual camera consumers in 49,152
native scenarios and compares 1536 original/linked-MIPS traces. Complete owners
cover three banks of three 24-key tracks, three selectable cameras, two vehicles,
bounds/pointer tables and the full pointer-bearing projection scratch prefix.
Cases cover every selected key or no key, positive/zero/negative durations,
absolute/relative endpoints, callback changes, segment-output aliases, counter
limits and wrapped products. Matrix/render services are instrumented; signed
DIV emulation is checked independently against 1024 Unicorn blocks. One SDK
reserved position word is undefined when an original relative endpoint is
copied whole; only that word is excluded on those paths. All coordinates,
remaining owner bytes, store bounds, stack guards and saved registers are checked.

The older 32,768-case initializer/controller fixture retains its original RNG
input format through a test-only frozen decoder. PAL serialization resolves
track tokens to complete native row owners. Aggregate hashes of all original
inputs and complete service traces prevent the wider pointer layout from
silently changing the existing scenario matrix or its byte oracles.

`game/gfx/fontdraw.h` now owns the single public `AddTextSprites` declaration.
Both menu-facing headers re-export it. The actual definition accepts a packet
arena cursor and a readable character string, and returns the next full native
cursor. Its menu, UI, link-screen and intro consumers no longer declare an
integer return value or thread packet addresses through 32-bit temporaries.
The menu callbacks likewise accept real ordering/cursor pointers. Their option
and badge-row accesses use native pointer arithmetic, without integer address
casts. The two UI labels at 0x80010924/28 are constant strings "1" and "2";
their read-only declarations also preserve the fully prototyped caller's match.
The intro's former integer-array text view is thirteen complete16-byte string
rows, from "WELCOME" to "MUSIC PLAYER", retaining the original quotation marks.
Its DMA link is encoded through the native-aware GPU address interface.

`AddSpritePrim` uses the shared sprite/UV records and agrees with its public
arena-cursor interface. The two flat-line emitters share `GpuLineF2Packet` and
the declarations in `rr/gpu_line.h`. Coordinates remain full words until the
packet's low16 stores. `AddPartialLine` takes a real ordering pointer and keeps
its scratch cursor at native width. Its Q8 displacement helper multiplies with
word wrapping, then arithmetic-shifts the signed product; endpoint additions
also wrap. `DrawUiBox` has a seven-scalar, void interface, full pointer cursors
and defined wrapping for its corners and dimensions. Other HUD/menu packet
kernels, UI-window scratch accesses and private declarations remain debt.

`test_primitive_pointer_api.py` exercises the three actual sprite/line emitter
bodies in65,536 native cases with UBSan and1536 original-MIPS traces. The
linked-ELF option exercises the current compiled functions. Complete guarded
arenas cover all64 UV/head alias pairs, three output arenas, full word inputs,
helper mutations, every helper-entry/exit state and the returned native cursor.
SDK setters and ordering insertion are instrumented, including register
clobbers in the MIPS service adapters; these tests do not simulate GPU hardware.

One text-spacing issue remains explicit: an initial space in proportional
mode reads incoming s2/s4 before establishing glyph atlas coordinates. This is
observable in the original EXE with identical function arguments and different
caller register contents. `test_text_sprite_spacing.py` records that MIPS
dependency separately; it does not invent initial glyph coordinates or claim
a native-C oracle for this unresolved state. Fixed-width initial spaces are
independent of those registers. Further native models are still needed for
the full text loop and the changed menu/UI/link/intro consumers.

The resident controller-layout label at0x80042DBC now implements the actual
`PanelDraw` interface: the table's slot12 at0x8009F9B4 names it, and
`DrawBadgeRow` consumes its returned cursor. The old void definition discarded
that C-level return contract even though the last renderer's value happened to
remain in the MIPS return register. The implementation now explicitly returns
the full `AddDrawModePrim` cursor. `game/menu/mnlist.h` owns all three resident
panel callback declarations, the fraction formatter and the typed four-byte
`MenuStepArrowUv` UV record; the menu-label header re-exports the formatter.

The controller label reads `ControllerState5C.protocol10` and the existing
`RuntimeSettings` owner, replacing its three private prefix records and the
character-array declarations of `PlayerControllers`/`Options`. The standard,
calibrated and wheel records name their actual button-layout fields. The
settings-application function uses those same fields instead of casting the
halfword/byte views. Its two calls still read live settings separately.
`game/car/carstat.h` owns both pad-layout service declarations, re-exported by
the race-setup header and included by the frame initializer and overlay665.
Layout assertions live with the runtime-settings types rather than being
duplicated in a consumer. Selector subtraction wraps as an unsigned word.
Existing register/barrier constraints and the original defensive index clamp
remain explicit reconstruction debt; no new constraints were introduced.

`test_controller_panel_callback.py` checks65,536 native cases under UBSan and
1536 original/linked-MIPS traces. It invokes the actual callback through a
full-width `PanelDraw` table and checks the original table entry. Complete
settings, two controllers, two guarded six-row arrays, the glyph and two packet
arenas support both controller slots, negative indices into valid row prefixes,
all observed/default protocol branches, selector boundaries and signed layout
values. Instrumented services change live roots/settings/glyphs and return
different full native cursors. Every service state, all eight local text bytes,
the final owners, return cursor, saved registers and store/stack bounds are
checked. Independent references decode the original field offsets. Aggregate
input/trace hashes freeze the full case matrix for future owner migrations;
renderer and pad-device implementations are outside this fixture.
The race-setup fixture's declaration generator also imports `rr/player_cars.h`;
its original 131,072 scenarios, transition model and assertions are unchanged.


`DrawStartCountdown` now declares its ordering pointer and signed frame in
`game/race/racegrid.h`. It threads a full packet cursor through the sprite and
area/mode emitters; `game/ui/badgeprim.h` and `game/ui/badge1.h` own those public
interfaces. Countdown UVs use `UiUvRect`, and the four adjacent X offsets have
one array owner, including the former scalar aliases at its last two bytes.
Frame-range subtraction wraps as a MIPS word; negative tick scaling uses
bounded multiplication rather than a negative signed left shift.
`test_start_countdown_native.py` exercises the actual body in 16,384 native
cases and 512 original/compiled MIPS cases per image, checking complete frame
banks, arenas, ROM/scalar owners, callback changes to roots and local/shared
UVs, every service entry/exit, full native pointers, and ABI/store guards under
UBSan. GPU service implementations are covered separately. The older pause
menu's aggregate/scalar triangle-argument mismatch remains unresolved; a
matching caller declaration alone does not settle that interface. Its scratch
cursor and helper returns now preserve native pointer width.


`GetTrackTargetHeading` now removes the redundant identical-branch condition
and its former `$v0` pin together. Node adjustments, heading subtraction,
trigonometric products and position/direction differences explicitly preserve
MIPS word wrapping. The unit imports the public trigonometry/math interfaces.
One pre-existing half-turn constant pin remains; removing it still changes the
relocated instruction sequence. Its native branch uses an ordinary C local.
`test_track_target_native.py` compares the actual body against original or
compiled MIPS in 8,192 cases per image, using complete serialized vehicle
owners, full native argument/output pointers, signed extrema and random word
inputs. Services mutate the car, track count and local interpolation output;
all arguments and service-entry/exit states, final result, saved registers and
stack/store guards are checked under UBSan. Interpolation and trigonometry
implementations are instrumented here; track counts are positive, as required
by the live track state. The shared signed-DIV emulator extension is independently
cross-checked against Unicorn by the existing camera-track fixture.


The controller bank now has one public owner: `ControllerState5C` and
`PlayerControllers[2]` in `game/system/pad.h`. The former menu-specific
`MenuItemState5C` view is removed. Its protocol/action/repeat fields use the same
names as the pad driver. The header, edge buttons and four analog bytes expose
both their real packed words and their component fields; code no longer casts
an unrelated controller prefix to read these words. The link handshake uses a
pointer to the actual two-controller array, including its second controller's
`action28` and `action2C`, rather than the private `T38` view. HUD and card-menu
controller selection use normal typed array indexing.
The same header owns the two complete `PadPort28` input buffers and two complete
`PadPulseState18` records. Pulse counters are unsigned halfwords; signed terminal
comparisons remain explicit. `InitGamePads` clears the sizes of the real arrays
and passes byte representations of the two complete port objects to the SDK.
`psyq/libpad/padgate.h` owns `PadInitDirect`'s public interface.

The 28 adapted native modules retain their original numeric scenario/model
matrices, seeds, loop bounds and assertions; ignored audit records compare
those matrices before/after the member-path changes. Native model controller
rows remain 92 bytes and pulse records remain 24 bytes. The HUD and pulse
fixtures now select their actual definitions through `source_owners` and
`definition_source`, rather than compiling frozen bodies. The pulse fixture
also retains the actual empty barriers; its existing register bindings have
ordinary native C branches, with the MIPS bindings unchanged.
`test_game_pad_init.py` adds 4,096 native/original and native/compiled MIPS cases
covering the complete guarded controller/pulse/port banks, adjacent driver
storage, exact clearing ranges, helper mutations, full transport pointers,
call order and ABI/store guards under UBSan. The driver services themselves
are instrumented here and covered separately by the existing channel fixtures.

The overlay662 course-controller fixture imports its specific public controller
header. Source include order does not select the fixture's declarations.


`PadPort28.analog04` uses the same four-byte word/byte representation as the
published controller axes. The pad update copies its word directly, and the
calibration controller reads its unsigned bytes. The former `u32 *` casts from
individual packet bytes are removed. The port's wire size remains 40 bytes,
with the analog payload at4 and the remaining response bytes at8; the real
word access is now represented by the port type's natural alignment. The two
SDK input pointers still refer to complete 40-byte port objects.
No scenario, reference model or test-module source changes accompany this
payload migration. The guarded initialization fixture also asserts that the
complete adjacent pulse/port owner and its native wire offsets stay unchanged.

The pad SDK now uses one `PadChannel` owner in `rr/channels.h`. Its PAL size
remains 0xF0; native pointer-bearing fields retain their full widths. The former
status, transfer, patch-command, actuator-layout and metadata-loader views are
removed. Modes, actuator descriptions, combination descriptors, multitap children,
owner, update/completion callbacks, alignment, output/input and packet buffers
refer to their actual objects. Transfer index45 and control phase46 remain
separate. The output area54..62 has one contiguous 15-byte owner containing nine
gates and six output IDs. Payload pointers derived from its byte array retain
valid prefix storage when the transmitter accesses indices-3..-1.

`PadGetState` forwards its selector through the actual bank lookup. `PadSetAct`
and `PadSetActAlign` accept native buffer pointers, and the alignment function
returns the actual readiness result consumed by the game. Controller output19
is a real two-byte array used by the pulse writer and the SDK. Driver stage0
and stage1 return their serial-transfer results. The byte-provider callback has
two arguments: the driver computes its mode explicitly, while the installed
transmitter selects bytes from channel state. Public declarations, stored
callbacks and implementations now agree without function-pointer ABI casts.
`PadInterruptQueueRecord` in `psyq/libpad/padenab.h` owns the next pointer,
two typed interrupt callbacks and trailing word; `_padSetVsyncParam` fills
those actual fields rather than storing callback addresses as integers.

Actuator metadata retains the original 128-byte PAL protocol budget. Native
combination descriptors need larger pointer slots and stronger alignment;
`metadata63` provides their additional physical storage. `padmeta.h` translates
the scan boundary to that storage without increasing accepted packet capacity.
The discovery parser, allocator, repeat detection and installed metadata
callbacks operate on the same complete channel owner. PAL layout assertions
remain enabled for MIPS; pointer-free layout assertions also run natively.

The bank selectors walk complete owners and retain the original mask: any bit
in `selector & 0xF0` selects bank one, with other bits ignored. Reverse lookup
recognizes only bank bases, returning 0x10, 0x20 or 0xFF. The selector fixture covers
524,288 selectors and every byte address in both native records.
The new ASan/UBSan fixtures exercise 56 game-to-SDK actuator flows, 1024 state
forwarding checks, 655,360 wire-size cases, 9,364 allocation/readiness cases,
8,960 bounded reply scans,4,096 complete discovery/allocation/replay handshakes
and 4,096 interrupt-callback installations with typed invocation of both slots.

Existing native cases retain their original models, seeds, loops and aliases.
`channel_legacy_inputs.h` describes only frozen test inputs, never production
views. The constructor still consumes its original 800-byte native input World
and 16,384 alias cases; gate selection retains its original 1632-byte World and
32,768-case RNG stream. Explicit bindings translate their active aliases to
actual SDK owners, then check both the original byte oracle and every byte of
the expanded native owner. The transfer fixture retains its 104-byte input
surface, all command/phase/length cases and 10,000 random records. Other channel
fixtures share the actual native owner through `channel_native_types.py` and
select current function definitions. `matrix_preservation_report.json` in the
verification bundle records the unchanged numeric test matrices and identical
original C reference/RNG/main functions after binding-name normalization.

`_padAdvanceChannel` indexes the actual `PadChannelRecordArray` and multitap-slot
arrays directly; its former integer-address reconstruction is removed. The
three distinct SDK slots `PadSio0Base`, `JOY_DATA` and `D_800ABD00` all initially
point to SIO0 at 0x1F801040 in the original EXE. They share the existing
`SerialPortRegisters` type while remaining independent mutable pointers.
Channel advancement writes `control`; the packet driver reads `baud` and `data`.
The previous private packet-driver register view and anonymous halfword
index5 access are removed. Existing compiler constraints remain reconstruction
debt; this change introduces no new bindings or barriers.

`test_pad_packet_transport.py` exercises the actual transmitter, command saver
and packet driver under ASan/UBSan with complete native channel/child owners.
It checks 2,080 successful traces and 96 early read/timeout failures across both
banks, multitap completion scripts, baud modes and live SIO pointer changes.
The existing channel-advance fixture retains its two original 24-byte word
oracles, now sharing storage with actual SIO owners. Its exhaustive cases and
the constructor's 16,384 original alias cases remain unchanged.


The projection prefix embeds a real `ProjectionView32` at `pose08`, holding the
position and full-word Euler-angle vectors. Camera producers, scene consumers,
model selection and overlays share that subobject. `model.c` no longer invents a
private `ViewPosYaw` prefix or a scratch layout with integer pointer slots.
`GetViewTrackNode`, `GetViewTrackSegment` and `GetSegmentViewTable` declare and
forward their actual position/player inputs. Player selection is explicit through
`BuildViewSegmentObjectList` and the player/mirror view entry points. Visibility
lookup indexes `RelativeSlot.absolute`, retaining full native pointers instead of
multiplying by a PAL pointer size. The segment/yaw output owns both halfwords.

`YawToOctant` expresses rounding with ordinary division. `GetTrackSegmentAndYaw`
uses unsigned subtraction before its12-bit mask, preserving PAL wrap without
signed-overflow dependence. These changes introduce no register pins, barriers,
compiler settings or alternate ABI declarations. The relative-file header's fixed
PAL size/offset checks apply to MIPS; native relocation slots hold real pointers.

`test_view_track_selection.py` executes the actual node/segment/octant/table chain
with4096 native cases under UBSan/bounds and8192 original/linked MIPS executions.
It covers both players, signed yaw/node boundaries, all selection modes and live
replacement of the camera yaw, track-bank root and visibility-bank root by the
geometric search service. Native owners and pointer targets remain complete and
above4 GiB. Whole-owner guards, exact output stores, stack guards and saved
registers are checked. Only geometric search is instrumented. Seventeen older
fixtures have exactly reversible member-path adaptations, preserving their full
cases, models, aliases and assertions;426 older modules are unchanged.

The visible-object C list builder and appender now retain full native pointers
through a common counted owner, described below. Horizon rendering remains
handwritten assembly. Parts of the articulated-model path, Fog arithmetic and
the model-bank loop condition still need source reconstruction.


`course_view_types.h` describes the actual visibility input and output formats.
The nine PAL course resources contain7,632 segment/octant lists and1,095 records
of80 bytes. Each record carries16 primitive ranges after four reserved words;
the ranges store a halfword count and first primitive index. Across every record
and range, the first index continues the preceding record's count. The reserved
words are preserved. Original range names have not been recovered.

`CourseViewRecord` is a packed wire descriptor: the relocator transports arbitrary
serialized byte displacements, so its pointer representation must also accept
unaligned addresses without requiring a native record read. Actual PAL geometry
records are aligned in the disc data. `RelFileViewRecords` has this one shared
pointer declaration. `CourseViewIndices` describes the word count and flexible
halfword indices consumed by `GetSegmentViewTable` and the C list builder.

The output `VisibleCourseViews` owns255 pointer slots and its count, matching the
0x400-byte PAL interval before the root-pointer symbol. The `stream` view describes
the counted slots consumed by assembly; the `objects` view describes the C count
and record array. Both live in the same explicit union, with the pointer-array
start checked against slot size on PAL and native hosts. All pointer cells hold
complete native pointers. The appender's cursor changes from an owner pointer
while reading its count to a pointer-array cursor while appending; its local union
records that change without pointer/integer casts or mismatched pointee views.
The primary list, active root and selected input packet have one owner header.
The extra-record input owns its count byte and31 index bytes.

The builder retains its original unsigned clamp to255. The appender compares each
extra record only with the initial active list, publishing the new count at the
end. Repeated extra records absent from that initial list therefore remain
repeated. A geometric-search service may replace both the active list and record
base before the builder resumes; the C definitions preserve those live reads.
`SubmitCourseHorizon` receives the real list pointer through its explicit assembly
boundary declaration. Its handwritten implementation is unchanged.

`test_visible_course_records.py` decodes complete native file/relocation owners
and executes the current relocator, camera selector, builder and appender under
UBSan/bounds. All7,632 original lists and96 synthetic count boundaries run through
this chain, including0,254,255,256 and full-word count extremes. Both players,
selection modes, live list/record root replacement, mixed-bank pointers and extra
duplicates are covered. Native file and owner guards include pointer padding and
all unused slots. Another1,022 original/linked MIPS flows and18 actual relocations
check complete PAL lists, serialized file state, globals, stack and saved registers.
Only geometric search is instrumented; no target code or emulator is patched.

The existing relocation fixture retains its full131,072-case input/RNG/reference
matrix, including arbitrary backward and unaligned byte displacements. Its second
published pointer now uses the public packed wire-record type; member order and
pointer widths are unchanged. It selects the actual current function body instead
of a frozen generated view. That body differs only by its typed pointer transport.

The course-light initializer and builder share the real `MatrixWords32` color
matrix, three `Vec3sFields8` directions and three halfword strengths through
`game/camera/viewini.h`. The direction records include their fourth reserved
halfword and retain their natural halfword alignment. The initializer copies
these actual objects; the builder uses a matrix, short input vector and full-word
result vector instead of private arrays and interleaved halfword stand-ins.
`RotMatrixYXZ` has an explicit pointer interface, including its actual output
pointer return. Its handwritten assembly implementation is unchanged.

`SetColorMatrix` takes a const matrix pointer in its definition, public header
and every C caller. GTE submission transfers five words; the final matrix control
register retains the signed low halfword, so native adapters read the nine
defined coefficients without inspecting uninitialized high-halfword padding.
The RGB SDK setters shift unsigned words before writing their control registers,
preserving PAL wrapping for all signed input bit patterns without signed-shift
undefined behavior. Other geometry setters and Fog arithmetic still need review.

`test_view_lighting_pipeline.py` runs the actual initializer, builder, submission
and SDK bodies under UBSan/bounds. It covers 8,192 native flows, 664 original and
linked MIPS flows, and 524,288 matrix padding/coefficient transfers. Only the
handwritten trigonometric service is instrumented; it also mutates live lighting
inputs to check reloads and call order. The fixture checks complete software
owners, defined GTE state, signed RGB wrapping and native/target guards. GTE FLAG
and timing are outside its functional model. Existing scene and track lighting
fixtures retain their case matrices and oracles, with only the public matrix
pointer binding adapted. The animated renderer in `poly1.c` shares these lighting
fields, but its private object layouts and integer addresses remain unresolved.

`rr/animated_track_types.h` replaces the incompatible integer initializer and
renderer views of animated track objects with one owner. The original course7
scene reserves five banks of255 particles (36 bytes each), five248-byte PAL
states, two sets of five255-packet Gouraud line lists, and18 relative color-key
offsets. Nine key pairs resolve inside that same resource; the first sequences
have26 or27 entries and the second sequences contain one sentinel. The GPU
packet and color-key layouts stay fixed-width. The state contains real particle,
key and packet pointers; native layouts follow their actual alignment and stride.
Palette initialization uses word slots while color interpolation uses the shared
byte command type through an explicit union. Particle vector copies preserve
the fourth halfword, including when only three components were just transformed.

The actual initializer derives each pool from the complete arena layout and
writes typed heads/tails. Its buffer-index register binding is removed. State
updates and matrix setup share that same object; the C renderer uses real packet
cursors and the shared scene scratch prefix. Its stack matrices, short direction
and transformed vectors have their actual types, and local names describe their
roles. `VSync` uses its public single-argument interface. Translation addition,
color-tick shifts and tick increment retain PAL word wrapping without signed
arithmetic undefined behavior. Other geometry arithmetic remains to be reviewed.

`CatPrim` and `AddPrims` accept complete native packet pointers and encode them
only when writing GPU DMA link words through `GpuDmaEncodeAddress`. Their actual
bodies and all C callers share those declarations. `DpqColor` takes the shared
color-command type. The handwritten particle emitter has an explicit pointer
boundary, including its returned cursor; its instructions are unchanged.
`SceneLoadStep` retains the full address for the initializer while publishing
the existing PAL wire word. Its other encoded-address loading operations and
native resource allocation/expansion remain unresolved, as do the surrounding
legacy scratch views in `DrawTrackObjects`. This is not a complete native game
asset loader or a native implementation of the handwritten particle emitter.

`test_animated_track_owners.py` allocates the complete native arena with real
pointer widths, inserts all18 original color-key offsets and payloads, and runs
actual initialization, state, matrix setup and C drawing bodies. Real packet
setters/linkers and the color-submission body are included. Matrix/trigonometric
services, nondeterministic BIOS services and the handwritten emitter are
instrumented; a functional DPCS model excludes FLAG/timing. The fixture compares
4,320 pipeline cases and576 primitive alias cases with9,794 original/linked MIPS
flows, including initialization, complete serialized owners, inserted native
padding, service traces, native/target guards and UBSan/bounds. Cases cover all
five objects, nine styles, two buffers and both update modes; additional inputs
exercise sentinel/nonterminal second-color streams, reserved vector halves,
full-word tick edges and mutable translation/buffer state. Existing native test
modules are unchanged.

## Menu loading and allocation pointer owners

`rr/cd_directory_buffer.h` owns the reusable byte-buffer pointer, while
`rr/menu_arena_base.h` owns the active resource/workspace root. Directory words
and saved-card blocks are explicit views of the buffer contents; callers do not
redeclare the shared pointer cell as an integer, a word array or a card block.
`MenuBaseLoadCursor` also retains a full CPU pointer, including native builds.

All five menu row allocators accept `void *` buffers. They publish the actual row
pointer and return their existing scalar allocation size. `MenuRectRow10` is the
one rectangle record shared by initialization and animation. Its pointer stays
complete on the host. Numeric address arithmetic in its current consumers still
constrains the PAL register allocation and remains reconstruction debt; the width
check prevents it from silently becoming a 32-bit native address.

The original `cd_read_ptr` diagnostic at `0x8001075C` contains one conversion.
`InitMenuArenas` passes only its buffer address after the format string. The old
extra `0x2D000` argument and the register binding used to support it are removed.
`GetR4FileIndex` has two scalar arguments; its caller in `LoadSlotBankStep` no
longer appends a register-lifetime artifact as a third argument.

`test_menu_arena_pipeline.py` executes the current initializer, all five actual
allocators, the rectangle constructor and two real loading callers. It compares
original and linked MIPS with native allocations above 4 GiB, complete guarded
buffers, pointer publications, service-entry state and loader transitions. It
imports the public headers with their layout assertions enabled. The test's
address transport checks known complete owners and cannot repair a truncated
pointer. A separate negative-control probe confirms that such truncation fails.

## Dialogue state and page animation ownership

`rr/menu_text_state.h` owns both dialogue roots and the complete dialogue state.
The PAL object is 0x348 bytes: a script cursor, scalar control fields, an image
bank pointer at 0x20, the reserved word at 0x24, and 200 four-byte animation
cells at 0x28. The previous 0x340-byte record incorrectly folded the image
pointer into a 200-word array and omitted its final two animation cells.
`DrawAnimatedText` imports this owner rather than defining a shorter private
record with a single two-halfword animation slot.

Initialization clears `sizeof(MenuTextState)` and the loader reserves that same
native size. Both CPU pointers retain their actual host width. The renderer
indexes real animation cells and both page resets clear the declared 200-cell
array. Their C loops no longer construct pointers before an array or write ten
words beyond a nominal loop cursor. A distinct page-header byte lifetime also
removes the decompiler's unrelated reuse of one temporary while retaining the
relocated instruction match.

`InitDialogueState`, `SkipDialoguePage` and `DrawAnimatedText` expose the same
record type to their actual callers. The selection entry has no arguments; its
stage index is in 0..7 when selection mode4 dispatches through the eight cases.
Its settings/retry accesses use the shared scalar record, without carrying a
CPU pointer through `int`. Save-slot copies use that same record type. The
unused arguments to the selection-rank helper and dialogue update are removed.
The overlay665 controller also imports the shared owner instead of declaring a
five-byte surrogate for `CardDialogueStatePtr`.

`test_dialogue_pipeline.py` executes the current initializer, actual selector,
page-table loader, streaming loader, controller, skip helper and text renderer.
Original and linked MIPS are compared with complete native owners above 4 GiB,
including the native-sized state embedded before the loading asset buffer.
Transport maps actual pointer fields and animation cells to their PAL positions;
it neither narrows pointers nor restores missing address bits. Complete buffers,
guards, scalar state, pointer tables, callee-saved registers and helper-entry
snapshots are checked. A negative control rejects using the fixed PAL clear size
on the larger native object.

The existing menu-choice model, 131072 cases, legacy World layout and PRNG stream
are retained. Its adapter supplies complete native dialogue objects and maps
only legacy highlight byte4 to `MenuTextState.b4`, the sole accessed field. Every
other byte of each native dialogue owner must remain unchanged. New field access
requires extending that explicit adapter, rather than silently treating the
legacy eight-byte prefix as a complete native object.

The existing empty-asm constraint in the text renderer and remaining offset-based
field names are still reconstruction debt. This owner cleanup does not establish
that the wider C reconstruction is complete.

The GPU coordinate helpers `get_cs` and `get_ce` retain their public signed-word
arguments and explicitly narrow to signed halfwords before clamping. Their
former K&R parameter definitions are gone. The coordinate fixture now executes
the current production bodies; its original cases, oracle and separately
instrumented legacy read-order regression remain unchanged.

The scroll controller's countdown subtraction, scale shift and channel offset
addition use unsigned operands where the PAL operations wrap. Sprite-effect
and fade intensity products likewise wrap before signed division. Casting only
the result would leave signed overflow in C. These changes preserve relocated
instruction Levenshtein 0; the original native cases and UBSan checks are retained.




## Complete audio owner and bank descriptors

`rr/audio_state_types.h` owns the complete resident SoundState. The PAL layout
remains0xA8D0; native LP64 uses0xA920 with real pointers for the two stream tables,
two sequence data pointers and two complete SequenceChannel rows. Public size
and field assertions account for those pointer widths and natural alignment.
The initializer clears sizeof(*state), including every native pointer and tail.
SoundSlots names slots38 in this same owner; its one declaration is shared.

AudioBanks20 exposes the ten signed bank IDs and the genuine overlapping AI
counter at entries8/9. AudioSignedWord4 describes full-word bank/program stores
and signed-halfword reads. Consumers use these actual members. AudioStreamEntry
names the two-word on-disc entry, and stream-bank table pointers distinguish
null from the unavailable sentinel without converting pointers to scalar words.

AudioProgramData is the complete0x5BC resource record:two groups of ten curves,
two groups of five program IDs and the four following halfwords. SlotLoadCurves
copies non-overlapping source bytes into this actual nested record. The source
may be unaligned; sizeof the record and builtin memcpy retain both original
aligned and unaligned copy paths. Program/key-on/update consumers index genuine
program/pitch arrays and actual word/halfword groups, without shifted whole-slot
views or truncating pointers to stack work.

`rr/audio_bank_setup_types.h` defines the one AudioBankSetup descriptor shared
by the track loaders, SlotSetupBank and ComSlotSetupBank. Its first three fields
are resource pointers; the fourth is the player slot selector, unused by Com.
Both constructors import complete public declarations and read the live descriptor
again after services. Com no longer interprets native pointers as three integers.
Both use actual slots/program/pitch arrays and VagAtr outputs. SlotSetupBank's
old shifted AudioState views, register bindings, empty asm and dead offset
calculations are removed. Existing compiler settings remain unchanged.

The track VAB loader uses real resource-header/configuration records and the
sequence data pointer array. SsSeqOpen receives a real byte pointer and calls
the three-argument initializer; its raw word selector is explicitly normalized
where the original instructions narrow it. Original upper-half selector tests
and callback-driven pointer/root replacement remain.

Native audio fixtures preserve their original0xA8D0 wire protocol, models, cases
and PRNG streams while transporting real pointer fields and scalar spans into
the complete native owner. Native pointers exceed4 GiB; complete snapshots,
unused pointer cells, alignment gaps, exterior guards and callback reentry are
checked. The bank fixture compiles the current bodies without erasing target
constraints, with public layout assertions enabled. Player-slot cases remain;
new Com cases exercise both MIPS images and replace an owned curve payload
between its two copies. Other SDK entry-point declarations and existing compiler
constraints remain separate reconstruction debt.

## Axis rotation matrix interface

`game/math/rotmat.h` owns the complete public declarations of BuildRotMatrixX,
BuildRotMatrixY and BuildRotMatrixZ: a `MatrixWords32 *`, a signed word angle,
and no return value. All resident and overlay users import that declaration.
The constructors write the nine signed rotation halfwords through `rotation00`;
the alignment halfword and three translation words remain unchanged. Their
4096-unit angles retain the existing word normalization in the actual sine and
cosine helpers. The old local int-returning declarations, void-pointer constructor
interfaces and the chase-camera call's unused third argument are removed.

Existing 32-byte character work buffers in the race cameras and angle builder are
actual MatrixWords32 objects. Existing larger work capacities in camview and
trkintp2 expose actual matrix union members while preserving their old extents;
the untouched 32 bytes in InitDemoComCars and 8 bytes in each of camview and
CarLocalToWorld are not newly recovered fields.
Those unexplained capacities, other old scratch views and remaining register
constraints in the callers are reconstruction debt. No new padding, register
binding, barrier, volatile access or compiler setting is introduced.

`test_axis_rotation_matrices.py` compiles the current three constructors and
current sine/cosine helpers against the public types. An independent quarter-wave
ROM oracle checks every phase plus signed-word boundaries, upper-half patterns
and random word angles against native C and original/linked MIPS. Complete matrix
and exterior snapshots verify that alignment/translation cells and guards survive;
MIPS writes and callee-saved registers are checked. Existing scene/animated-object/
overlay observers use the same public matrix prototypes and preserve their prior
models, cases and PRNG streams.

## Shared SDK sequence rows

`rr/sequence_channel.h` owns the complete 0xB0-byte PAL sequence row and the
single declaration of `_ss_score`. `LibStateB0`, `ChannelStreamB0` and
`ChannelValueRecordB0` are spellings of that same C type. Its first five cells
are CPU pointers, including the start and alternate cursors previously declared
as scalar words in one partial view. Native rows occupy 200 bytes; field and
size assertions account for full pointer widths and remain enabled.

Sequence initialization, stream completion, tempo, volume and controller users
share this owner. Ten controller/note handlers now use ordinary table, row and
array indexing with the existing stock 970318 compiler and GNU assembler
profile. No compiler backend or match verifier is changed. The three volume setters in `ssvolset` likewise use typed bank/row lookups and
ordinary halfword value normalization, without register constraints or a PAL-page
lookup. The selected-channel accesses in `cc_62` use a byte cursor over the actual
owner and named field offsets. `sstick3` uses the stock 970318 compiler and GNU
assembler profile; these are reconstructed matching settings, not recovered
original build flags. Replay indexes the real sequence-row array. Stop copies
its saved start cursor in ordinary C, removing the artificial volatile pointer
reload. One numeric row-address expression in Stop remains reconstruction debt;
the direct array spelling still has relocated instruction Levenshtein 1, so it
has not replaced the matching expression.

The three getters in `vmchval` now index `_ss_score` and its real sequence rows
in ordinary C. The existing 2.7.2 compiler uses the stock GNU assembler profile
and disables the second scheduling pass for this TU; these are reconstructed
matching settings, not recovered original build flags. All three functions have
relocated instruction Levenshtein 0. Their public header declares the complete
argument and return types. The previous fixed RAM page and eight empty asm
statements are removed, and native fixtures no longer replace a physical address
or erase asm from these getter bodies. Their original 256-bank/256-row models,
upper-word cases, overlapping outputs and callback-driven ramp cases remain.

`test_sequence_channel_pipeline.py` executes the current table initializer, SEQ
initializer, delta decoder, volume getter and playback/pause/replay/stop/end/tempo
chain against both original and linked MIPS. It compares complete records,
streams, pointer roots, scalar state and callback boundaries, including root and
start-cursor replacement during callbacks. A separate replay/stop sweep covers
three banks, five signed relative rows, four upper-half input patterns and both
stable and callback-replaced roots/cursors against each MIPS image. Replay calls
supply its declared signed-halfword argument with the required sign extension;
Stop receives raw word arguments and performs its own narrowing. All original
chain cases remain. Native pointers remain above 4 GiB.
The older scalar fixtures retain their original 176-byte oracle protocols and
cases through explicit row/field transport into complete native owners. Their
production bodies come from the current source; pointer cells and native
alignment bytes are guarded rather than replaced with 32-bit fake pointers.


### VAB opening and transfer interfaces

`psyq/libspu/spumem.h` owns the public sticky/fake opening wrappers and their
allocation callback. The header/program buffer is writable: the parser patches
program records in that allocation. `AudioBankSetup.bank00`, `VabData.header`,
the resource producers and native service observers share that pointer type.
The wrapper takes a raw word bank, explicitly narrows it to signed low16 before
the parser call and sign-extends the parser result's low16. Both narrowing
operations are present in the original wrapper's instructions. `VsGetAddr`
implements the parser's three-argument allocation callback type; its size and
bank arguments are deliberately unused.

`SsVabTransBody` uses the real `_svm_vab_used`, `_svm_vab_start` and
`_svm_vab_total` arrays. The physical RAM page, numeric field offsets and old
register binding are removed. It captures the allocation before the mode call,
reloads the size after address selection, and publishes the final flag after
the write. The original low16 selector range 0..16 is retained.

`test_vab_sdk_interface.py` executes the current opening wrappers, actual
`VsGetAddr`, parser, busy-state and mode helpers, transfer wrapper and
`SpuWrite` on native
buffers above 4 GiB and against both original and linked MIPS. An independent
resource-byte oracle checks complete buffers, table publications, callback
arguments/mutations, rollback behavior, unsigned end-address wrap, pointer
widths, exterior/alignment guards and MIPS callee-save/stack preservation.
A separate wrapper observer covers high bank words and parser returns whose
high and low halfwords differ. The existing game constructor/loader service
observer tests remain separate; this SDK test does not claim to execute a
complete game-to-hardware sound pipeline.

`test_spu_bank_transfer.py` now extracts the current transfer and helper bodies,
uses the public SPU header with its assertions enabled, and preserves all
previous selectors, flags, address/size cases and callback mutations. Its table
and transfer-state snapshot protocols map named native array fields; no target
register annotations, barriers or physical-page expressions are erased from
these current bodies for host compilation.

`SpuSetTransferMode` uses ordinary global assignments without its old register
binding, empty barriers or physical page. Its switch and the completion wrapper
match together with stock gcc 2.7.2 and the stock GNU assembler profile. This is
a verified reconstructed configuration, not an identification of the original
assembler. `psyq/libspu/spumode.h` owns the shared mode/completion declarations.
`SsVabTransCompleted` explicitly narrows the raw wait word and returned status
to signed low16 at the boundaries present in the original instructions. The
wrapper observer also covers high wait words and differing status halfwords
against native C, original MIPS and the linked image.


### Shared VAB program format and resource views

`psyq/libsnd/vab_format.h` owns the complete 32-byte `VabHdr`, eight-byte
`ProgAtrAttributes` prefix and 16-byte `ProgAtr` record. The parser's word at+8
is the tone-block index; the halfwords at+12/+14 are paired SPU sample addresses.
The previous private parser/controller program structs, placeholder public
`ProgAtr` and `SpuProgram` byte-plus-padding view are removed. The controller
uses the same embedded attribute prefix as the parser.

`_svm_vab_pg` and `_svm_pg` have one canonical `ProgAtrBuffer` pointer type,
representing 16-byte resource records with byte alignment. Native publication
aliases can point at a selector byte or pointer cell, and allocation can read
an address halfword from a source aligned to 2 bytes. Treating those cursors as
complete word-aligned `ProgAtr` objects would introduce undefined behavior.
The parser uses the full record only on its existing word-aligned VAB buffer;
other consumers select named wire offsets derived from that same format.
No packed structs, alignment overrides, additional padding or register
constraints are used. The selected row retains its real 16-byte stride and
signed prefix indexing. `_SsVmKeyOn` also drops its old pointer-to-int program
address calculation; its other existing constraints remain separate debt.

`SsUtGetProgAtr` copies five attribute bytes and the aligned halfword at+6,
preserving output byte5. Its eight-byte output is no longer cast to a full
16-byte program object. Selection failure still does not cancel the copy.

`test_spu_program_selection.py` now compiles the actual selector, accessors
and attribute copy with the public assertions enabled. It uses a complete
properly aligned `SvmCur` owner, keeps the original PAL byte-snapshot protocol,
and retains every previous signed selector, byte value, pointer-publication
alias, overlapping copy and ignored-failure case. The selector-only source
alias may be odd-aligned; no fake shifted `SvmCur` object is formed.
`test_spu_voice_cache.py` extracts the current allocator, enables the public
layout assertions and preserves its original PRNG, 98,304 cases and independent
oracle, including the halfword-only program alias. Its preexisting host adapter
for the allocator's remaining register/page constraints is still explicit debt.
`test_vab_sdk_interface.py` additionally executes the actual parser, transfer,
selector and attribute-copy chain with complete native pointer cells and
`SvmCur`, original/linked MIPS, full resource snapshots and an independent byte
oracle. The original parser cases and resource PRNG streams are retained.

Race sound startup and shutdown now use the canonical scene/audio owners in
`sndcmd.c` and `sndctrl.c`. The startup controller no longer declares a private
`RaceContext` prefix. Slot shutdown indexes `SoundState.slots38` directly instead
of advancing a pointer to the whole `AudioState`. `SoundPaused` is the existing
PAL symbol at `SoundState + 4`, so all pause/resume/shutdown writes use the common
`muted04` field; the linker alias is preserved for address identification.

`test_sound_scene_owner.py` checks 1024 startup/shutdown/pause/resume scenarios,
including every initial combination of the four slot active flags and service
mutations of later-read configuration and slot state. Its independent model
compares complete native scene/audio owners and guards at every service boundary.
The same full-state trace runs the original and linked MIPS implementations;
three scene setters and reverb shutdown execute their real C/MIPS bodies.
Native owners are allocated above 4 GiB. Resource pointer cells remain null in
this controller test; resource loading/decoding is outside its scope.

The audio frame path in `sndslot.c`, `sndchan.c`, `sndvol2.c` and `sndvab.c`
now uses `SoundState` for slots, player voice records, master scale/gain and bank
IDs. The historical `SoundSlots` (+38), `PlayerEngineVoiceState` (+195C),
`VoiceVolumeScale` (+8), `SoundMasterGain` (+18), and three `AudioPan*Angle`
labels (+24/+26/+32, actually bank-ID slots0/1/7) remain linker address names, not
separate C owners. `SoundUpdate` advances a real `AudioSlot620` pointer rather
than stepping from one flags member to another. Player voice parameters are a
shared typed three-record table with program, unknown word and gain fields.
Descriptive locals replace register names in slot/voice updates and panning;
the panning path uses an actual signed16 conversion instead of its shift idiom.

`test_audio_dispatch_owner.py` exercises six actual C functions through 1024
reset/set/frame/voice/panning scenarios on native, original and linked MIPS.
It checks full audio/settings/table state at every controlled service boundary,
all sixteen combinations of slot-active flags and player modes0..3, both viewport selection modes, dynamic
slot changes, signed parameter conversions and wrapped hardware volume scaling.
The native full owner and guards include native pointer cells/alignment gaps;
resource pointers are null in this test. The master fade, per-slot synthesis,
CD/sequence/listener and remaining audio controllers are controlled services,
not additional bodies claimed by this pipeline test.

`test_audio_pan.py` now reads the current `TriggerSfx` definition. Its original
fixture World, independent arithmetic model, PRNG and 131072 cases are unchanged.
A gain/bank-only adapter transfers fields to/from a complete native `AudioState`
while guarding all remaining bytes and valid full-width pointer cells. SDK
snapshots still use the original fixture protocol. Native compilation retains
UBSan and additionally checks bounds and warnings without suppressions.
The preexisting ignored-argument call from `SoundMasterFadeStep` to `SoundStopAll`
and remaining compiler constraints/manual rounding sequences remain explicit
reconstruction work; this audio-owner migration does not retire them.

Two failed concurrent canvas cleanups were rolled back to their previously
verified bodies: pointer/configuration identity operations and an uninitialized
second-flag read are not accepted replacements for compiler constraints. The two
preexisting empty barriers were restored in that verified rollback. Other canvas
functions were preserved. A subsequent concurrent, semantically valid rewrite
of the option flags removes one barrier but currently has relocated instruction
Levenshtein distance2; its full match remains unfinished. The existing update-controller test now rejects compiler warnings
and checks bounds; a negative control rejects the uninitialized read. Its two
frame-root assignments now select the complete `FrameStorage` owner at the
same asserted native address, preserving the original World/model/PRNG/cases.

`PatchModelBankTexPages` now uses a plain unsigned positive-count loop.
The previous loop predicate added a local zero twenty times to inhibit compiler
loop optimization; that workaround and its unused temporaries are removed.
Named texture-page/CLUT masks and the atlas page describe the real packet-field
operations shared by the raw and environment-mapped paths. No compiler flags,
register bindings, memory barriers or dummy operations replace the workaround.
The entire `cdfile2.c` TU retains relocated instruction Levenshtein distance0.

The existing model-bank lifecycle test is unchanged: 4576 complete native
registration/setup/selection/patch/detachment cases, including full-width pointers,
backwards relative offsets and repeated command streams. Original and compiled
MIPS additionally run 208 fixtures each on both the instruction runner and
Unicorn, checking complete snapshots, ABI and ordered stores. The sibling mesh
texture patcher and track-resource controller tests also remain unchanged.
The legacy integer `DrawMeshPrimList` interface and its model/resource owners
still need a separate coordinated reconstruction; this loop cleanup does not
claim to migrate that path.

## Card header and save-buffer interfaces

The memory-card header builders use one `CardFileHeader280` layout: the title
starts at `+4`, the 16-colour palette at `+0x60`, and the icon payload at
`+0x80`. Animated headers write three 128-byte frames. Static headers write
one frame and three copies of a 128-byte extra block, ending at application
data `+0x280`. `CardFileImage4000` describes the complete 16 KiB image used
by the card-copy mode. Size and member offsets are checked on both compilers.

The title is a real `const char *`, and the extra block has a shared type.
`SaveBuffer` has one `u8 *` declaration in `rr/save_buffer.h`: its allocated
arena carries alternate serialized layouts, so consumers select the relevant
`SaveBuf`, `SaveBuf11`, header or complete-image view at the access boundary.
The byte arena declaration does not imply that the remaining readers are
native pointer-safe; two integer-address expressions in `StepCardDataTransfer`
still need reconstruction.

The animated builder no longer contains the permuter's discarded address-of
expression. Its local changing X coordinate is a signed 16-bit GPU coordinate;
advancing it wraps through the same low 16 bits written by the original code.
The old compiler still verifies the complete three-function TU at relocated
instruction Levenshtein distance 0. The static builder retains its existing
single-iteration loop; removing that loop changes the current compiler output.

The four card read/write APIs carry full name and buffer pointers, including
`const` source-buffer views for writes. Their path storage is a character
array. `PsyqSprintf` has the actual implementation's signed-character variadic
prototype, and `DrawSync` is declared by its owning GPU unit. The formatter
implementation itself remains a PAL stack-varargs implementation; declaring
its true interface is not a native port of that implementation. Existing
register annotations and barriers in the offset I/O functions remain debt.

`test_card_header_pipeline.py` compiles the actual two header builders,
`CardSaveStep`, `CardReadFile` and `CardWriteFile` without production-body
rewrites. Across 256 scenarios, an independent byte model checks native C
with full pointers above 4 GiB and original/linked MIPS with the instruction
runner. It checks complete buffers, title and extra inputs, rectangle and
selection owners, callback snapshots, stack/register ABI and guards.
Scenarios cover coordinate extremes, extra data aliasing the icon frame,
both save initialization modes, all six controller states, retries, create
and overwrite I/O, and pending reads/writes. Formatting, GPU transfers,
`BuildSaveData` and BIOS services are controlled boundaries; this test does
not establish the native correctness of the formatter or save-data writer.
The native build keeps warnings as errors and undefined/bounds sanitizers.

## Persistent menu records and card blocks

`SavedMenuSettings` is the complete 44-byte owner shared by `SelectionState`,
`SaveSlotSelections[3]` and `SaveSlotSnapshots[3]`. Their only declarations are
in `rr/save_slots.h`. The runtime fields remain `MenuSettings2C`, selected
through `.fields`; signed/unsigned halfword and byte arrays describe the same
serialized record. Checksum halfword8 is at `+0x10`. This replaces the former
`Struct2C`, byte-array declarations and three private card-transfer projections.
Existing locals that operate on runtime fields point to the actual `.fields`
subobject, rather than casting unrelated prefixes of the global.

`GameSaveData1AA8` owns both the structured game-save fields and the complete
word array used for its checksum. Named fields identify progress, options,
course choices, slots, lap times, lap/race records and emblem data. The header
is the shared `CardFileHeader280`, and time records use `TimeRecord16`. The
alternate `EmblemSaveData1380` contains the edit-frame count and two real
`EmblemTim840` blocks. The name-only import paths use character-array pointers
and the actual progress-name member.

`CardSaveBlock2000` owns a complete 8 KiB card block. The writer's initial
clear continues through `+0x2000`, beyond the shorter game-data structure,
so it uses the block's actual word array. The 256-byte header-scan window
receives file bytes `+0x300..+0x400` at arena `+0x1F00`; its three records begin
at window `+0x60`. Size and offset assertions cover these views. Whole-save
checksum accumulation uses unsigned32 wrapping, and record checksum accesses
stay within the declared 22-halfword arrays.

Eleven existing native fixtures migrate only their owner type, dependency
include, field selectors and one whole-owner binding. Reversing those exact
transport changes reproduces each previous fixture text; models, Worlds'
record sizes/alignments, cases and PRNG streams are preserved. No frozen fixture
contents or symlink targets change.

`test_save_record_writer.py` additionally executes the current complete
`BuildSaveData` body without body rewrites or substituted services. Across
256 scenarios and nine mode values, an independent serialized-byte oracle
checks native C with undefined/bounds sanitizers and warnings as errors,
original MIPS, and linked MIPS. Both buffers, every input and record owner,
all output bytes, real full-width source pointers, pointer guards and MIPS
register/stack ABI are checked. Source TIMs can be independent objects or
actual TIM subobjects of either buffer, including forward-copy aliases.

Remaining reconstruction debt is explicit: `StepCardDataTransfer` still has
two legacy integer-address expressions and an unused 56-byte stack array.
Ordinary pointer forms currently give relocated instruction Levenshtein
distance2; fully selected scan-record forms give distance29. Neither failed
probe is promoted. The selection-history backing and historical scalar aliases
need further owner reconstruction. The writer test alone does not establish
native correctness of those consumers.


## Shared custom-emblem images

`rr/emblem_tim_types.h` owns the 0x840-byte TIM representation. `EmblemTim840`
retains byte alignment for unaligned source files; `EmblemTimImage840` supplies
aligned word and palette views. `CustomEmblemImages1080` owns both images
initialized at the custom-buffer base and base+0x840, with complete byte and
word arrays spanning the pair. `rr/emblem_images.h` declares that pair and the
separate `SaveDataWork` image. Resident initialization, save construction/import,
backup/restore and garage code share these owners instead of incompatible
char/int/single-image externs. `InitTimTemplate` accepts an actual image pointer
through its owning header.

`ApplySaveData` retains the complete custom-buffer pointer while selecting its
serialized destination. Its byte view spans both real images; word accesses
are aligned and stay within the owner. The game-save checksum field is unsigned32,
matching the wrapping calculation. Header-frame imports select the complete
card block's first word. These changes do not repair `StepCardDataTransfer`.

`test_save_import_pipeline.py` executes the current `InitTimTemplate`,
`BackupSaveData`, `RestoreSaveData`, `BuildSaveData`, `ApplySaveData` and
`CopyStringClearTail` bodies in native C and original/linked MIPS. Its 256
scenarios cover eight import modes, valid/corrupted checksums, actual TIM
subobject aliases, both custom images, the work image, and pointer/root changes
at the controlled audio boundary. Template, copy, constructor, string and import
operations run their actual code; only audio application is substituted.
Explicit card-input transport is fixture I/O after verifying the preceding
operation's complete state. Native roots remain above 4 GiB. The checks include
warnings as errors, undefined/bounds sanitizers, complete owners, outer guards,
and MIPS stack/register ABI.

The older writer and TIM-compositor tests retain their original models, Worlds,
cases and PRNG streams. Their native transport binds real paired-image owners
and preserves known aliases; additional guards check the second image and
workspace. Reversing these explicit transport and selector changes restores
the complete previous test texts. Frozen fixture contents and symlink targets
remain unchanged.

The garage flood-fill caller retains its existing scheduling barriers and
legacy local-variable workarounds. Its replacement-color shift now operates
on unsigned32, removing signed-shift UB while preserving its relocated match.
This is not a claim that the flood-fill caller is already clean reconstructed C.


## Card-port channels and completion status

`CardUpdate(port, slot)` takes two integers. Its BIOS channel is the numeric
selector `port*16+slot`, not a RAM address. The former `char *` argument/local
and pointer-to-int expression were incompatible with the real caller's low-byte
slot and with `_cardCmd5ABegin` and `_card_clear`. BIOS `_card_info` and
`_card_load` now have explicit integer-channel declarations in `rr/bios_devices.h`.
The owning game header declares the five complete two-port state arrays and
the 48-byte raw-reply scratch allocation; each port's response window is17 bytes.
Named phase constants describe the controller's existing seven states.
Result-range checks perform unsigned32 subtraction, preserving word wrapping.

`MemCardSync` has one typed three-argument prototype and ordinary named
parameters. Its switch no longer nests a case label inside an else block or
uses the completion goto. `CardCommandStatus` has a single volatile declaration
in `rr/commands.h`: the IRQ writes it and the synchronous mode polls it.
The pointer-containing command types retain PAL size/offset assertions on MIPS;
native consumers retain complete host-width pointer fields.

Twelve command-start writes now address `CardCommandStatus` directly instead
of casting a fixed RAM page. The IRQ's two counter writes use `CardTxCount`, and
reset writes use `CardCommandRequest`. SIO control stores retain the actual
volatile hardware lvalue. The IRQ prepares its event argument on the retry and
no-retry paths after publishing completion. Existing register/scheduling
constraints in that handler remain explicit debt; this does not add constraints
or establish native interrupt timing.

`test_card_port_pipeline.py` runs the actual `CardUpdate`, `MemCardSync`,
`PollHwCardEvents`, both event-clear helpers, `WaitSwCardEvent`, and `_card_clear`
bodies without rewriting them. Across4608 scenarios, native C and original/linked
MIPS execute both ports, all256 slot values, all seven phases, unknown phases,
event priorities, pending/completed command outcomes, timeouts and loaded-card
branches. BIOS event/info/load/new-card/write calls and command-start calls are
scripted boundaries; `MemCardSync` is real code. Scripted callbacks can change
port state, counters and reply contents. Complete owner snapshots, service
arguments, full native buffer pointers above4 GiB, untouched bytes, and MIPS
store bounds/stack/register ABI are checked. The command-sync calls use mode1;
the test does not simulate an interrupt waking a native blocking mode0 call.

Four older command fixtures change only the status qualifier, public-header
filter and qualified RAM-cell binding. Reversing those exact transport changes
restores their previous texts, preserving models, cases and PRNG streams.
Frozen fixture files and symlink targets do not change.

## Sprite and tile packet owners

`GpuSpriteCommand` and `GpuTileCommand` own complete GPU packets, exposing the
existing coordinate/color fields and packed command words through the same
shared union. `spfade.c` uses these owners instead of its private word-only
packet structs. Tag lengths use the shared DMA tag member.

`EmitTitleMaskSprites` now keeps the packet cursor and ordering-table root as
full pointers through `GpuScratchPointers`. Its public interface names the mask
and animation, and the vertex declaration names the eleven consumed positions.
CPU pointers are encoded into GPU link words through `GpuDmaEncodeAddress`;
the native backend uses registered real buffers and explicit PSX CPU aliases,
without reconstructing native pointers from truncated address bits. Animation
negation wraps in unsigned arithmetic before the signed phase shift, including
`INT_MIN`. The emitter retains its original order of packet and head writes.

`test_title_mask_packets.py` compiles the actual current function body and real
native DMA backend without body rewriting. It compares native C, original MIPS
and linked MIPS in 8,192 cases covering all 2,048 low masks, high mask bits,
animation extrema, both packet arenas, live ordering-head/packet aliases and
empty roots for empty masks. Full packet arenas, input vertices, root pointers,
guards, MIPS stores and register/stack ABI are checked. Native pointer roots
are separate from GPU RAM; this fixture does not model wire writes into host
pointer cells. The preset harness now extracts the actual current definition
from its owning TU and binds the hardware scratchpad explicitly. Its historical
World, independent oracle, cases, PRNG and test methods remain byte-identical,
including the pointer-cell alias cases. Its existing native register-annotation
adapter is unchanged. The former frozen function source is retained unchanged.

The existing register pins and empty barriers in the other three `spfade.c`
emitters remain reconstruction debt. Sharing packet types does not establish
that those functions have clean dataflow or full-width native pointer handling.

## Recovered game call contracts

Game callers use their defining unit headers for vehicle reset/transforms,
chase-camera setup, sound setup, SFX requests, menu controls and formatters.
Declarations that disagreed with the actual void return or omitted parameter
types have been removed. `SetModelTexturePage` owns its public scalar state
and accepts only the page word. `InitCameraState` calls that actual interface;
its former three additional arguments were observations of unused MIPS
registers, not inputs consumed by the setter. The historical camera fixture
retains all old scenarios, oracle images, hashes and PAL register checks. Its
native observation adapter reads the equivalent already-published camera
state. A separate fixture compiles the actual initializer and actual setter
together, with only the BIOS clear boundary controlled.

`EmitSpinningWheelIcon` now receives full ordering/cursor pointers and returns
the actual sprite builder's cursor. Its packed animation arithmetic wraps
explicitly before signed remainder/halfword narrowing. The course-info caller
keeps the returned scratch cursor as a pointer. The new contract fixture runs
the actual icon and sprite chain with seven additional C helpers, an independent
packet oracle, full native buffers and original/linked MIPS. Native GPU linking
is a controlled boundary; MIPS executes the original/linked AddPrim too.

`FrameTaskState` owns the sixteen-byte prefix reserved after the TIM list:
frame, three intensities, three ramp steps and a sprite tick. It exposes the
existing grid-clock view at the same address. `FrmTaskModeUpdate` now passes
that explicit clock to `EmitSpriteGrid`, as required by the original MIPS
argument setup. Grid packets use the shared complete `GpuSpriteCommand`
instead of the intro renderer's private pointer-bearing packet view. The old
grid fixture now extracts the actual current function, preserving its complete
World, byte oracle, all tick/color/alias cases and PRNG unchanged. The rest of
the frame-task renderer still has legacy integer scratch accesses; this pass
does not establish full native portability of that caller.

`ScreenPosition` shares the signed XY/packed-word union previously private to
the RRO renderer. Remaining aggregate triangle arguments, the extra menu-frame
sprite argument and the progress decoder caller still need dataflow recovery;
failed candidates for those consumers are not promoted. Existing register pins,
empty barriers and compiler profiles are unchanged by this interface pass.

The fade-sprite fixture imports AddSpritePrim from its real public header rather
than searching the caller for a local declaration. Its controlled native helper
uses that same void-pointer signature; its oracle, World, effects, input cases
and PRNG are unchanged. All native owner/observation adapter migrations in this
pass have exact reversible normalization records, with frozen fixtures intact.

## Frame-task controller scratch ownership

`FrmTaskModeUpdate` now accesses scratch cursor and ordering roots through
`GpuScratchPointers`, and the final eight-byte GPU command through the shared
command/tag types. It encodes the hardware link with the real DMA address
backend and captures the current head before encoding the cursor. No CPU
memory access reconstructs a pointer from a truncated integer. The old local
volatile projection is removed; no replacement pin, barrier or qualifier was
introduced. Named phases and named script fields replace numeric state/word
views, and the common renderer follows the switch without a goto. Ramp
accumulators use their permitted unsigned-halfword view for wrapping arithmetic.

`test_frame_task_controller.py` compiles the actual enter/reset/update and ramp
bodies without rewriting them. It checks 4,224 native cases and 1,152 cases on
each of original and linked MIPS. Explicit native DMA regions use physical,
KSEG0 and KSEG1 aliases. Complete script, frame-bank and packet owners, root
pointers, scalar state, service entry/exit images, guards, MIPS stores and ABI
are compared. Cases cover all named/unused/unknown phases, reveal thresholds,
fade edges, null early-return roots, callback root replacement, OT/packet
aliases and the final accessible packet slot with a one-past cursor.

IO, audio, TIM parsing and GPU emitters are controlled boundaries in this
controller fixture; their algorithms are not exercised by it. The existing
actual-body sprite/grid fixtures remain unchanged. MIPS service effects are
implemented independently rather than copied from expected after-state images.
The outer MainLoop still retains its existing register constraints and physical
bank-index projection; the rejected direct-index candidate is not promoted.


### Row animation and rumble constructors

`BadgeRow1C.animationTick12` is a signed animation counter, not byte padding.
The badge update names its observed phases and captures each trig result
before reading the live row root and dimensions. Multiplication by 1024
replaces a signed left shift, preserving the target instruction sequence for
negative counters as well. Rectangle initialization and animation index the
shared `MenuRectRow10` array directly. Their two intensity initialization loops
use array indices instead of forming a pointer before the array.

`InitRect16Row` retains its matching signed-halfword parameter interface.
Its menu caller imports the owner header for both initialization and badge
updates. `PadSetRumbleParams` instead accepts full signed call words, narrowing
only at the actual signed16 direction and unsigned8 level stores. The steering
caller includes the same public header; no private weak prototype is needed.

`test_ui_row_owners.py` runs five actual production bodies without rewriting:
these four functions plus `InitBadgeRow`. It compares 2,560 input scenarios
against independent models on native C and original/linked MIPS. Trig, badge
drawing and partial-line emission are explicit controlled boundaries; their
algorithms are not covered here. The test checks complete guarded row owners,
both controllers, full frame storage, input tables, service snapshots, live
root/table changes, null roots on skipped paths, negative valid indices,
last-row access, unknown phases, signed counter extremes and call/store ABI.
Existing frozen fixtures, old models, cases and PRNG streams are unchanged.

Remaining rectangle cleanup includes the six-edge traversal's byte-addressed
corner/level reads and its jump label. Direct indexing and structured loops
have not yet preserved the relocated match; no new compiler constraints are
introduced to compensate.


### Rectangle line definitions

`MenuRectLineCorners` is the six-entry array of directed corner pairs consumed
by `UpdateRect16Row`; `MenuRectLineIntensitySlots` selects one of four Q8
intensity slots for each line. The previous names `SmallDigitSpriteUvs` and
`BigDigitGlyphWidths` were incorrect descriptive guesses, not recovered
identifiers. Original data directs the left/right sides first, then top/bottom,
then both diagonals; slot selection is `0,0,1,1,2,3`. The matching public layout
uses two unsigned bytes per pair. `MenuRectCorners` owns the two full signed32
coordinate columns used while the row is animated. Phase constants now name
those observed stages and the completed/restart states.

The resident symbol registry and generated data labels use these same names.
Only the existing native fixture's symbol bindings and its C table declaration
change; exact reverse normalization reproduces its previous source. Its state
model, owner buffers, case selection, mutation logic and PRNG are unchanged,
as are all frozen fixture contents and symlink targets. The six-line caller
still uses its established byte-addressed coordinate/slot reads and jump label;
removing those remaining constructs has not yet preserved the relocated match.

### Menu image row ownership

`MenuGlyphRow1C` has one shared definition in `rr/menu_glyph_types.h`.
Its frame index/count, tick, frame duration, image-sector stride, texture page,
CLUT and UV fields replace the old overlapping padding views. `imageBase18`
is a CPU image pointer, previously represented by the misleading signed word
`phase18`. The PAL record remains 0x1C bytes; LP64 uses a 0x20-byte record with
a complete pointer. `AllocGlyphRows` reserves and clears the actual native
record size. Constructors, image publication, results setup and overlay
controllers all index the same declared owner. Their descriptive field and
phase names are inferred from use, not recovered original identifiers.

`InitResultsMenuRows` and `NewCarModeUpdate` use the complete shared dialogue
and badge owners, replacing their private short-prefix layouts. The latter
keeps `LoadCursor` in a real pointer across layout services. The shared
`UpdateResultsBarSelection` interface accepts the direction word observed at
its caller; the current implementation ignores that argument. The direction
load is present in the original caller, so simply deleting the call slot does
not reproduce its instruction sequence. This is an interface reconstruction,
not evidence of an original parameter name. Semitransparent tile ordering
heads use the same unsigned GPU words as their callers.

`test_glyph_row_owner.py` includes eight actual production bodies without
rewriting them. Independent state/event models check native C and original
and linked MIPS, complete guarded row/dialogue/badge owners, full image and
cursor pointers, service-induced live-root changes and actual multi-call
allocation/loading/animation sequences. CD, GPU emission, rectangle, badge
and layout services remain explicitly controlled boundaries. No GPU emitter
or TIM parser algorithm is claimed as covered by this fixture.

Ten older native fixtures retain their literal models, Worlds, cases and PRNG
streams. Their explicit transport converts between the existing PAL oracle
and full native row/image owners, checking the real allocation size before
normalizing PAL byte counts. Reverse normalization reproduces each prior
test file exactly; frozen fixture contents and symlink targets are unchanged.

### Results label renderer

`DrawResultsStandingsOverlay` keeps the scratch packet cursor as a complete
pointer and reads/writes it through `GpuScratchPointers`. Its labels use the
shared `UiUvRect`, also used by `AddSpritePrim`. The public owner declares 24
four-byte rectangles and 16 index bytes (eight pairs). The following object is
`CarUpdateStages`, not part of the index array. These extents follow the actual
data boundaries and original texture records. Both table annotations in the
symbol registry use byte data; the generated data and full EXE remain identical.

The left-label loop uses normal C control flow and named values instead of
register names and its old skip label. The captured UV record is retained
across the drawing helper, then its width is read live for the next position.
Mode, selected-course, table contents and current frame can change through a
helper; subsequent reads preserve the observed target behavior. Fade arithmetic
uses unsigned wrapping before narrowing to the actual signed flags word.

`test_results_label_pipeline.py` checks complete native frame/packet owners,
table guards, pointer identities and the independent geometry/packet oracle.
It runs the current C bodies verbatim, original MIPS and linked MIPS, with
ordinary game modes, all eight index pairs, course choices, signed fade limits,
both frame banks and KSEG0/KSEG1 aliases. A controlled-helper layer also checks
live mutations and cursor-return threading. The packet layer includes the
actual sprite/page builders, SDK setters and `AddPrim` on native, original and
linked MIPS. Native tracing binds the literal linker definition to a test name
and observes its result after calling that definition; it does not replace its
link algorithm. Native DMA serialization uses `GpuDmaEncodeAddress` and actual
registered owners. Existing models, cases, PRNG streams and frozen fixtures are
unchanged.


### GPU tag address fields

`AddPrim` now uses the existing `GpuDmaTag.bits.address` view for each 24-bit
link assignment. The complete tag already defines its eight-bit length and
24-bit address, so the two assignments preserve the original protocol without
manual masks or the previous forced `$a2` mask-register binding. The shared
layout, public word-pointer interface and `GpuDmaEncodeAddress` backend are
unchanged. The complete SDK TU retains relocated instruction distance zero.

`test_dma_tag_owner.py` executes the literal function on complete guarded
native tag arrays and original/linked MIPS. Its independent word oracle checks
every ordered slot pair, self-alias and adjacent tags, both CPU address aliases,
length-byte preservation, untouched guards and register/stack ABI. All native
CPU pointers retain their full width; serialized DMA links use the registered
PSX addresses. Three older native pipelines no longer remove a register
annotation from the source before compiling it. Their exact reversible source
construction migrations leave the old Worlds, models, cases and PRNG streams
unchanged. The viewport fixture binds the literal definition to its existing
observed service name instead of rewriting that definition.

### Tile packets and the common primitive header

`AddTilePrim` and `AddTileSemiTransPrim` use the complete `GpuTilePacket` owner
instead of indexing byte/halfword pointers. The semitransparent builder writes
the existing 24-bit tag address field and calls `GpuDmaEncodeAddress`; it no
longer casts a CPU packet pointer to a signed word. Their returned cursors keep
the full native pointer width. `AddTileGrid2x2` uses unsigned coordinate words
for the target's wrapping additions, with signed division by two for negative
dimensions. No compiler constraints or dummy operations are introduced.

The semitransparent tile's coordinates are full-word arguments, as observed in
the resident and garage callers. Narrowing belongs to the packet stores. The
public declaration, definition and consumers now share that interface; local
K&R declarations and the garage's private full-word declaration are removed.
The glyph-controller fixture changes only its service parameter binding from
signed16 to signed32. Its World, model, case stream and PRNG are unchanged;
exact reverse normalization reproduces the previous fixture file.

`GpuPrimitiveHeader` is the real eight-byte drawing-primitive prefix: a DMA tag
and a color/command word. Sprite and tile command unions expose this same view
without changing their complete sizes. Six SDK initializers and the shade and
semitransparency setters use it instead of claiming a `PolyG4Packet` object for
every primitive format. They continue to preserve all other packet storage.

`test_tile_packet_owner.py` includes all three tile builders, the actual DMA
linker and all eight header setters literally. Independent wire models check
native C and original/linked MIPS on complete guarded packet arenas, ordering
heads that alias packet words, both CPU mirrors, last-slot/one-past cursors,
full-word coordinate/color limits and all initial command bytes. The grid
model checks negative odd dimensions and wrapping coordinates. No helper is
mocked and no production body is rewritten for host compilation.

### Current selection and serial exchange ownership

`SelectionRows` has one public declaration in `game/data/selection.h`: a full
32-byte `CurrentSelection` object. Its byte view, eight `SelectionRow` entries,
eight packed choice records and menu-list view share that allocation. Resident
and overlay consumers import the declaration instead of declaring separate
four-byte prefixes, arrays or record lists. The fourth row byte is named
`option`: callers use it for menu confirmation and car setup, so it is not an
unused reserved byte.

`ActiveSelectionRowValue` is the historical PAL label for byte1; active C uses
`SelectionRows.choice.bytes[1]`. `LinkRacePositionSyncBuf` is the historical
label at byte8. Its serial exchange uses records2/3 and records0/1 of this same
owner. Neither label describes a separate C object. Peer role determines which
pair is sent and which is received.

`LinkExchange` accepts full buffer pointers. Both BIOS writes supply their real
buffer and count explicitly, including the formerly omitted initial write
arguments. Its public declaration and file-service declarations agree. The
role is interpreted as a signed halfword without a pointer reinterpretation.

`test_selection_owner_pipeline.py` runs literal glyph codecs and the car-variant
consumer on a complete guarded owner, native C and original/linked MIPS. It
covers all eight records, signed glyph limits, option-byte preservation, both
CPU mirrors and register/stack ABI. `test_selection_link_exchange.py` adds the
literal exchange body with controlled BIOS boundaries: full native pointer
identities, overlapping buffers, both roles, short I/O, live handle changes and
CPU mirrors are checked against an independent model on all three backends.

Legacy native fixtures retain their literal Worlds, models, cases and PRNGs.
`selection_fixture_transport.py` supplies a complete guarded native owner and
synchronizes old wire views at entries and controlled service boundaries. It
also maps real pointer aliases between the two allocations at full width. The
production bodies remain literal; the transport does not reconstruct CPU
pointers from low address bits. Existing full-size byte views remain valid.

This unifies the selection allocation and its active aliases. The fixed-page
accesses and existing compiler constraints inside `InitSelectionRows` still
need separate reconstruction; this pass does not claim to retire them.

### Start grid, car specifications and lane roots

The first sixteen bytes of `SceneRaceContext` are the eight signed start-order
entries. The following word is the computer-car count. The resource cells at
`+0x53C..+0x54C` are full native pointers: a complete start grid, the car-spec
array, two spec targets and the lane-offset array. `TrackStartNode`,
`CarSpecTable`, `ActiveDraftCarSpecPtr` and `TrackLaneOffsets` remain historical
PAL labels for these cells; active C consumers use the shared context fields.
The old one-element spec-target projection is now the actual two-element array.

`RaceStartGrid60` describes the complete resource: a starting track node, eight
XZ positions and eight headings. Its name records its size rather than the
context offset of its pointer. `SetTrackStartNode` retains its historical API
name and accepts this complete grid type. The resource loader and both start
position consumers share that declaration. Car-spec and lane record types live
in small shared headers, so the context does not import unrelated vehicle or
track-global declarations.

`BlendCarSpec` and `AverageCarSpec` access `ComCars[index].control` directly.
`CarSpecControlPool` is a PAL label inside the first vehicle, with no separate
active C declaration. The private padded `T` array and the macro hiding the
public context declaration are removed. Averaging explicitly reads the unsigned
gear-ratio bit patterns used by the original code. Position scaling and blended
speed scaling use unsigned shifts to retain the MIPS wrapping result for
negative values without undefined signed left shifts. `InitComCars` clears eleven
complete vehicle objects on both target and native builds.

`test_race_spec_owner.py` executes five literal C functions in a six-call chain,
with 512 scenarios on native C and original/linked MIPS. An independent integer
model checks every PAL context/vehicle field after each call. It covers both spec
target slots, independent source tables, signed blend boundaries, full sixteen-bit
ratios, source/destination aliases, complete readonly owners, store bounds and
register/stack ABI. `test_race_grid_initializer.py` checks the literal grid/spec
setters and computer-car initializer in 1,024 native scenarios against a separate
full-owner model. Its controlled track/model services can replace the live grid
and spec roots; the first spec remains captured across that boundary. It covers
zero through seven cars, all mode branches and the full eleven-car clear. It does
not test the algorithms behind the controlled services.

Old camera inputs keep their literal PAL tokens and frozen layouts.
`scene_context_wire.h` binds the five newly typed cells to guarded complete native
owners and serializes their saved identities separately. Equal spec tokens retain
equal native identities. The lane-control fixture likewise retains its literal
World, model, cases and PRNG; entry/service adapters synchronize its old lane root
with a complete guarded scene owner. Production calculations remain literal.
Other private context views, existing barriers and the pointer-shaped loop inside
`ApplyRubberBandSpec` remain reconstruction work.

### Demo fade and intro state

`SceneRaceContext.fade3E0` is one complete `DemoFadeState64`. Its constructor
clears that member directly; no consumer declares a padded `FadeRaceContext`
or recovers the context by subtracting a PAL offset from an interior alias.
The record names the frame/opacity/scale controls, particle enable, sprite and
line progress, sound state and ripple state. `sceneStateB0` and `sceneStep444`
name the adjacent shared state words. The PAL and native context sizes and all
pointer member positions remain unchanged by these scalar-record refinements.
`DemoFadeCommand8` describes the actual frame/command timeline entries.

`FadeSpriteRaceContext` is the historical label at `RaceContext+0x3E0`.
`DemoFadeSkipEnabled` is the label for `fade3E0.scaleMode48`, not a separate
flag. `MotionSequenceIndex` is the label for `SoundState.seqTrack2868`.
The fade constructor and presentation sequence reader now use that same audio
field. These historical labels have no active standalone C declarations.

The intro frame uses both complete `FrameStorage` banks and reads the full
native scratch cursor before calling the tile emitter. It no longer passes
only an integer-sized fragment of that pointer. The ripple controller uses the
shared eight-byte GPU command packet and ordinary constant arguments; the old
assignment/read of `new_var` in separate call arguments was unsequenced C.
The particle renderer uses the common textured-quad packet, explicit DMA tag
fields and `GpuDmaEncodeAddress`, with no private packet type or linking macro.
The three gradient emitters likewise encode real DMA identities. The slanted
emitter's public interface, definition and caller agree on rectangle/color
records, and its frame-base address calculation retains the full native width.
Some renderer word-addressing expressions still need further readability work.

`test_demo_fade_owner.py` executes nine literal producer functions on complete
scene/audio/particle owners, native C and original/linked MIPS. In 256 scenarios
per backend it checks a multi-call pipeline, independent state models, every
controlled service argument and pre-service snapshot, live sequence/bank changes,
captured bank order, both historical aliases, untouched owner storage, MIPS
store bounds and register/stack ABI. No production body is rewritten.

`test_particle_render_owner.py` executes the actual particle renderer and all
three actual gradient emitters together. Its independent wire model checks
4,096 cases, every packet byte and ordering word, both full frame banks, all
24 particle slots, signed coordinate/speed/scale limits, full scratch storage,
exterior guards and both CPU aliases through the real native DMA backend.
There are no substituted rendering services in this test.

`test_intro_fade_frame_owner.py` executes the literal intro step, tint writer,
ripple controller, particle-fade writer and skip setter in 1,008 native cases.
Command expectations cover all eleven commands plus the default path, opacity
and scale boundaries, exact line/sprite/glyph arguments, complete frame colors,
DMA command linking and full-width cursor publication. Glyph and tile emission,
trigonometry and particle updates are controlled boundaries. A glyph service
switches the current frame bank during the ripple loop; subsequent arguments
and final command/tile publication must use the live bank.

The old presentation fixture retains its literal World, model, cases and PRNG.
`motion_index_fixture_transport.py` binds its historical scalar input to a
complete guarded native `AudioState` for the actual restart entry. This is a
storage adapter; it does not change the production calculation or reconstruct
host pointers from address bits. Frozen fixtures remain untouched. Other private
context views and existing register/asm constraints remain reconstruction debt.
